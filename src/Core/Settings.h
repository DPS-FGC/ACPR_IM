#pragma once
#include "D3D9Wrapper/d3d9.h"

#include <string>

struct settingsIni_t
{
	//X-Macro
#define SETTING_STRING(_var, _inistring, _defaultval) \
    std::string _var

#define SETTING_INT(_var, _inistring, _defaultval) \
    int _var

#define SETTING_BOOL(_var, _inistring, _defaultval) \
    bool _var

#define SETTING_FLOAT(_var, _inistring, _defaultval) \
	float _var
#include "settings.def"
#undef SETTING_STRING
#undef SETTING_INT
#undef SETTING_BOOL
#undef SETTING_FLOAT
};

struct savedSettings_t
{
	RECT newSourceRect;
	D3DVIEWPORT9 newViewport;
	D3DXVECTOR2 origViewportRes;
	bool isDuelFieldSprite;
	bool isFiltering;


};

class Settings
{
public:
	static settingsIni_t settingsIni;
	static savedSettings_t savedSettings;

	static void applySettingsIni(D3DPRESENT_PARAMETERS* pPresentationParameters);
	static bool loadSettingsFile();
	static void initSavedSettings();
	//static void setViewportSize(D3DVIEWPORT9 Viewport);
	static short getButtonValue(std::string button);
private:
	static int readSettingsFilePropertyInt(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename);
	static float readSettingsFilePropertyFloat(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename);
	static std::string readSettingsFilePropertyString(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename);
};