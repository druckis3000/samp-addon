#ifndef EVENT_H
#define EVENT_H

#include "game/samp.h"
#include <iostream>
#include <windows.h>

// ----- Type definitions -----

typedef void(*setCheckpointCallback)(float x, float y, float z, float size);
typedef void(*addMessageCallback)(const char *msg, DWORD color);
typedef void(*showBigMessageCallback)(const char *text, unsigned int time, unsigned int style);
typedef void(*scoreboardUpdateCallback)();
typedef void(*onDialogResponseCallback)(uint8_t button, enum eDialogStyle style, uint32_t selectedIndex, char *input);

template <typename T>
struct stEventListener {
	T functionCall;
	bool first;
	stEventListener<T> *next;
};

// ----- Function declarations -----

void setupEventListeners();

void addSetCheckpointCallback(setCheckpointCallback eventCallback);
void invokeSetCheckpointCallbacks(float x, float y, float z, float size);

void addAddMessageCallback(addMessageCallback eventCallback);
void invokeOnMessageCallbacks(const char *msg, DWORD color);

void addShowBigMessageCallback(showBigMessageCallback eventCallback);
void invokeOnBigMessageCallbacks(const char *text, unsigned int time, unsigned int style);

void addScoreboardUpdateCallback(scoreboardUpdateCallback eventCallback);
void invokeOnScoreboardUpdateCallbacks();

void addOnDialogResponseCallback(onDialogResponseCallback eventCallback);
void invokeOnDialogResponseCallbacks(uint8_t button, enum eDialogStyle style, uint32_t selectedIndex, char *input);

// ----- Extern linked list declarations -----

extern struct stEventListener<setCheckpointCallback>	*g_SetCheckpointCallbacks;
extern struct stEventListener<addMessageCallback>		*g_AddMessageCallbacks;
extern struct stEventListener<showBigMessageCallback>	*g_ShowBigMessageCallbacks;
extern struct stEventListener<scoreboardUpdateCallback>	*g_ScoreboardUpdateCallbacks;
extern struct stEventListener<onDialogResponseCallback>	*g_OnDialogResponseCallbacks;

#endif
