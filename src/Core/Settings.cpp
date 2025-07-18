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
	if (settingsIni.togglebutton.length() != 2 || settingsIni.togglebutton[0] != 'F')
		settingsIni.togglebutton = "F1";

	if (settingsIni.toggleOnlineButton.length() != 2 || settingsIni.toggleOnlineButton[0] != 'F')
		settingsIni.toggleOnlineButton = "F2";

	if (settingsIni.saveCBRbutton.length() != 2 || settingsIni.saveCBRbutton[0] != 'F')
		settingsIni.saveCBRbutton = "F8";
	if (settingsIni.discardCBRbutton.length() != 2 || settingsIni.discardCBRbutton[0] != 'F')
		settingsIni.discardCBRbutton = "F9";

	return true;
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

	//default to F1
	button = "F1";
	return 112;
}
