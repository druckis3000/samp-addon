#include "input.h"
#include <windows.h>

// ----- keycombo -----

void updateKeyCombo(const uint32_t key1, const uint32_t key2, struct KeyCombo &keyCombo)
{
	if(GetAsyncKeyState(key1) & 0x8000){
		keyCombo.bKey1 = true;
	}else{
		keyCombo.bKey1 = false;
		keyCombo.bKeyComboProcessed = false;
	}

	if(GetAsyncKeyState(key2) & 0x8000){
		keyCombo.bKey2 = true;
	}else{
		keyCombo.bKey2 = false;
		keyCombo.bKeyComboProcessed = false;
	}
}

// ----- Input simulation -----

void mouseClick(int btn, int delay)
{
	INPUT inputs = {0};

	// Mouse down
	ZeroMemory(&inputs, sizeof(INPUT));
	inputs.type = INPUT_MOUSE;
	switch(btn){
		case LEFT_MOUSE_BUTTON:
		{ inputs.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break; }
		case RIGHT_MOUSE_BUTTON:
		{ inputs.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break; }
	}
	SendInput(1, &inputs, sizeof(INPUT));
	
	// Add some delay between input events
	Sleep(delay);
	
	// Mouse up
	ZeroMemory(&inputs, sizeof(INPUT));
	inputs.type = INPUT_MOUSE;
	switch(btn){
		case LEFT_MOUSE_BUTTON:
		{ inputs.mi.dwFlags = MOUSEEVENTF_LEFTUP; break; }
		case RIGHT_MOUSE_BUTTON:
		{ inputs.mi.dwFlags = MOUSEEVENTF_RIGHTUP; break; }
	}
	SendInput(1, &inputs, sizeof(INPUT));
}

void keyDown(int key)
{
	INPUT inputs = {0};
	ZeroMemory(&inputs, sizeof(INPUT));
	inputs.type = INPUT_KEYBOARD;
	inputs.ki.wVk = key;
	inputs.ki.dwFlags = 0x0000;
	SendInput(1, &inputs, sizeof(INPUT));
}

void keyUp(int key)
{
	INPUT inputs = {0};
	ZeroMemory(&inputs, sizeof(INPUT));
	inputs.type = INPUT_KEYBOARD;
	inputs.ki.wVk = key;
	inputs.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &inputs, sizeof(INPUT));
}

void keyPress(int key, int delay)
{
	keyDown(key);
	
	// Add some delay between input events
	Sleep(delay);
	
	keyUp(key);
}