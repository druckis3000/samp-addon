#ifndef SAMP_H
#define SAMP_H

#include "sa_structs.h"
#include <windows.h>
#include <iostream>

//#define SAMP_VERSION_R4
#define SAMP_VERSION_R4_2

// ----- SAMP Related defines -----

#define SAMP_PTR_SAMP_INFO_OFFSET		0x26EA0C
#define SAMP_PTR_CHAT_INFO_OFFSET		0x26E9F8
#define SAMP_PTR_CHAT_INPUT_INFO_OFFSET	0x26E9FC
#define SAMP_PTR_TEXTDRAW_OFFSET		0x26E9E0
#define SAMP_PTR_MISC_OFFSET			0x26EA24
#define SAMP_PTR_DIALOG_INFO_OFFSET		0x26E9C8
#define SAMP_PTR_SCOREBOARD_INFO_OFFSET	0x26E9C4
#define SAMP_PTR_COLORS_OFFSET			0x1516A0

#if defined(SAMP_VERSION_R4_2)
	#define SAMP_FUNC_ADDCLIENTCMD		0x69770 
	#define SAMP_FUNC_ADDTOCHATWND		0x67BE0 
	#define SAMP_FUNC_SAY				0x05A10 
	#define SAMP_FUNC_SAY_CMD			0x69900 
	#define SAMP_FUNC_UPDATESCOREBOARD	0x08F10 
	#define SAMP_FUNC_TOGGLECURSOR		0xA0750 
	#define SAMP_FUNC_ENABLE_CURSOR		0x696F0 
	#define SAMP_FUNC_SETCHECKPOINT		0xA1E20 // (This func works for hooking, but doesn't toggle drawing of checkpoint, but can change current shown checkpoint coords)
	#define SAMP_FUNC_SHOWGAMETEXT		0xA0D40 
	#define SAMP_FUNC_RESTARTGAME		0x0A540 
	#define SAMP_FUNC_GETPLAYERCOLOR	0xA7270 
	#define SAMP_FUNC_SETPLAYERCOLOR	0xA7250 
	#define SAMP_FUNC_SHOWSCOREBOARD	0x6F3D0 
	#define SAMP_FUNC_CLOSESCOREBOARD	0x6E9E0 
	#define SAMP_FUNC_SHOWCHATINPUT		0x69480 
	#define SAMP_FUNC_CLOSECHATINPUT	0x69580 
	#define SAMP_FUNC_SHOWDIALOG		0x70010 
	#define SAMP_FUNC_HIDEDIALOG		0x6F860 
	#define SAMP_FUNC_CLOSEDIALOG		0x70690 
	#define SAMP_FUNC_DRAWDIALOG		0x6F500 
	#define SAMP_FUNC_SETMARKERSTATE	0x14500 
	#define SAMP_FUNC_SETWORLDWEATHER	0xA0C10 
	#define SAMP_FUNC_WNDPROC			0x61650 
	#define SAMP_FUNC_GAME_PROC			0xA26A0 
	#define SAMP_FUNC_PLAYAUDIOSTR		0x66960
	#define SAMP_FUNC_STOPAUDIOSTR		0x66560
	#define SAMP_FUNC_TAKESCREENSHOT	0x75620

	#define SAMP_HOOK_ADD_TO_CHAT		0x67BE2
	#define SAMP_HOOK_UPDATE_SCOREBOARD	0x10533
	#define SAMP_HOOK_SET_CHECKPOINT	0xA1E24
	#define SAMP_HOOK_SHOW_BIG_MESSAGE	0xA0D43
	#define SAMP_HOOK_ON_DIALOG_BUTTON	0x70693
	#define SAMP_HOOK_WND_PROC			0x61655

	#define SAMP_NOP_SHOWDIALOG		0x0FCB3	// size: 5 NOP's
	#define SAMP_NOP_NAMETAG_DIST_1	0x75458
	#define SAMP_NOP_NAMETAG_DIST_2	0x74338
	#define SAMP_NOP_ANTI_CRASH_1	0x60A4D
	#define SAMP_NOP_ANTI_CRASH_2	0x60A5A

	#define SAMP_PATCH_FAST_CONNECT	"\x3D\xB8\x0B\x00\x00\x76\x4F"
