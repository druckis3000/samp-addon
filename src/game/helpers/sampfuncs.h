#ifndef SAMP_FUNCS_H
#define SAMP_FUNCS_H

#include "game/samp.h"
#include "game/sa_structs.h"
#include "game/vecmath.h"

// When any function from below is not used, compiler shows warning, so ignore "unused" warnings 
#pragma GCC diagnostic ignored "-Wunused-function"

#define PLAYER_ID_SELF	-1

/** @return boolean value indicating player id validity. It will be valid only if
 * player is connected and initialized
 */
static bool isValidPlayerId(int16_t playerId)
{
	if(!g_IsSampReady){
		return false;
	}

	if(playerId < 0 || playerId > SAMP_MAX_PLAYERS){
		return false;
	}

	if(g_Samp->pPools->pPlayer->iIsListed[playerId] != 1){
		return false;
	}

	if(g_Samp->pPools->pPlayer->pRemotePlayer[playerId] == nullptr){
		return false;
	}

	return true;
}

/**
 * @return pointer to remote player struct (struct stRemotePlayer)
 * (Can be nullptr if samp is not loaded or invalid player id or player not connected)
*/
static struct stRemotePlayer* getRemotePlayer(int playerId)
{
	if(!g_IsSampReady) return nullptr;

	if(!isValidPlayerId(playerId)) return nullptr;

	return g_Samp->pPools->pPlayer->pRemotePlayer[playerId];
}

/** 
 * @return pointer to remote player struct data (struct stRemotePlayerData)
 * (Can be nullptr if samp is not loaded or invalid player id or player not connected)
 */
static struct stRemotePlayerData* getRpData(int playerId)
{
	// Safety checks are done in getRemotePlayer, so skip them here
	return getRemotePlayer(playerId)->pPlayerData;
}

/** @return afk state of remote player (not working for local player) */
static bool isPlayerAFK(int playerId)
{
	if(playerId == PLAYER_ID_SELF) return true;

	struct stRemotePlayerData *rpData = getRpData(playerId);

	if(rpData == nullptr) return true;

	return rpData->iAFKState > 0;
}

/** @return boolean value telling if remote player is streamed or not */
static bool isPlayerStreamed(int16_t playerId)
{
	// Safety checks are done in getRemotePlayer function which is called inside getRpData

	// Get remote player data struct
	struct stRemotePlayerData *rpData = getRpData(playerId);

	// Let's see if player is connected/valid id & etc..
	if(rpData == nullptr) return false; // Nope, something is not good

	// If both stSAMPPed && stSAMPVehicle are nullptr, it means that player is not streamed, ez :)
	if(rpData->pSAMP_Actor == nullptr && rpData->pSAMP_Vehicle == nullptr) return false;

	return true;
}

/** Copy player name into char array
 * 
 * @param playerId player id to copy name
 * @param dest destination char array for player name, size must be at least 32 bytes
 * 
 * @return boolean indicating whether name was copied, or function failed
*/
static bool getPlayerName(int playerId, char *dest)
{
	// Safety checks are done inside isValidPlayerId, so skip them here

	// Make sure player is connected
	if(!isValidPlayerId(playerId)){
		return false;
	}

	// Get player's name
	if(g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->iNameLength > 15){
		strcpy(dest, g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->pszPlayerName);
	}else{
		strcpy(dest, g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->szPlayerName);
	}

	return true;
}

/** @return boolean value telling if player is in vehicle or not
 * (returns false if player is not streamed or not connected or invalid id) */
static bool isPlayerInVehicle(int16_t playerId)
{
	if(!g_IsSampReady) return false;

	if(playerId == PLAYER_ID_SELF){
		// Return local actor incar state
		bool bIsInVehicle = !(g_Samp->pPools->pPlayer->pLocalPlayer->sCurrentVehicleID == 0xFFFF);
		return bIsInVehicle;
	}

	// Make sure remote player is connected
	if(!isValidPlayerId(playerId)) return false;

	// Make sure remote player is streamed
	if(!isPlayerStreamed(playerId)) return false;

	// Find out remote player's in car state
	bool bIsInVehicle = !(getRpData(playerId)->pSAMP_Vehicle == nullptr);
	return bIsInVehicle;
}

