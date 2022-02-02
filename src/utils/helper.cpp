#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <algorithm>
#include "helper.h"

// ----- Logging -----

void Log(const char *msg)
{
	std::cout << msg << std::endl;
}

void Logf(const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	char buffer[2048];
	vsnprintf(buffer, 2048, fmt, argptr);
	va_end(argptr);
	
	std::cout << buffer << std::endl;
}

// ----- Timing -----

double GetTimeMillis()
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	double ms = st.wHour * 3600 + st.wMinute * 60 + st.wSecond + st.wMilliseconds * 0.001;
	return ms;
}

// ----- String -----

bool isNumber(const char *s)
{
	std::string str(s);
    std::string::const_iterator it = str.begin();
    while (it != str.end() && std::isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}

bool findStringIC(const std::string & strHaystack, const std::string & strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(),   strNeedle.end(),
		[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != strHaystack.end());
}