#include "keycombo.h"
#include <windows.h>

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