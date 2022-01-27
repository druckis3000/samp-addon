#include "cheat_esp.h"
#include "game/samp.h"
#include "game/helpers/sampfuncs.h"
#include "game/gtasa.h"
#include "settings.h"
#include "utils/helper.h"
#include <string.h>
#include <cmath>
#include <thread>

// For debugging
//#define ENABLE_DEBUG_INFO

namespace CheatESP {

	// ----- Private types -----

	/*struct ESPLabel {
		bool bIsLabelSet = false;
		bool bIsPlayerInVehicle = false;
		bool bIsAFK = false;

		char szNickname[32];
		char szTextToDisplay[48];

		DWORD dwColor;
		float fHealth;
	};*/

	// ----- Private constants -----

	// First 3d labels ids are used by the server,
	// to avoid overwriting server labels, create
	// esp labels at 512 offset (2048 MAX)
//	const int TEXT_3D_OFFSET = 512;

	// Number of maximum esp labels possible at a moment
//	const int MAX_ESP_LABELS = 512;

	// Maximum distance of esp label, more than 300 is probably not neccessary
	const int MAX_ESP_DISTANCE = 300.0;

	// Maximum distance of aimbot, more than 200 is probably not neccessary
	const int MAX_AIM_DISTANCE = 200.0;
	const int MAX_AIM_ANGLE = 0.15;

//	const unsigned int ESP_COLOR_INFO = 0xFFEEDD00;
//	const unsigned int ESP_COLOR_ERROR = 0xFFEE1800;

	// ----- Private vars -----

	volatile bool bESPon = false;
	volatile bool bAimOn = false;

	float fOldNametagDist;

//	uint8_t bLoopCounter;

	// List of esp labels
//	struct ESPLabel espLabels[MAX_ESP_LABELS];

	// For aimbot
	struct actor_info *targetAim = nullptr;

	// ----- Private function declarations -----

/*	inline bool createESPLabel(uint16_t playerId);
	inline void deleteESPLabel(uint16_t playerId);
	
	inline void checkPlayerVehicleState(uint16_t playerId);
	inline void checkPlayerColor(uint16_t playerId);
	inline void checkPlayerAFKState(uint16_t playerId);
	inline void checkPlayerHealth(uint16_t playerId);

	inline void updateLabelCoords(uint16_t playerId);
	inline void updateDisplayText(uint16_t playerId);*/

	// ---- Aimbot -----
	bool isPlayerInFront(uint16_t playerId, const float localPos[3], const float remotePos[3]);
}

