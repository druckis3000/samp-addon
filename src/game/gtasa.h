#ifndef GTASA_H
#define GTASA_H

#include <windows.h>

#define GTA_FUNC_SHOW_BIG_MSG	        0x588FC0
#define GTA_FUNC_SHOW_MESSAGE_JUMP_Q	0x69F1E0
#define GTA_FUNC_SHOW_MESSAGE	        0x69F2B0

namespace GTA_SA {

	// ----- Public functions -----

	bool setupSystem();
	void loop();

	// ----- gtasa.cpp Functions -----

	float getFPS();
	float getDelta();

	// ----- GTA_SA Functions -----

	void showBigMessage(const char *message, unsigned short style);
	void addMessage(const char *text, unsigned int time, unsigned short flag, bool bPreviousBrief);
	void forceWeather(int state);
	void setDrawDistance(float distance);
	float getDrawDistance();
}

namespace CCamera {
	float* getCamPosition();
	float getCamXAngle();
	float getCamZAngle();
	void setCamXAngle(float xAngle);
	void setCamZAngle(float yAngle);
}

// ----- GTA_SA Enums -----

enum eMessageStyle : unsigned short
{
	// Used in GTA_SA::addMessage();
    STYLE_MIDDLE,                 // InTheMiddle
    STYLE_BOTTOM_RIGHT,           // AtTheBottomRight
    STYLE_WHITE_MIDDLE,           // WhiteText_InTheMiddle
    STYLE_MIDDLE_SMALLER,         // InTheMiddle_Smaller
    STYLE_MIDDLE_SMALLER_HIGHER,  // InTheMiddle_Smaller_ABitHigherOnTheScreen
    STYLE_WHITE_MIDDLE_SMALLER,   // SmallWhiteText_InTheMiddleOfTheScreen
    STYLE_LIGHTBLUE_TOP           // LightBlueText_OnTopOfTheScreen
};

#endif