#elif defined(SAMP_VERSION_R4)
	#define SAMP_FUNC_ADDCLIENTCMD		0x69730 
	#define SAMP_FUNC_ADDTOCHATWND		0x67BA0 
	#define SAMP_FUNC_SAY				0x05A00 
	#define SAMP_FUNC_SAY_CMD			0x698C0 
	#define SAMP_FUNC_UPDATESCOREBOARD	0x08F00 
	#define SAMP_FUNC_TOGGLECURSOR		0xA0720 
	#define SAMP_FUNC_ENABLE_CURSOR		0x696B0 
	#define SAMP_FUNC_SETCHECKPOINT		0xA1DF0 // (This func works for hooking, but doesn't toggle drawing of checkpoint, but can change current shown checkpoint coords)
	#define SAMP_FUNC_SHOWGAMETEXT		0xA0D10 
	#define SAMP_FUNC_RESTARTGAME		0x0A530 
	#define SAMP_FUNC_GETPLAYERCOLOR	0xA7240 
	#define SAMP_FUNC_SETPLAYERCOLOR	0xA7220 
	#define SAMP_FUNC_SHOWSCOREBOARD	0x6F3A0 
	#define SAMP_FUNC_CLOSESCOREBOARD	0x6E9A0 
	#define SAMP_FUNC_SHOWCHATINPUT		0x69440 
	#define SAMP_FUNC_CLOSECHATINPUT	0x69540 
	#define SAMP_FUNC_SHOWDIALOG		0x6FFE0 
	#define SAMP_FUNC_HIDEDIALOG		0x6F830 
	#define SAMP_FUNC_CLOSEDIALOG		0x70660 
	#define SAMP_FUNC_DRAWDIALOG		0x6F4D0 
	#define SAMP_FUNC_SETMARKERSTATE	0x144B0 
	#define SAMP_FUNC_SETWORLDWEATHER	0xA0BE0 // or 68E90
	#define SAMP_FUNC_WNDPROC			0x61610
	#define SAMP_FUNC_GAME_PROC			0xA2670
	#define SAMP_FUNC_PLAYAUDIOSTR		0x66920
	#define SAMP_FUNC_STOPAUDIOSTR		0x66520
	#define SAMP_FUNC_TAKESCREENSHOT	0x755F0

	#define SAMP_HOOK_ADD_TO_CHAT		0x67BA2
	#define SAMP_HOOK_UPDATE_SCOREBOARD	0x1050E
	#define SAMP_HOOK_SET_CHECKPOINT	0xA1DF4
	#define SAMP_HOOK_SHOW_BIG_MESSAGE	0xA0D13
	#define SAMP_HOOK_ON_DIALOG_BUTTON	0x70663
	#define SAMP_HOOK_WND_PROC			0x61615

	#define SAMP_NOP_SHOWDIALOG		0x0FC93	// size: 5 NOP's
	#define SAMP_NOP_NAMETAG_DIST_1	0x75428
	#define SAMP_NOP_NAMETAG_DIST_2	0x74308
	#define SAMP_NOP_ANTI_CRASH_1	0x609FD
	#define SAMP_NOP_ANTI_CRASH_2	0x60A0A

	#define SAMP_PATCH_FAST_CONNECT	"\x3D\xB8\x0B\x00\x00\x76\x50"
#endif

// ----- SAMP.dll enums -----

enum ChatMessageType
{
	CHAT_TYPE_NONE = 0,
	CHAT_TYPE_CHAT = 2,
	CHAT_TYPE_INFO = 4,
	CHAT_TYPE_DEBUG = 8
};

enum eLimits
{
	#if defined(SAMP_VERSION_R4)
		SAMP_MAX_PLAYERS = 502,
	#elif defined(SAMP_VERSION_R4_2)
		SAMP_MAX_PLAYERS = 1004,
	#endif
	SAMP_MAX_VEHICLES = 2000,
	SAMP_MAX_PICKUPS = 4096,
	SAMP_MAX_OBJECTS = 1000,
	SAMP_MAX_GANGZONES = 1024,
	SAMP_MAX_3DTEXTS = 2048,
	SAMP_MAX_TEXTDRAWS = 2048,
	SAMP_MAX_PLAYERTEXTDRAWS = 256,
	SAMP_MAX_MENUS = 128,
	SAMP_MAX_DIALOGS = 65535,
	SAMP_MAX_CLIENTCMDS = 144,
	SAMP_MAX_CMD_LENGTH = 32,
	SAMP_MAX_PLAYER_NAME_LENGTH = 24,
	SAMP_MAX_INPUT_LENGTH = 129,
	SAMP_MAX_CHAT_LENGTH = 144,
	SAMP_MAX_TEXTDRAW_TEXT_LENGTH = 0x321,
	
	GTA_MAX_INVENTORY_WEAPONS = 13
};

enum eGameState
{
	GAME_STATE_STARTED = 1,
	GAME_STATE_CONNECTING = 2,
	GAME_STATE_PLAYING = 5,
	GAME_STATE_CONNECTED = 6
};

enum eDialogStyle
{
	DIALOG_STYLE_MSGBOX = 0,
	DIALOG_STYLE_INPUT_TEXT = 1,
	DIALOG_STYLE_LIST = 2,
	DIALOG_STYLE_INPUT_PASSWORD = 3,
	DIALOG_STYLE_TABLIST = 4
};

enum eCursorMode {
	CMODE_NONE,
	CMODE_LOCKKEYS_NOCURSOR, 
	CMODE_LOCKCAMANDCONTROL, 
	CMODE_LOCKCAM,
	CMODE_LOCKCAM_NOCURSOR
};

