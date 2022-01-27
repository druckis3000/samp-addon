#ifndef HELPER_H
#define HELPER_H

#include <string>

//#define LOG_VERBOSE

// ----- Logging -----

void Log(const char *msg);
void Logf(const char *fmt, ...);

// ----- Timing -----

double GetTimeMillis();

// ----- String -----

bool isNumber(const char *s);
bool findStringIC(const std::string &strHaystack, const std::string &strNeedle);

// ----- Input simulation -----

#define LEFT_MOUSE_BUTTON	0
#define RIGHT_MOUSE_BUTTON	1

void mouseClick(int btn, int delay);

#endif
