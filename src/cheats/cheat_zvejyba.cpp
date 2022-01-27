#include "cheat_zvejyba.h"
#include "utils/helper.h"
#include "game/samp.h"
#include "utils/event.h"
#include "game/helpers/sampfuncs.h"
#include "utils/keycombo.h"
#include "game/gtasa.h"
#include "settings.h"
#include <windows.h>
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
	const int PACKET_ZVEJYBA_NOTIFICATION = 910;
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
	volatile double lastCatchTime;
	
	// Used for watching info textdraw text changes
	char szOldInfoTextdrawText[SAMP_MAX_TEXTDRAW_TEXT_LENGTH] = "";

	// Used for toggling afk mode
	bool afkEnabled = false;

	// Fish count textdraw
	bool showFishTextdraw = false;
	int fishTextdrawId = -1;

	// Fish in inventory counter
	int fishCount = 0;

	// ----- Private functions -----

	void toggleAfk();
	
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
		Log("cheat_zvejyba.cpp: Registering /afk /ztd /zzv commands");
	#endif

    // Register afk command
    addClientCommand("afk", CheatZvejyba::toggleAfk);

	// Register ztd command
	addClientCommand("ztd", []{
		showFishTextdraw = !showFishTextdraw;

		if(showFishTextdraw){
			if(fishTextdrawId == -1){
				fishTextdrawId = createTextdrawWithId(88, "~w~Fish: ~g~", 575.0, 350.0, 0xFFFFFFFF);
			}
			
			setTextdrawVisible(FISH_TEXTDRAW_ID, 1);
			updateFishCount(fishCount);
		}else{
			setTextdrawVisible(FISH_TEXTDRAW_ID, 0);
		}
	});

	addClientCommand("zzv", []{
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

				if(fishCount != 20){
					updateFishCount(20);
				}
			}else if(strstr(szOldInfoTextdrawText, "Paleidai") != NULL){
				// Rod pulled, if player was afk, throw it again
				if(afkEnabled){
					throwAgain();
				}
			}else if(strstr(szOldInfoTextdrawText, "Istraukei") != NULL){
				if(strstr(szOldInfoTextdrawText, "meskere") != NULL){
					// Rod pulled, if player was afk, throw it again
					if(afkEnabled){
						throwAgain();
					}
				}else{
					if(strstr(szOldInfoTextdrawText, "Bata") == NULL){
						// Caught something wealthy
						updateFishCount(fishCount + 1);
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

			toggleAfk();
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
	if(afkEnabled){
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: AFK enabled, hiding cursor");
		#endif

		double start = GetTimeMillis();
		toggleSampCursor(0, true, false);
		double timeDiff = GetTimeMillis() - start;
		int sleepTime = t2 - (int)(timeDiff * 1000);
		if(sleepTime > 0) Sleep(sleepTime);
	}

	#ifdef LOG_VERBOSE
		Log("cheat_zvejyba.cpp: Simulate LMB click");
	#endif

	mouseClick(LEFT_MOUSE_BUTTON, t3);

	// If cursor was shown, continue showing it
	if(afkEnabled){
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: AFK Enabled, showing cursor again");
		#endif

		toggleSampCursor(2, false, false);
	}

	// Show dialog again if it was hidden
	if(bDialogActive){

	// if commented for debugging (mouse click was not detected when hiding and showing dialog)
	//	if(afkEnabled){
			// Wait after last toggleSampCursor
			Sleep(t2);
	//	}

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

void CheatZvejyba::toggleAfk()
{
	afkEnabled = !afkEnabled;

	if(afkEnabled){
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: Enabling AFK mode");
		#endif

		// Don't pause when in ESC menu
		memset((BYTE*)0x74542B, 0x90, 8);
		// Keep SAMP working when not in focus
		//memset((BYTE*)0x53EA88, 0x90, 6);

		// Disable menu after alt-tab
		//memset((BYTE*)0x53BC78, 0x00, 1);

		// ALLOW ALT+TABBING WITHOUT PAUSING
		memset((BYTE*)0x748A8D, 0x90, 6);
		//injector::MakeJMP(0x6194A0, AllowMouseMovement, true);
		//writeMemory(0x6194A0, 0x90, 6);
		
		// Wait until samp hides cursor after typing /afk command
		Sleep(200);
		
		// Show cursor, so that input actions done not in GTA window will not be processed by GTA
		toggleSampCursor(2, false, true);
		
		GTA_SA::addMessage((char*)"AFK system~n~~g~On", 2000, 0, false);
	}else{
		#ifdef LOG_VERBOSE
			Log("cheat_zvejyba.cpp: Disabling AFK mode");
		#endif

		// Reset overwritten memory
		memcpy((BYTE*)0x74542B, "\x50\x51\xFF\x15\x00\x83\x85\x00", 8);
		//memcpy((BYTE*)0x53EA88, "\x0F\x84\x7B\x01\x00\x00", 6);
		memcpy((BYTE*)0x748A8D, "\x0F\x84\x20\x03\x00\x00", 6);

		// Wait until samp hides cursor itself after typing /afk command
		Sleep(200);
		
		// Hide cursor
		toggleSampCursor(0, true, true);
		
		GTA_SA::addMessage((const char*)"AFK system~n~~r~Off", 2000, 0, false);
	}
}

void CheatZvejyba::updateFishCount(int c)
{
	fishCount = c;

	if(fishCount == 20){
		PlaySoundA((LPCSTR)"beep-07.wav", NULL, SND_FILENAME | SND_ASYNC);
	}

	if(showFishTextdraw){
		char tdBuffer[64];
		sprintf(tdBuffer, "~w~Fish: ~g~%d", c);
		setTextdrawText(fishTextdrawId, tdBuffer);
	}
}