enum eMarkersMode
{
	PLAYER_MARKERS_MODE_OFF,
	PLAYER_MARKERS_MODE_GLOBAL,
	PLAYER_MARKERS_MODE_STREAMED,
};

// ----- SAMP.dll Struct types -----

#pragma pack(push, 1)
#if defined(SAMP_VERSION_R4)

	struct stSAMPPools
	{
		void					*pActor;
		void					*pGangzone;
		struct stPlayerPool		*pPlayer;		// 8
		struct stVehiclePool	*pVehicle;		// 12
		struct stPickupPool		*pPickupPool;	// 16
		struct stObjectPool		*pObject;		// 20
		void					*pPlayerLabels;	// 24 (Type is just guess!!)
		struct stTextLabelPool	*pText3D;		// 28
		struct stTextdrawPool	*pTextdraw; 	// 32
	};

#elif defined(SAMP_VERSION_R4_2)

	struct stSAMPPools
	{
		struct stVehiclePool	*pVehicle;		// 0
		struct stPlayerPool		*pPlayer;		// 4
		struct stPickupPool		*pPickupPool;	// 8
		struct stObjectPool		*pObject;		// 12
		void					*pGangzone;		// 16
		void					*pPlayerLabels;	// 20 (Type is just probably!!)
		struct stTextLabelPool	*pText3D;		// 24
		struct stTextdrawPool	*pTextdraw; 	// 28
		void					*pActor;		// 32
	};

#endif

struct stServerPresets
{
	bool			bUseCJWalk;						// 0
	unsigned int	nDeadDropsMoney;				// 1
	float			fWorldBoundaries[4];			// 5
	bool			bAllowWeapons;					// 21
	float			fGravity;						// 22
	bool			bEnterExit;						// 26
	BOOL			bVehicleFriendlyFire;			// 27
	bool			bHoldTime;						// 31
	bool			bInstagib;						// 32
	bool			bZoneNames;						// 33
	bool			bFriendlyFire;					// 34
	BOOL			bClassesAvailable;				// 35
	float			fNameTagsDrawDist;				// 39
	bool			bManualVehicleEngineAndLight;	// 43
	unsigned char	nWorldTimeHour;					// 44
	unsigned char	nWorldTimeMinute;				// 45
	unsigned char	nWeather;						// 46
	bool			bNoNametagsBehindWalls;			// 47
	int				nPlayerMarkersMode;				// 48
	float			fChatRadius;					// 52
	bool			bNameTags;						// 56
	bool			bLtdChatRadius;					// 57
};

struct stSAMP
{
	char					_unkOffset1[44];		// 0		
	void					*pRakClientInterface;	// 44		0x2C
	char					szHostAddress[257];		// 48		0x30
	char					szHostname[257];		// 305		0x131
	bool					bDisableCollision;		// 562		0x232
	bool					bUpdateCameraTarget;	// 563		0x233
	bool					bNametagStatus;			// 564		0x234
	uint32_t				ulPort;					// 565		0x235
	BOOL					bLanMode;				// 569		0x239
	uint32_t				ulMapIcons[100];		// 573		0x23D
	int						iGameState;				// 973		0x3CD
	uint32_t				iLastConnectAttempt;	// 977		0x3D1
	struct stServerPresets	*pSettings;				// 981		0x3D5
	char					_unkOffset3[5];			// 985		0x3D9
	struct stSAMPPools		*pPools;				// 990		0x3DE
};

typedef void(__cdecl *CMDPROC) ();
struct stInputInfo
{
	void				*pD3DDevice;
	void				*pDXUTDialog;
	void				*pDXUTEditBox;
	CMDPROC				pCMDs[SAMP_MAX_CLIENTCMDS];
	char				szCMDNames[SAMP_MAX_CLIENTCMDS][33];
	int					iCMDCount;
	int					iInputEnabled;
	char				szInputBuffer[129];
	char				szRecallBufffer[10][129];
	char				szCurrentBuffer[129];
	int					iCurrentRecall;
	int					iTotalRecalls;
	CMDPROC				pszDefaultCMD;
};

struct stChatEntry
{
	uint32_t	SystemTime;
	char		szPrefix[28];
	char		szText[144];
	uint8_t		unknown[64];
	int			iType;			// 2 - text + prefix, 4 - text (server msg), 8 - text (debug)
	DWORD		clTextColor;
	DWORD		clPrefixColor;	// or textOnly colour
};

