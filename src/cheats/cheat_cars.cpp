#include "cheat_cars.h"
#include "game/samp.h"
#include "game/helpers/sampfuncs.h"
#include "game/gtasa.h"
#include "game/vecmath.h"
#include "utils/helper.h"
#include <windows.h>
#include <cstdlib>

namespace CheatSpeed {

	// ----- Private variables -----

	volatile bool speedHackEnabled = false;
	bool leftAltPressed = false;
	double speed = 0.075;

	volatile bool hopHackEnabled = false;
	bool hPressed = false;
	double hopHeight = 0.2;

	volatile bool quick180TurnEnabled = true;
	bool backspacePressed = false;

	volatile bool lmbPressed = false;
	volatile bool ctrlPressed = false;

	// ----- Private functions -----

	void infiniteNos();
	void toggleSpeedHack(const char *arg);
	void toggleHopHack(const char *arg);
}

void CheatSpeed::setupCheat()
{
	#ifdef LOG_VERBOSE
		Log("cheat_speed.hpp: Registering commands");
	#endif
	
	addClientCommand("zsh", (CMDPROC)toggleSpeedHack);
	addClientCommand("zhh", (CMDPROC)toggleHopHack);
	addClientCommand("zqt", []{
		quick180TurnEnabled = !quick180TurnEnabled;

		if(quick180TurnEnabled) GTA_SA::addMessage((const char*)"Quick 180 Turn~n~~g~Ijungtas", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Quick 180 Turn~n~~r~Isjungtas", 2000, 0, false);
	});
}

void CheatSpeed::updateCheat()
{
	// Don't do this cheat if player is on foot
	if(!isPlayerInVehicle(PLAYER_ID_SELF)) return;

	// Don't do this cheat if chat or scoreboard or dialog is active
	if(isSampChatInputActive() || isSampScoreboardActive() || isSampDialogActive()) return;

	// Check if left alt key was pressed
	if((GetAsyncKeyState(VK_LMENU) & 0x8000) && speedHackEnabled){
		if(leftAltPressed) return;
		leftAltPressed = true;

		// Find local player vehicle
		struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
		if(gtaVehicle == nullptr) return;

		// Get vehicle movement direction
		float forwardVec[4] = {0.0f, 1.0f, 0.0f, 0.0f};
		float vehDirection[4];
		matrix_vect4_mult(gtaVehicle->base.matrix, forwardVec, vehDirection);

		// If car is moving, multiply car's direction by the movement speed and add a little bit more :)
		if (!vect3_near_zero(vehDirection) && !vect3_near_zero(gtaVehicle->speed)){
			float vehSpeed = vect3_length(gtaVehicle->speed) + CheatSpeed::speed;
			vect3_mult(vehDirection, vehSpeed, gtaVehicle->speed);
		}
	}else{
		leftAltPressed = false;
	}

	// Check if h key was pressed
	if((GetAsyncKeyState(0x48) & 0x8000) && hopHackEnabled){
		if(hPressed) return;
		hPressed = true;

		// Find local player vehicle
		struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
		if(gtaVehicle == nullptr) return;

		// Increase z-axis speed
		gtaVehicle->speed[2] += hopHeight;
	}else{
		hPressed = false;
	}

	// Check if backspace key was pressed
	if((GetAsyncKeyState(VK_BACK) & 0x8000) && quick180TurnEnabled){
		if(backspacePressed) return;
		backspacePressed = true;

		// Find local player vehicle
		struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
		if(gtaVehicle == nullptr) return;

		// Invert x and y axis
		vect3_invert(&gtaVehicle->base.matrix[4 * 0], &gtaVehicle->base.matrix[4 * 0]);
		vect3_invert(&gtaVehicle->base.matrix[4 * 1], &gtaVehicle->base.matrix[4 * 1]);

		// Invert vehicle speed
		vect3_invert(gtaVehicle->speed, gtaVehicle->speed);

		showGameText("~y~Mirrored!", 500, 3);
	}else{
		backspacePressed = false;
	}

	// Infinite nos (lmb)
	if(GetAsyncKeyState(VK_LBUTTON) & 0x8000){
		if(lmbPressed) return;
		lmbPressed = true;
		infiniteNos();
	}else{
		lmbPressed = false;
	}

	// Infinite nos (ctrl)
	if(GetAsyncKeyState(VK_LCONTROL) & 0x8000){
		if(ctrlPressed) return;
		ctrlPressed = true;
		infiniteNos();
	}else{
		ctrlPressed = false;
	}
}

// ----- Private functions -----

void CheatSpeed::infiniteNos()
{
	// Find local player vehicle
	struct vehicle_info *gtaVehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
	if(gtaVehicle == nullptr) return;

	if(gtaVehicle->vehicle_type != 0) return;

	// Check if vehicle has nos installed
	bool hasNos = false;
	for(int i=0; i<15; i++){
		if(gtaVehicle->UpgradeModelID[i] == 1008 || gtaVehicle->UpgradeModelID[i] == 1009 || gtaVehicle->UpgradeModelID[i] == 1010){
			hasNos = true;
			break;
		}
	}
	if(!hasNos) return;

	// Set number of nos'es left to 10
	gtaVehicle->m_nNitroBoosts = 10;
	
	// Refill nos
	gtaVehicle->fNitroCount = 1.0;
}

void CheatSpeed::toggleSpeedHack(const char *arg)
{
	if(strlen(arg) > 0){
		double speed = atof(arg);

		if(speed == 0.0) CheatSpeed::speed = 0.05;
		else CheatSpeed::speed = speed;

		GTA_SA::addMessage((const char*)"Greitis pakeistas!", 1500, 0, false);
	}else{
		speedHackEnabled = !speedHackEnabled;

		if(speedHackEnabled){
			GTA_SA::addMessage((const char*)"Speed hack~n~~g~Ijungtas", 2000, 0, false);
		}else{
			GTA_SA::addMessage((const char*)"Speed hack~n~~r~Isjungtas", 2000, 0, false);
		}
	}
}

void CheatSpeed::toggleHopHack(const char *arg)
{
	if(strlen(arg) > 0){
		double hopHeight = atof(arg);

		if(hopHeight == 0.0) CheatSpeed::hopHeight = 0.05;
		else CheatSpeed::hopHeight = hopHeight;

		GTA_SA::addMessage((const char*)"Hop pakeistas!", 1500, 0, false);
	}else{
		hopHackEnabled = !hopHackEnabled;

		if(hopHackEnabled){
			GTA_SA::addMessage((const char*)"Hop hack~n~~g~Ijungtas", 2000, 0, false);
		}else{
			GTA_SA::addMessage((const char*)"Hop hack~n~~r~Isjungtas", 2000, 0, false);
		}
	}
}
