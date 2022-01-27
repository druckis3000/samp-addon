#include "cheat_laikrasciai.h"
#include "game/samp.h"
#include "utils/event.h"
#include "utils/helper.h"
#include "game/gtasa.h"
#include "game/vecmath.h"
#include <iostream>
#include <string.h>
#include <map>
#include <cstdlib>
#include "utils/timercpp.h"
#include "game/helpers/sampfuncs.h"
#include "settings.h"

/** How it works? When you pickup mails pickup, server sends you message
 * containing house ids to where you need to deliver mails. When cheat
 * captures such message, it parses ids and after it will finish parsing ids,
 * cheat will send command for (every house id) "/kur [houseID]" to the server
 * (with some delay between commands, to prevent spamming). Next, when setCheckpoint
 * is called, cheat assumes that it's a checkpoint for last sent "/kur [houseID]"
 * houseID location, cheats saves that location. When cheat gets all house locations
 * it will calculate distances between each house and local player location.
 * After this step is done it will send one more "/kur [houseID]" to the server,
 * but this time houseID is ID of a nearest house. Next, when you reach targeted house
 * (or actually any other), server will send message containing IDs that left to deliver
 * mails to, cheatwill synchronize those IDs list with it's own stored IDs list and
 * calculates distances again, and sends command "/kur [houseID]" with nearest house ID again,
 * this happens repeatedly until you deliver all mails or you drop the mails, when
 * you do so, cheat will clear IDs list, reset vars and just goes back to start.
 */

// ----- Type definitions -----

namespace CheatLaikrasciai {

	// ----- Private typedefs -----

	struct stHouse {
		int id;
		float xyz[3];
		float dist;
		bool bActive;
	};

	// ----- Private constants -----

	const unsigned int INFO_MSG_COLOR = 0xFF7B68EE;

	// ----- Private vars -----

	volatile bool bCheatEnabled = false;

	std::map<int, stHouse> targetHouses;
	// House id of which cheat is waiting for checkpoint after sending /kur %d command,
	// used for getting house coordinates
	int waitingForCheckpoint = 0;
	// House id to which player should deliver newspaper
	int targetHouseId = 0;

	// For watching textdraw changes
	char oldInfoTextdrawText[401] = "";

	// Needed for setTimeout();
	Timer timer;

	// ----- Private function declarations -----

	void onCheckpoint(float x, float y, float z, float size);
	void onMessage(const char *msg, DWORD color);
	void showNextCheckpoint();
	void resetVars();
	int randDelayTime();
}