struct stChatInfo
{
	int					pagesize;
	char				*pLastMsgText;
	int					iChatWindowMode;
	uint8_t				bTimestamps;
	uint32_t			m_iLogFileExist;
	char				logFilePathChatLog[MAX_PATH + 1];
	void				*pGameUI; // CDXUTDialog
	void				*pEditBackground; // CDXUTEditBox
	void				*pDXUTScrollBar;
	DWORD				clTextColor;
	DWORD				clInfoColor;
	DWORD				clDebugColor;
	DWORD				m_lChatWindowBottom;
	struct stChatEntry	chatEntry[100];
	void				*m_pFontRenderer;
	void				*m_pChatTextSprite;
	void				*m_pSprite;
	void				*m_pD3DDevice;
	int					m_iRenderMode; // 0 - Direct Mode (slow), 1 - Normal mode
	void				*pID3DXRenderToSurface;
	void				*m_pTexture;
	void				*pSurface;
	void				*pD3DDisplayMode;
	int					iUnk1[3];
	int					iUnk2; // smth related to drawing in direct mode
	int					m_iRedraw;
	int					m_nPrevScrollBarPosition;
	int					m_iFontSizeY;
	int					m_iTimestampWidth;
	int					m_iTimeStampTextOffset;
};

struct stScoreboardInfo 
{
	bool		bIsEnabled;
	char		_offset[3];
	int			iPlayerCount;
	float		fPosition[2];
	float		fScalar;
	float		fSize[2];
	float		pad_[5];
	void		*pDevice; // IDirect3DDevice9
	void		*pDialog; // CDXUTDialog
	void		*pListbox; // CDXUTListBox
	int			iCurrentOffset;
	bool		bIsSorted;
};

struct stDialogInfo
{
	void				*pDevice;			// 0x0 IDirect3DDevice9
	unsigned long		ulPosition[2];		// 0x4
	unsigned long		ulSize[2];			// 0xC
	unsigned long		ulButtonOffset[2];	// 0x14
	void				*pDialog;			// 0x1C CDXUTDialog
	void				*pListbox;			// 0x20 CDXUTListBox
	void				*pEditbox;			// 0x24 CDXUTIMEEditBox
	int					iIsActive;			// 0x28
	int					iType;
	unsigned int		iId;
	char				*szText;
	unsigned long		ulTextSize[2];
	char				szCaption[65];
	int					iServerside;
	char				pad[536];
};

struct stTextdraw
{
	char		szText[SAMP_MAX_TEXTDRAW_TEXT_LENGTH];
	char		szString[0x641];
	char		_unk;
	float		fLetterWidth;
	float		fLetterHeight;
	DWORD		dwLetterColor;
	BYTE		byte_unk;
	BYTE		byteCenter;
	BYTE		byteBox;
	float		fBoxSizeX;
	float		fBoxSizeY;
	DWORD		dwBoxColor;
	BYTE		byteProportional;
	DWORD		dwShadowColor;
	BYTE		byteShadowSize;
	BYTE		byteOutline;
	BYTE		byteLeft;
	BYTE		byteRight;
	int			iStyle;
	float		fX;
	float		fY;
	BYTE		unk[16];
	DWORD		index;
	BYTE		byteSelectable;
	uint16_t	sModel;
	float		fRot[3];
	float		fZoom;
	BYTE		bColor;
	DWORD		dwColor;

	unsigned char		field_1;
	unsigned char		field_2;
	unsigned char		field_3;
	unsigned long		field_4;
	unsigned long		field_5;
	unsigned long		field_6;
	unsigned long		field_7;
	unsigned char		field_8;
	unsigned long		field_9;
};

struct stTextdrawPool
{
	int					iIsListed[SAMP_MAX_TEXTDRAWS];
	int					iPlayerTextDraw[SAMP_MAX_PLAYERTEXTDRAWS];
	struct stTextdraw	*textdraw[SAMP_MAX_TEXTDRAWS]; // Textdraws that are same for all players
	struct stTextdraw	*playerTextdraw[SAMP_MAX_PLAYERTEXTDRAWS]; // Textdraws shown on local player screen
};

#if defined(SAMP_VERSION_R4)

	struct stPlayerPool
	{
		uint32_t				ulMaxPlayerID;			// 0
		int						iLocalPlayerPing;		// 4
		int						iLocalPlayerScore;		// 8
		uint16_t				sLocalPlayerID;			// 12
		
		// After sLocalPlayerID there's 00 00 00 00
		void					*pVTBL_txtHandler;		// 14 (nullptr)
		
		// After four zero bytes there's player name
		union											// 18
		{
			// If iLocalPlayerNameLen is < 0xF then szLocalPlayerName is used
			// If it's longer that 0xF, then pszLocalPlayerName is used
			char szLocalPlayerName[16]; // SAMP_MAX_PLAYER_NAME_LENGTH == 24, but this char array size may be only 16
			char *pszLocalPlayerName;
		};
		int						iLocalPlayerNameLen;	// 34
		int						iScoreboardSomething;	// 38 (Value is same as iLocalPlayerNameLen)
		
		/// Local player struct
		struct stLocalPlayer	*pLocalPlayer;			// 42
		
		struct stRemotePlayer	*pRemotePlayer[SAMP_MAX_PLAYERS];	// 46
		DWORD					dwPlayerIP[SAMP_MAX_PLAYERS]; 		// 2054 (always 0)
		int						iIsListed[SAMP_MAX_PLAYERS];		// 4062
	};

