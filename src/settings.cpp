#include "settings.h"
#include "ini/INIReader.h"
#include "utils/helper.h"

namespace Settings {
	
	// ----- Private variables -----

	INIReader settingsIni("scr.ini");
};

bool Settings::loadSettings()
{
	Log("settings.cpp: Loading settings file");

	int rc = settingsIni.ParseError();
	if(rc < 0){
		Logf("settings.cpp: Failed to load settings file (%d)", rc);
		return false;
	}

	Log("settings.cpp: Settings file loaded!");
	return true;
}

bool Settings::getBool(std::string section, std::string key, bool defaultValue)
{
	return settingsIni.GetBoolean(section, key, defaultValue);
}

int Settings::getInt(std::string section, std::string key, int defaultValue)
{
	return settingsIni.GetInteger(section, key, defaultValue);
}

float Settings::getFloat(std::string section, std::string key, float defaultValue)
{
	return settingsIni.GetFloat(section, key, defaultValue);
}

std::string Settings::getString(std::string section, std::string key, std::string defaultValue)
{
	return settingsIni.GetString(section, key, defaultValue);
}