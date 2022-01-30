#include "utils/event.h"
#include "gtasa.h"
#include "utils/helper.h"
#include "utils/memfuncs.h"
#include "samp.h"
#include "helpers/sampfuncs.hpp"

// ----- gta_sa.exe module vars -----

#define FAR_CLIP_ADDRESS		0x00B7C4F0
#define FOG_ADDRESS				0x00B7C4F4
#define GTA_HOOK_GAME_PROCESS	0x0053E981
#define GTA_HOOK_DRAW_DISTANCE	0x0055FCC8
#define GTA_PATCH_INFINITE_RUN	0x00B7CEE4

namespace GTA_SA {

	DWORD gtaBaseAddress = (DWORD)NULL;
	HWND hwndGtaSa = NULL;

	// ----- Function hooking -----

	FHook drawDistanceHook;

	// ----- Script related vars -----

	int g_uFPSCounter;
	float g_fTotalFps, g_fAvgFps, g_fDeltaTime;
	double g_fQuartSecondTimer;

	int g_uFPSTextdraw = -1;

	// ----- gtasa.cpp function declarations -----

	DWORD getModuleAddress(DWORD address);
}

// ----- gta_sa.exe function hooks -----

__declspec(naked) void HOOK_drawDistance_NAKED();
void HOOK_gameProcess();

// ----- gtasa.cpp functions -----

bool GTA_SA::setupSystem()
{
	// Get gta_sa.exe base address
    gtaBaseAddress = (DWORD)GetModuleHandleA(NULL);
    
    // Get gta_sa.exe HWND
    do {
    	hwndGtaSa = FindWindow(NULL, "GTA:SA:MP");
    	Sleep(250);
    } while(hwndGtaSa == NULL);

	// Patch infinite run
	*(bool*)(GTA_PATCH_INFINITE_RUN) = true;
	Log("Infinite run installed!");

	// Patch to enabled changing drawing distance
	drawDistanceHook.patchAddress = GTA_HOOK_DRAW_DISTANCE;
	drawDistanceHook.hookAddress = (DWORD)HOOK_drawDistance_NAKED;
	drawDistanceHook.overwriteSize = 7;
	MidFuncHook(&drawDistanceHook);
	Log("Draw distance patched!");

	// Hook game process function
	RedirectCall(GTA_HOOK_GAME_PROCESS, (void*)HOOK_gameProcess);

	return true;
}

void GTA_SA::loop()
{
	// FPS Counter
	double currentTime = GetTimeMillis();
	if(currentTime - g_fQuartSecondTimer >= .25){
		// Update current fps
		const float currentFps = *((float*)0xB7CB50);
		g_fTotalFps += currentFps;
		g_uFPSCounter++;

		// Calculate delta time
		g_fDeltaTime = (1000.0 / currentFps) / 1000.0;

		if(g_uFPSCounter == 4){
			// After 1 second, calculate average fps
			g_fAvgFps = g_fTotalFps / 4.0;
			g_fTotalFps = 0.0;
			g_uFPSCounter = 0;

			if(g_uFPSTextdraw == -1){
				// Create textdraw for displaying fps
				g_uFPSTextdraw = createTextdrawWithId(99, "000", 20.0, 340.0, 0xFFFFFFFF);
				if(g_uFPSTextdraw){
					Log("Created textdraw for FPS");
				}
			}

			if(g_uFPSTextdraw > -1){
				// Update FPS text
				setTextdrawText(g_uFPSTextdraw, std::to_string((int)g_fAvgFps).c_str());
			}
		}

		// Update timer
		g_fQuartSecondTimer = currentTime;
	}
}

void HOOK_gameProcess()
{
	// Always call samp game proc, otherwise samp will be freezed
	SAMP::callGameProc();
}

float GTA_SA::getFPS(){ return GTA_SA::g_fAvgFps; }

float GTA_SA::getDelta(){ return GTA_SA::g_fDeltaTime; }

