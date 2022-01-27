#ifndef HOOKING_H
#define HOOKING_H

#include <windows.h>

/* Struct used to hold function hook information */
typedef struct {
	DWORD patchAddress;		// Absolute
	DWORD hookAddress;		// Hook function address
	size_t overwriteSize;	// How many bytes to overwrite, when writing jmp instruction
	DWORD jmpBackAddress;	// Absolute
} FHook;

// ----- REGULAR HOOKING FUNCTIONS -----

void MidFuncHook(FHook *hook);

// ----- GTA MODULE FUNCTIONS -----

int __attribute__ ((noinline)) GetGTABaseAddress();
int __attribute__ ((noinline)) GetGlobalAddress(int address);
void RedirectGTACall(int address, void *func, bool vp);

// ----- SAMP MODULE FUNCTIONS -----

void RedirectCall(DWORD dwPatchLoc, void *newFuncAddress);

// ----- Utils -----

void writeMemory(DWORD address, BYTE value, int size);
void writeMemoryArray(DWORD address, const char *value, int size);
DWORD FindPattern(char *pattern, char *mask);

#endif
