#include "cheat_zvejyba.h"
#include "utils/helper.h"
#include "game/samp.h"
#include "utils/event.h"
#include "game/helpers/sampfuncs.hpp"
#include "utils/keycombo.h"
#include "game/gtasa.h"
#include "settings.h"
#include <thread>

/** How it works? When player is fishing and fish is on hook, server shows
 * big message, informing player about fish on hook. When cheat captures
 * such message, it simulates left mouse click. Additionally, there is client
 * command "/afk", which toggle afk mode and enables minimazing samp window.
 * When inventory is full, sound will be played to inform player.
 * 
 * /ztd - for showing fish count textdraw
 * /zzv - toggle fishing bot
 * /afk - toggle afk mode (toggle cursor as well)
 */

namespace CheatZvejyba {

	// ----- Private constants -----

	const unsigned int TXT_COLOR = 0xFF7B68EE;
	const int INFO_TEXTDRAW_ID = 15;
	const int FISH_TEXTDRAW_ID = 88;
	const bool AFK_HOTKEY_ENABLED = true;

	#define AFK_HOTKEY_1	0xA0	// LShift
	#define AFK_HOTKEY_2	0x31	// 1
	
	struct KeyCombo AFKkeyCombo;

	// Timing constants for mouse clicking and dialog hiding/showing
	const int t1 = 0, t2 = 100, t3 = 250;

	// ----- Private variables -----

	volatile bool bCheatEnabled = true;

	// Used for catching with mouse press
	volatile double fLastCatchTime;
	
	// Used for watching info textdraw text changes
	char szOldInfoTextdrawText[SAMP_MAX_TEXTDRAW_TEXT_LENGTH] = "";

	// Fish count textdraw
	bool bShowFishTextdraw = false;
	int dFishTextdrawId = -1;

	// Fish in inventory counter
	int dFishCount = 0;

	// ----- Private functions -----
	
	void throwAgain();
	void updateFishCount(int c);
}