HWND GTA_SA::getHwnd(){ return GTA_SA::hwndGtaSa; }

// ----- Function hooks -----

__declspec(naked) void HOOK_drawDistance_NAKED()
{
	__asm("pushf");
	__asm("pusha");
	__asm("leal 0x50(%esi), %eax");
	__asm("cmp $0x00B7C4F0, %eax");
	__asm("je nax1");
	__asm("popa");
	__asm("popf");
	__asm("fstp 0x50(%esi)");
	__asm("jmp nax2");
	__asm("nax1:");
	__asm("movl (%eax), %edx");
	__asm("fstp 0x50(%esi)");
	__asm("movl %edx, (%eax)");
	__asm("popa");
	__asm("popf");
	__asm("nax2:");
	__asm("fld 0x18(%esp)");
	__asm("jmp *(%0)"
			:
			:"m"(GTA_SA::drawDistanceHook.jmpBackAddress));
	__asm("nop");
}

// ----- Helper functions -----

DWORD GTA_SA::getModuleAddress(DWORD address)
{
	return GTA_SA::gtaBaseAddress - 0x400000 + address;
}

// ----- gta_sa.exe functions -----

void GTA_SA::showBigMessage(const char *message, unsigned short style)
{
	reinterpret_cast<void(__cdecl *)(const char *, unsigned short)>(GTA_FUNC_SHOW_BIG_MSG)(message, style);
}

void GTA_SA::addMessage(const char *text, unsigned int time, unsigned short flag, bool bPreviousBrief)
{
	reinterpret_cast<void(__cdecl *)(const char *, int, unsigned short, bool)>(GTA_FUNC_SHOW_MESSAGE_JUMP_Q)(text, time, flag, bPreviousBrief);
}

void GTA_SA::forceWeather(int state)
{
	*(uint16_t*)0x00C81318 = (uint16_t)state;
	*(uint16_t*)0x00C8131C = (uint16_t)state;
	*(uint16_t*)0x00C81320 = (uint16_t)state;
}

void GTA_SA::setDrawDistance(float distance)
{
	*(float*)0x00B7C4F0 = distance;
}

float GTA_SA::getDrawDistance()
{
	return (*(float*)0x00B7C4F0);
}

// ----- CCamera.cpp -----

float* CCamera::getCamPosition()
{
	void *camera = (void*)0xB6F028;
	return reinterpret_cast<float*(__thiscall*)(void *_this)>(0x50AE50)(camera);
}

float CCamera::getCamXAngle()
{
	return *(float*)0xB6F248;
}

float CCamera::getCamZAngle()
{
	return *(float*)0xB6F258;
}

void CCamera::setCamXAngle(float xAngle)
{
	*(float*)0xB6F248 = xAngle;
}

void CCamera::setCamZAngle(float zAngle)
{
	*(float*)0xB6F258 = zAngle;
}

/** If vecScreen[2] < 1.0f then it's behind camera (culled) */
void CCamera::calcScreenCoords(float *vecWorld, float *vecScreen)
{
	// View matrix
	float *m = (float*)(0xB6FA2C);

	vecScreen[0] = (vecWorld[2] * m[8]) + (vecWorld[1] * m[4]) + (vecWorld[0] * m[0]) + m[12];
	vecScreen[1] = (vecWorld[2] * m[9]) + (vecWorld[1] * m[5]) + (vecWorld[0] * m[1]) + m[13];
	vecScreen[2] = (vecWorld[2] * m[10]) + (vecWorld[1] * m[6]) + (vecWorld[0] * m[2]) + m[14];

	// Display resolution
	DWORD dWidth = *((DWORD*)(0xC17044));
	DWORD dHeight = *((DWORD*)(0xC17048));

	float zInv = (float)1.0 / vecScreen[2];
	vecScreen[0] *= (zInv * (float)dWidth);
	vecScreen[1] *= (zInv * (float)dHeight);
}