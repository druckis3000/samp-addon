#include "samp.h"
#include "gtasa.h"

#include "utils/helper.h"
#include "hooking.h"
#include "vecmath.h"
#include "utils/event.h"
#include "utils/tqueue.h"

#include "helpers/sampfuncs.h"
#include "helpers/helper_cmds.hpp"
#include "helpers/quickload.hpp"
#include "utils/timercpp.h"

#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <cmath>

// These should be outside namespace, sine they are declared in samp.h, outside namespace
stScoreboardInfo	*g_Scoreboard = nullptr;
stDialogInfo		*g_Dialog = nullptr;
stInputInfo			*g_Input = nullptr;
stChatInfo			*g_Chat = nullptr;
stSAMP				*g_Samp = nullptr;
void				*g_Misc = nullptr;
bool				g_IsSampReady = false;

namespace SAMP {

	// ----- samp.dll module vars -----

	DWORD g_SampBaseAddress = (DWORD)NULL;

	// ----- samp.dll function hooks -----

	FHook startGameHook;
	FHook onChatMessageHook;
	FHook onScoreboardUpdateHook;
	FHook onSetCheckpointHook;
	FHook onBigMessageHook;
	FHook onDialogButtonHook;
	//FHook wndProcHook;

	// ----- Private samp.cpp vars -----

	// Used for storing samp.dll::addToChatWindow function parameter values (pointers are of samp.dll thread)
	char *pLastServerMessage;
	char *pLastServerMessagePrefix;
	ChatMessageType eLastMessageType;
	DWORD dwLastMessageColor;
	DWORD dwLastMessagePrefixColor;
	TQueue<struct stAddMessageParams> g_AddMessageCallsQueue;

	// Used for storing samp.dll::setCheckpoint function parameter values (pointers are of samp.dll thread)
	float *pCheckpoint_XYZ;
	float checkpoint_Size;
	TQueue<struct stSetCheckpointParams> g_SetCheckpointCallsQueue;

	// Used for storing samp.dll::showBigMessage function parameter values (pointers are of samp.dll thread)
	char *pLastBigMessageText;
	unsigned int lastBigMessageTime;
	unsigned int lastBigMessageStyle = -1;
	TQueue<struct stShowBigMessageParams> g_ShowBigMessageCallsQueue;

	// Used for storing samp.dll::onDialogButton function parameters value
	uint8_t g_bDialogButton;
	TQueue<struct stOnDialogResponseParams> g_OnDialogResponseQueue;

	// Used for invoking scoreboard update callbacks in main thread
	volatile bool g_bUpdateScoreboard = false;

	// Used for processing toggleCursor function call in main thread
	volatile struct stToggleCursor g_CursorToggleInfo = {0, true, true};

	// For anti spamming
	double lastSayCommandTime = 0.0;
	double lastSayTime = 0.0;
	char lastSayText[512];
	char lastSayCommandText[512];

	// For testing and debugging
	bool isKey2Pressed = true;

	// For list dialog selector with keyboard numbers
	bool numberKeysState[9] = {};

	// ----- Private function declarations -----

	void getSAMPPointers();
	template<typename T>T GetSAMPPtrInfo(uint32_t offset);
	const char* getWeaponName(uint8_t id);
}

// These naked functions should be outside namespace to avoid name decorating
__declspec(naked) void HOOK_onChatMessage_NAKED();
__declspec(naked) void HOOK_onScoreboardUpdate_NAKED();
__declspec(naked) void HOOK_onSetCheckpoint_NAKED();
__declspec(naked) void HOOK_onBigMessage_NAKED();
__declspec(naked) void HOOK_onDialogButton_NAKED();
//__declspec(naked) void HOOK_wndProc_NAKED();

// ----- samp.cpp functions -----

using namespace SAMP;