void CheatESP::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /zesp command");
	#endif

	// Register /zesp command
	/*addClientCommand("zesp", []{
		#ifdef LOG_VERBOSE
			Log("cheat_esp.cpp: /zesp command called");
		#endif

		// Toogle esp
		CheatESP::bESPon = !CheatESP::bESPon;

		// Inform player about current esp state
		if(CheatESP::bESPon) GTA_SA::addMessage((const char*)"ESP~n~~g~Ijungtas", 2000, 0, false);
		else{
			GTA_SA::addMessage((const char*)"ESP~n~~r~Isjungtas", 2000, 0, false);

			#ifdef LOG_VERBOSE
				Log("cheat_esp.cpp: ESP off, deleting ESP labels");
			#endif

			// Delete created labels
			for(int i=0; i<CheatESP::MAX_ESP_LABELS; i++)
				if(CheatESP::espLabels[i].bIsLabelSet)
					CheatESP::deleteESPLabel(i);
		}
	});*/
	addClientCommand("zesp", []{
		#ifdef LOG_VERBOSE
			Log("cheat_esp.cpp: /zesp command called");
		#endif

		// Toogle esp
		CheatESP::bESPon = !CheatESP::bESPon;

		// Change nametag settings and inform player about current state
		if(CheatESP::bESPon){
			bool d1, d2;
			getNametagSettings(&fOldNametagDist, &d1, &d2);
			setNametagSettings(MAX_ESP_DISTANCE, false, true);
			GTA_SA::addMessage((const char*)"ESP~n~~g~Ijungtas", 2000, 0, false);
		}else{
			setNametagSettings(fOldNametagDist, true, true);
			GTA_SA::addMessage((const char*)"ESP~n~~r~Isjungtas", 2000, 0, false);
		}
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /znt command");
	#endif

	// Register /znt command
	addClientCommand("znt", []{
		#ifdef LOG_VERBOSE
			Log("cheat_esp.cpp: /znt command called");
		#endif

		// Toogle nametag status
		float f1;
		bool b1, nametagStatus;
		getNametagSettings(&f1, &b1, &nametagStatus);
		nametagStatus = !nametagStatus;
		setNametagSettings(f1, b1, nametagStatus);

		// Inform player about current state
		if(nametagStatus) GTA_SA::addMessage((const char*)"Nametag~n~~g~Rodomi", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Nametag~n~~r~Nerodomi", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: registering /zab command");
	#endif

	// Register /zab command
	addClientCommand("zab", []{
		#ifdef LOG_VERBOSE
			Log("cheat_esp.cpp: /zab command called");
		#endif

		// Toggle aimbot
		CheatESP::bAimOn = !CheatESP::bAimOn;

		// Inform player about current aimbot state
		if(CheatESP::bAimOn) GTA_SA::addMessage((const char*)"Aimbot~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Aimbot~n~~r~Isjungtas", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_esp.cpp: Loading settings");
	#endif
	
	bool bEspEnabled = Settings::getBool("settings", "espEnabled", false);
	if(bEspEnabled){
		std::thread async([]{
			int tries = 0;
			while(g_Samp->iGameState != 5 && tries < 500){
				Sleep(10);
				tries++;
			}

			Sleep(1000);

			CheatESP::bESPon = true;
			bool d1, d2;
			getNametagSettings(&fOldNametagDist, &d1, &d2);
			setNametagSettings(MAX_ESP_DISTANCE, false, true);
		});
		async.detach();
	}

	bool bHideNametags = Settings::getBool("settings", "hideNametags", false);
	if(bHideNametags){
		std::thread async([]{
			int tries = 0;
			while(g_Samp->iGameState != 5 && tries < 500){
				Sleep(10);
				tries++;
			}

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

	// Iterate over remote players
	for(uint16_t i=0; i<SAMP_MAX_PLAYERS; i++){
		/*if(!isValidPlayerId(i)){
			if(CheatESP::espLabels[i].bIsLabelSet){
				CheatESP::deleteESPLabel(i);
			}
			continue;
		}

		if(!isPlayerStreamed(i)){
			if(CheatESP::espLabels[i].bIsLabelSet){
				CheatESP::deleteESPLabel(i);
			}
			continue;
		}

		// ----- ESP Part

		if(CheatESP::bESPon){
			if(!CheatESP::espLabels[i].bIsLabelSet){
				// This player does not have label, let's create one
				CheatESP::createESPLabel(i);
			}else{
				// Do checks only every second loop
				if(CheatESP::bLoopCounter % 2 == 0){
					CheatESP::checkPlayerVehicleState(i);
					CheatESP::checkPlayerColor(i);
					CheatESP::checkPlayerAFKState(i);
					CheatESP::checkPlayerHealth(i);

					CheatESP::updateLabelCoords(i);
				}
			}
		}*/

		// ----- Aimbot Part

		if(CheatESP::bAimOn){
			if(getPlayerPosition(i, fRemotePos)){
				float fDist = vect3_dist(fRemotePos, fLocalPos);
				// Calculate distance
				if(fDist < fClosestPlayerDistance){
					// Also check if remote player is in front of local player
					if(CheatESP::isPlayerInFront(i, fLocalPos, fRemotePos)){
						sClosestPlayerId = i;
						fClosestPlayerDistance = fDist;
					}
				}
			}
		}
	}

	// ----- Aimbot part

	if(CheatESP::bAimOn){
		// If remote player is found close to local player, and player is aiming,
		// automatically aim to the closest remote player

		if(GetAsyncKeyState(VK_RBUTTON) & 0x8000){
			bool uiOn = isSampChatInputActive() || isSampDialogActive() || isSampScoreboardActive();
			if(!uiOn){
				if(sClosestPlayerId != PLAYER_ID_SELF){
					uint8_t byteWeaponId = getPlayerWeapon(PLAYER_ID_SELF);
					if(((byteWeaponId >= 22 && byteWeaponId <= 36) || byteWeaponId == 38)){
						struct actor_info *remoteActor = getGTAPedFromSampId(sClosestPlayerId);
						CheatESP::targetAim = remoteActor;
					}
				}
			}
		}else{
			CheatESP::targetAim = nullptr;
		}

		if(CheatESP::targetAim != nullptr){
			getGTAPedFromSampId(PLAYER_ID_SELF)->ptr_autoAimTarget = CheatESP::targetAim;
		}
	}

	// Increase loop counter
	//CheatESP::bLoopCounter++;
}

/*inline bool CheatESP::createESPLabel(uint16_t playerId)
{
	#ifdef ENABLE_DEBUG_INFO
		infoMsgf(ESP_COLOR_INFO, "cheat_esp.cpp: Create text label for player id: %d", playerId);
		infoMsgf(ESP_COLOR_INFO, "cheat_esp.cpp: Label index: %d", (TEXT_3D_OFFSET + playerId));
	#endif

	// Get player color
	espLabels[playerId].dwColor = getPlayerColorARGB(playerId);

	// Get player afk state
	espLabels[playerId].bIsAFK = isPlayerAFK(playerId);

	// Get player in vehicle state
	espLabels[playerId].bIsPlayerInVehicle = isPlayerInVehicle(playerId);

	// Get player health
	espLabels[playerId].fHealth = getPlayerHealth(playerId);

	// Get player name
	if(!getPlayerName(playerId, espLabels[playerId].szNickname)){
		#ifdef ENABLE_DEBUG_INFO
			infoMsg(ESP_COLOR_ERROR, "ESP: Failed to get player name!");
		#endif
		return false;
	}

	// Update display text
	updateDisplayText(playerId);

	// Create text label
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].pText = &espLabels[playerId].szTextToDisplay[0];
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].color = espLabels[playerId].dwColor;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[0] = 1;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[1] = 1;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[2] = 1;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fMaxViewDistance = MAX_ESP_DISTANCE;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].byteShowBehindWalls = 0;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].sAttachedToPlayerID = 0xFFFF;
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].sAttachedToVehicleID = 0xFFFF;

	// Mark as listed
	g_Samp->pPools->pText3D->iIsListed[TEXT_3D_OFFSET + playerId] = 1;

	// Mark label as created
	espLabels[playerId].bIsLabelSet = true;
	
	return true;
}

inline void CheatESP::deleteESPLabel(uint16_t playerId)
{
	// Delete previously created label
	#ifdef ENABLE_DEBUG_INFO
		infoMsgf(ESP_COLOR_INFO, "cheat_esp.cpp: Remove text label: %d", (TEXT_3D_OFFSET + playerId));
	#endif
	memset(&g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId], '\0', sizeof(struct stTextLabel));
	g_Samp->pPools->pText3D->iIsListed[TEXT_3D_OFFSET + playerId] = 0;

	espLabels[playerId].bIsLabelSet = false;
	espLabels[playerId].bIsPlayerInVehicle = false;
	espLabels[playerId].bIsAFK = false;
	espLabels[playerId].fHealth = 0.0;
	espLabels[playerId].dwColor = 0;
}

inline void CheatESP::checkPlayerVehicleState(uint16_t playerId)
{
	// Check if in vehicle state changed, if so update label offset
	bool inVehicle = isPlayerInVehicle(playerId);
	espLabels[playerId].bIsPlayerInVehicle = inVehicle;
}

inline void CheatESP::checkPlayerColor(uint16_t playerId)
{
	// Check if player color changed
	DWORD color = getPlayerColorARGB(playerId);
	if(espLabels[playerId].dwColor != color){
		// Color changed, update
		espLabels[playerId].dwColor = color;
		g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].color = color;
	}
}

inline void CheatESP::checkPlayerAFKState(uint16_t playerId)
{
	// Check if player afk state changed
	bool afkState = isPlayerAFK(playerId);
	if(espLabels[playerId].bIsAFK != afkState){
		// Update afk state
		espLabels[playerId].bIsAFK = afkState;
		updateDisplayText(playerId);
	}
}

inline void CheatESP::checkPlayerHealth(uint16_t playerId)
{
	float fHealth = getPlayerHealth(playerId);

	if(espLabels[playerId].fHealth != fHealth){
		// Update health
		espLabels[playerId].fHealth = fHealth;
		updateDisplayText(playerId);
	}
}

inline void CheatESP::updateLabelCoords(uint16_t playerId)
{
	float fPos[3];
	if(!getPlayerPosition(playerId, fPos))
	{
		infoMsgf("ESP: Failed to get player pos");
		bESPon = false;
		return;
	}

	float fPosLoc[3];
	if(!getPlayerPosition(PLAYER_ID_SELF, fPosLoc)){
		infoMsgf("ESP: Failed to get local player pos");
		bESPon = false;
		return;
	}

	// Calculate distance from local player to remote player
	// and adjust label z coordinate according to distance
	float fDist = vect3_dist(fPos, fPosLoc) / MAX_ESP_DISTANCE;

	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[0] = fPos[0];
	g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[1] = fPos[1];
	if(espLabels[playerId].bIsPlayerInVehicle){
		g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[2] = fPos[2] + 0.5;
	}else{
		g_Samp->pPools->pText3D->textLabel[TEXT_3D_OFFSET + playerId].fPosition[2] = fPos[2] + 0.75 + (fDist / 2.0);
	}
}

inline void CheatESP::updateDisplayText(uint16_t playerId)
{
	if(espLabels[playerId].bIsAFK){
		sprintf(espLabels[playerId].szTextToDisplay, "%s [AFK]\nHP: %.1f", espLabels[playerId].szNickname, espLabels[playerId].fHealth);
	}else{
		sprintf(espLabels[playerId].szTextToDisplay, "%s\nHP: %.1f", espLabels[playerId].szNickname, espLabels[playerId].fHealth);
	}
}*/

// ----- Aimbot

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