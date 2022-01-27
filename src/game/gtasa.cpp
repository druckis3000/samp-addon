#include "utils/event.h"
#include "gtasa.h"
#include "utils/helper.h"
#include "hooking.h"
#include "samp.h"
#include "helpers/sampfuncs.h"

// ----- gta_sa.exe module vars -----

#define FAR_CLIP_ADDRESS	0x00B7C4F0
#define FOG_ADDRESS			0x00B7C4F4

namespace GTA_SA {

	DWORD gtaBaseAddress = (DWORD)NULL;
	HWND hwndGtaSa = NULL;

	// ----- Function hooking -----

	FHook drawDistanceHook;
	DWORD drawDistanceTargetAddr = 0x0055FCCF;

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
	*(bool*)(0x00B7CEE4) = true;
	Log("Infinite run installed!");

	// Patch to enabled changing drawing distance
	drawDistanceHook.patchAddress = 0x0055FCC8;
	drawDistanceHook.hookAddress = (DWORD)HOOK_drawDistance_NAKED;
	drawDistanceHook.overwriteSize = 5;
	MidFuncHook(&drawDistanceHook);
	writeMemory(0x0055FCCD, 0x90, 2);
	Log("Draw distance patched!");
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

float GTA_SA::getFPS(){ return GTA_SA::g_fAvgFps; }

float GTA_SA::getDelta(){ return GTA_SA::g_fDeltaTime; }

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
			:"m"(GTA_SA::drawDistanceTargetAddr));
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
