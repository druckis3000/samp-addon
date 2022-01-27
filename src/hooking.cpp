#include "hooking.h"
#include <iostream>
#include <psapi.h>

// ----- Mid function hooking -----

void MidFuncHook(FHook *hook)
{
	// Calculate addresses
	BYTE *patchAddress = (BYTE*)hook->patchAddress;
	DWORD dwRelativeJumpAddress = (hook->hookAddress - hook->patchAddress) - 5;
	
	// Overwrite memory protection and save previous protection mode
	DWORD dwOldProtect;
	VirtualProtect(patchAddress, hook->overwriteSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	// Write first byte as "call" instruction
	*patchAddress = 0xE9;

	// Write "call target" address
	*((DWORD *)(patchAddress + 0x1)) = dwRelativeJumpAddress;
	
	// Overwrite left bytes with NOP
	for(DWORD x = 0x5; x < hook->overwriteSize; x++){
		*(patchAddress + x) = 0x90;
	}
	
	// Reset memory protection
	VirtualProtect(patchAddress, hook->overwriteSize, dwOldProtect, NULL);
	
	// Set "jump back to original function" address location
	hook->jmpBackAddress = hook->patchAddress + hook->overwriteSize;
}

// ----- Redirect calls -----

/** address - absolute call instruction address
	newFuncAddress - new function to redirect call to */
void RedirectCall(DWORD dwPatchLoc, void *newFuncAddress)
{
	DWORD dwRelativeTargetAddr = ((DWORD)newFuncAddress) - (dwPatchLoc + 5);
	
	BYTE *patchAddress = (BYTE*)(dwPatchLoc + 1);

	DWORD dwOldProtect;
	VirtualProtect(patchAddress, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*((DWORD*)patchAddress) = dwRelativeTargetAddr;
	VirtualProtect(patchAddress, 4, dwOldProtect, NULL);
}

// ----- Utils -----

void writeMemory(DWORD address, BYTE value, int size)
{
	BYTE *memory = (BYTE*)address;

	DWORD dwOldProtect;
	VirtualProtect(memory, size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	for(int i=0; i<size; i++){
		*(memory + i) = value;
	}

	VirtualProtect(memory, size, dwOldProtect, NULL);
}

void writeMemoryArray(DWORD address, const char *value, int size)
{
	BYTE *memory = (BYTE*)address;

	DWORD dwOldProtect;
	VirtualProtect(memory, size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	for(int i=0; i<size; i++){
		*(memory + i) = value[i];
	}

	VirtualProtect(memory, size, dwOldProtect, NULL);
}

DWORD FindPattern(char *pattern, char *mask)
{
	MODULEINFO mInfo = { 0 };

	GetModuleInformation(GetCurrentProcess(), GetModuleHandle("samp.dll"), &mInfo, sizeof(MODULEINFO));

	DWORD base = (DWORD)mInfo.lpBaseOfDll;
	DWORD size = (DWORD)mInfo.SizeOfImage;

	DWORD patternLength = (DWORD)strlen(mask);

	for (DWORD i = 0; i < size - patternLength; i++)
	{
		bool found = true;
		for (DWORD j = 0; j < patternLength; j++)
		{
			found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
		}

		if (found)
		{
			return base + i;
		}
	}

	return (DWORD)NULL;
}