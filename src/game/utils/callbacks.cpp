#include "callbacks.h"
#include "utils/helper.h"

struct stGameCallbackList<setCheckpointCallback>		*g_SetCheckpointCallbacks;
struct stGameCallbackList<addMessageCallback>			*g_AddMessageCallbacks;
struct stGameCallbackList<showBigMessageCallback>		*g_ShowBigMessageCallbacks;
struct stGameCallbackList<scoreboardUpdateCallback>	*g_ScoreboardUpdateCallbacks;
struct stGameCallbackList<onDialogResponseCallback>	*g_OnDialogResponseCallbacks;

// ----- Private function declarations -----

template <typename T>
struct stGameCallbackList<T>* findLast(struct stGameCallbackList<T> *eventListeners);

// ----- Public functions -----

void setupGameCallbacks()
{
	// Initialize linked lists
	g_SetCheckpointCallbacks = new stGameCallbackList<setCheckpointCallback>;
	g_SetCheckpointCallbacks->functionCall = NULL;
	g_SetCheckpointCallbacks->next = NULL;
	
	g_AddMessageCallbacks = new stGameCallbackList<addMessageCallback>;
	g_AddMessageCallbacks->functionCall = NULL;
	g_AddMessageCallbacks->next = NULL;
	
	g_ShowBigMessageCallbacks = new stGameCallbackList<showBigMessageCallback>;
	g_ShowBigMessageCallbacks->functionCall = NULL;
	g_ShowBigMessageCallbacks->next = NULL;
	
	g_ScoreboardUpdateCallbacks = new stGameCallbackList<scoreboardUpdateCallback>;
	g_ScoreboardUpdateCallbacks->functionCall = NULL;
	g_ScoreboardUpdateCallbacks->next = NULL;

	g_OnDialogResponseCallbacks = new stGameCallbackList<onDialogResponseCallback>;
	g_OnDialogResponseCallbacks->functionCall = NULL;
	g_OnDialogResponseCallbacks->next = NULL;
}

// ----- Checkpoints -----

void addSetCheckpointCallback(setCheckpointCallback eventCallback)
{
	auto *last = findLast<setCheckpointCallback>(g_SetCheckpointCallbacks);
	
	// Set callback function
	last->functionCall = eventCallback;
	
	// Create next item in linked list
	auto *newItem = new stGameCallbackList<setCheckpointCallback>;
	newItem->functionCall = NULL;
	newItem->next = NULL;
	last->next = newItem;
}

void invokeSetCheckpointCallbacks(float x, float y, float z, float size)
{
	auto *current = g_SetCheckpointCallbacks;
	
	while(current != nullptr){
		if(current->functionCall != nullptr){
			// Call event listener callback
			current->functionCall(x, y, z, size);
		}
		
		// Iterate
		current = current->next;
	}
}

// ----- Messages -----

void addAddMessageCallback(addMessageCallback eventCallback)
{
	auto *last = findLast<addMessageCallback>(g_AddMessageCallbacks);
	
	// Set callback function
	last->functionCall = eventCallback;
	
	// Create next item in linked list
	auto *newItem = new stGameCallbackList<addMessageCallback>;
	newItem->functionCall = NULL;
	newItem->next = NULL;
	last->next = newItem;
}

void invokeOnMessageCallbacks(const char *msg, DWORD color)
{
	auto *current = g_AddMessageCallbacks;
	
	while(current != nullptr){
		if(current->functionCall != nullptr){
			// Call event listener callback
			current->functionCall(msg, color);
		}
		
		// Iterate
		current = current->next;
	}
}

// ----- Big Messages -----

void addShowBigMessageCallback(showBigMessageCallback eventCallback)
{
	auto *last = findLast<showBigMessageCallback>(g_ShowBigMessageCallbacks);

	// Set callback function
	last->functionCall = eventCallback;

	// Create next item in linked list
	auto *newItem = new stGameCallbackList<showBigMessageCallback>;
	newItem->functionCall = NULL;
	newItem->next = NULL;
	last->next = newItem;
}

void invokeOnBigMessageCallbacks(const char *text, unsigned int time, unsigned int style)
{
	auto *current = g_ShowBigMessageCallbacks;
	
	while(current != nullptr){
		if(current->functionCall != nullptr){
			// Call event listener callback
			current->functionCall(text, time, style);
		}
		
		// Iterate
		current = current->next;
	}
}

// ----- Scoreboard update -----

void addScoreboardUpdateCallback(scoreboardUpdateCallback eventCallback)
{
	auto *last = findLast<scoreboardUpdateCallback>(g_ScoreboardUpdateCallbacks);

	// Set callback function
	last->functionCall = eventCallback;

	// Create next item in linked list
	auto *newItem = new stGameCallbackList<scoreboardUpdateCallback>;
	newItem->functionCall = NULL;
	newItem->next = NULL;
	last->next = newItem;
}

void invokeOnScoreboardUpdateCallbacks()
{
	auto *current = g_ScoreboardUpdateCallbacks;
	
	while(current != nullptr){
		if(current->functionCall != nullptr){
			// Call event listener callback
			current->functionCall();
		}
		
		// Iterate
		current = current->next;
	}
}

// ----- On dialog response -----

void addOnDialogResponseCallback(onDialogResponseCallback eventCallback)
{
	auto *last = findLast<onDialogResponseCallback>(g_OnDialogResponseCallbacks);

	// Set callback function
	last->functionCall = eventCallback;

	// Create next item in linked list
	auto *newItem = new stGameCallbackList<onDialogResponseCallback>;
	newItem->functionCall = NULL;
	newItem->next = NULL;
	last->next = newItem;
}

void invokeOnDialogResponseCallbacks(uint8_t button, enum eDialogStyle style, uint32_t selectedIndex, char *input)
{
	auto *current = g_OnDialogResponseCallbacks;
	
	while(current != nullptr){
		if(current->functionCall != nullptr){
			// Call event listener callback
			current->functionCall(button, style, selectedIndex, input);
		}
		
		// Iterate
		current = current->next;
	}
}

// ----- Helper functions -----

template <typename T>
struct stGameCallbackList<T>* findLast(struct stGameCallbackList<T> *eventListeners)
{
	struct stGameCallbackList<T> *current = eventListeners;
	while(current->next != nullptr){
		current = current->next;
	}
	
	return current;
}