/**
 * @return pointer to player textdraw struct (struct stTextdraw)
 * (Can return nullptr if samp is not ready or textdraw not found or due to other unknown reasons)
 */
static struct stTextdraw* getPlayerTextdraw(int index)
{
	if(!g_IsSampReady) return nullptr;
	if(g_Samp->pPools->pTextdraw->iPlayerTextDraw[index] != 1) return nullptr;
	
	return g_Samp->pPools->pTextdraw->playerTextdraw[index];
}

// ----- Textdraw creation and manipulation -- START -----

/**
 * Create new textdraw with specified id
 * @return id of textdraw, or -1 if textdraw creation failed
*/
static int createTextdrawWithId(int id, const char *text, float x, float y, DWORD color)
{
	if(!g_IsSampReady) return -1;

	// Allocate space for new textdraw
	struct stTextdraw *textdraw = (struct stTextdraw*)malloc(sizeof(*textdraw));
	
	// If memory allocation failed, stop further execution
	if(textdraw == nullptr) return -1;

	strncpy(textdraw->szText, text, SAMP_MAX_TEXTDRAW_TEXT_LENGTH);
	strncpy(textdraw->szString, text, 0x642);
	textdraw->fLetterWidth = 0.5f;
	textdraw->fLetterHeight = 1.2f;
	textdraw->dwLetterColor = color;
	textdraw->byte_unk = 0;
	textdraw->byteCenter = 1;
	textdraw->byteBox = 1;
	textdraw->fBoxSizeX = -3.0;
	textdraw->fBoxSizeY = 320.0;
	textdraw->dwBoxColor = 0x0;
	textdraw->byteProportional = 1;
	textdraw->dwShadowColor = 0xFF000000;
	textdraw->byteShadowSize = 2;
	textdraw->byteOutline = 1;
	textdraw->byteLeft = 0;
	textdraw->byteRight = 0;
	textdraw->iStyle = 1;
	textdraw->fX = x;
	textdraw->fY = y;
	textdraw->index = 0xFFFFFFFF;
	textdraw->byteSelectable = 0;
	textdraw->sModel = 0;
	textdraw->fRot[0] = 0;
	textdraw->fRot[1] = 0;
	textdraw->fRot[2] = 0;
	textdraw->fZoom = 1.0;
	textdraw->bColor = 255;
	textdraw->dwColor = 0xFFFFFF;
	textdraw->byteShadowSize = 2;

	for(int i=0; i<8; i++) textdraw->unk[i] = 0;
	for(int i=8; i<16; i++) textdraw->unk[i] = 255;

	/*textdraw->field_1 = 1;
	textdraw->field_2 = 0;
	textdraw->field_3 = 128;
	textdraw->field_4 = 2919235584;
	textdraw->field_5 = 2030043137;
	textdraw->field_6 = 2852126722;
	textdraw->field_7 = 1;
	textdraw->field_8 = 0;
	textdraw->field_9 = 0;*/

	// Add new textdraw to the textdraws pool
	g_Samp->pPools->pTextdraw->iPlayerTextDraw[id] = 1;
	g_Samp->pPools->pTextdraw->playerTextdraw[id] = textdraw;

	return id;
}


/**
 * Create new textdraw
 * @return id of textdraw, or -1 if textdraw creation failed
*/
static int createTextdraw(const char *text, float x, float y, DWORD color)
{
	if(!g_IsSampReady) return -1;
	
	int freeId = -1;
	for(int i=0; i<SAMP_MAX_TEXTDRAWS; i++){
		if(g_Samp->pPools->pTextdraw->iPlayerTextDraw[i] < 1){
			freeId = i;
			break;
		}
	}

	return createTextdrawWithId(freeId, text, x, y, color);
}