#elif defined(SAMP_VERSION_R4_2)

	struct stPlayerPool
	{
		int						iLocalPlayerScore;		// 0
		uint16_t				sLocalPlayerID;			// 4
		int						iUnk;					// 6
		
		// After four zero bytes there's player name
		union											// 10
		{
			// If iLocalPlayerNameLen is < 0xF then szLocalPlayerName is used
			// If it's longer that 0xF, then pszLocalPlayerName is used
			char szLocalPlayerName[16]; // SAMP_MAX_PLAYER_NAME_LENGTH == 24, but this char array size may be only 16
			char *pszLocalPlayerName;
		};

		int						iLocalPlayerNameLen;	// 26
		int						iScoreboardSomething;	// 30 (Value is same as iLocalPlayerNameLen_
		int						iLocalPlayerPing;		// 34

		/// Local player struct
		struct stLocalPlayer	*pLocalPlayer;			// 38

		int						iIsListed[SAMP_MAX_PLAYERS];
		DWORD					dwPlayerIP[SAMP_MAX_PLAYERS];
		struct stRemotePlayer	*pRemotePlayer[SAMP_MAX_PLAYERS];

		void					*pVTBL_txtHandler;
	};

#endif

template <typename T>
struct stSAMPEntity
{
	// #pragma pack( 1 )
	void		*pVTBL;
	uint8_t		byteUnk0[60]; // game CEntity object maybe. always empty.
	T			*pGTAEntity;
	uint32_t	ulGTAEntityHandle; // VehicleID or ObjectID
};

struct stSAMPPed : public stSAMPEntity<actor_info>
{
	// #pragma pack( 1 )
	int					usingCellPhone;
	uint8_t				byteUnk0[600];
	struct actor_info	*pGTA_Ped;
	uint32_t			ulGTAMarkerHandle;
	uint32_t			ulGTAArrowHandle;
	bool				bNeedToCreateMarker;
	bool				bInvulnerable;
	uint8_t				byteUnk1[12];
	uint8_t				byteKeysId;
	uint16_t			ulGTA_UrinateParticle_ID;
	int					DrinkingOrSmoking;
	int					object_in_hand;
	int					drunkLevel;
	uint8_t				byteUnk2[5];
	int					isDancing;
	int					danceStyle;
	int					danceMove;
	uint8_t				byteUnk3[20];
	int					isUrinating;
};

struct stSAMPVehicle : public stSAMPEntity<vehicle_info>
{
	// #pragma pack( 1 )
	uint32_t			ulUnk;
	uint8_t				byteUnknown2[8];
	int					iIsMotorOn;
	int					iIsLightsOn;
	int					iIsLocked;
	uint8_t				byteIsObjective;
	int					iObjectiveBlipCreated;
	uint8_t				byteUnknown3[20];
	uint8_t				byteUnk[2];
	int					iColorSync;
	int					iColor_something;

	uint8_t				byteColor[5];
	struct vehicle_info *pGTA_Vehicle;
};

struct stSAMPObject : public stSAMPEntity<object_info>
{
	// #pragma pack( 1 )
	uint8_t				byteUnk0[2];
	uint32_t			ulUnk1;
	int					iModel;
	uint16_t			byteUnk2;
	float				fDrawDistance;
	float				fUnk;
	float				fPos[3];
	uint8_t				byteUnk3[68];
	uint8_t				byteUnk4;
	float				fRot[3];
};

struct stObjectPool
{
	#pragma pack( 1 )
	uint32_t			iMaxObjectID;
	int					iIsListed[SAMP_MAX_OBJECTS];
	struct stSAMPObject *pSAMP_Object[SAMP_MAX_OBJECTS];
};

struct stTextLabel
{
	char		*pText;
	DWORD		color;
	float		fPosition[3];
	float		fMaxViewDistance;
	uint8_t		byteShowBehindWalls;
	uint16_t	sAttachedToPlayerID;
	uint16_t	sAttachedToVehicleID;
};

struct stTextLabelPool
{
	// 0 - 1024 are plain text labels,
	// 1025 - SAMP_MAX_3DTEXTS are labels attached to the player (remote)
	struct stTextLabel	textLabel[SAMP_MAX_3DTEXTS];
	int					iIsListed[SAMP_MAX_3DTEXTS];
};

struct stPickup
{
	int		iModelID;
	int		iType;
	float	fPosition[3];
};

struct stPickupPool
{
	int				iPickupsCount;
	uint32_t		ul_GTA_PickupID[SAMP_MAX_PICKUPS];
	int				iPickupID[SAMP_MAX_PICKUPS];
	int				iTimePickup[SAMP_MAX_PICKUPS];
	uint8_t			unk[SAMP_MAX_PICKUPS * 3];
	struct stPickup	pickup[SAMP_MAX_PICKUPS];
};

