#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

namespace Settings {

	// ----- Public functions -----

	void loadSettings();

	bool getBool(std::string section, std::string key, bool defaultValue);
	int getInt(std::string section, std::string key, int defaultValue);
	float getFloat(std::string section, std::string key, float defaultValue);
	std::string getString(std::string section, std::string key, std::string defaultValue);
};

#endif