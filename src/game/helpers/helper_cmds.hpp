#ifndef HELPER_CMDS_HPP
#define HELPER_CMDS_HPP

#include "game/gtasa.h"
#include "game/samp.h"
#include "sampfuncs.hpp"
#include "settings.h"
#include <windows.h>
#include <thread>

namespace HelperCmds {

	// ----- Private vars -----

	volatile int g_dWeatherFreeze = 2;
	volatile float g_fDrawDistance = 180;

	char szKkurTargetName[32];
	volatile unsigned int iter = 0;

	// ----- Function declarations -----

	void registerCmds();
	void oneSecondTimer();

	// ----- Client commands -----

	void testCmd();
	void debugPlayer(const char *arg);
	void changeWeather(const char *arg);
	void freezeWeather(const char *arg);
	void setDrawDistance(const char *arg);
	void trackPlayer(const char *arg);
	void changeNumberPlate(const char *arg);

	// ----- Helper functions -----

	int getPlayerIdx(const char *arg);
}

void HelperCmds::registerCmds()
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: Registering commands");
	#endif

	SAMP::addClientCommand("zcmds", testCmd);
	SAMP::addClientCommand("i", (CMDPROC)debugPlayer);
	SAMP::addClientCommand("fw", (CMDPROC)freezeWeather);
	SAMP::addClientCommand("dd", (CMDPROC)setDrawDistance);
	SAMP::addClientCommand("kkur", (CMDPROC)trackPlayer);
	SAMP::addClientCommand("afk", ([]{ SAMP::toggleAfkMode(); }));
	SAMP::addClientCommand("znp", (CMDPROC)changeNumberPlate);

	#ifdef LOG_VERBOSE
		Log("helper_cmds.cpp: Loading settings");
	#endif

	g_dWeatherFreeze = Settings::getInt("settings", "freezeWeather", -1);
	g_fDrawDistance = Settings::getFloat("settings", "drawDistance", 0.0f);
}

void HelperCmds::oneSecondTimer()
{
	if(g_dWeatherFreeze >= 0){
		GTA_SA::forceWeather(g_dWeatherFreeze);
	}

	if(g_fDrawDistance > 0.0){
		GTA_SA::setDrawDistance(g_fDrawDistance);
	}

	if(strlen(szKkurTargetName) > 0){
		// Differentiate name length, to prevent spamming
		char command[48];
		if(iter % 2 == 0){
			sprintf(command, "/kur %s", szKkurTargetName);
		}else{
			sprintf(command, "/kur %.*s", (int)(strlen(szKkurTargetName) - 1), szKkurTargetName);
		}
		sayCommand(command);

		iter++;
	}
}

// ----- Custom client commands -----

void HelperCmds::testCmd()
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /zcmds command called");
	#endif

	infoMsg(0xFF7b68ee, "lsgyvenimas.lt exclusively:");
	infoMsg(0xFF7b68ee, "/zie - toggle ievent");
	infoMsg(0xFF7b68ee, "/zae - toogle auto engine on");
	infoMsg(0xFF7b68ee, "/zlr - toggle newspaper helper");
	infoMsg(0xFF7b68ee, "/zzv - toggle fishing bot");
	infoMsg(0xFF7b68ee, "/ztd - toggle fish count textdraw");
	infoMsg(0xFF7b68ee, "/kkur [id/name] - track player (without [id] - stop tracking)");
	infoMsg(0xFF7b68ee, "-------------------------------------------------------------");
	infoMsg(0xFF7b68ee, "/zsh [0.0 - 0.2] - set speed hack multiplier value (toggle without [value])");
	infoMsg(0xFF7b68ee, "/zbh - toggle bunny hop");
	infoMsg(0xFF7b68ee, "/zqt - toggle quick turn");
	infoMsg(0xFF7b68ee, "/zasc [speed_limit] - change speed cam limit (without [value] - toggle anti speed cam)");
	infoMsg(0xFF7b68ee, "/zesp - toggle nametag esp");
	infoMsg(0xFF7b68ee, "/znt - toggle nametags");
	infoMsg(0xFF7b68ee, "/zab - toggle aimbot");
	infoMsg(0xFF7b68ee, "/zrc - toogle racecam");
	infoMsg(0xFF7b68ee, "/zafk - toggle afk mode");
	infoMsg(0xFF7b68ee, "/znp [number plate] - change vehicle number plate");
	infoMsg(0xFF7b68ee, "/i (id/name) - info about player");
	infoMsg(0xFF7b68ee, "/fw [0 - 20] - freeze weather (without [id] - stop freeze)");
	infoMsg(0xFF7b68ee, "/dd [1 - 3600] - change draw distance (without [value] - default draw distance)");
}