/**
 * Set textdraw text
 * @return true if text changed successfully, false if failed
*/
static bool setTextdrawText(int id, const char *text)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	strncpy(g_Samp->pPools->pTextdraw->playerTextdraw[id]->szText, text, SAMP_MAX_TEXTDRAW_TEXT_LENGTH);
	strncpy(g_Samp->pPools->pTextdraw->playerTextdraw[id]->szString, text, 0x641);
	return false;
}

/**
 * Set textdraw color
 * @return true if color changed successfully, false if failed
*/
static bool setTextdrawColor(int id, DWORD color)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->dwLetterColor = color;
	return true;
}

/**
 * Set textdraw box color
 * @return true if color changed successfully, false if failed
*/
static bool setTextdrawBoxColor(int id, DWORD boxColor)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->dwBoxColor = boxColor;
	return true;
}

/**
 * Set textdraw background color
 * @return true if color changed successfully, false if failed
*/
static bool setTextdrawBgColor(int id, DWORD bgColor)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->dwColor = bgColor;
	return true;
}

/**
 * Set textdraw font
 * @return true if font changed successfully, false if failed
*/
static bool setTextdrawFont(int id, int font)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->iStyle = font;
	return true;
}

/**
 * Set textdraw letter size
 * @return true if letter size changed successfully, false if failed
*/
static bool setTextdrawLetterSize(int id, float xSize, float ySize)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->fLetterWidth = xSize;
	g_Samp->pPools->pTextdraw->playerTextdraw[id]->fLetterHeight = ySize;
	return true;
}

/**
 * Set textdraw outline
 * @return true if outline changed successfully, false if failed
*/
static bool setTextdrawOutline(int id, BYTE outline)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->byteOutline = outline;
	return true;
}

/**
 * Set textdraw shadow size. 1 is generally used for a normal shadow size. 0 disables the shadow completely.
 * @return true if outline changed successfully, false if failed
*/
static bool setTextdrawShadow(int id, BYTE shadowSize)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->byteShadowSize = shadowSize;
	return true;
}

/**
 * Set textdraw proportionallity. 1 to enable proportionality, 0 to disable.
 * @return true if proportionallity changed successfully, false if failed
*/
static bool setTextdrawProportional(int id, BYTE proportional)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->byteProportional = proportional;
	return true;
}

/**
 * Set textdraw box. 1 to show a box or 0 to not show a box.
 * @return true if box state changed successfully, false if failed
*/
static bool setTextdrawUseBox(int id, BYTE useBox)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->playerTextdraw[id]->byteBox = useBox;
	return true;
}

/**
 * Set textdraw visibility. 1 to show textdraw, 0 to hide.
 * @return true if visibility changed successfully, false if failed
*/
static bool setTextdrawVisible(int id, BYTE visible)
{
	if(!g_IsSampReady) return false;
	if(g_Samp->pPools->pTextdraw->playerTextdraw[id] == nullptr) return false;

	g_Samp->pPools->pTextdraw->iPlayerTextDraw[id] = visible;
	return true;
}

// ----- Textdraw creation and manipulation -- END -----

/** @return local player id or -1 if retrieving id fails */
static int16_t getLocalPlayerId()
{
	if(!g_IsSampReady) return -1;

	return g_Samp->pPools->pPlayer->sLocalPlayerID;
}

/** @return vehicle id of player, or -1 if not in vehicle, or not if player is not streamed */
static int getPlayerVehicleId(int playerId)
{
	if(!g_IsSampReady) return -1;

	if(playerId == PLAYER_ID_SELF){
		if(isPlayerInVehicle(PLAYER_ID_SELF)){
			return g_Samp->pPools->pPlayer->pLocalPlayer->sCurrentVehicleID;
		}else{
			return -1;
		}
	}

	if(!isValidPlayerId(playerId)) return -1;

	if(!isPlayerInVehicle(playerId)) return -1;

	return getRpData(playerId)->sVehicleID;
}

