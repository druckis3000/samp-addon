#include <string>

#include "utils/keycombo.h"
#include "addon_ievent.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/samp.h"
#include "utils/event.h"
#include "game/gtasa.h"
#include "settings.h"

namespace AddonIevent {

	// ----- Private constants -----

	#define IEVENT_KEY_1	0xA0 // L-Shift
	#define IEVENT_KEY_2	0x32 // 2

	struct KeyCombo iEventKeyCombo;

	// ----- Private variables -----
	
	volatile bool bCheatEnabled = false;
	int iCurrentEvent = -1;
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

	bCheatEnabled = Settings::getBool("lsg", "ieventEnabled", false);
}

void AddonIevent::updateAddon()
{
	if(!bCheatEnabled) return;

	// Skip if chat or scoreboard or dialog is active
	if(isSampChatInputActive() || isSampScoreboardActive() || isSampDialogActive()) return;

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