#ifndef QUICK_LOAD_HPP
#define QUICK_LOAD_HPP

void ShowRaster_Prox();
void StartGame_Prox();
void ChangeMenu_Prox();

const int VAR_FRAME_LIMITER = 0;

BOOL HookInstall( DWORD dwInstallAddress, DWORD dwHookHandler, int iJmpCodeSize );

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

void disableFPSLock()
{
	DWORD dwSAMPBase = (DWORD)GetModuleHandle("samp.dll");

	if (dwSAMPBase != 0)
	{
		DWORD dwFPSSleep[4];

		dwFPSSleep[0] = FindPattern((char*)"\x57\x8B\xF9\xE8\x96\xCB\xF8\xFF\x8B\xF0\xA1", (char*)"xxxx????xxx") + 0xA; // \x57\x8B\xF9\xE8\x76\xCB\xF8\xFF\x8B\xF0\xA1 r4-1
		dwFPSSleep[1] = FindPattern((char*)"\xBA\x0A\x00\x00\x00\x2B\xD6", (char*)"xxxxxxx") + 0x5;
		dwFPSSleep[2] = FindPattern((char*)"\xB8\x00\x00\x80\x3F\xA3", (char*)"xxxxxx") + 0x5; 
		dwFPSSleep[3] = FindPattern((char*)"\xBA\x80\x1A\x56\x00\xFF\xE2", (char*)"xxxxxxx") - 0x7;

		frame_limiter_off_addr = dwFPSSleep[2] - 0x5;
		frame_limiter_on_addr = dwFPSSleep[2] - 0x9;

		DWORD oldProt;
		if (dwFPSSleep[1] != 0) {
			// Disable the 100FPS Lock
			VirtualProtect((LPVOID)dwFPSSleep[0], 7, PAGE_EXECUTE_READWRITE, &oldProt);
			HookInstall(dwFPSSleep[0], (DWORD)FPSStrafeHook, 7);

			VirtualProtect((LPVOID)dwFPSSleep[1], 7, PAGE_EXECUTE_READWRITE, &oldProt);

			BYTE val1 = 0x0;
			if(*(BYTE*)dwFPSSleep[1] + 0x2 != val1)
        		memcpy((BYTE*)dwFPSSleep[1] + 0x2, &val1, sizeof(BYTE));

			BYTE val2 = 0x90;
			if(*(BYTE*)dwFPSSleep[1] + 0x4 != val2)
        		memcpy((BYTE*)dwFPSSleep[1] + 0x4, &val2, sizeof(BYTE));

			VirtualProtect((LPVOID)dwFPSSleep[2], 5, PAGE_EXECUTE_READWRITE, &oldProt);
			memcpy((void*)dwFPSSleep[2], (char*)"\x90\x90\x90\x90\x90", 5);

			VirtualProtect((LPVOID)dwFPSSleep[3], 1, PAGE_EXECUTE_READWRITE, &oldProt);

			BYTE val3 = 0x0;
			if(*(BYTE*)dwFPSSleep[3] != val3)
				memcpy((void*)dwFPSSleep[3], &val3, sizeof(BYTE));
		}
	}
}

BYTE* CreateJump( DWORD dwFrom, DWORD dwTo, BYTE * ByteArray )
{
    ByteArray[0] = 0xE9;
	*(DWORD*)(&ByteArray[1]) = (dwTo - (dwFrom + 5));
    return ByteArray;
}

BOOL HookInstall( DWORD dwInstallAddress, DWORD dwHookHandler, int iJmpCodeSize )
{
    BYTE JumpBytes[50];
    memset(JumpBytes, 0x90, 50);
    if(CreateJump(dwInstallAddress, dwHookHandler, JumpBytes)){
        memcpy((PVOID)dwInstallAddress, JumpBytes, iJmpCodeSize);
        return TRUE;
    }else{
        return FALSE;
    }
}

#endif