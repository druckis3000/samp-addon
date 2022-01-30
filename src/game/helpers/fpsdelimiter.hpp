#ifndef QUICK_LOAD_HPP
#define QUICK_LOAD_HPP

#include "game/samp.h"
#include "settings.h"
#include "utils/helper.h"

void ShowRaster_Prox();
void StartGame_Prox();
void ChangeMenu_Prox();

int VAR_FRAME_LIMITER = 0;

DWORD frame_limiter_off_addr = -1;
DWORD frame_limiter_on_addr = -1;
void __declspec(naked) FPSStrafeHook() {
	__asm("pusha");

	if(VAR_FRAME_LIMITER == 0) {
		__asm("popa");
		__asm("jmp *(%0)"
			:
			:"m"(frame_limiter_off_addr));
	}

	__asm("popa");
	__asm("jmp *(%0)"
			:
			:"m"(frame_limiter_on_addr));
}

void disableFPSLock(DWORD dwSAMPBase)
{
	if (dwSAMPBase != 0)
	{
		int fpsLimit = Settings::getInt("settings", "fpsLimit", -1);
		if(fpsLimit == -1) return;
		VAR_FRAME_LIMITER = fpsLimit;

		DWORD dwFPSSleep[4];

		#if defined(SAMP_VERSION_R4_2)
			dwFPSSleep[0] = FindPattern(dwSAMPBase, (char*)"\x57\x8B\xF9\xE8\x96\xCB\xF8\xFF\x8B\xF0\xA1", (char*)"xxxx????xxx") + 0xA;
		#elif defined(SAMP_VERSION_R4)
			dwFPSSleep[0] = FindPattern(dwSAMPBase, (char*)"\x57\x8B\xF9\xE8\x76\xCB\xF8\xFF\x8B\xF0\xA1", (char*)"xxxx????xxx") + 0xA;
		#endif
		dwFPSSleep[1] = FindPattern(dwSAMPBase, (char*)"\xBA\x0A\x00\x00\x00\x2B\xD6", (char*)"xxxxxxx") + 0x5;
		dwFPSSleep[2] = FindPattern(dwSAMPBase, (char*)"\xB8\x00\x00\x80\x3F\xA3", (char*)"xxxxxx") + 0x5; 
		dwFPSSleep[3] = FindPattern(dwSAMPBase, (char*)"\xBA\x80\x1A\x56\x00\xFF\xE2", (char*)"xxxxxxx") - 0x7;

		frame_limiter_off_addr = dwFPSSleep[2] - 0x5;
		frame_limiter_on_addr = dwFPSSleep[2] - 0x9;

		//DWORD oldProt;
		if (dwFPSSleep[1] != 0) {
			// Disable the 100FPS Lock
			MakeJump(dwFPSSleep[0], (DWORD)FPSStrafeHook, 5);

			BYTE val1 = 0x0;
			if(*(BYTE*)dwFPSSleep[1] + 0x2 != val1)
        		writeMemory(dwFPSSleep[1] + 0x2, val1, 1);

			BYTE val2 = 0x90;
			if(*(BYTE*)dwFPSSleep[1] + 0x4 != val2)
        		writeMemory(dwFPSSleep[1] + 0x4, val2, 1);

			writeMemory(dwFPSSleep[2], (BYTE)0x90, 5);

			BYTE val3 = 0x0;
			if(*(BYTE*)dwFPSSleep[3] != val3)
				writeMemory(dwFPSSleep[3], val3, 1);
		}
	}
}

#endif