void CheatLaikrasciai::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_laikrasciai.cpp: Adding callbacks");
	#endif

	addSetCheckpointCallback(CheatLaikrasciai::onCheckpoint);
	addAddMessageCallback(CheatLaikrasciai::onMessage);

	#ifdef LOG_VERBOSE
		Log("cheat_laikrasciai.cpp: Registering /zlr command");
	#endif

	addClientCommand("zlr", []{
		bCheatEnabled = !bCheatEnabled;

		// Inform player about current cheat state
		if(bCheatEnabled) GTA_SA::addMessage((const char*)"Laikrasciai~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Laikrasciai~n~~r~Isjungtas", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("cheat_laikrasciai.cpp: Loading settings");
	#endif

	bCheatEnabled = Settings::getBool("lsg", "laikrasciai", false);
}

void CheatLaikrasciai::updateCheat()
{
	if(!bCheatEnabled) return;

	struct stTextdraw *infoTextdraw = getPlayerTextdraw(15);
	if(infoTextdraw != nullptr){
		if(strcmp(oldInfoTextdrawText, infoTextdraw->szText) != 0){
			strcpy(oldInfoTextdrawText, infoTextdraw->szText);
			
			if(strstr(oldInfoTextdrawText, "Ismetei") != 0 && strstr(oldInfoTextdrawText, "laiskus") != 0){
				#ifdef LOG_VERBOSE
					Log("cheat_laikrasciai.cpp: Newspapers dropped, cheat done");
				#endif
				
				// Inform player
				infoMsg(INFO_MSG_COLOR, "cheat_laikrasciai.cpp: done!");
				
				// Reset vars
				resetVars();
			}
		}
	}
}

void CheatLaikrasciai::onCheckpoint(float x, float y, float z, float size)
{	
	if(!bCheatEnabled) return;

	if(waitingForCheckpoint > 0){
		// Set house xyz location in the list
		targetHouses[waitingForCheckpoint].xyz[0] = x;
		targetHouses[waitingForCheckpoint].xyz[1] = y;
		targetHouses[waitingForCheckpoint].xyz[2] = z;
		
		// House ids are incremented by one, so set waiting checkpoint id to +1
		waitingForCheckpoint++;

		infoMsgf("got house coordinates");
		
		// Check if all house locations were found
		if(targetHouses.find(waitingForCheckpoint) != targetHouses.end()){
			// Got more houses to find out locations
			
			// Sleep between commands random time, so server won't count it as spam
			timer.setTimeout([]{
				// Find another house location
				char buffer[64];
				sprintf(buffer, "/kur %d", waitingForCheckpoint);
				sayCommand(buffer);
				
				#ifdef LOG_VERBOSE
					Logf("sent command: /kur %d", waitingForCheckpoint);
				#else
					infoMsgf("sent command: /kur %d", waitingForCheckpoint);
				#endif
			}, randDelayTime());
		}else{
			// Found all house locations
			waitingForCheckpoint = 0;
			#ifdef LOG_VERBOSE
				Log("cheat_laikrasciai.cpp: Found all houses coordinates");
			#else
				infoMsg("cheat_laikrasciai.cpp: Found all houses coordinates");
			#endif
			
			timer.setTimeout([]{
				// Show next location checkpoint
				showNextCheckpoint();
			}, randDelayTime());
		}
	}
}

void CheatLaikrasciai::onMessage(const char *msg, DWORD color)
{
	if(!bCheatEnabled) return;

	/* If server is telling player to start delivering newspapers, obtain house ids,
	coordinates and store them in a map. Then call showNextCheckpoint() function.
	
	If server is telling that newspaper was delivered, just call showNextCheckpoint() function.
	
	If server is telling that player finished delivering newspapers, then just reset
	global vars and clear houses map. */

	if(strstr(msg, "Turi isvezioti laiskus i siuos namus") != 0 || strstr(msg, "yra isvezioti laiskus i siuos namus") != 0){
		// Copy const char* to std::string, for easier string operations
		std::string zinute = msg;
		size_t indexOfNamus = zinute.find("namus:");

		#ifdef LOG_VERBOSE
			Log("cheat_laikrasciai.cpp: Starting newspaperboy job");
		#endif
		
		if(indexOfNamus != std::string::npos){
			#ifdef LOG_VERBOSE
				Log("cheat_laikrasciai.cpp: Getting house IDs");
			#else
				infoMsg("cheat_laikrasciai.cpp: Getting house IDs");
			#endif

			// Clear target houses map
			targetHouses.clear();
		
			// Find all house ids string part
			std::string houseIds = zinute.substr(indexOfNamus+7);
			
			// Used for iterating over house ids string and extracting ids
			size_t lastCommaIndex = 0;
			size_t commaIndex = 0;
			
			// Extract house ids list from server message
			while((commaIndex = houseIds.find(",", lastCommaIndex)) != std::string::npos){
				int hId = std::stoi(houseIds.substr(lastCommaIndex, commaIndex));
				targetHouses[hId] = {hId, 0, 0, 0, true};
				
				// +1 so loop don't stuck at same id, same position
				lastCommaIndex = commaIndex+1;
			}
			
			// Get last house id (last house is not iterated above)
			int lastHouseId = std::stoi(houseIds.substr(lastCommaIndex));
			targetHouses[lastHouseId] = {lastHouseId, 0, 1, 2};
		}
		
		// Get first house coordinates (other coordinates will be obtained in onSetCheckpoint callback)
		int id = targetHouses.begin()->first;
		
		// Set flag indicating that cheat is waiting for setCheckpoint function call
		waitingForCheckpoint = id;
		
		// Send samp command: "/kur [houseId]"
		char buffer[32];
		sprintf(buffer, "/kur %d", id);
		sayCommand(buffer);
		
		#ifdef LOG_VERBOSE
			Logf("cheat_laikrasciai.cpp: Sent /kur %d command", id);
		#else
			infoMsgf("cheat_laikrasciai.cpp: Sent /kur %d command", id);
		#endif
	}else if(strstr(msg, "Tau liko isvezioti laiskus i siuos namus") != 0){
		// Reached house checkpoint and there's more houses to visit
		#ifdef LOG_VERBOSE
			Log("cheat_laikrasciai.cpp: Reached house checkpoint!");
		#endif

		{ // Sync house ids with server side
			std::string zinute = msg;
			size_t indexOfNamus = zinute.find("namus:");
			
			if(indexOfNamus != std::string::npos){
				#ifdef LOG_VERBOSE
					Log("cheat_laikrasciai.cpp: Syncing house IDs");
				#endif

				// Find all house ids string part
				std::string houseIds = zinute.substr(indexOfNamus+7);

				// Set all houses bActive = false
				for(auto it = targetHouses.begin(); it != targetHouses.end(); ++it)
					it->second.bActive = false;

				// Used for iterating over house ids string and marking houses as active
				size_t lastCommaIndex = 0;
				size_t commaIndex = 0;

				// Iterate over house ids in message and mark those ids as active
				while((commaIndex = houseIds.find(",", lastCommaIndex)) != std::string::npos){
					int hId = std::stoi(houseIds.substr(lastCommaIndex, commaIndex));
					targetHouses[hId].bActive = true;
					
					// +1 so that loop don't stuck at same id, same position
					lastCommaIndex = commaIndex+1;
				}
				
				// Mark last house as active (last house is not iterated above.. :/)
				int lastHouseId = std::stoi(houseIds.substr(lastCommaIndex));
				targetHouses[lastHouseId].bActive = true;

				// Remove all inactive houses from targetHouses list
				for(auto it = targetHouses.cbegin(); it != targetHouses.cend();){
					if(!it->second.bActive){
						targetHouses.erase(it++);
					}else{
						++it;
					}
				}

				#ifdef LOG_VERBOSE
					Log("cheat_laikrasciai.cpp: Sync done!");
				#endif
			}
		} // End of sync

		// Server sent "Tau liko isvezioti.." message, thus, at this moment
		// map shouldn't be empty, but better be safe to prevent crashes
		if(targetHouses.empty()){
			#ifdef LOG_VERBOSE
				Log("cheat_laikrasciai.cpp: Oops, something wrong happend");
			#endif

			// Something bad happend, reset cheat lol
			infoMsg(INFO_MSG_COLOR, "cheat_laikrasciai.cpp: done!");

			// Reset vars
			resetVars();

			return;
		}
		
		// Show next house checkpoint
		showNextCheckpoint();
	}else if(strstr(msg, "sveziojai visus laiskus") != 0){
		#ifdef LOG_VERBOSE
			Log("cheat_laikrasciai.cpp: Done!");
		#endif

		// All mails delivered!
		infoMsg(INFO_MSG_COLOR, "cheat_laikrasciai.cpp: done!");
		
		// Reset vars
		resetVars();
	}
}

void CheatLaikrasciai::showNextCheckpoint()
{
	/* This function iterates over targetHouses list and calculate distance
	from player to each house. This way it'll find the nearest house and will
	send "/kur [nearestHouseId]" command to the server */

	#ifdef LOG_VERBOSE
		Log("cheat_laikrasciai.cpp: Finding nearest house");
	#else
		infoMsg("cheat_laikrasciai.cpp: Finding nearest house");
	#endif
	
	// Get local player position
	float playerPos[3];
	if(!getPlayerPosition(PLAYER_ID_SELF, playerPos)){
		#ifdef LOG_VERBOSE
			Log("cheat_laikrasciai.cpp: Failed to get local player position, canceling...");
		#endif
		infoMsg(INFO_MSG_COLOR, "cheat_laikrasciai.cpp: done! [error]");
		resetVars();
		return;
	}
	
	// Used for finding nearest house
	float bestDistance = vect3_dist(playerPos, targetHouses.begin()->second.xyz);
	int bestDistanceId = targetHouses.begin()->first;
	
	// Iterate over houses list (skip first)
	for(std::map<int, stHouse>::iterator it = ++targetHouses.begin(); it != targetHouses.end(); ++it){
		// Get current iterating house
		struct stHouse house = it->second;
		
		// Find out distance from player to the house
		float distance = vect3_dist(playerPos, house.xyz);
		
		// Check if it's closer than previously checked house
		if(distance < bestDistance){
			// Yep, it is, save it
			bestDistance = distance;
			bestDistanceId = it->first;
		}
	}
	
	// Send /kur [bestDistanceId] command to the server!
	#ifdef LOG_VERBOSE
		Logf("cheat_laikrasciai.cpp: Nearest house id: %d, distance: %.2f", bestDistanceId, bestDistance);
		Logf("cheat_laikrasciai.cpp: Sending /kur %d command", bestDistanceId);
	#endif
	char buffer[32];
	sprintf(buffer, "/kur %d", bestDistanceId);
	sayCommand(buffer);
	
	// Set targetHouseId
	targetHouseId = bestDistanceId;
	
	// Give some info to the player, so he knows what's happening
	infoMsgf(INFO_MSG_COLOR, "Vaziuojam i nama: [id: %d]", targetHouseId);
}

int CheatLaikrasciai::randDelayTime()
{
	/* This function generates random time in millisecond to wait between sending
	command to the server, to that server will not find out that bot is typing */

	srand(time(NULL));

	const int minTime = 600;
	const int addRandomTimeUpTo = 150;
	const int onTypoAddMinTime = 150;
	const int onTypoAddTimeUpTo = 250;
	const int typoProbability = 20; // percent

	bool typo = ((rand() % 101) < typoProbability);
	int addTypoTime = 0;
	
	if(typo){
		addTypoTime = onTypoAddMinTime + (rand() % (onTypoAddTimeUpTo - onTypoAddMinTime + 1));
	}
	
	int addRandomTime = rand() % (addRandomTimeUpTo + 1);
	
	int time = minTime + addRandomTime + addTypoTime;
	return time;
}

void CheatLaikrasciai::resetVars()
{
	waitingForCheckpoint = 0;
	targetHouseId = 0;
	targetHouses.clear();
}
