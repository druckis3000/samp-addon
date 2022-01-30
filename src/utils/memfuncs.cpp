#include "memfuncs.h"
#include <iostream>
#include <psapi.h>

// ----- Hooking/redirecting -----

void MidFuncHook(FHook *hook)
{
	// Make jump
	MakeJump(hook->patchAddress, hook->hookAddress, hook->overwriteSize);
	
	// Set "jump back to original function" address location
	hook->jmpBackAddress = hook->patchAddress + hook->overwriteSize;
}

void MakeJump(DWORD dwJumpAddress, DWORD dwHookHandler, int size)
{
	// Calculate relative addresses
	DWORD jmpRelativeAddress = (dwHookHandler - (dwJumpAddress + 5));

	// Overwrite memory protection and save previous protection mode
	DWORD dwOldProtect;
	VirtualProtect((LPVOID)dwJumpAddress, size, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// Write first byte as "call" instruction
	memset((PVOID)dwJumpAddress, (BYTE)0xE9, 1);
	// Write relative address
	memcpy((BYTE*)(dwJumpAddress+0x1), &jmpRelativeAddress, 4);

	// Overwrite left bytes with NOP
	memset((BYTE*)(dwJumpAddress+0x5), 0x90, size - 0x5);

	// Reset memory protection
	VirtualProtect((LPVOID)dwJumpAddress, size, dwOldProtect, NULL);
}

/**
 * @param dwPatchLoc - absolute call instruction address
 * @param newFuncAddress - new address to redirect call to
 */
void RedirectCall(DWORD dwPatchLoc, void *newFuncAddress)
{
	DWORD dwRelativeTargetAddr = ((DWORD)newFuncAddress) - (dwPatchLoc + 5);
	
	BYTE *patchAddress = (BYTE*)(dwPatchLoc + 1);

	DWORD dwOldProtect;
	VirtualProtect(patchAddress, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*((DWORD*)patchAddress) = dwRelativeTargetAddr;
	VirtualProtect(patchAddress, 4, dwOldProtect, NULL);
}

// ----- Writing to memory -----

void writeMemory(DWORD address, void *value, int size)
{
	void *memory = (void*)address;

	DWORD dwOldProtect;
	VirtualProtect(memory, size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memcpy(memory, value, size);
	VirtualProtect(memory, size, dwOldProtect, NULL);
}

void writeMemory(DWORD address, BYTE value, int size)
{
	BYTE *memory = (BYTE*)address;

	DWORD dwOldProtect;
	VirtualProtect(memory, size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	memset(memory, value, size);
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

DWORD FindPattern(DWORD moduleHandle, char *pattern, char *mask)
{
	MODULEINFO mInfo = { 0 };

	GetModuleInformation(GetCurrentProcess(), (HMODULE)moduleHandle, &mInfo, sizeof(MODULEINFO));

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