struct stVehiclePool
{
	#pragma pack( 1 )
	uint32_t				iMaxVehicleID;
	//char					_unk[10];
	//int						iInitiated; // not tested
	#if defined(SAMP_VERSION_R4)
	char					_unk[0x1170];
	#elif defined(SAMP_VERSION_R4_2)
	char					_unk[0x1130];
	#endif
	
	struct stSAMPVehicle	*pSAMP_Vehicle[SAMP_MAX_VEHICLES]; // offset: 0x1134 bytes
	int						iIsListed[SAMP_MAX_VEHICLES];
	struct vehicle_info		*pGTA_Vehicle[SAMP_MAX_VEHICLES];
	
	uint8_t					byteUnknown[0xBCAC];
	float					fSpawnPos[SAMP_MAX_VEHICLES][3];
};

struct stSAMPKeys // (size: 16 bytes)
{
	uint8_t keys_primaryFire : 1;
	uint8_t keys_horn__crouch : 1;
	uint8_t keys_secondaryFire__shoot : 1;
	uint8_t keys_accel__zoomOut : 1;
	uint8_t keys_enterExitCar : 1;
	uint8_t keys_decel__jump : 1;			// on foot: jump or zoom in
	uint8_t keys_circleRight : 1;
	uint8_t keys_aim : 1;					// hydra auto aim or on foot aim
	uint8_t keys_circleLeft : 1;
	uint8_t keys_landingGear__lookback : 1;
	uint8_t keys_unknown__walkSlow : 1;
	uint8_t keys_specialCtrlUp : 1;
	uint8_t keys_specialCtrlDown : 1;
	uint8_t keys_specialCtrlLeft : 1;
	uint8_t keys_specialCtrlRight : 1;
	uint8_t keys__unused : 1;
};

struct stOnFootData // (size: 56 bytes)
{
	uint16_t	sLeftRightKeys;
	uint16_t	sUpDownKeys;
	union
	{
		uint16_t			sKeys;
		struct stSAMPKeys	stSampKeys;
	};
	float		fPosition[3];
	float		fQuaternion[4];
	uint8_t		byteHealth;
	uint8_t		byteArmor;
	uint8_t		byteCurrentWeapon;
	uint8_t		byteSpecialAction;
	float		fSurfingOffsets[3];
	uint16_t	sSurfingVehicleID;
	short		sCurrentAnimationID;
	short		sAnimFlags;
};

struct stInCarData // (size: 63 bytes)
{
	#pragma pack( 1 )
	uint16_t	sVehicleID;
	uint16_t	sLeftRightKeys;
	uint16_t	sUpDownKeys;
	union
	{
		uint16_t			sKeys;
		struct stSAMPKeys	stSampKeys;
	};
	float		fQuaternion[4];
	float		fPosition[3];
	float		fMoveSpeed[3];
	float		fVehicleHealth;
	uint8_t		bytePlayerHealth;
	uint8_t		byteArmor;
	uint8_t		byteCurrentWeapon;
	uint8_t		byteSiren;
	uint8_t		byteLandingGearState;
	uint16_t	sTrailerID;
	union
	{
		uint16_t	HydraThrustAngle[2];	//nearly same value
		float		fTrainSpeed;
	};
};

struct stAimData // (size: 31 bytes)
{
	BYTE	byteCamMode;
	float	vecAimf1[3];
	float	vecAimPos[3];
	float	fAimZ;
	BYTE	byteCamExtZoom : 6;		// 0-63 normalized
	BYTE	byteWeaponState : 2;	// see eWeaponState
	BYTE	bUnk;
};

struct stTrailerData // (size: 54 bytes)
{
	uint16_t	sTrailerID;
	float		fPosition[3];
	float		fQuaternion[4];
	float		fSpeed[3];
	float		fSpin[3];
};

struct stPassengerData // (size: 24 bytes)
{
	uint16_t	sVehicleID;
	uint8_t		byteSeatID;
	uint8_t		byteCurrentWeapon;
	uint8_t		byteHealth;
	uint8_t		byteArmor;
	uint16_t	sLeftRightKeys;
	uint16_t	sUpDownKeys;
	union
	{
		uint16_t			sKeys;
		struct stSAMPKeys	stSampKeys;
	};
	float	fPosition[3];
};

struct stDamageData // (size: 12 bytes)
{
	uint16_t	sVehicleID_lastDamageProcessed;
	int			iBumperDamage;
	int			iDoorDamage;
	uint8_t		byteLightDamage;
	uint8_t		byteWheelDamage;
};

struct stSurfData // (size: 38 bytes)
{
	int			iIsSurfing;
	float		fSurfPosition[3];
	int			iUnk0;
	uint16_t	sSurfingVehicleID;
	uint32_t	ulSurfTick;
	struct stSAMPVehicle *pSurfingVehicle;
	int			iUnk1;
	int			iSurfMode;	//0 = not surfing, 1 = moving (unstable surf), 2 = fixed on vehicle
};

