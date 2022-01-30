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

	char g_szTargetName[32];
	volatile unsigned int iter = 0;

	// ----- Function declarations -----

	void registerCmds();
	void oneSecondTimer();

	// ----- Client commands -----

	void testCmd();
	void debugPlayer(const char *arg);
	void changeWeather(const char *arg);
	void lockWeather(const char *arg);
	void setDrawDistance(const char *arg);
	void trackPlayer(const char *arg);

	// ----- Helper functions -----

	int getPlayerIdx(const char *arg);
}

void HelperCmds::registerCmds()
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: Registering commands");
	#endif

	addClientCommand("hack1", testCmd);
	addClientCommand("i", (CMDPROC)debugPlayer);
	addClientCommand("lw", (CMDPROC)lockWeather);
	addClientCommand("dd", (CMDPROC)setDrawDistance);
	addClientCommand("kkur", (CMDPROC)trackPlayer);

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

	if(g_fDrawDistance != 0.0){
		GTA_SA::setDrawDistance(g_fDrawDistance);
	}

	if(strlen(g_szTargetName) > 0){
		// Differentiate name length, to prevent spamming
		char command[48];
		if(iter % 2 == 0){
			sprintf(command, "/kur %s", g_szTargetName);
		}else{
			sprintf(command, "/kur %.*s", (int)(strlen(g_szTargetName) - 1), g_szTargetName);
		}
		sayCommand(command);

		iter++;
	}
}

// ----- Custom client commands -----

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

void HelperCmds::lockWeather(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /lw command called");
	#endif

	int weatherId = -1;
	if(isNumber(arg)){
		// Parse weather id from string in command arg
		weatherId = strtol(arg, NULL, 10);
	}

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
		infoMsg("Draw distance disabled");
		g_fDrawDistance = 0.0;
		return;
	}
	
	if(!isNumber(arg)){
		infoMsg("Usage: /dd [1 - 3600]");
		return;
	}

	float drawDistance = (float)strtol(arg, NULL, 10);
	g_fDrawDistance = drawDistance;
}

void HelperCmds::trackPlayer(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /kkur command called");
	#endif

	if(strlen(arg) == 0){
		// Clear target name and tracking will be stopped
		infoMsg("Tracking finished!");
		memset(&g_szTargetName[0], 0, 32);
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
		strcpy(g_szTargetName, targetName);

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

void HelperCmds::testCmd()
{
	#ifdef LOG_VERBOSE
		Log("helper_cmds.hpp: /hack1 command called");
	#endif

	addToChatWindow(CHAT_TYPE_DEBUG, "Hello! :)", nullptr, 0xFF7b68ee, 0);
	
	infoMsg("/zsh - toggle speed hack");
	infoMsg("/zhh - toggle bunny hop");
	infoMsg("/zqt - toggle quick turn");
	infoMsg("/zrib [speed_limit] - toggle radar speed limiter");
	infoMsg("/zesp - toggle nametag esp");
	infoMsg("/znt - toggle nametags");
	infoMsg("/zab - toggle aimbot");
	infoMsg("/zae - toogle auto engine on");
	infoMsg("/zrc - toogle racecam");
	infoMsg("/zlr - toggle laikrasciai cheat");
	infoMsg("/zzv - toggle zvejyba bot");
	infoMsg("/zie - toggle ievent");
	infoMsg("/i [id/name] - info about player");
	infoMsg("/lw [id] - lock weather (without [id] - stop lock)");
	infoMsg("/dd [distance] - change draw distance (acceptable value: 0 - 3600)");
	infoMsg("/kkur [id/name] - track player (without [id] - stop tracking)");
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
