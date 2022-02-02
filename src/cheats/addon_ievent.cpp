#include <string>
#include <thread>

#include "utils/input.h"
#include "addon_ievent.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/samp.h"
#include "game/utils/callbacks.h"
#include "game/gtasa.h"
#include "settings.h"

namespace AddonIevent {

	// ----- Private constants -----

	#define IEVENT_KEY_1	0xA0 // L-Shift
	#define IEVENT_KEY_2	0x33 // 3

	struct KeyCombo iEventKeyCombo;

	// ----- Private variables -----
	
	volatile bool bCheatEnabled = false;
	bool bGoingToEvent = false;
	int iCurrentEvent = -1;

	// For watching textdraw changes
	char oldInfoTextdrawText[401] = "";

	// ----- Private functions ------

	void onMessage(const char *msg, DWORD color);
	void goToEvent();
};

void AddonIevent::setupAddon()
{
	#ifdef LOG_VERBOSE
		Log("addon_ievent.hpp: adding AddMessageCallback");
	#endif

	// Add on samp message callback
	addAddMessageCallback(onMessage);

	#ifdef LOG_VERBOSE
		Log("addon_ievent.cpp: Registering /zie command");
	#endif

	SAMP::addClientCommand("zie", []{
		bCheatEnabled = !bCheatEnabled;

		// Inform player about current addon state
		if(bCheatEnabled) GTA_SA::addMessage((const char*)"IEvent: ~g~on", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"IEvent: ~r~off", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("addon_event.cpp: Loading settings");
	#endif

	// Cheat is only meant for lsgyvenimas.lt server, so
	// disable it if player is not connecting to that server
	if(!strncmp(g_Samp->szHostAddress, "54.36.124.11", 12)){
		bCheatEnabled = Settings::getBool("lsg", "ieventEnabled", false);
	}else{
		bCheatEnabled = false;
	}
}

void AddonIevent::updateAddon()
{
	if(!bCheatEnabled) return;

	// Skip if chat or scoreboard or dialog is active
	if(isSampChatInputActive() || isSampScoreboardActive() || isSampDialogActive()) return;

	if(bGoingToEvent){
		// Engine is shutting down, watch for shutdown message
		struct stTextdraw *infoTextdraw = getPlayerTextdraw(15);
		if(infoTextdraw != nullptr){
			if(strcmp(oldInfoTextdrawText, infoTextdraw->szText) != 0){
				strcpy(oldInfoTextdrawText, infoTextdraw->szText);

				if(strstr(oldInfoTextdrawText, "ariklis") != 0 && strstr(oldInfoTextdrawText, "sjungtas")){
					std::thread runAsync([=]{
						// Wait a little bit after last key combo (engine shutdown)
						Sleep(100);
						
						// Engine off, pull handbrake
						keyPress(VK_SPACE, 150);

						// Go to event
						goToEvent();
					});
					runAsync.detach();

					// Unset going to event flag
					bGoingToEvent = false;
				}
			}
		}
	}

	updateKeyCombo(IEVENT_KEY_1, IEVENT_KEY_2, iEventKeyCombo);

	if(isKeyComboPressed(iEventKeyCombo)){
		#ifdef LOG_VERBOSE
			Log("addon_ievent.cpp: Detected IEvent key combo");
		#endif

		// Reset key flags
		resetKeyStates(iEventKeyCombo);
		
		// Skip if there's no event atm
		if(iCurrentEvent == -1){
			#ifdef LOG_VERBOSE
				Log("addon_ievent.cpp: No event happening atm");
			#endif
			infoMsg("No event happening atm!");
			return;
		}

		// Check if player is in vehicle
		if(isPlayerInVehicle(PLAYER_ID_SELF)){
			// Check if vehicle engine is on
			struct stSAMPVehicle *sampVehicle = getSAMPVehicle(getPlayerVehicleId(PLAYER_ID_SELF));
			
			if(sampVehicle->iIsMotorOn){
				std::thread runAsync([=]{
					// Wait until IEVENT_KEY_2 is released
					while(GetAsyncKeyState(IEVENT_KEY_2) & 0x8000) Sleep(5);
					
					// Shutdown engine
					bGoingToEvent = true;
					keyDown(VK_SPACE);
					Sleep(100);
					mouseClick(LEFT_MOUSE_BUTTON, 150);
					keyUp(VK_SPACE);
				});
				runAsync.detach();
			}else{
				// Engine is off
				goToEvent();
			}
		}else{
			// Player not in vehicle
			goToEvent();
		}
	}
}

void AddonIevent::goToEvent()
{
	// Send /event cmd
	#ifdef LOG_VERBOSE
		Logf("addon_ievent.cpp: Going to event: %d", iCurrentEvent);
	#endif
	infoMsgf("Going to event: %d", iCurrentEvent);

	char cmdBuffer[32];
	sprintf(cmdBuffer, "/event %d", iCurrentEvent);
	sayCommand(cmdBuffer);

	// Reset current event id
	iCurrentEvent = -1;
}

void AddonIevent::onMessage(const char *msg, DWORD color)
{
	if(!bCheatEnabled) return;

	std::string strMsg = msg;
	if(strMsg.rfind(" * ", 0) == 0){
		if(strMsg.find("Administratorius") != std::string::npos){
			if(strMsg.find("rengia event") != std::string::npos){
				#ifdef LOG_VERBOSE
					Log("addon_ievent.hpp: Event detected");
				#endif

				std::size_t rasykitePos = strMsg.find("/event ");
				if(rasykitePos != std::string::npos){
					try {
						int eventNr = std::stoi(strMsg.substr(rasykitePos + 7));
						iCurrentEvent = eventNr;
						#ifdef LOG_VERBOSE
							Logf("addon_ievent.hpp: Got event id: %d", iCurrentEvent);
						#endif
					}catch(...){
						// Failed..
						#ifdef LOG_VERBOSE
							Logf("addon_ievent.hpp: Failed to acquire event id");
						#endif
						iCurrentEvent = -1;
					}

					if(iCurrentEvent > -1){
						// Inform player about event
						GTA_SA::addMessage((const char*)"I event? LShift+2", 2000, 0, false);
						
						#ifdef LOG_VERBOSE
							Log("addon_ievent.hpp: Player informed!");
						#endif
					}
				}
			}
		}
	}

	if(strMsg.rfind(" * ", 0) == 0){
		if(strMsg.find("Eventas pilnas dalyvi") != std::string::npos){
			#ifdef LOG_VERBOSE
				Log("Event is full, resetting iCurrentEvent");
			#endif
			infoMsgf("Event %d is full", iCurrentEvent);
			iCurrentEvent = -1;
		}else if(strMsg.rfind("Eventas nutrauktas") != std::string::npos){
			#ifdef LOG_VERBOSE
				Log("Event cancelled, resetting iCurrentEvent");
			#endif
			infoMsgf("Event %d cancelled!", iCurrentEvent);
			iCurrentEvent = -1;
		}
	}
}