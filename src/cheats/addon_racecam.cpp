#include "addon_racecam.h"
#include "game/helpers/sampfuncs.h"
#include "game/gtasa.h"
#include "hooking.h"
#include "settings.h"
#include <math.h>

#define FUNC_CDebug_DebugDisplayTextBuffer	0x532260

namespace AddonRacecam {

	// ----- Private variables -----

	volatile bool bRacecamEnabled = false;
	volatile bool bApplyRacecam = false;

	float fCamTargetMultiplier = 6.0;
	float fSmoothAnglesPower = 6.0;
	float fCamXAngle = -0.10;

	// ----- Private functions -----

	void calculateCamera();
};

static float GetZAngleForPoint(float p[]);
static void SmoothAngles(float *in, float power);
static void FixRadian(float *in);

FHook renderHook;
__declspec(naked) void HOOK_render_NAKED(); 

void AddonRacecam::setupAddon()
{
	#ifdef LOG_VERBOSE
		Log("addon_racecam.cpp: Hooking render function");
	#endif

	// Install render hook
	renderHook.patchAddress = (DWORD)FUNC_CDebug_DebugDisplayTextBuffer;
	renderHook.hookAddress = (DWORD)HOOK_render_NAKED;
	renderHook.overwriteSize = 5;
	MidFuncHook(&renderHook);

	#ifdef LOG_VERBOSE
		Log("addon_racecam.cpp: Registering /zrc command");
	#endif

	// Register /zrc command
	addClientCommand("zrc", []{
		bRacecamEnabled = !bRacecamEnabled;

		if(bRacecamEnabled) GTA_SA::addMessage((const char*)"Racecam~n~~g~Ijungta", 2000, 0, false);
		else GTA_SA::addMessage((const char*)"Racecam~n~~r~Isjungta", 2000, 0, false);
	});

	#ifdef LOG_VERBOSE
		Log("addon_racecam: Loading settings");
	#endif

	bRacecamEnabled = Settings::getBool("settings", "racecamEnabled", false);
	fCamTargetMultiplier = Settings::getFloat("settings", "racecamTargetMultiplier", 5.0);
	fSmoothAnglesPower = Settings::getFloat("settings", "racecamAngleSmoothPower", 6.0);
	fCamXAngle = Settings::getFloat("settings", "racecamXangle", -0.1);
}

void AddonRacecam::updateAddon()
{
	if(!bRacecamEnabled){
		bApplyRacecam = false;
		return;
	}

	// Don't do this cheat if player is on foot
	if(!isPlayerInVehicle(PLAYER_ID_SELF)){
		bApplyRacecam = false;
		return;
	}

	// Find local player vehicle
	struct vehicle_info *vehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
	if(vehicle == nullptr){
		bApplyRacecam = false;
		return;
	}

	// Find local player actor
	struct actor_info *actor = getGTAPedFromSampId(PLAYER_ID_SELF);
	if(actor == nullptr){
		bApplyRacecam = false;
		return;
	}

	bApplyRacecam = true;
}

void AddonRacecam::calculateCamera()
{
	struct vehicle_info *vehicle = getGTAVehicleFromSampId(getPlayerVehicleId(PLAYER_ID_SELF));
	if(vehicle == nullptr) return;

	// Get car facing direction
	float forwardVec[4] = {0.0f, 1.0f, 0.0f, 0.0f};
	float vehFacingDir[4];
	if(vehicle == nullptr) return;
	matrix_vect4_mult(vehicle->base.matrix, forwardVec, vehFacingDir);
	vect3_normalize(vehFacingDir, vehFacingDir);
	if(!vect3_near_zero(vehFacingDir)){
		vect3_mult(vehFacingDir, fCamTargetMultiplier, vehFacingDir);
	}

	// Add car direction to it's position
	float vehPos[3];
	vect3_copy(g_Samp->pPools->pPlayer->pLocalPlayer->inCarData.fPosition, vehPos);
	vect3_vect3_add(vehPos, vehFacingDir, vehPos);

	// Get camera position
	float camPos[3];
	vect3_copy(CCamera::getCamPosition(), camPos);

	vehPos[0] -= camPos[0];
	vehPos[1] -= camPos[1];
	float zAngle = GetZAngleForPoint(vehPos);
	zAngle -= 90.0;
	zAngle /= 57.2957795;
	float xAngle = CCamera::getCamXAngle();
	
	float angles[2] = {zAngle, xAngle};
	SmoothAngles(angles, fSmoothAnglesPower);

	writeMemoryArray(0xB70118, "\x00\x00\x48\x42", 4);

	CCamera::setCamXAngle(fCamXAngle);
	CCamera::setCamZAngle(angles[0]);
}

void HOOK_render()
{
	if(AddonRacecam::bApplyRacecam)
		AddonRacecam::calculateCamera();
}

__declspec(naked) void HOOK_render_NAKED()
{
	__asm("pushal");
	HOOK_render();
	__asm("popal");
	__asm("ret");
}

static void SmoothAngles(float *in, float power)
{
	float currentZAngle = *(float*)0xB6F258;
	float currentXAngle = *(float*)0xB6F248;

	in[0] -= currentZAngle;
	in[1] -= currentXAngle;

	FixRadian(in);

	in[0] /= power;
	in[1] /= power;

	in[0] += currentZAngle;
	in[1] += currentXAngle;

	FixRadian(in);
}

static void FixRadian(float *in)
{
	if(in[0] > 3.14159265359) in[0] -= 6.28318530717958647693;
	if(in[0] < -3.14159265359) in[0] += 6.28318530717958647693;
	if(in[1] > 3.14159265359) in[1] -= 6.28318530717958647693;
	if(in[1] < -3.14159265359) in[1] += 6.28318530717958647693;
}

static float GetZAngleForPoint(float point[]) {
	float angle = ((float(__cdecl *)(float, float))0x53CC70)(point[0], point[1]) * 57.295776f - 90.0f;
	while (angle < 0.0f) angle += 360.0f;
	return angle;
}

static float getZAngle(float v1[], float v2[])
{
	// Calculate direction between local and remote player
	float fDirectionBetween[3];
	vect3_vect3_sub(v1, v2, fDirectionBetween);
	vect3_normalize(fDirectionBetween, fDirectionBetween);

	// Calculate dot product of forward and direction vector
	//float dot = vect3_dot_product(fLocalForward, fDirectionBetween);
	float dot = 1.0;

	// Calculate angle
	float angle = acos(dot);

	return angle;
}