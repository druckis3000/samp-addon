#include <windows.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stddef.h>
#include <thread>
#include "utils/helper.h"
#include "main.h"
#include "settings.h"
#include "utils/memfuncs.h"
#include "game/samp.h"
#include "game/gtasa.h"
#include "game/utils/callbacks.h"
#include "game/helpers/fastloader.hpp"

#include "cheats/cheat_zvejyba.h"
#include "cheats/cheat_speedcam.h"
#include "cheats/cheat_laikrasciai.h"
#include "cheats/cheat_cars.h"
#include "cheats/cheat_esp.h"
#include "cheats/addon_ievent.h"
#include "cheats/addon_engineon.h"
#include "cheats/addon_racecam.h"

// ---------- DLL SPECIFIC STUFF ----------

namespace Main {

	// ----- Private variables -----

	double g_ulNextSecondTick = 0;
	
	volatile bool g_bShouldExit = false;
}

void MainThreadFunc()
{
	// Redirect cout to C:\thread.txt for debugging
	std::ofstream outFile;
	outFile.open("C:\\thread.txt");
	std::cout.rdbuf(outFile.rdbuf());

	Log("SCR loading...");
	if(!Settings::loadSettings()){
		Log("Failed to read settings file, using default values");
	}

	// Initialize fast loader
	initFastLoader();
	Log("FastLoader ready!");

	// Setup event listening system
	setupGameCallbacks();

	// Setup samp hooks/pointers/patches
	SAMP::setupSystem();
	
	// Setup gta hooks/pointers/patches
	GTA_SA::setupSystem();
	
	// Setup cheats
	CheatZvejyba::setupCheat();
	CheatAntiSpeedcam::setupCheat();
	CheatLaikrasciai::setupCheat();
	CheatSpeed::setupCheat();
	CheatESP::setupCheat();
	AddonIevent::setupAddon();
	AddonEngineon::setupAddon();
	AddonRacecam::setupAddon();

	Log("Script ready! Starting main thread");

	while(!Main::g_bShouldExit){
		// Update gta/samp systems
		GTA_SA::loop();
		SAMP::loop();
		
		// Update cheats
		if(g_Samp != nullptr && g_Samp->pPools != nullptr && g_Samp->pPools->pPlayer != nullptr && g_Samp->pPools->pPlayer->pLocalPlayer != nullptr) {
			CheatZvejyba::updateCheat();
			CheatAntiSpeedcam::updateCheat();
			CheatLaikrasciai::updateCheat();
			CheatSpeed::updateCheat();
			CheatESP::updateCheat();
			AddonIevent::updateAddon();
			AddonEngineon::updateAddon();
			AddonRacecam::updateAddon();
		}

		// One second timer
		if(GetTimeMillis() >= Main::g_ulNextSecondTick){
			SAMP::oneSecondTimer();

			// Update next tick
			Main::g_ulNextSecondTick = GetTimeMillis() + 1.0;
		}
		
		Sleep(18);
	}
	
	Log("SCR unloading...");
}

extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
	{
		// Create main dll thread
		try {
			std::thread t(MainThreadFunc);
			t.detach();
			Log("Created dll thread successfully!");
		}catch(...){
			Log("Failed to create dll thread!");
		}
		break;
	}
	case DLL_PROCESS_DETACH:
		// Game shutting down, stop all threads and cleanup
		Log("DLL_PROCESS_DETACH called! Exiting..");
		Main::g_bShouldExit = true;
		Log("DLL_PROCESS_DETACH finished!");
		break;
	}
	return TRUE;
}