bool SAMP::setupSystem()
{
	// Get samp.dll base address
	while((g_SampBaseAddress = (DWORD)GetModuleHandle("samp.dll")) == (DWORD)NULL){
		Sleep(100);
	}

	Logf("SAMP Base address: 0x%x", (void*)g_SampBaseAddress);
	Logf("Waiting for SAMP to load...");

	// Wait until samp loads and get struct pointers
	getSAMPPointers();
	while(g_Samp == nullptr || g_Chat == nullptr || g_Input == nullptr || g_Dialog == nullptr || g_Scoreboard == nullptr){
		Sleep(100);
		getSAMPPointers();
	}
	
	// Wait until pools get initialized
	while(g_Samp->pPools == nullptr) Sleep(100);
	while(g_Samp->pPools->pPlayer == nullptr) Sleep(100);
	while(g_Samp->pPools->pVehicle == nullptr) Sleep(100);
	while(g_Samp->pPools->pObject == nullptr) Sleep(100);
	while(g_Samp->pPools->pTextdraw == nullptr) Sleep(100);

	g_Misc = *(void**)(g_SampBaseAddress + SAMP_PTR_MISC_OFFSET);
	
	Log("SAMP Loaded! Hooking functions");
	
	// Hook addToChatWindow
	onChatMessageHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_ADD_TO_CHAT;
	onChatMessageHook.hookAddress = (DWORD)HOOK_onChatMessage_NAKED;
	onChatMessageHook.overwriteSize = 9;
	MidFuncHook(&onChatMessageHook);
	
	// Hook onScoreboardUpdate
	onScoreboardUpdateHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_UPDATE_SCOREBOARD;
	onScoreboardUpdateHook.hookAddress = (DWORD)HOOK_onScoreboardUpdate_NAKED;
	onScoreboardUpdateHook.overwriteSize = 6;
	MidFuncHook(&onScoreboardUpdateHook);
	
	// Hook onSetCheckpoint
	onSetCheckpointHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_SET_CHECKPOINT;
	onSetCheckpointHook.hookAddress = (DWORD)HOOK_onSetCheckpoint_NAKED;
	onSetCheckpointHook.overwriteSize = 6;
	MidFuncHook(&onSetCheckpointHook);

	// Hook onShowBigMessage
	onBigMessageHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_SHOW_BIG_MESSAGE;
	onBigMessageHook.hookAddress = (DWORD)HOOK_onBigMessage_NAKED;
	onBigMessageHook.overwriteSize = 7;
	MidFuncHook(&onBigMessageHook);

	// Hook onDialogButton
	onDialogButtonHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_ON_DIALOG_BUTTON;
	onDialogButtonHook.hookAddress = (DWORD)HOOK_onDialogButton_NAKED;
	onDialogButtonHook.overwriteSize = 5;
	MidFuncHook(&onDialogButtonHook);

	// Hook WndProc
	/*wndProcHook.patchAddress = g_SampBaseAddress + SAMP_HOOK_WND_PROC;
	wndProcHook.hookAddress = (DWORD)HOOK_wndProc_NAKED;
	wndProcHook.overwriteSize = 6;
	MidFuncHook(&wndProcHook);*/

	Log("SAMP Functions hooked!");
	Log("Installing patches");

	// Anti crash
	writeMemory(g_SampBaseAddress + SAMP_NOP_ANTI_CRASH_1, 0x90, 5);
	writeMemory(g_SampBaseAddress + SAMP_NOP_ANTI_CRASH_2, 0x90, 5);
	Log("Anti-crash ready!");

	// Fast connect
	DWORD address = FindPattern((char*)SAMP_PATCH_FAST_CONNECT, (char*)"xxxxxx") + 0x1;
	writeMemory(address, 0x0, 4);
	Log("Fast connect ready!");

	// Unlimited fps
	disableFPSLock();

	// Add own commands
	HelperCmds::registerCmds();

	// Custom nametag distance patch
	writeMemory(g_SampBaseAddress + SAMP_NOP_NAMETAG_DIST_1, 0x90, 6);
	writeMemory(g_SampBaseAddress + SAMP_NOP_NAMETAG_DIST_2, 0x90, 6);
	Log("Nametag distance patched!");

	// Set samp ready flag
	g_IsSampReady = true;

	return true;
}

Timer dbgTimer;

