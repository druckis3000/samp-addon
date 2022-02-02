#ifndef KEY_COMBO_H
#define KEY_COMBO_H

#include <stdint.h>

// ----- Helper for keycombo -----

struct KeyCombo {
	bool bKey1 = false;
	bool bKey2 = false;
	bool bKeyComboProcessed = false;
};

void updateKeyCombo(const uint32_t key1, const uint32_t key2, struct KeyCombo &keyCombo);

inline bool isKeyComboPressed(struct KeyCombo &keyCombo)
{
	return keyCombo.bKey1 && keyCombo.bKey2 && !keyCombo.bKeyComboProcessed;
}

inline void resetKeyStates(struct KeyCombo &keyCombo)
{
	keyCombo.bKey1 = false;
	keyCombo.bKey2 = false;
	keyCombo.bKeyComboProcessed = true;
}

// ----- Input simulation -----

#define LEFT_MOUSE_BUTTON	0
#define RIGHT_MOUSE_BUTTON	1

void mouseClick(int btn, int delay);

void keyDown(int key);
void keyUp(int key);
void keyPress(int key, int delay);

#endif