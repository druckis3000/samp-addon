#include <string>
#include <iostream>

#include "utils/keycombo.h"
#include "addon_ievent.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/samp.h"
#include "utils/event.h"
#include "game/gtasa.h"
#include "settings.h"

namespace AddonIevent {

	// ----- Private constants -----

	#define IEVENT_KEY_1	0xA0
	#define IEVENT_KEY_2	0x32

	struct KeyCombo iEventKeyCombo;

	// ----- Private variables -----
	
	volatile bool g_bCheatEnabled = false;
	int g_iCurrentEvent = -1;
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

	addClientCommand("zie", []{
		g_bCheatEnabled = !g_bCheatEnabled;

		// Inform player about current addon state
		if(g_bCheatEnabled) GTA_SA::addMessage((const char*)"IEvent~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"IEvent~n~~r~Isjungtas", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("addon_event.cpp: Loading settings");
	#endif

	g_bCheatEnabled = Settings::getBool("lsg", "ieventEnabled", false);
}

void AddonIevent::updateAddon()
{
	if(!g_bCheatEnabled) return;

	// Skip if chat or scoreboard or dialog is active
	if(isSampChatInputActive() || isSampScoreboardActive() || isSampDialogActive()) return;

	updateKeyCombo(IEVENT_KEY_1, IEVENT_KEY_2, iEventKeyCombo);

	if(isKeyComboPressed(iEventKeyCombo)){
		// Reset key flags
		resetKeyStates(iEventKeyCombo);

		#ifdef LOG_VERBOSE
			Log("addon_ievent.hpp: Key combo pressed");
		#endif
		
		// Skip if there's no event atm
		if(g_iCurrentEvent == -1){
			#ifdef LOG_VERBOSE
				Log("addon_ievent.hpp: No event is happening right now");
			#endif

			infoMsgf("Siuo metu joks eventas nevyksta!");
			return;
		}

		#ifdef LOG_VERBOSE
			Log("addon_ievent.hpp: Send /event %d command");
		#endif

		// Send /event cmd
		char cmdBuffer[32];
		sprintf(cmdBuffer, "/event %d", g_iCurrentEvent);
		sayCommand(cmdBuffer);

		// Reset current event id
		g_iCurrentEvent = -1;
	}
}

void AddonIevent::onMessage(const char *msg, DWORD color)
{
	if(!g_bCheatEnabled) return;

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
						g_iCurrentEvent = eventNr;
						#ifdef LOG_VERBOSE
							Logf("addon_ievent.hpp: Got event id: %d", g_iCurrentEvent);
						#endif
					}catch(...){
						// Failed..
						#ifdef LOG_VERBOSE
							Logf("addon_ievent.hpp: Failed to acquire event id");
						#endif
						g_iCurrentEvent = -1;
					}

					if(g_iCurrentEvent > -1){
						// Inform player about event
						GTA_SA::addMessage((const char*)"I event? Q+2", 2000, 0, false);
						
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
			g_iCurrentEvent = -1;
			#ifdef LOG_VERBOSE
				Log("addon_ievent.hpp: Event full!");
			#endif
		}else if(strMsg.rfind("Eventas nutrauktas") != std::string::npos){
			g_iCurrentEvent = -1;
			#ifdef LOG_VERBOSE
				Log("addon_ievent.hpp: Event cancelled!");
			#endif
		}
	}
}