struct stUnoccupiedData // (size: 67 bytes)
{
	int16_t sVehicleID;
	uint8_t byteSeatID;
	float	fRoll[3];
	float	fDirection[3];
	float	fPosition[3];
	float	fMoveSpeed[3];
	float	fTurnSpeed[3];
	float	fHealth;
};

struct stBulletData // (size: 40 bytes)
{
	uint8_t		byteType;
	uint16_t	sTargetID;
	float		fOrigin[3];
	float		fTarget[3];
	float		fCenter[3];
	uint8_t		byteWeaponID;
};

struct stSpectatorData // (size: 18 bytes)
{
	uint16_t	sLeftRightKeys;
	uint16_t	sUpDownKeys;
	union
	{
		uint16_t			sKeys;
		struct stSAMPKeys	stSampKeys;
	};
	float	fPosition[3];
};

struct stStatsData // (size: 8 bytes)
{
	int iMoney;
	int iAmmo;	// ?
};

struct stHeadSync // (size: 20 bytes)
{
	float	fHeadSync[3];
	int		iHeadSyncUpdateTick;
	int		iHeadSyncLookTick;
};

struct stLocalPlayer
{
	struct stInCarData		inCarData;		// 0
	struct stAimData		aimData;		// 63  (0x03F)
	struct stTrailerData	trailerData;	// 94  (0x05E)
	struct stOnFootData		onFootData;		// 148 (0x094)
	char					_offset0[12];	// 204 (might be stDamageData according to size)
	struct stPassengerData	passengerData;	// 216
	
	int						iIsActive;			// 240
	int						iIsWasted;			// 244
	uint16_t				sCurrentVehicleID;	// 248 (0x0F8)
	uint16_t				sLastVehicleID;		// 250 (0x0FA)
	
	uint32_t				ulUnk0;				// 252
	uint32_t				ulUnk1;				// 256
	struct stSAMPPed		*pSAMP_Actor;		// 260
	
	/*int						iSpawnSkin;
	uint8_t					byteUnk1;
	float					fSpawnPos[3];
	float					fSpawnRot;
	int						iSpawnWeapon[3];
	int						iSpawnAmmo[3];
	int						iIsActorAlive;
	int						iSpawnClassLoaded;
	uint32_t				ulSpawnSelectionTick;
	uint32_t				ulSpawnSelectionStart;
	int						iIsSpectating;
	uint8_t					byteTeamID2;
	uint16_t				usUnk2;
	uint32_t				ulSendTick;
	uint32_t				ulSpectateTick;
	uint32_t				ulAimTick;
	uint32_t				ulStatsUpdateTick;
	uint32_t				ulWeapUpdateTick;
	uint16_t				sAimingAtPid;
	uint16_t				usUnk3;*/
	char					_offset2[129];
	
	uint8_t					byteCurrentWeapon;
	uint8_t					byteWeaponInventory[GTA_MAX_INVENTORY_WEAPONS];
	int						iWeaponAmmo[GTA_MAX_INVENTORY_WEAPONS];
	
	int						iPassengerDriveBy;
	uint8_t					byteCurrentInterior;
	int						iIsInRCVehicle;
	uint16_t				sTargetObjectID;
	uint16_t				sTargetVehicleID;
	uint16_t				sTargetPlayerID;
	struct stHeadSync		headSyncData;
	uint32_t				ulHeadSyncTick;
	BYTE					byteSpace3[260];
	struct stSurfData		surfData;
	int						iClassSelectionOnDeath;
	int						iSpawnClassID;
	int						iRequestToSpawn;
	int						iIsInSpawnScreen;
	uint32_t				ulUnk4;
	uint8_t					byteSpectateMode;		// 3 = vehicle, 4 = player, side = 14, fixed = 15
	uint8_t					byteSpectateType;		// 0 = none, 1 = player, 2 = vehicle
	int						iSpectateID;
	int						iInitiatedSpectating;
	struct stDamageData		vehicleDamageData;
};

struct stRemotePlayerData
{
	int						_unk0;					// 0   (0x000)
	int						iShowNameTag;			// 4   (0x004)
	int						iUsingJetPack;			// 8   (0x008)
	uint8_t					byteSpecialAction;		// 12  (0x00C)
	uint32_t				_unk1[3];				// 13  (0x00D)

	struct stInCarData		inCarData;				// 25  (0x019)
	
	//struct stTrailerData	trailerData;			// 88	or swap these
	//struct stAimData		aimData;				// 142	or swap these
	char					_unk2[85];				// 88  (0x058)

	struct stPassengerData	passengerData;			// 173 (0x0AD)
	struct stOnFootData		onFootData;				// 197 (0x0C5)

	char 					_unk3[127];				// 253 (0x0FD)

	float					fOnFootPos[3];			// 380 (0x17C)
	float					fOnFootMoveSpeed[3];	// 392 (0x188)
	float					fVehiclePosition[3];	// 404 (0x194)
	float					fVehicleMoveSpeed[3];	// 416 (0x1A0)
	
