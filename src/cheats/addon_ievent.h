#ifndef ADDON_IEVENT_H
#define ADDON_IEVENT_H

#include <windows.h>

namespace AddonIevent {

	// ----- Public functions -----

	void setupAddon();
	void updateAddon();
	void onMessage(const char *msg, DWORD color);
};

#endif