/**
 * @return pointer to GTA ped (struct actor_info) of player which id equals to playerId
 * (Can return nullptr if samp is not ready or player is not connected
 *  or player is not streamed or due to other unknown reasons)
*/
static struct actor_info* getGTAPedFromSampId(int playerId)
{
	if(!g_IsSampReady) return nullptr;

	// Return local player actor if caller wants so
	if(playerId == PLAYER_ID_SELF) return g_Samp->pPools->pPlayer->pLocalPlayer->pSAMP_Actor->pGTAEntity;

	// Do safety checks and after that return ptr to actor_info struct

	if(g_Samp->pPools->pPlayer->iIsListed[playerId] != 1) return nullptr;

	if(g_Samp->pPools->pPlayer->pRemotePlayer[playerId] == nullptr) return nullptr;

	if(g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->pPlayerData == nullptr) return nullptr;

	if(g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->pPlayerData->pSAMP_Actor == nullptr) return nullptr;

	return g_Samp->pPools->pPlayer->pRemotePlayer[playerId]->pPlayerData->pSAMP_Actor->pGTAEntity;
}

/**
 * @return pointer to GTA vehicle (struct vehicle_info) of vehicle which id equals to vehicleId
 * (Can return nullptr if samp is not ready or vehicle is not streamed or due to other unknown reasons)
*/
static struct vehicle_info* getGTAVehicleFromSampId(int vehicleId)
{
	if(!g_IsSampReady) return nullptr;

	// Safety check
	if(g_Samp->pPools->pVehicle->iIsListed[vehicleId] != 1) return nullptr;

	return g_Samp->pPools->pVehicle->pGTA_Vehicle[vehicleId];
}

/**
 * Copies player position into float dest[3]
 * 
 * @param playerId id of player to copy position (can be -1 to get local player position)
 * 
 * @return boolean telling if copying was successful or not
*/
static bool getPlayerPosition(int16_t playerId, float dest[3])
{
	if(!g_IsSampReady) return false;

	// TODO: check if player is in vehicle, if so, return vehicle pos

	// Check if caller wants to get local player position
	if(playerId == PLAYER_ID_SELF){
		if(isPlayerInVehicle(PLAYER_ID_SELF)){
			// Player is in vehicle
			vect3_copy(g_Samp->pPools->pPlayer->pLocalPlayer->inCarData.fPosition, dest);
		}else{
			// Player is not in vehicle
			vect3_copy(&getGTAPedFromSampId(PLAYER_ID_SELF)->base.matrix[4 * 3], dest);
		}
		return true;
	}

	// Safety check are done in getRpData
	struct stRemotePlayerData *rpData = getRpData(playerId);

	// Check if rpData was found
	if(rpData == nullptr){
		// Nope
		return false;
	}
	
	if(isPlayerInVehicle(playerId)){
		// Player is in vehicle
		vect3_copy(rpData->inCarData.fPosition, dest);
	}else{
		// Player is not in vehicle
		vect3_copy(&rpData->pSAMP_Actor->pGTAEntity->base.matrix[4 * 3], dest);
	}

	return true;
}

/**
 * @return player's score. 0 if failed to get score
*/
static int getPlayerScore(int playerId)
{
	if(!g_IsSampReady) return 0.0f;

	if(playerId == PLAYER_ID_SELF) return 0; // TODO: Finish here

	if(!isValidPlayerId(playerId)) return 0;

	return getRemotePlayer(playerId)->iScore;
}

/**
 * @return player's health
 * (Returns 0.0f if player is not connected or invalid player id or samp is not loaded)
 */
static float getPlayerHealth(int playerId)
{
	if(!g_IsSampReady) return 0.0f;

	if(playerId == PLAYER_ID_SELF) return getGTAPedFromSampId(PLAYER_ID_SELF)->hitpoints;

	if(!isValidPlayerId(playerId)) return 0.0f;

	return getRpData(playerId)->fActorHealth;
}

/**
 * @return player's armor
 * (Returns 0.0f if player is not connected or invalid player id or samp is not loaded)
 */