	float					fActorArmour;			// 428 (0x1AC)
	float					fActorHealth;			// 432 (0x1B0)

	char					_unk4[16];				// 436 (0x1B4)

	int						iAFKState;				// 452 (0x1C4) (512 when afk, 0 when not)

	char					_unk5[21];				// 456 (0x1C8)

	struct stSAMPPed		*pSAMP_Actor;			// 477 (0x1DD)
	struct stSAMPVehicle	*pSAMP_Vehicle;			// 481 (0x1E1)

	uint16_t				sPlayerID;				// 485 (0x1E5)
	uint16_t				sVehicleID;				// 487 (0x1E7)
	
	int						iGlobalMarkerLoaded;		// 489 (0x1E9)
	int						iGlobalMarkerPosition[3];	// 493 (0x1ED)
	uint32_t				ulGlobalMarker_GTAID;		// 505 (0x1F9)

	// 39 bytes left here

													// 528 (0x210) (END)

	//struct stBulletData		bulletData;				// 1170 (0x492)

	/*struct stSAMPPed		*pSAMP_Actor;
	struct stSAMPVehicle	*pSAMP_Vehicle;
	uint8_t					byteTeamID;
	uint8_t					bytePlayerState;
	uint8_t					byteSeatID;
	uint32_t				ulUnk3;
	int						iPassengerDriveBy;
	void					*pUnk0;
	uint8_t					byteUnk1[60];
	float					fSomething[3];
	float					fVehicleRoll[4];
	uint32_t				ulUnk2[3];
	float					fOnFootPos[3];		 // offset: 203
	float					fOnFootMoveSpeed[3];
	float					fVehiclePosition[3]; // offset: 194
	float					fVehicleMoveSpeed[3];
	uint16_t				sPlayerID;
	uint16_t				sVehicleID;
	uint32_t				ulUnk5;
	int						iShowNameTag;
	int						iHasJetPack;
	uint8_t					byteSpecialAction;
	uint32_t				ulUnk4[3];
	struct stOnFootData		onFootData;
	struct stInCarData		inCarData;
	struct stTrailerData	trailerData;
	struct stPassengerData	passengerData;
	struct stAimData		aimData;
	float					fActorArmor;
	float					fActorHealth;
	uint32_t				ulUnk10;
	uint8_t					byteUnk9;
	uint32_t				dwTick;
	uint32_t				dwLastStreamedInTick;	// is 0 when currently streamed in
	uint32_t				ulUnk7;
	int						iAFKState;
	struct stHeadSync		headSyncData;
	int						iGlobalMarkerLoaded;
	int						iGlobalMarkerLocation[3];
	uint32_t				ulGlobalMarker_GTAID;*/
};

struct stRemotePlayer
{
	int					_offset;
	int					iScore;	
	int					iIsNPC;
	int					iPing;
	stRemotePlayerData	*pPlayerData;
	void				*pVTBL_txtHandler;
	union {
		char	szPlayerName[16];
		char	*pszPlayerName;
	};
	int					_offset2;
	uint32_t			iNameLength;
};

struct stToggleCursor {
	int iMode;
	bool bBoolean;
	
	bool bProcessed;
};

// Structs for invoking hook callbacks in main thread

struct stAddMessageParams {
	ChatMessageType eMessageType;
	char szMessage[512];
	char szMessagePrefix[512];
	DWORD dwMessageColor;
	DWORD dwMessagePrefixColor;
};

struct stSetCheckpointParams {
	float fX, fY, fZ, fSize;
};

struct stShowBigMessageParams {
	char szLastBigMessageText[1024];
	unsigned int ulLastBigMessageTime;		// milliseconds
	unsigned int ulLastBigMessageStyle = -1;
};

struct stOnDialogResponseParams {
	uint8_t				bButton;
	enum eDialogStyle	eStyle;
	uint32_t			iSelectedIndex;
	char				*szInputText;
};

// ----- samp.cpp Functions -----

typedef __cdecl void (*GameProcessFunc)();

namespace SAMP {

	bool setupSystem();
	void loop();
	void oneSecondTimer();
	const DWORD getSampDllHandle();

	// ----- samp.dll -----
	void callGameProc();
}

// ----- samp.dll Functions -----

// ----- Client/Game
void addClientCommand(const char *cmdName, CMDPROC functionPtr);

// ----- User interface
void toggleSampCursor(int cursorMode, bool bImmediatelyHideCursor, bool bProcessInMainThread);

// ----- samp.dll Global var declarations -----

extern struct stScoreboardInfo	*g_Scoreboard;
extern struct stDialogInfo		*g_Dialog;
extern struct stInputInfo		*g_Input;
extern struct stChatInfo		*g_Chat;
extern struct stSAMP			*g_Samp;
extern void 					*g_Misc;
extern bool 					g_IsSampReady;

#endif