void SAMP::loop()
{
	if(!g_IsSampReady) return;

	if(GetAsyncKeyState(VK_NUMPAD2) & 0x8000){
		if(!isKey2Pressed){
			// Do something
			isKey2Pressed = true;

			
		}
	}else{
		isKey2Pressed = false;
	}

	// List dialog selector with upper keyboard numbers
	if(isSampDialogActive()){
		if(g_Dialog->iType == DIALOG_STYLE_LIST || g_Dialog->iType == DIALOG_STYLE_TABLIST){
			for(int i=0; i<9; i++){
				if(GetAsyncKeyState(0x31 + i) & 0x8000){
					if(numberKeysState[i] == false){
						numberKeysState[i] = true;
						setDialogSelectedItemIndex(i);
					}
				}else{
					numberKeysState[i] = false;
				}
			}
		}
	}
	
	// Do cheating stuff
	if(g_IsSampReady){
		// Process toggleCursor function call, if not processed yet
		if(g_CursorToggleInfo.bProcessed == false){
			reinterpret_cast<void(__thiscall *)(void *_this, int, bool)>(g_SampBaseAddress + SAMP_FUNC_TOGGLECURSOR)(g_Misc, g_CursorToggleInfo.iMode, g_CursorToggleInfo.bBoolean);
			
			// Mark as processed
			g_CursorToggleInfo.bProcessed = true;
		}

		// Invoke callbacks in main thread
		struct stAddMessageParams *queueMsg = nullptr;
		while(g_AddMessageCallsQueue.next(&queueMsg)){
			invokeOnMessageCallbacks(queueMsg->szMessage, queueMsg->dwMessageColor);
			free(queueMsg);
		}

		struct stSetCheckpointParams *queueCheckpoint = nullptr;
		while(g_SetCheckpointCallsQueue.next(&queueCheckpoint)){
			invokeSetCheckpointCallbacks(queueCheckpoint->fX, queueCheckpoint->fY, queueCheckpoint->fZ, queueCheckpoint->fSize);
			free(queueCheckpoint);
		}

		struct stShowBigMessageParams *queueBigMsg = nullptr;
		while(g_ShowBigMessageCallsQueue.next(&queueBigMsg)){
			invokeOnBigMessageCallbacks(queueBigMsg->szLastBigMessageText, queueBigMsg->ulLastBigMessageTime, queueBigMsg->ulLastBigMessageStyle);
			free(queueBigMsg);
		}

		struct stOnDialogResponseParams *queueDialogResponse = nullptr;
		while(g_OnDialogResponseQueue.next(&queueDialogResponse)){
			invokeOnDialogResponseCallbacks(queueDialogResponse->bButton, queueDialogResponse->eStyle, queueDialogResponse->iSelectedIndex, queueDialogResponse->szInputText);
			
			if(queueDialogResponse->szInputText != nullptr)
				free(queueDialogResponse->szInputText);

			free(queueDialogResponse);
		}

		if(g_bUpdateScoreboard){
			invokeOnScoreboardUpdateCallbacks();
			g_bUpdateScoreboard = false;
		}
	}
}

void SAMP::oneSecondTimer()
{
	HelperCmds::oneSecondTimer();
}

void SAMP::callGameProc()
{
	if(g_SampBaseAddress != 0)
		reinterpret_cast<void(*)()>(g_SampBaseAddress + SAMP_FUNC_GAME_PROC)();
}

// ----- Hooked functions -----

void HOOK_onChatMessage()
{
	if(pLastServerMessage == nullptr) return;

	// Save samp.dll function call parameters into thread-safe queue and
	// execute function later in scr script thread
	struct stAddMessageParams *addMessageParams = (struct stAddMessageParams*)malloc(sizeof(*addMessageParams));
	addMessageParams->eMessageType = eLastMessageType;
	strncpy(addMessageParams->szMessage, pLastServerMessage, 512);
	addMessageParams->dwMessageColor = dwLastMessageColor;

	if(pLastServerMessagePrefix != nullptr){
		strncpy(addMessageParams->szMessagePrefix, pLastServerMessagePrefix, 512);
		addMessageParams->dwMessagePrefixColor = dwLastMessagePrefixColor;
	}else{
		memset(addMessageParams->szMessagePrefix, '\0', 512);
		addMessageParams->dwMessagePrefixColor = 0;
	}

	// Add to queue
	g_AddMessageCallsQueue.push(addMessageParams);
}

