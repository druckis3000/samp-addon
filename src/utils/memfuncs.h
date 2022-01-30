#ifndef MEM_FUNCS_H
#define MEM_FUNCS_H

#include <windows.h>

/* Struct used to hold function hook information */
typedef struct {
	DWORD patchAddress;		// Absolute
	DWORD hookAddress;		// Hook function address
	size_t overwriteSize;	// How many bytes to overwrite, when writing jmp instruction
	DWORD jmpBackAddress;	// Absolute
} FHook;

// Hooking/redirecting

void MidFuncHook(FHook *hook);
void MakeJump(DWORD dwJumpAddress, DWORD dwHookHandler, int size);
void RedirectCall(DWORD dwPatchLoc, void *newFuncAddress);

// Writing to memory

void writeMemory(DWORD address, void *value, int size);
void writeMemory(DWORD address, BYTE value, int size);
void writeMemoryArray(DWORD address, const char *value, int size);
DWORD FindPattern(DWORD moduleHandle, char *pattern, char *mask);

#endif
