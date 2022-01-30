#include "cheat_speedcam.h"
#include "game/samp.h"
#include "game/gtasa.h"
#include "utils/helper.h"
#include "utils/memfuncs.h"
#include "game/vecmath.h"
#include "game/helpers/sampfuncs.hpp"
#include "settings.h"

#include <cmath>
#include <windows.h>

#include <sstream>
#include <iterator>
#include <list>

/** How it works? This cheat is working in indefinite loop and constantly
 * checking if player is on foot or in car. When in car, cheat iterates over
 * nearby objects list and looks if there's SPEED_CAMERA_MODEL_ID nearby, if
 * there's one, it will check if player speed is over specified limit, if so, it slows down car.
 * 
 * EDIT: Instead of iterating nearby objects, now it loads list of speed cam
 * coordintes from settings file.
 */

#define SPEED_CAMERA_MODEL_ID	18880

namespace CheatAntiSpeedcam {

	// ----- Private vars -----
	
	typedef struct {
		float x, y, z;
	} Coord;
	std::list<Coord> listSpeedCams;

	uint8_t g_byteAntiSpeedCamSetting = 0;
	uint8_t g_byteAntiSpeedCamEnabled = 1;
	double g_fLastLimitTime = 0.0;
	float g_fSpeedLimit = 98.0;
	bool g_bPlayerInformed = false;

	// Constants for speed reduction algorithm

	const float g_fSpeedMultiplier = 165.0;
	const float g_fSlowDownRadius = 100.0;
	const float g_fSpeedCamRadius = 50.0;
	const float g_fSlowDownFactor = 325.0;

	// ----- Private functions -----

	void toggleOrChangeSpeedLimit(const char *arg);
	float getAngleToSpeedCam(struct vehicle_info *veh, const float playerPos[3], const float camPos[3]);
}

// ----- Public functions -----

void CheatAntiSpeedcam::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_speedcam.cpp: Register /zasc command");
	#endif

	// Register /zrib command
	SAMP::addClientCommand("zasc", (CMDPROC)CheatAntiSpeedcam::toggleOrChangeSpeedLimit);

	#ifdef LOG_VERBOSE
		Log("cheat_speedcam.cpp: Loading settings");
	#endif

	g_byteAntiSpeedCamEnabled = g_byteAntiSpeedCamSetting = Settings::getInt("settings", "antiSpeedCam", 0);
	g_fSpeedLimit = Settings::getFloat("settings", "speedCamLimit", 98.0f);
	if(g_fSpeedLimit < 1.0) g_fSpeedLimit = 1.0;
	else if(g_fSpeedLimit > 500.0) g_fSpeedLimit = 500.0;
	
	// Load speed cams list
	std::string speedCams = Settings::getString("settings", "speedCams", "");
	if(speedCams.length() > 0){
		char separator;
		std::istringstream sstream(speedCams);
		
		while(!sstream.eof()){
			float x, y, z;
			sstream >> x; sstream >> separator;
			sstream >> y; sstream >> separator;
			sstream >> z; sstream >> separator;

			listSpeedCams.push_back({x, y, z});
		}
	}
}