__declspec(naked) void HOOK_onChatMessage_NAKED()
{
	__asm("mov %ecx, %ebp");
	__asm("push %edi");
	__asm("push %ecx");
	
//	__asm("movl 0x8(%%esp), %0"
//			:"=r"(pLastServerMessage));

	// +20 = MSG_TYPE
	// +24 = text
	// +28 = prefix
	// +32 = text color
	// +36 = prefix color
	__asm("movl 0x14(%%esp), %0"
			:"=r"(eLastMessageType));
	__asm("movl 0x18(%%esp), %0"
			:"=r"(pLastServerMessage));
	__asm("movl 0x1C(%%esp), %0"
			:"=r"(pLastServerMessagePrefix));
	__asm("movl 0x20(%%esp), %0"
			:"=r"(dwLastMessageColor));
	__asm("movl 0x24(%%esp), %0"
			:"=r"(dwLastMessagePrefixColor));

	HOOK_onChatMessage();
	
	__asm("pop %ecx");
	__asm("lea 0x132(%ebp), %edi");
	__asm("jmp *(%0)"
			:
			:"m"(onChatMessageHook.jmpBackAddress));
}

void onScoreboardUpdate()
{
	if(g_Samp == nullptr || g_Samp->pPools == nullptr || g_Samp->pPools->pPlayer == nullptr) return;

	g_bUpdateScoreboard = true;
}

__declspec(naked) void HOOK_onScoreboardUpdate_NAKED()
{
	onScoreboardUpdate();
	
	__asm("add $0x12C, %esp");
	__asm("jmp *(%0)"
			:
			:"m"(onScoreboardUpdateHook.jmpBackAddress));
}

void HOOK_onSetCheckpoint()
{
	// Copy samp.dll pointer to scr.asi array
//	vect3_copy(pCheckpoint_XYZ, checkpoint_XYZ);
//	invokeSetCheckpointCallbacks(checkpoint_XYZ[0], checkpoint_XYZ[1], checkpoint_XYZ[2], checkpoint_Size);

	// New version with thread-safe queue
	struct stSetCheckpointParams *setCheckpointParams = (struct stSetCheckpointParams*)malloc(sizeof(*setCheckpointParams));
	setCheckpointParams->fX = pCheckpoint_XYZ[0];
	setCheckpointParams->fY = pCheckpoint_XYZ[1];
	setCheckpointParams->fZ = pCheckpoint_XYZ[2];
	setCheckpointParams->fSize = checkpoint_Size;
	g_SetCheckpointCallsQueue.push(setCheckpointParams);
}

__declspec(naked) void HOOK_onSetCheckpoint_NAKED()
{
	// Save _this
	__asm("push %ecx");
	
	// Move function parameters to global vars
	__asm("movl 0x8(%%esp), %0"
			:"=r"(pCheckpoint_XYZ));
	__asm("movl 0xC(%esp), %eax");
	__asm("mov (%%eax), %0"
			:"=r"(checkpoint_Size));

	// Call hook function
	HOOK_onSetCheckpoint();
	
	// Restore _this and execute overwritten instructions
	__asm("pop %ecx");
	__asm("mov 4(%esp), %eax");
	__asm("mov (%eax), %edx");
	__asm("push %esi");
	__asm("mov %ecx, %esi");
	__asm("push %edi");
	
	// Jump back to the original samp.dll code
	__asm("jmp *(%0)"
			:
			:"m"(onSetCheckpointHook.jmpBackAddress));
}

void HOOK_onBigMessage()
{
	if(pLastBigMessageText == nullptr) return;

	// Copy show big message parameters and put them into thread-safe queue
	struct stShowBigMessageParams *showBigMessageParams = (struct stShowBigMessageParams*)malloc(sizeof(*showBigMessageParams));
	strncpy(showBigMessageParams->szLastBigMessageText, pLastBigMessageText, 1024);
	showBigMessageParams->ulLastBigMessageTime = lastBigMessageTime;
	showBigMessageParams->ulLastBigMessageStyle = lastBigMessageStyle;
	g_ShowBigMessageCallsQueue.push(showBigMessageParams);
}

__declspec(naked) void HOOK_onBigMessage_NAKED()
{
	// Save _this
	__asm("push %ecx");

	// Save params
	__asm("mov 0x8(%%ebp), %0"
			:"=r"(pLastBigMessageText));
	__asm("mov 0xC(%%ebp), %0"
			:"=r"(lastBigMessageStyle));
	__asm("mov 0x10(%%ebp), %0"
			:"=r"(lastBigMessageTime));

	// Call another non-naked hook function
	HOOK_onBigMessage();

	// Restore _this
	__asm("pop %ecx");

	// Execute overwritten instruction
	__asm("cmpl $0x0C8, 0x10(%ebp)");
	// jmp back to samp.dll
	__asm("jmp *(%0)"
			:
			:"m"(onBigMessageHook.jmpBackAddress));
}

