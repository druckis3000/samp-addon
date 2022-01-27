#include "cheat_matuoklis.h"
#include "game/samp.h"
#include "game/gtasa.h"
#include "utils/helper.h"
#include "hooking.h"
#include "game/vecmath.h"
#include "game/helpers/sampfuncs.h"
#include "settings.h"
#include <cmath>
#include <windows.h>

/** How it works? This cheat is working in indefinite loop and constantly
 * checking if player is on foot or in car, cheat is skipped if player is 
 * on foot. When in car, cheat iterates over nearby objects list and looks
 * if there's SPEED_CAMERA_MODEL(_ID) nearby, if there's one, script will
 * check if player speed is over specified limit, if so, it slows down car.
 */

#define SPEED_CAMERA_MODEL_ID	18880
#define NUM_SPEED_CAMS			7

namespace CheatMatuoklis {

	// ----- Private constants -----

	const float g_vecSpeedCams[NUM_SPEED_CAMS][3] = {
		{639.0950,	-1227.9506,	18.4291},
		{925.9537,	-963.6793,	38.6155},
		{1349.8900,	-1124.0645,	24.0457},
		{1320.8353,	-1411.6094,	13.7008},
		{1069.6991,	-1844.7166,	13.7344},
		{1559.7080,	-1740.4873,	13.7344},
		{1320.8353,	-1411.6094,	13.7008}
	};

	// ----- Private vars -----

	bool g_bMatuoklisEnabled = true;
	double g_fLastLimitTime = 0.0;
	float g_fSpeedLimit = 98.0;

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

void CheatMatuoklis::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_matuoklis.cpp: Register /zrib command");
	#endif

	// Register /zrib command
	addClientCommand("zrib", (CMDPROC)CheatMatuoklis::toggleOrChangeSpeedLimit);

	#ifdef LOG_VERBOSE
		Log("cheat_matuoklis.cpp: Loading settings");
	#endif

	g_bMatuoklisEnabled = Settings::getBool("lsg", "matuoklioRibotuvas", false);
}

void CheatMatuoklis::updateCheat()
{
	if(!g_bMatuoklisEnabled) return;

	// Don't do this cheat if player is on foot
	if(!isPlayerInVehicle(PLAYER_ID_SELF)) return;

	// Check timer
	if(GetTimeMillis() - g_fLastLimitTime < 0.1) return;

	// Find local player vehicle
	struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
	if(gtaVehicle == nullptr) return;

	// Iterate over speed cam coordinates array
	for(uint32_t i=0; i<NUM_SPEED_CAMS; i++){
		// Get speed cam and player positions
		float objPos[3], playerPos[3];
		vect3_copy(g_vecSpeedCams[i], objPos);
		vect3_copy(g_Samp->pPools->pPlayer->pLocalPlayer->inCarData.fPosition, playerPos);
		
		// Ignore height for calculating distance
		objPos[2] = 0.0;
		playerPos[2] = 0.0;
		
		// Calculate distance
		float dist = vect3_dist(objPos, playerPos);
		
		// If player is close to speed cam, but going away from it,
		// let him accelerate, and stop further execution
		if(dist < g_fSlowDownRadius){
			if(dist > (g_fSpeedCamRadius / 3.0 * 2.0)){
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
	}
}

void CheatMatuoklis::toggleOrChangeSpeedLimit(const char *arg)
{
	#ifdef LOG_VERBOSE
		Log("cheat_matuoklis.cpp: /zrib command called");
	#endif

	if(strlen(arg) > 0){
		float speedLimit = (float)atof(arg);
		if(speedLimit != 0.0){
			g_fSpeedLimit = speedLimit;
			GTA_SA::addMessage((const char*)"Greitis pakeistas!", 2000, 0, false);
		}
	}else{
		g_bMatuoklisEnabled = !g_bMatuoklisEnabled;
	
		if(g_bMatuoklisEnabled) GTA_SA::addMessage((const char*)"Ribotuvas~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Ribotuvas~n~~r~Isjungtas", 2000, 0, false);
	}
}

float CheatMatuoklis::getAngleToSpeedCam(struct vehicle_info *veh, const float vehiclePos[3], const float camPos[3])
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