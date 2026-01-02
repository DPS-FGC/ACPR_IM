#include "Settings.h"
#include "logger.h"

#include "Core/interfaces.h"

#include <atlstr.h>
#include <ctime>

#define VIEWPORT_DEFAULT 1

settingsIni_t Settings::settingsIni = {};
savedSettings_t Settings::savedSettings = {};

void Settings::applySettingsIni(D3DPRESENT_PARAMETERS* pPresentationParameters)
{

}

int Settings::readSettingsFilePropertyInt(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strNotificationPopups;
	GetPrivateProfileString(_T("Settings"), key, defaultVal, strNotificationPopups.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strNotificationPopups.ReleaseBuffer();
	return _ttoi(strNotificationPopups);
}

float Settings::readSettingsFilePropertyFloat(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strCustomHUDScale;
	GetPrivateProfileString(_T("Settings"), key, defaultVal, strCustomHUDScale.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strCustomHUDScale.ReleaseBuffer();
	return _ttof(strCustomHUDScale);
}

std::string Settings::readSettingsFilePropertyString(LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strBuffer;
	GetPrivateProfileString(_T("Settings"), key, defaultVal, strBuffer.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strBuffer.ReleaseBuffer();
	CT2CA pszConvertedAnsiString(strBuffer);
	return pszConvertedAnsiString.m_psz;
}

bool Settings::loadSettingsFile()
{
	CString strINIPath;

	_wfullpath((wchar_t*)strINIPath.GetBuffer(MAX_PATH), L"settings.ini", MAX_PATH);
	strINIPath.ReleaseBuffer();

	if (GetFileAttributes(strINIPath) == 0xFFFFFFFF)
	{
		MessageBoxA(NULL, "Settings INI File Was Not Found!", "Error", MB_OK);
		return false;
	}

	//X-Macro
	
#define SETTING_STRING(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyString(L##_inistring, L##_defaultval, strINIPath);

#define SETTING_INT(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyInt(L##_inistring, L##_defaultval, strINIPath);

#define SETTING_BOOL(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyInt(L##_inistring, L##_defaultval, strINIPath) != 0;

#define SETTING_FLOAT(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyFloat(L##_inistring, L##_defaultval, strINIPath);

#include "settings.def"
#undef SETTING_STRING
#undef SETTING_BOOL
#undef SETTING_FLOAT
#undef SETTING_INT


	// Set buttons back to default if their values are incorrect
	if (checkButton(settingsIni.togglebutton))
		settingsIni.togglebutton = "F1";

	if (checkButton(settingsIni.toggleOnlineButton))
		settingsIni.toggleOnlineButton = "F2";

	if (checkButton(settingsIni.toggleOverlaybutton))
		settingsIni.toggleOverlaybutton = "F3";

	if (checkButton(settingsIni.toggleLegendbutton))
		settingsIni.toggleLegendbutton = "F4";

	if (checkButton(settingsIni.toggleHSDbutton))
		settingsIni.toggleHSDbutton = "F5";

	if (checkButton(settingsIni.toggleFMbutton))
		settingsIni.toggleFMbutton = "F3";

	if (checkButton(settingsIni.toggleGamePausebutton))
		settingsIni.toggleGamePausebutton = "F7";

	if (checkButton(settingsIni.frameStepbutton))
		settingsIni.frameStepbutton = "F8";

	if (checkButton(settingsIni.saveCBRbutton))
		settingsIni.saveCBRbutton = "F9";

	if (checkButton(settingsIni.discardCBRbutton))
		settingsIni.discardCBRbutton = "F10";

	return true;
}

bool Settings::checkButton(std::string button)
{
	return (button.length() != 2 && button.length() != 3) || button[0] != 'F';
}

void Settings::initSavedSettings()
{
	LOG(7, "initSavedSettings\n");

	savedSettings.origViewportRes.x = 0.0;
	savedSettings.origViewportRes.y = 0.0;

	savedSettings.isDuelFieldSprite = false;

	savedSettings.isFiltering = false;
}

short Settings::getButtonValue(std::string button)
{
	if (button == "F1")
		return 112;
	if (button == "F2")
		return 113;
	if (button == "F3")
		return 114;
	if (button == "F4")
		return 115;
	if (button == "F5")
		return 116;
	if (button == "F6")
		return 117;
	if (button == "F7")
		return 118;
	if (button == "F8")
		return 119;
	if (button == "F9")
		return 120;
	if (button == "F10")
		return 121;
	if (button == "F11")
		return 122;
	if (button == "F12")
		return 123;

	//default to F1
	button = "F1";
	return 112;
}