void HOOK_onDialogButton()
{
	struct stOnDialogResponseParams *onDialogResponseParams = (struct stOnDialogResponseParams*)malloc(sizeof(*onDialogResponseParams));
	
	onDialogResponseParams->bButton = g_bDialogButton;
	onDialogResponseParams->eStyle = static_cast<eDialogStyle>(g_Dialog->iType);
	onDialogResponseParams->iSelectedIndex = 0;
	onDialogResponseParams->szInputText = nullptr;
	
	if(g_Dialog->iType == DIALOG_STYLE_LIST || g_Dialog->iType == DIALOG_STYLE_TABLIST){
		onDialogResponseParams->iSelectedIndex = getDialogSelectedItemIndex();
	}else if(g_Dialog->iType == DIALOG_STYLE_INPUT_TEXT || g_Dialog->iType == DIALOG_STYLE_INPUT_PASSWORD){
		onDialogResponseParams->szInputText = new char[getDialogInputTextLength() + 1];
		strcpy(onDialogResponseParams->szInputText, getDialogInputText().c_str());
	}
	
	g_OnDialogResponseQueue.push(onDialogResponseParams);
}

__declspec(naked) void HOOK_onDialogButton_NAKED()
{
	// Save __this
	__asm__("push %ecx");

	// Save ebp register
	__asm__("push %ebp");

	// Save params
	__asm__("mov 0x10(%%esp), %0"
				:"=r"(g_bDialogButton));

	// Call non-naked function
	HOOK_onDialogButton();

	// Restore ebp register
	__asm__("pop %ebp");

	// Restore __This
	__asm__("pop %ecx");

	// jmp back to samp.dll
	__asm__("jmp *(%0)"
			:
			:"m"(onDialogButtonHook.jmpBackAddress));
}

/*HWND hwnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
bool dbgPressed = false;

void HOOK_wndProc() // HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam
{
	if(message == WM_SHOWWINDOW){
		infoMsg("WM_SHOWWINDOW!!!");
	}else if(message == WM_ACTIVATEAPP){
		infoMsg("WM_ACTIVATEAPP!!!");
	}else if(message == WM_MOUSEACTIVATE){
		infoMsg("WM_MOUSEACTIVATE!!!");
	}else if(message == WM_NOTIFY){
		infoMsg("WM_NOTIFY!!!");
	}
	//Logf("HWND: %u, message: %d, wParam: %d, lParam: %d", hwnd, message, wParam, lParam);
}

__declspec(naked) void HOOK_wndProc_NAKED()
{
	// 0x04 - hwnd param
	// 0x08 - msg param
	// 0x0c - wparam param
	// 0x10 - lparam param

	// Save registers
	__asm("push %eax");
	__asm("push %ebx");
	__asm("push %ecx");
	__asm("push %edx");

	// Save wndProc() callback params
	__asm("mov 0x14(%%esp), %0"
			:"=r"(hwnd));
	__asm("mov 0x18(%%esp), %0"
			:"=r"(message));
	__asm("mov 0x1C(%%esp), %0"
			:"=r"(wParam));
	__asm("mov 0x20(%%esp), %0"
			:"=r"(lParam));

	HOOK_wndProc();

	// Restore registers
	__asm("pop %edx");
	__asm("pop %ecx");
	__asm("pop %ebx");
	__asm("pop %eax");

	// Original, overwritten instructions
	__asm("sub $0x10, %esp");
	__asm("cmpl $0x0A, %eax");

	// Jmp back to original samp.dll function
	__asm("jmp *(%0)"
			:
			:"m"(wndProcHook.jmpBackAddress));
}*/

// ----- samp.dll functions -----

void say(const char *text)
{
	if(g_Samp == nullptr) return;
	
	// Prevent spamming
	double currentTime = GetTimeMillis();
	if(currentTime - lastSayTime < 0.6){
		Log("scr.asi wants to spam... :(( [0x3]");
		infoMsg("scr.asi wants to spam... :(( [0x3]");
		return;
	}

	// Prevent spamming
	if(strcmp(lastSayText, text) == 0){
		Log("scr.asi wants to spam... :(( [0x4]");
		infoMsg("scr.asi wants to spam... :(( [0x4]");
		return;
	}

	// Update anti-spam info
	lastSayTime = currentTime;
	strcpy(lastSayText, text);

	reinterpret_cast<void(__thiscall *)(void *, const char *)>(g_SampBaseAddress + SAMP_FUNC_SAY)(g_Samp, text);
}

