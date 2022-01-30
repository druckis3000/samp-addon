#include "cheat_esp.h"
#include "game/samp.h"
#include "game/helpers/sampfuncs.hpp"
#include "game/gtasa.h"
#include "settings.h"
#include "utils/helper.h"
#include <string.h>
#include <cmath>
#include <thread>

namespace CheatESP {

	// ----- Private constants -----

	// Maximum distance of esp label, more than 300 is probably not neccessary
	const int MAX_ESP_DISTANCE = 300.0;

	// Maximum distance of aimbot, more than 200 is probably not neccessary
	const int MAX_AIM_DISTANCE = 200.0;
	const int MAX_AIM_ANGLE = 0.15;

	// ----- Private vars -----

	volatile bool bESPon = false;
	volatile bool bAimOn = false;
	float fEspDistance = 0.0;
	float fOldNametagDist;

	struct actor_info *targetAim = nullptr;

	// ---- Aimbot -----

	bool isPlayerInFront(uint16_t playerId, const float localPos[3], const float remotePos[3]);
}

void CheatESP::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /zesp command");
	#endif

	SAMP::addClientCommand("zesp", []{
		// Toogle esp
		CheatESP::bESPon = !CheatESP::bESPon;

		// Change nametag settings and inform player about current state
		if(CheatESP::bESPon){
			bool d1, d2;
			getNametagSettings(&fOldNametagDist, &d1, &d2);
			setNametagSettings(MAX_ESP_DISTANCE, false, true);
			GTA_SA::addMessage((const char*)"ESP: ~g~on", 2000, 0, false);
		}else{
			setNametagSettings(fOldNametagDist, true, true);
			GTA_SA::addMessage((const char*)"ESP: ~r~off", 2000, 0, false);
		}
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /znt command");
	#endif

	// Register /znt command
	SAMP::addClientCommand("znt", []{
		// Toogle nametag status
		float f1;
		bool b1, nametagStatus;
		getNametagSettings(&f1, &b1, &nametagStatus);
		nametagStatus = !nametagStatus;
		setNametagSettings(f1, b1, nametagStatus);

		// Inform player about current state
		if(nametagStatus) GTA_SA::addMessage((const char*)"Nametag: ~g~visible", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Nametag: ~r~hidden", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /zab command");
	#endif

	// Register /zab command
	SAMP::addClientCommand("zab", []{
		// Toggle aimbot
		CheatESP::bAimOn = !CheatESP::bAimOn;

		// Inform player about current aimbot state
		if(CheatESP::bAimOn) GTA_SA::addMessage((const char*)"Aimbot: ~g~on", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Aimbot: ~r~off", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: Loading settings");
	#endif
	
	// Load settings
	fEspDistance = Settings::getFloat("settings", "espDistance", MAX_ESP_DISTANCE);
	if(fEspDistance > MAX_ESP_DISTANCE) fEspDistance = MAX_ESP_DISTANCE;
	if(fEspDistance < 1.0) fEspDistance = 1.0;

	bool bEspEnabled = Settings::getBool("settings", "espEnabled", false);
	bool bHideNametags = Settings::getBool("settings", "hideNametags", false);

	// Enable ESP if espEnabled=true && hideNametags=false
	if(bEspEnabled && !bHideNametags){
		std::thread async([]{
			int tries = 0;
			while(g_Samp->iGameState != GAME_STATE_PLAYING && tries < 500){
				Sleep(10);
				tries++;
			}

			Sleep(1000);

			CheatESP::bESPon = true;
			bool d1, d2;
			getNametagSettings(&fOldNametagDist, &d1, &d2);
			setNametagSettings(fEspDistance, false, true);
		});
		async.detach();
	}else if(bHideNametags){
		// Hide nametags if hideNametags=true
		std::thread async([]{
			int tries = 0;
			while(g_Samp->iGameState != GAME_STATE_PLAYING && tries < 500){
				Sleep(10);
				tries++;
			}

			Sleep(1000);

			float f1;
			bool b1, nametagStatus;
			getNametagSettings(&f1, &b1, &nametagStatus);
			setNametagSettings(f1, b1, false);
		});
		async.detach();
	}
}

void CheatESP::updateCheat()
{
	// This is needed for aimbot
	int16_t sClosestPlayerId = PLAYER_ID_SELF;
	float fClosestPlayerDistance = CheatESP::MAX_AIM_DISTANCE;
	float fRemotePos[3], fLocalPos[3];

	// Get local player position
	getPlayerPosition(PLAYER_ID_SELF, fLocalPos);

	// Fnd closest player in front of local player
	if(CheatESP::bAimOn){
		for(uint16_t i=0; i<SAMP_MAX_PLAYERS; i++){
			if(getPlayerPosition(i, fRemotePos)){
				float fDist = vect3_dist(fRemotePos, fLocalPos);
				// Calculate distance
				if(fDist < fClosestPlayerDistance){
					// Check if remote player is in front of local player
					if(CheatESP::isPlayerInFront(i, fLocalPos, fRemotePos)){
						sClosestPlayerId = i;
						fClosestPlayerDistance = fDist;
					}
				}
			}
		}

		// If there's remote player close enough and in front of local player
		// and local player is aiming, automatically aim to the closest remote player

		CheatESP::targetAim = nullptr;
		if(GetAsyncKeyState(VK_RBUTTON) & 0x8000){
			if(sClosestPlayerId != PLAYER_ID_SELF){
				bool uiOn = isSampChatInputActive() || isSampDialogActive() || isSampScoreboardActive();
				if(!uiOn){
					uint8_t byteWeaponId = getPlayerWeapon(PLAYER_ID_SELF);
					if(((byteWeaponId >= 22 && byteWeaponId <= 36) || byteWeaponId == 38)){
						struct actor_info *remoteActor = getGTAPedFromSampId(sClosestPlayerId);
						CheatESP::targetAim = remoteActor;
					}
				}
			}
		}

		// Set auto aim target
		getGTAPedFromSampId(PLAYER_ID_SELF)->ptr_autoAimTarget = CheatESP::targetAim;
	}
}

bool CheatESP::isPlayerInFront(uint16_t playerId, const float localPos[3], const float remotePos[3])
{
	// Calculate forward vector
	float fForward[4] = {0, 1, 0, 0}, fLocalForward[4];
	matrix_vect4_mult(getGTAPedFromSampId(PLAYER_ID_SELF)->base.matrix, fForward, fLocalForward);
	
	// Calculate direction between local and remote player
	float fDirectionBetween[3];
	vect3_vect3_sub(remotePos, localPos, fDirectionBetween);
	vect3_normalize(fDirectionBetween, fDirectionBetween);

	// Calculate dot product of forward and direction vector
	float dot = vect3_dot_product(fLocalForward, fDirectionBetween);

	// Calculate angle
	float angle = acos(dot);
	
	// Only use aimbot if remote player is in front of local player
	if(angle <= MAX_AIM_ANGLE)
		return true;

	return false;
}