void CheatAntiSpeedcam::updateCheat()
{
	if(g_byteAntiSpeedCamEnabled == 0) return;

	// Don't do this cheat if player is on foot
	if(!isPlayerInVehicle(PLAYER_ID_SELF)) return;

	// Check timer
	if(GetTimeMillis() - g_fLastLimitTime < 0.1) return;

	// Find local player vehicle
	struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
	if(gtaVehicle == nullptr) return;

	bool foundCamNearby = false;

	// Iterate over speed cam coordinates array
	std::list<Coord>::iterator it;
	for(it = listSpeedCams.begin(); it != listSpeedCams.end(); ++it){
		// Get speed cam and player positions
		float objPos[3], playerPos[3];
		objPos[0] = it->x;
		objPos[1] = it->y;
		objPos[2] = it->z;
		vect3_copy(g_Samp->pPools->pPlayer->pLocalPlayer->inCarData.fPosition, playerPos);
		
		// Ignore height for calculating distance
		objPos[2] = 0.0;
		playerPos[2] = 0.0;
		
		// Calculate distance
		float dist = vect3_dist(objPos, playerPos);

		if(g_byteAntiSpeedCamEnabled == 1){
			// If player is close to speed cam, but going away from it,
			// let him accelerate, and stop further execution
			if(dist < g_fSlowDownRadius){
				if(dist > (g_fSpeedCamRadius / 1.5)){
					float angle = getAngleToSpeedCam(gtaVehicle, playerPos, objPos);
					if(angle > (M_PI / 2.0) + 0.1) return;
				}
			}

			if(dist < g_fSpeedCamRadius){
				// Player is in speed cam radius, at this moment speed
				// should not be greater than g_fSpeedLimit

				// Get vehicle movement direction
				float forwardVec[4] = {0.0f, 1.0f, 0.0f, 0.0f};
				float vehDirection[4];
				matrix_vect4_mult(gtaVehicle->base.matrix, forwardVec, vehDirection);

				// Get vehicle speed
				float vehSpeed = vect3_length(gtaVehicle->speed) * g_fSpeedMultiplier;

				if(vehSpeed >= g_fSpeedLimit){
					// Limit speed in moving direction
					float movementDirection[3];
					vect3_normalize(gtaVehicle->speed, movementDirection);

					// Calculate new speed
					const float newSpeed = g_fSpeedLimit / g_fSpeedMultiplier;
					vect3_mult(movementDirection, newSpeed, gtaVehicle->speed);
				}

				// Stop iterating over other cams, if player is in radius of one cam,
				// it's impossible to be in other cam radius
				break;
			}else if(dist < g_fSlowDownRadius){
				// Close enough to start slowing down

				// Get vehicle movement direction
				float forwardVec[4] = {0.0f, 1.0f, 0.0f, 0.0f};
				float vehDirection[4];
				matrix_vect4_mult(gtaVehicle->base.matrix, forwardVec, vehDirection);

				// Get vehicle speed
				float vehSpeed = vect3_length(gtaVehicle->speed) * g_fSpeedMultiplier;
				
				if(vehSpeed >= g_fSpeedLimit){
					// Slow down in moving direction
					float movementDirection[3];
					vect3_normalize(gtaVehicle->speed, movementDirection);

					// Calculate new speed
					const float newSpeed = (vehSpeed - (g_fSlowDownFactor * GTA_SA::getDelta())) / g_fSpeedMultiplier;
					vect3_mult(movementDirection, newSpeed, gtaVehicle->speed);
				}

				// Update timer
				g_fLastLimitTime = GetTimeMillis();

				// Stop iterating over other cams, if player is in radius of one cam,
				// it's impossible to be in other cam radius
				break;
			}
		}else if(g_byteAntiSpeedCamEnabled == 2){
			if(dist < g_fSlowDownRadius){
				foundCamNearby = true;
				break;
			}
		}
	}

	if(g_byteAntiSpeedCamEnabled == 2){
		if(foundCamNearby){
			if(!g_bPlayerInformed){
				// Inform player about speed cam nearby
				infoMsg(0xFFFF2222, "Approaching speed cam!");
				showGameText("~r~Approaching speed cam!", 1500, 3);
				g_bPlayerInformed = true;
			}
		}else{
			g_bPlayerInformed = false;
		}
	}
}

void CheatAntiSpeedcam::toggleOrChangeSpeedLimit(const char *arg)
{
	if(strlen(arg) > 0){
		float speedLimit = (float)atof(arg);
		if(speedLimit != 0.0){
			g_fSpeedLimit = speedLimit;
			GTA_SA::addMessage((const char*)"Speed limit changed!", 2000, 0, false);
		}
	}else{
		if(g_byteAntiSpeedCamEnabled > 0) g_byteAntiSpeedCamEnabled = 0;
		else g_byteAntiSpeedCamEnabled = g_byteAntiSpeedCamSetting;
	
		if(g_byteAntiSpeedCamEnabled > 0) GTA_SA::addMessage((const char*)"Anti speed cam: ~g~on", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Anti speed cam: ~r~off", 2000, 0, false);
	}
}

float CheatAntiSpeedcam::getAngleToSpeedCam(struct vehicle_info *veh, const float vehiclePos[3], const float camPos[3])
{
	// Calculate forward vector
	float fForward[4] = {0, 1, 0, 0}, fLocalForward[4];
	matrix_vect4_mult(veh->base.matrix, fForward, fLocalForward);
	
	// Calculate direction between local and remote player
	float fDirectionBetween[3];
	vect3_vect3_sub(camPos, vehiclePos, fDirectionBetween);
	vect3_normalize(fDirectionBetween, fDirectionBetween);

	// Calculate dot product of forward and direction vector
	float dot = vect3_dot_product(fLocalForward, fDirectionBetween);

	// Calculate angle
	float angle = acos(dot);

	return angle;
}