void sayCommand(const char *cmd)
{
	if(g_Input == nullptr) return;
	
	// Prevent spamming
	double currentTime = GetTimeMillis();
	if(currentTime - lastSayCommandTime < 0.6){
		Log("scr.asi wants to spam... :(( [0x1]");
		infoMsg("scr.asi wants to spam... :(( [0x1]");
		return;
	}
	
	// Prevent spamming
	/*if(strcmp(lastSayCommandText, cmd) == 0){
		Log("scr.asi wants to spam... :(( [0x2]");
		infoMsg("scr.asi wants to spam... :(( [0x2]");
		return;
	}*/

	// Update anti-spam info
	lastSayCommandTime = currentTime;
	strcpy(lastSayCommandText, cmd);

	reinterpret_cast<void(__thiscall *)(void *, const char *)>(g_SampBaseAddress + SAMP_FUNC_SAY_CMD)(g_Input, cmd);
}

void infoMsg(const char *text)
{
	addToChatWindow(CHAT_TYPE_DEBUG, text, nullptr, 0xffffffff, 0);
}

void infoMsg(DWORD color, const char *text)
{
	addToChatWindow(CHAT_TYPE_DEBUG, text, nullptr, color, 0);
}

void infoMsgf(const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	char buffer[512];
	vsnprintf(buffer, 512, fmt, argptr);
	va_end(argptr);
	
	addToChatWindow(CHAT_TYPE_DEBUG, buffer, nullptr, 0xFFFFFFFF, 0);
}

void infoMsgf(DWORD color, const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	char buffer[512];
	vsnprintf(buffer, 512, fmt, argptr);
	va_end(argptr);
	
	addToChatWindow(CHAT_TYPE_DEBUG, buffer, nullptr, color, 0);
}

void addToChatWindow(ChatMessageType msgType, const char *text, const char *prefix, DWORD textColor, DWORD prefixColor)
{
	if(g_Chat == nullptr) return;

	if(prefix == nullptr){
		reinterpret_cast<void(__thiscall *)(void *, ChatMessageType, const char *, const char *, DWORD, DWORD)>(g_SampBaseAddress + SAMP_FUNC_ADDTOCHATWND)(g_Chat, msgType, text, nullptr, textColor, 0);
	}else{
		reinterpret_cast<void(__thiscall *)(void *, ChatMessageType, const char *, const char *, DWORD, DWORD)>(g_SampBaseAddress + SAMP_FUNC_ADDTOCHATWND)(g_Chat, msgType, text, prefix, textColor, prefixColor);
	}
}

void addClientCommand(const char *cmdName, CMDPROC functionPtr)
{
	if(g_Input == nullptr) return;
	
	// Call the original samp.dll function
	reinterpret_cast<void(__thiscall *)(void *_this, const char *command, CMDPROC function)>(g_SampBaseAddress + SAMP_FUNC_ADDCLIENTCMD)(g_Input, cmdName, functionPtr);
}

void toggleSampCursor(int cursorMode, bool immediatelyHideCursor, bool executeInMainThread)
{
	if(g_Misc == nullptr) return;
	
	if(executeInMainThread){
		/* This function needs to be called in scr thread, it doesn't work when called from samp.dll hooks.
			Thus save args into global struct and then call this function in scr thread */
		g_CursorToggleInfo.iMode = cursorMode;
		g_CursorToggleInfo.bBoolean = immediatelyHideCursor;
		g_CursorToggleInfo.bProcessed = false;
	}else{
		reinterpret_cast<void(__thiscall *)(void *_this, int, bool)>(g_SampBaseAddress + SAMP_FUNC_TOGGLECURSOR)(g_Misc, cursorMode, immediatelyHideCursor);
	}
}

bool isToggleCursorProcessed()
{
	return g_CursorToggleInfo.bProcessed;
}

void updateScoreboardInfo()
{
	if(g_Samp == nullptr) return;
	reinterpret_cast<void(__thiscall *)(void *)>(g_SampBaseAddress + SAMP_FUNC_UPDATESCOREBOARD)(g_Samp);
}