static float getPlayerArmor(int playerId)
{
	if(!g_IsSampReady) return 0.0f;

	if(playerId == PLAYER_ID_SELF) return getGTAPedFromSampId(PLAYER_ID_SELF)->armor;

	if(!isValidPlayerId(playerId)) return 0.0f;

	return getRpData(playerId)->fActorArmour;
}

static uint8_t getPlayerWeapon(int playerId)
{
	if(!g_IsSampReady) return 0;

	if(playerId == PLAYER_ID_SELF) return g_Samp->pPools->pPlayer->pLocalPlayer->onFootData.byteCurrentWeapon;

	if(!isValidPlayerId(playerId)) return 0;

	return getRpData(playerId)->onFootData.byteCurrentWeapon;
}

/** @return boolean value telling is samp chat is active (T) */
static bool isSampChatInputActive()
{
	if(!g_IsSampReady) return false;

	return g_Input->iInputEnabled > 0;
}

/** @return boolean value telling is samp scoreboard is active (TAB) */
static bool isSampScoreboardActive()
{
	if(!g_IsSampReady) return false;

	return g_Scoreboard->bIsEnabled;
}

/** @return boolean value telling is any player dialog is shown in samp */
static bool isSampDialogActive()
{
	if(!g_IsSampReady) return false;

	return g_Dialog->iIsActive > 0;
}

/** @return length of last input text in last shown dialog */
static int getDialogInputTextLength()
{
				// C++ fuckery again
	return *((int*)((uint8_t*)g_Dialog->pEditbox + 0x119)); // or 11E
}

/** @return const std::string of last input text in last shown dialog */
static const std::string getDialogInputText()
{
	// Convert unicode string to ascii string
	char *unicodeText = *(char**)((char*)g_Dialog->pEditbox + 0x4D);
	int textLength = getDialogInputTextLength();
	char asciiText[textLength + 1];

	for(int i=0; i<textLength; i++){
		// Unicode index is two times higher because every second char is null-terminator
		asciiText[i] = unicodeText[i * 2];
	}

	// Append null-terminator at the end
	asciiText[textLength] = '\0';

	// RVO - so no problem returning string from stack
	// (return value is actually copied into caller object)
	return std::string(asciiText);
}

/** @return index value of last selected item in last shown dialog */
static int getDialogSelectedItemIndex()
{
						// Some c++ fuckery
	int selectedItem = *((int*)((uint8_t*)g_Dialog->pListbox + 0x143));
	return selectedItem;
}

/** Changes currently selected item in list dialog */
static void setDialogSelectedItemIndex(int index)
{
	*((int*)((uint8_t*)g_Dialog->pListbox + 0x143)) = index;
}

/** @return id of last shown dialog */
static int getDialogId()
{
	return g_Dialog->iId;
}
/** @return style of last shown dialog */
static eDialogStyle getDialogStyle()
{
	return static_cast<eDialogStyle>(g_Dialog->iType);
}

/** @return boolean value telling if last shown dialog was server-side */
static bool isDialogServerside()
{
	return g_Dialog->iServerside > 0;
}

static uint16_t getSampPlayersCount()
{
	if(!g_IsSampReady) return 0;

	uint16_t n = 1;
	for(int i=0; i<SAMP_MAX_PLAYERS; i++)
		if(g_Samp->pPools->pPlayer->iIsListed[i]) n++;

	return n;
}

static void setNametagSettings(float distance, bool noNameTagsBehindWalls, bool showNameTags)
{
	g_Samp->pSettings->fNameTagsDrawDist = distance;
	g_Samp->pSettings->bNoNametagsBehindWalls = noNameTagsBehindWalls;
	g_Samp->pSettings->bNameTags = showNameTags;
}

static void getNametagSettings(float *distance, bool *noNameTagsBehindWalls, bool *showNameTags)
{
	*distance = g_Samp->pSettings->fNameTagsDrawDist;
	*noNameTagsBehindWalls = g_Samp->pSettings->bNoNametagsBehindWalls;
	*showNameTags = g_Samp->pSettings->bNameTags;
}

#endif