void CheatZvejyba::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Adding ShowBigMessage callback");
	#endif

	// Add callback for showBigMessage function
    addShowBigMessageCallback([](const char *text, unsigned int time, unsigned int style) -> void {
		if(!bCheatEnabled) return;
		
		if(strlen(text) > 0){
			if(strstr(text, "Kimba") != 0){
				#ifdef LOG_VERBOSE
					Log("cheat_zvejyba.cpp: ShowBigMessage: Fish on hook!");
				#endif

				// Got fish!
				CheatZvejyba::pullFish();
			}
		}
	});

	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Registering /ztd /zzv commands");
	#endif

	// Register ztd command
	SAMP::addClientCommand("ztd", []{
		bShowFishTextdraw = !bShowFishTextdraw;

		if(bShowFishTextdraw){
			if(dFishTextdrawId == -1){
				dFishTextdrawId = createTextdrawWithId(88, "~w~Fish: ~g~", 575.0, 350.0, 0xFFFFFFFF);
			}
			
			setTextdrawVisible(FISH_TEXTDRAW_ID, 1);
			updateFishCount(dFishCount);
		}else{
			setTextdrawVisible(FISH_TEXTDRAW_ID, 0);
		}
	});

	SAMP::addClientCommand("zzv", []{
		bCheatEnabled = !bCheatEnabled;

		// Inform player about current cheat state
		if(bCheatEnabled) GTA_SA::addMessage((const char*)"Fishing bot~n~~g~On", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Fishing bot~n~~r~Off", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Loading settings");
	#endif

	bCheatEnabled = Settings::getBool("lsg", "zvejybaBot", true);
}

void CheatZvejyba::updateCheat()
{
	if(!bCheatEnabled) return;

	// Read info messages from the server
	struct stTextdraw *infoTextdraw = getPlayerTextdraw(INFO_TEXTDRAW_ID);
	if(infoTextdraw != nullptr){
		if(strcmp(szOldInfoTextdrawText, infoTextdraw->szText) != 0){
			// Text have changed, copy new text
			strcpy(szOldInfoTextdrawText, infoTextdraw->szText);
			
			if(strstr(szOldInfoTextdrawText, "per daug zuvies") != NULL){
				#ifdef LOG_VERBOSE
					Log("cheat_zvejyba.cpp: Full inventory!");
				#endif

				// Full inventory!
				// Inform player
				infoMsg(TXT_COLOR, "Full inventory!");

				if(dFishCount != 20){
					updateFishCount(20);
				}
			}else if(strstr(szOldInfoTextdrawText, "Paleidai") != NULL){
				// Rod pulled, if player was afk, throw it again
				if(SAMP::isAfkModeEnabled()){
					throwAgain();
				}
			}else if(strstr(szOldInfoTextdrawText, "Istraukei") != NULL){
				if(strstr(szOldInfoTextdrawText, "meskere") != NULL){
					// Rod pulled, if player was afk, throw it again
					if(SAMP::isAfkModeEnabled()){
						throwAgain();
					}
				}else{
					if(strstr(szOldInfoTextdrawText, "Bata") == NULL){
						// Caught something wealthy
						updateFishCount(dFishCount + 1);
					}
				}
			}else if(strstr(szOldInfoTextdrawText, "Pardavei") != NULL && strstr(szOldInfoTextdrawText, "zuvies") != NULL){
				updateFishCount(0);
			}
		}
	}

	if(AFK_HOTKEY_ENABLED){
		updateKeyCombo(AFK_HOTKEY_1, AFK_HOTKEY_2, AFKkeyCombo);
		if(isKeyComboPressed(AFKkeyCombo)){
			resetKeyStates(AFKkeyCombo);

			#ifdef LOG_VERBOSE
				Log("cheat_zvejyba.cpp: AFK key combo pressed");
			#endif

			SAMP::toggleAfkMode();
		}
	}
}

void CheatZvejyba::pullFish()
{
	// Inform player about pulling
	infoMsg(TXT_COLOR, "Pulling!!");
	
	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Closing dialogs (if there's any)");
	#endif

	// If chat input or scoreboard is shown, hide them
	if(isSampScoreboardActive()) closeSampScoreboard(false);
	if(isSampChatInputActive()) closeSampChatInput();

	bool bDialogActive = isSampDialogActive();
	if(bDialogActive){
		double start = GetTimeMillis();
		hideSampDialog();
		while(isSampDialogActive());
		double timeDiff = GetTimeMillis() - start;
		int sleepTime = t2 - (int)(timeDiff * 1000);
		if(sleepTime > 0) Sleep(sleepTime * 2);
	}

	// If cursor is shown, hide it, so gta receives input
	if(SAMP::isAfkModeEnabled()){
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: AFK enabled, hiding cursor");
		#endif

		double start = GetTimeMillis();
		SAMP::toggleSampCursor(0, true, false);
		double timeDiff = GetTimeMillis() - start;
		int sleepTime = t2 - (int)(timeDiff * 1000);
		if(sleepTime > 0) Sleep(sleepTime);
	}

	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Simulate LMB click");
	#endif

	mouseClick(LEFT_MOUSE_BUTTON, t3);

	// If cursor was shown, continue showing it
	if(SAMP::isAfkModeEnabled()){
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: AFK Enabled, showing cursor again");
		#endif

		SAMP::toggleSampCursor(2, false, false);
	}

	// Show dialog again if it was hidden
	if(bDialogActive){
		// Wait until LMB click is processed
		Sleep(t2);

		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: Unhiding shown dialog");
		#endif

		unhideSampDialog();
	}
}

void CheatZvejyba::throwAgain()
{
	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Rod pulled out, throw it again");
	#endif

	infoMsg(TXT_COLOR, "Oops, continue fishing");
	// Put some delay
	std::thread runAsync([=]{
		Sleep(670);
		sayCommand("/zvejoti");
	});
	runAsync.detach();
}

void CheatZvejyba::updateFishCount(int c)
{
	dFishCount = c;

	if(dFishCount == 20){
		PlaySoundA((LPCSTR)"beep-07.wav", NULL, SND_FILENAME | SND_ASYNC);
	}

	if(bShowFishTextdraw){
		char tdBuffer[64];
		sprintf(tdBuffer, "~w~Fish: ~g~%d", c);
		setTextdrawText(dFishTextdrawId, tdBuffer);
	}
}