void HelperCmds::debugPlayer(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /i command called");
	#endif
	
	updateScoreboardInfo();

	if(strlen(arg) == 0) return;

	int playerId = getPlayerIdx(arg);
	if(playerId == -1){
		infoMsg("Invalid player ID or name!");
		return;
	}
	
	// Do something useful with playerId

	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: Retrieving remote player information");
	#endif

	char playerName[32] = {0x0};
	getPlayerName(playerId, playerName);
	//struct stRemotePlayerData *rpData = g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->pPlayerData;

	infoMsgf("ID: %u, Name: %s", playerId, playerName);
	infoMsgf("HP: %.1f, Armor: %.1f", getPlayerHealth(playerId), getPlayerArmor(playerId));
	infoMsgf("Score: %u", getPlayerScore(playerId));
}

void HelperCmds::freezeWeather(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /fw command called");
	#endif

	if(strlen(arg) == 0){
		infoMsg("Freeze weather disabled");
		g_dWeatherFreeze = -1;
		return;
	}

	if(!isNumber(arg)){
		infoMsg("Usage: /fw [0 - 20]");
		return;
	}

	int weatherId = strtol(arg, NULL, 10);
	if(weatherId == -1){
		infoMsg("Freeze weather disabled!");
		g_dWeatherFreeze = weatherId;
	}else{
		if(g_dWeatherFreeze == -1)
			infoMsg("Freeze weather enabled");

		g_dWeatherFreeze = weatherId;
	}
}

void HelperCmds::setDrawDistance(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /dd command called");
	#endif

	if(strlen(arg) == 0){
		infoMsg("Draw distance set to default");
		g_fDrawDistance = 0.0;
		return;
	}
	
	if(!isNumber(arg)){
		infoMsg("Usage: /dd [1 - 3600]");
		return;
	}

	float drawDistance = (float)strtol(arg, NULL, 10);
	g_fDrawDistance = drawDistance;

	if(drawDistance == 0.0){
		infoMsg("Draw distance set to default");
	}
}

void HelperCmds::trackPlayer(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /kkur command called");
	#endif

	if(strlen(arg) == 0){
		// Clear target name and tracking will be stopped
		infoMsg("Tracking finished!");
		memset(&szKkurTargetName[0], 0, 32);
		return;
	}

	int playerId = getPlayerIdx(arg);
	if(playerId == -1){
		infoMsg("Player not found!");
		return;
	}

	// Get remote player name
	char targetName[32];
	getPlayerName(playerId, targetName);
	
	if(strlen(targetName) > 0){
		// Copy target name to global string
		strcpy(szKkurTargetName, targetName);

		char msg[64];
		sprintf(msg, "Tracking %s", targetName);
		infoMsg(msg);
	}else{
		#ifdef LOG_VERBOSE
			Log("helper_cmds.hpp: Failed to get remote player name!");
		#endif

		infoMsg("Failed to get player name!");
	}
}

void HelperCmds::changeNumberPlate(const char *arg)
{
	if(strlen(arg) == 0){
		infoMsg("Usage: /znp [new number plate]");
		return;
	}

	if(!isPlayerInVehicle(PLAYER_ID_SELF)){
		infoMsg("You must be in vehicle in order to change number plate!");
		return;
	}

	if(strlen(arg) > 32){
		infoMsg("Number plate too long! Max length: 32");
		return;
	}

	// Change number plate
	setVehicleNumberPlate(getPlayerVehicleId(PLAYER_ID_SELF), arg);
	infoMsg("Number plate changed!");
}

// ----- Helper Functions -----

/** @return player id from command arg which can be id or (part of) player name */
int HelperCmds::getPlayerIdx(const char *arg)
{
	int playerId = -1;

	if(isNumber(arg)){
		// Parse id from id in command arg
		playerId = strtol(arg, NULL, 10);
		if(!isValidPlayerId(playerId)) playerId = -1;
	}else{
		// Parse id from name specified in command arg
		for(int i=0; i<SAMP_MAX_PLAYERS; i++){
			if(!isValidPlayerId(i)) continue;

			char playerName[32];
			getPlayerName(i, playerName);
			std::string strPlayerName(playerName);
			if(findStringIC(strPlayerName, std::string(arg))){
				playerId = i;
				break;
			}
		}
	}

	return playerId;
}

#endif
