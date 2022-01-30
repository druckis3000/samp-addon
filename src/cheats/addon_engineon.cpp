#include "addon_engineon.h"
#include "game/samp.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/gtasa.h"
#include "utils/helper.h"
#include "settings.h"
#include <thread>

namespace AddonEngineon {

	// ----- Private constants -----

	const unsigned int TXT_COLOR = 0xFF7B68EE;
	const int INFO_TEXTDRAW_ID = 15;
	bool bAddonEnabled = true;

	// Used for watching textdraw text changes
	char szOldInfoTextdrawText[SAMP_MAX_TEXTDRAW_TEXT_LENGTH] = "";

	volatile bool bEngineRestartThreadRunning = false;
	volatile bool bEngineStartCaptured = false;
}

void AddonEngineon::setupAddon()
{
	#ifdef LOG_VERBOSE
		Log("addon_engineon.cpp: Registering /zae command");
	#endif

	SAMP::addClientCommand("zae", []{
		#ifdef LOG_VERBOSE
			Log("addon_engineon.cpp: /zae command called");
		#endif

		if(bAddonEnabled){
			// To make sure thread stopped
			bEngineStartCaptured = true;
		}

		// Toggle
		bAddonEnabled = !bAddonEnabled;

		// Inform
		if(bAddonEnabled) GTA_SA::addMessage((const char*)"Auto engine start: ~g~On", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Auto engine start: ~r~off", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("addon_engineon.cpp: Loading settings");
	#endif

	bAddonEnabled = Settings::getBool("lsg", "autoEngineStart", true);
}

void AddonEngineon::updateAddon()
{
	if(!bAddonEnabled) return;

	struct stTextdraw *infoTextdraw = getPlayerTextdraw(INFO_TEXTDRAW_ID);
	if(infoTextdraw != nullptr){
		if(strcmp(szOldInfoTextdrawText, infoTextdraw->szText) != 0){
			// Text have changed, copy new text
			strcpy(szOldInfoTextdrawText, infoTextdraw->szText);
			
			if(strstr(szOldInfoTextdrawText, "Variklis neuzsikur") != 0 || strstr(szOldInfoTextdrawText, "Uzgeso varikl")){
				// Engine failed to start, or stopped during crash, start it

				// Set thread running flag
				bEngineRestartThreadRunning = true;

				std::thread runAsync([=]{
					// Press left mouse button until engine start is captured
					while(!bEngineStartCaptured){
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
							// Random delay after being informed about engine start failure
							Sleep(50);
						}

						mouseClick(LEFT_MOUSE_BUTTON, 200);

						if(dialogHidden){
							// If there was dialog shown, show it again
							Sleep(50);
							unhideSampDialog();
						}

						Sleep(250);
					}

					// Reset engine restarting flag after successful mouse click
					bEngineStartCaptured = false;

					// Thread finished
					bEngineRestartThreadRunning = false;
				});
				runAsync.detach();
			}
		}else if(strstr(szOldInfoTextdrawText, "vedineji") != 0 && strstr(szOldInfoTextdrawText, "varikl") != 0){
			if(bEngineRestartThreadRunning && !bEngineStartCaptured){
				// LMB click captured, engine starting again
				bEngineStartCaptured = true;
			}
		}
	}
}