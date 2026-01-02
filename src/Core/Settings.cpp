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

int Settings::readSettingsFilePropertyInt(LPCWSTR section, LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strNotificationPopups;
	GetPrivateProfileString(section, key, defaultVal, strNotificationPopups.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strNotificationPopups.ReleaseBuffer();
	return _ttoi(strNotificationPopups);
}

unsigned int Settings::readSettingsFilePropertyUInt(LPCWSTR section, LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strNotificationPopups;
	GetPrivateProfileString(section, key, defaultVal, strNotificationPopups.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strNotificationPopups.ReleaseBuffer();
	TCHAR* end = nullptr;
	return _tcstoui64(strNotificationPopups, &end, 0);
}

float Settings::readSettingsFilePropertyFloat(LPCWSTR section, LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strCustomHUDScale;
	GetPrivateProfileString(section, key, defaultVal, strCustomHUDScale.GetBuffer(MAX_PATH), MAX_PATH, filename);
	strCustomHUDScale.ReleaseBuffer();
	return _ttof(strCustomHUDScale);
}

std::string Settings::readSettingsFilePropertyString(LPCWSTR section, LPCWSTR key, LPCWSTR defaultVal, LPCWSTR filename)
{
	CString strBuffer;
	GetPrivateProfileString(section, key, defaultVal, strBuffer.GetBuffer(MAX_PATH), MAX_PATH, filename);
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
    settingsIni._var = readSettingsFilePropertyString(_T("Settings"), L##_inistring, L##_defaultval, strINIPath);

#define SETTING_INT(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyInt(_T("Settings"), L##_inistring, L##_defaultval, strINIPath);

#define SETTING_BOOL(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyInt(_T("Settings"), L##_inistring, L##_defaultval, strINIPath) != 0;

#define SETTING_FLOAT(_var, _inistring, _defaultval) \
    settingsIni._var = readSettingsFilePropertyFloat(_T("Settings"), L##_inistring, L##_defaultval, strINIPath);

#include "settings.def"
#undef SETTING_STRING
#undef SETTING_BOOL
#undef SETTING_FLOAT
#undef SETTING_INT

	// Set buttons back to default if their values are incorrect
	char default_key[4] = "";
	std::string* buttons[10] = { &settingsIni.togglebutton, &settingsIni.toggleOnlineButton, &settingsIni.toggleOverlaybutton,
	&settingsIni.toggleLegendbutton, &settingsIni.toggleHSDbutton, &settingsIni.toggleFMbutton, &settingsIni.toggleGamePausebutton,
	&settingsIni.frameStepbutton, &settingsIni.saveCBRbutton, &settingsIni.discardCBRbutton };
	for (int i = 0; i < 10; i++)
	{
		sprintf(default_key, "F%d", i + 1);
		if (checkButton(*buttons[i]))
			*buttons[i] = std::string(default_key);
	}

	//Check to make sure there are no button overlaps and resolve them if found
	bool buttons_taken[12];
	for (int i = 0; i < 12; i++)
		buttons_taken[i] = false;
	bool conflicts[10];
	for (int i = 0; i < 10; i++)
		conflicts[i] = false;

	int idx = 0;
	//Determine if there are any conflicting button assingments
	for (int i = 0; i < 10; i++)
	{
		idx = getButtonValue(*buttons[i]) - 112;
		if (!buttons_taken[idx])
			buttons_taken[idx] = true;
		else
			conflicts[i] = true;
	}

	//Resolve conflicts
	for (int i = 0; i < 10; i++)
	{
		if (conflicts[i])
		{
			idx = findNextFree(buttons_taken, 12);
			sprintf(default_key, "F%d", idx + 1);
			*buttons[i] = std::string(default_key);
			buttons_taken[idx] = true;
		}
	}
	

	return true;
}

int Settings::findNextFree(bool used[], int n)
{
	for (int i = 0; i < n; i++)
	{
		if (!used[i])
			return i;
	}
	return 0;
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