void setCheckpoint(float *xyz, float size)
{
	if(g_Misc == nullptr) return;
	reinterpret_cast<void(__thiscall *)(void *_this, float*, float*)>(g_SampBaseAddress + SAMP_FUNC_SETCHECKPOINT)(g_Misc, xyz, &size);
}

void showGameText(const char *text, unsigned int time, unsigned int style)
{
	if(g_Misc == nullptr) return;
	reinterpret_cast<void(__thiscall *)(void *_this, const char *, unsigned int, unsigned int)>(g_SampBaseAddress + SAMP_FUNC_SHOWGAMETEXT)(g_Misc, text, time, style);
}

void setWorldWeather(char weather)
{
	if(!g_IsSampReady) return;
	reinterpret_cast<void(__thiscall *)(void *_this, char)>(g_SampBaseAddress + SAMP_FUNC_SETWORLDWEATHER)(g_Samp, weather);
}

void restartGame()
{
	if(!g_IsSampReady) return;
	reinterpret_cast<void(__thiscall*)(void *_this)>(g_SampBaseAddress + SAMP_FUNC_RESTARTGAME)(g_Samp);
}

/**
 * @param playerId player id to change color
 * @param color	RGBA Color to set
 */
void setPlayerColor(int playerId, DWORD color)
{
	if(!g_IsSampReady) return;

	//reinterpret_cast<void(__thiscall*)(void *_this, int, DWORD)>(g_SampBaseAddress + SAMP_FUNC_SETPLAYERCOLOR)(nullptr, playerId, color);
	DWORD *colorTable = (DWORD*)((uint8_t*)(g_SampBaseAddress + SAMP_PTR_COLORS_OFFSET));
	colorTable[playerId] = color;
}

void setMarkerState(struct stRemotePlayer *rPlayer, int state)
{
	if(!g_IsSampReady) return;

	reinterpret_cast<void(__thiscall*)(void *_this, int)>(g_SampBaseAddress + SAMP_FUNC_SETMARKERSTATE)(rPlayer, state);
}

/** @return RGBA Color */
DWORD getPlayerColorRGBA(int playerId)
{
	if(!g_IsSampReady) return 0;

	// Make sure player is connected
	if(!isValidPlayerId(playerId)) return 0;

	//DWORD color = reinterpret_cast<DWORD(__thiscall*)(void *_this, int)>(g_SampBaseAddress + SAMP_FUNC_GETPLAYERCOLOR)(nullptr, playerId);
	DWORD *colorTable = (DWORD*)((uint8_t*)(g_SampBaseAddress + SAMP_PTR_COLORS_OFFSET));
	DWORD color = colorTable[playerId];
	
	return color;
}

/** @return ARGB Color, alpha will be FF */
DWORD getPlayerColorARGB(int playerId)
{
	if(!g_IsSampReady) return 0;

	// Make sure player is connected
	if(!isValidPlayerId(playerId)) return 0;

	//DWORD color = reinterpret_cast<DWORD(__thiscall*)(void *_this, int)>(g_SampBaseAddress + SAMP_FUNC_GETPLAYERCOLOR)(nullptr, playerId);
	DWORD *colorTable = (DWORD*)((uint8_t*)(g_SampBaseAddress + SAMP_PTR_COLORS_OFFSET));
	DWORD color = (colorTable[playerId] >> 8) | 0xFF000000;

	return color;
}

void showSampScoreboard()
{
	reinterpret_cast<void(__thiscall*)(void *_this)>(g_SampBaseAddress + SAMP_FUNC_SHOWSCOREBOARD)(g_Scoreboard);
}

void closeSampScoreboard(bool bHideCursor)
{
	reinterpret_cast<void(__thiscall*)(void *_this, bool)>(g_SampBaseAddress + SAMP_FUNC_CLOSESCOREBOARD)(g_Scoreboard, bHideCursor);
}

void showSampChatInput()
{
	reinterpret_cast<void(__thiscall*)(void *_this)>(g_SampBaseAddress + SAMP_FUNC_SHOWCHATINPUT)(g_Input);
}

void closeSampChatInput()
{
	reinterpret_cast<void(__thiscall*)(void *_this)>(g_SampBaseAddress + SAMP_FUNC_CLOSECHATINPUT)(g_Input);
}

