#include "addon_engineon.h"
#include "game/samp.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/gtasa.h"
#include "utils/helper.h"
#include "settings.h"
#include <windows.h>
#include <thread>

namespace AddonEngineon {

	// ----- Private constants -----

	const unsigned int TXT_COLOR = 0xFF7B68EE;
	const int INFO_TEXTDRAW_ID = 15;
	bool bAddonEnabled = true;

	// Used for watching textdraw text changes
	char szOldInfoTextdrawText[SAMP_MAX_TEXTDRAW_TEXT_LENGTH] = "";

	volatile bool bEngineRestartThreadRunning = false;
	volatile bool bEngineReStarting = false;
}

void AddonEngineon::setupAddon()
{
	#ifdef LOG_VERBOSE
		Log("addon_engineon.cpp: Registering /zae command");
	#endif

	addClientCommand("zae", []{
		#ifdef LOG_VERBOSE
			Log("addon_engineon.cpp: /zae command called");
		#endif

		if(bAddonEnabled){
			// To make sure thread stopped
			bEngineReStarting = true;
		}

		// Toggle
		bAddonEnabled = !bAddonEnabled;

		// Inform
		if(bAddonEnabled) GTA_SA::addMessage((const char*)"Auto engine start~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Auto engine start~n~~r~Isjungtas", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("addon_engineon.cpp: Loading settings");
	#endif

	bAddonEnabled = Settings::getBool("lsg", "autoEngineStart", true);
}

void AddonEngineon::updateAddon()
{
	if(!bAddonEnabled) return;

	// Checking for full inventory
	struct stTextdraw *infoTextdraw = getPlayerTextdraw(INFO_TEXTDRAW_ID);
	if(infoTextdraw != nullptr){
		if(strcmp(szOldInfoTextdrawText, infoTextdraw->szText) != 0){
			// Text have changed, copy new text
			strcpy(szOldInfoTextdrawText, infoTextdraw->szText);
			
			if(strstr(szOldInfoTextdrawText, "Variklis neuzsikur") != 0){
				// Engine failed to start, start again
				
				#ifdef LOG_VERBOSE
					Log("addon_engineon.cpp: Engine start failed!");
				#endif

				// Set thread running flag
				bEngineRestartThreadRunning = true;

				std::thread runAsync([=]{
				
					#ifdef LOG_VERBOSE
						Log("addon_engineon.cpp: runAsync started");
					#endif

					// Press left mouse button until engine start is trigerred
					while(!bEngineReStarting){
						// Hide dialog if there's any visible
						bool dialogHidden = false;
						if(isSampDialogActive()){
							#ifdef LOG_VERBOSE
								Log("addon_engineon.cpp: Hide active dialog");
							#endif

							hideSampDialog();
							Sleep(125);
							dialogHidden = true;
						}else{
							// Random delay after being informed
							Sleep(15);
						}

						#ifdef LOG_VERBOSE
							Log("addon_engineon.cpp: Click LMB");
						#endif

						mouseClick(LEFT_MOUSE_BUTTON, 200);

						if(dialogHidden){
							Sleep(20);

							#ifdef LOG_VERBOSE
								Log("addon_engineon.cpp: Unhide active dialog");
							#endif

							unhideSampDialog();
						}

						Sleep(250);
					}

					// Reset engine restarting flag after successful mouse click
					bEngineReStarting = false;

					// Thread finished
					bEngineRestartThreadRunning = false;

					#ifdef LOG_VERBOSE
						Log("addon_engineon.cpp: runAsync finished");
					#endif
				});
				runAsync.detach();
			}
		}else if(strstr(szOldInfoTextdrawText, "vedineji") != 0 && strstr(szOldInfoTextdrawText, "varikl") != 0){
			if(bEngineRestartThreadRunning && !bEngineReStarting){
				#ifdef LOG_VERBOSE
					Log("addon_engineon.cpp: LMB captured!");
				#endif

				// Mouse click captured, engine starting again
				bEngineReStarting = true;
			}
		}
	}
}