void showSampDialog(int iId, int iType, const char *szCaption, const char *szText, const char *szLeftButton, const char *szRightButton, int iServerside)
{
	reinterpret_cast<void(__thiscall*)(void *_this, int, int, const char*, const char*, const char*, const char*, int)>(g_SampBaseAddress + SAMP_FUNC_SHOWDIALOG)(g_Dialog, iId, iType, szCaption, szText, szLeftButton, szRightButton, iServerside);
}

void unhideSampDialog()
{
	// Show dialog
	*((uint8_t*)g_Dialog->pDialog + 0x13) = 1;
	g_Dialog->iIsActive = true;

	// Show cursor
	toggleSampCursor(2, false, false);
}

/**
 * This function only hides dialog, server will still know
 * that you have unclosed dialog. To truly close dialog, use
 * closeSampDialog(bProcessButton) function.
 * 
 * This function can be useful only for client-side dialogs.
 */
void hideSampDialog()
{
	reinterpret_cast<void(__thiscall*)(void *_this)>(g_SampBaseAddress + SAMP_FUNC_HIDEDIALOG)(g_Dialog);
}

/**
 * Closes dialog and tells server about it
 * 
 * @param bProcessButton which button was pressed, 0 - right, 1 - left
*/
void closeSampDialog(char bProcessButton)
{
	reinterpret_cast<void(__thiscall*)(void *_this, char)>(g_SampBaseAddress + SAMP_FUNC_CLOSEDIALOG)(g_Dialog, bProcessButton);
}

// ----- Private functions -----

void SAMP::getSAMPPointers()
{
	g_Scoreboard = GetSAMPPtrInfo<stScoreboardInfo*>(SAMP_PTR_SCOREBOARD_INFO_OFFSET);
	g_Dialog = GetSAMPPtrInfo<stDialogInfo*>(SAMP_PTR_DIALOG_INFO_OFFSET);
	g_Input = GetSAMPPtrInfo<stInputInfo*>(SAMP_PTR_CHAT_INPUT_INFO_OFFSET);
	g_Chat = GetSAMPPtrInfo<stChatInfo*>(SAMP_PTR_CHAT_INFO_OFFSET);
	g_Samp = GetSAMPPtrInfo<stSAMP*>(SAMP_PTR_SAMP_INFO_OFFSET);
}

template<typename T>
T SAMP::GetSAMPPtrInfo(uint32_t offset)
{
	if (g_SampBaseAddress == 0)
		return NULL;
	return *(T *)(g_SampBaseAddress + offset);
}

const char* SAMP::getWeaponName(uint8_t id)
{
	switch(id){
	case 0: return "Unarmed";
	case 1: return "Brass knuckles";
	case 2: return "Golf club";
	case 3: return "Nite stick";
	case 4: return "Knife";
	case 5: return "Baseball bat";
	case 6: return "Shovel";
	case 7: return "Pool cue";
	case 8: return "Kantana";
	case 9: return "Chainsaw";
	case 10: return "Purple dildo";
	case 11: return "Short vibrator";
	case 12: return "Long vibrator";
	case 13: return "White dildo";
	case 14: return "Flowers";
	case 15: return "Cane";
	case 16: return "Grenades";
	case 17: return "Tear gas";
	case 18: return "Molotov cocktail";
	case 19: return "Vehicle missile";
	case 20: return "Hydra flare";
	case 21: return "Jetpack";
	case 22: return "9mm Pistol";
	case 23: return "Silenced pistol";
	case 24: return "Desert eagle";
	case 25: return "Shotgun";
	case 26: return "Sawn-off shotgun";
	case 27: return "Combat shotgun";
	case 28: return "Micro Uzi";
	case 29: return "Mp5";
	case 30: return "Ak47";
	case 31: return "M4";
	case 32: return "Tec 9";
	case 33: return "Country rifle";
	case 34: return "Sniper rifle";
	case 35: return "Rpg";
	case 36: return "Heat seeking rocket";
	case 37: return "Flame-thrower";
	case 38: return "Mini gun";
	case 39: return "Satchel charges";
	case 40: return "Detonator";
	case 41: return "Spray can";
	case 42: return "Fire extinguisher";
	case 43: return "Camera";
	case 44: return "Night vision";
	case 45: return "Thermal goggles";
	case 46: return "Parachute";
	default: return "Unknown";
	}
}
