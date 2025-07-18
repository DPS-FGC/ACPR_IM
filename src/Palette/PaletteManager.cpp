#include "PaletteManager.h"

#include "impl_templates.h"

#include "Core/logger.h"
#include "Core/utils.h"
#include "Game/characters.h"
#include "Overlay/Logger/ImGuiLogger.h"

#include <atlstr.h>
#include <ctime>

#define MAX_NUM_OF_PAL_SLOTS 24

PaletteManager::PaletteManager()
{
	LOG(2, "PaletteManager::PaletteManager\n");

	CreatePaletteFolders();
	//CreatePaletteSlotsFile();
}

PaletteManager::~PaletteManager()
{
}

void PaletteManager::CreatePaletteFolders()
{
	LOG(2, "CreatePaletteFolders\n");

	CreateDirectory(L"ACPR_IM\\Download", NULL);
	CreateDirectory(L"ACPR_IM\\Palettes", NULL);
	CreateDirectory(L"ACPR_IM\\Replays", NULL);

	for (int i = 1; i < getCharactersCount(); i++)
	{
		std::wstring path = std::wstring(L"ACPR_IM\\Palettes\\") + getCharacterNameByIndexW(i);
		CreateDirectory(path.c_str(), NULL);
		path = std::wstring(L"ACPR_IM\\Download\\") + getCharacterNameByIndexW(i);
		CreateDirectory(path.c_str(), NULL);
	}
}

void PaletteManager::InitCustomPaletteVector()
{
	LOG(2, "InitCustomPaletteVector\n");

	m_customPalettes.clear();
	m_customPalettes.resize(getCharactersCount());

	for (int i = 1; i < getCharactersCount(); i++)
	{
		// Make the character palette array's 0th element an empty one, that will be used to set back to the default palette
		IMPL_data_t customPal { "Default" };
		m_customPalettes[i].push_back(customPal);
	}
}

void PaletteManager::LoadPalettesFromFolder()
{
	InitCustomPaletteVector();

	LOG(2, "LoadPaletteFiles\n");
	g_imGuiLogger->Log("[system] Loading local custom palettes...\n");

	for (int i = 1; i < getCharactersCount(); i++)
	{
		std::wstring wPath = std::wstring(L"ACPR_IM\\Palettes\\") + getCharacterNameByIndexW(i) + L"\\*";
		LoadPalettesIntoVector((CharIndex)i, wPath);
	}

	InitOnlinePalsIndexVector();

	for (int i = 1; i < getCharactersCount(); i++)
	{
		std::wstring wPath = std::wstring(L"ACPR_IM\\Download\\") + getCharacterNameByIndexW(i) + L"\\*";
		LoadPalettesIntoVector((CharIndex)i, wPath);
	}

	g_imGuiLogger->Log("[system] Finished loading local custom palettes\n");
}

void PaletteManager::InitOnlinePalsIndexVector()
{
	m_onlinePalsStartIndex.clear();

	m_onlinePalsStartIndex.push_back(0);
	for (int i = 1; i < getCharactersCount(); i++)
	{
		m_onlinePalsStartIndex.push_back(m_customPalettes[i].size());
	}
}

void PaletteManager::ApplyDefaultCustomPalette(CharIndex charIndex, CharPaletteHandle & charPalHandle)
{
	LOG(2, "ApplyDefaultCustomPalette\n");

	if (charIndex >= getCharactersCount())
		return;

	const int curPalIndex = charPalHandle.GetOrigPalIndex();
	const char* curPalName = m_paletteSlots[charIndex][curPalIndex].c_str();

	if (strncmp(curPalName, "", IMPL_PALNAME_LENGTH) == 0 ||
		strncmp(curPalName, "Default", IMPL_PALNAME_LENGTH) == 0)
		return;

	int foundCustomPalIndex = 0;

	if (strncmp(curPalName, "Random", IMPL_PALNAME_LENGTH) == 0)
	{
		std::srand(std::time(nullptr));
		foundCustomPalIndex = rand() % m_customPalettes[charIndex].size();
	}
	else
	{
		foundCustomPalIndex = FindCustomPalIndex(charIndex, curPalName);
	}

	if (foundCustomPalIndex < 0)
	{
		g_imGuiLogger->Log("[error] Palette file '%s' cannot be set as default: File not found.\n", curPalName);
		return;
	}

	SwitchPalette(charIndex, charPalHandle, foundCustomPalIndex);
}

void PaletteManager::LoadPalettesIntoVector(CharIndex charIndex, std::wstring& wFolderPath)
{
	std::string folderPath(wFolderPath.begin(), wFolderPath.end());
	LOG(2, "LoadPalettesIntoContainer %s\n", folderPath.c_str());

	HANDLE hFind;
	WIN32_FIND_DATA data;

	hFind = FindFirstFile(wFolderPath.c_str(), &data);

	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		// Ignore current and parent directories
		if (_tcscmp(data.cFileName, TEXT(".")) == 0 || _tcscmp(data.cFileName, TEXT("..")) == 0)
			continue;

		// Recursively search subfolders
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			std::wstring wSubfolderPath(wFolderPath.c_str());
			wSubfolderPath.pop_back(); // Delete "*" at the end
			wSubfolderPath += data.cFileName;
			wSubfolderPath += L"\\*";
			LoadPalettesIntoVector(charIndex, wSubfolderPath);
			continue;
		}

		std::wstring wFileName(data.cFileName);
		std::string fileName(wFileName.begin(), wFileName.end());
		std::string fullPath(folderPath);
		fullPath.pop_back(); // Delete "*" at the end
		fullPath += fileName;

		LOG(2, "\tFILE: %s", fileName.c_str());
		LOG(2, "\t\tFull path: %s\n", fullPath.c_str());

		if (fileName.find(IMPL_FILE_EXTENSION) != std::string::npos)
		{
			LoadImplFile(fullPath, fileName, charIndex);
		}
		else if (fileName.find(LEGACY_HPL_FILE_EXTENSION) != std::string::npos)
		{
			LoadHplFile(fullPath, fileName, charIndex);
		}
		else
		{
			LOG(2, "Unrecognized file format for '%s'\n", fileName.c_str());
			g_imGuiLogger->Log("[error] Unable to open '%s' : not an %s file\n", fileName.c_str(), IMPL_FILE_EXTENSION);
		}

	} while (FindNextFile(hFind, &data));
	FindClose(hFind);
}

void PaletteManager::LoadImplFile(const std::string& fullPath, const std::string& fileName, CharIndex charIndex)
{
	IMPL_t fileContents;

	if (!utils_ReadFile(fullPath.c_str(), &fileContents, sizeof(fileContents), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", fileName.c_str(), strerror(errno));
		return;
	}

	// Check for errors
	if (strncmp(fileContents.header.fileSig, IMPL_FILESIG, sizeof(fileContents.header.fileSig)) != 0)
	{
		LOG(2, "ERROR, unrecognized file format!\n");
		g_imGuiLogger->Log("[error] '%s' unrecognized file format!\n", fileName.c_str());
		return;
	}

	if (fileContents.header.dataLen != sizeof(IMPL_data_t))
	{
		LOG(2, "ERROR, data size mismatch!\n");
		g_imGuiLogger->Log("[error] '%s' data size mismatch!\n", fileName.c_str());
		return;
	}

	if (isCharacterIndexOutOfBound(fileContents.header.charIndex))
	{
		LOG(2, "ERROR, '%s' has invalid character index in the header\n", fileName.c_str());
		g_imGuiLogger->Log("[error] '%s' has invalid character index in the header\n", fileName.c_str());
	}
	else if (charIndex != fileContents.header.charIndex)
	{
		LOG(2, "ERROR, '%s' belongs to character %s, but is placed in folder %s\n",
			fileName.c_str(), getCharacterNameByIndexA(fileContents.header.charIndex).c_str(),
			getCharacterNameByIndexA(charIndex).c_str());

		g_imGuiLogger->Log("[error] '%s' belongs to character '%s', but is placed in folder '%s'\n",
			fileName.c_str(), getCharacterNameByIndexA(fileContents.header.charIndex).c_str(),
			getCharacterNameByIndexA(charIndex).c_str());
	}
	else
	{
		OverwriteIMPLDataPalName(fileName, fileContents.palData);
		PushImplFileIntoVector(charIndex, fileContents.palData);
	}
}

void PaletteManager::LoadHplFile(const std::string& fullPath, const std::string& fileName, CharIndex charIndex)
{
	if (fileName.find("_effectbloom") != std::string::npos)
	{
		std::string palName = fileName.substr(0, fileName.rfind("_effectbloom"));

		int palIndex = FindCustomPalIndex(charIndex, palName.c_str());

		if (palIndex < 0)
		{
			LOG(2, "ERROR, '%s' has no custom character palette to match with!\n", fileName.c_str());
			g_imGuiLogger->Log("[error] '%s' has no custom character palette to match with! Create a character palette named '%s' to load this bloom file on!\n",
				fileName.c_str(), (palName + ".hpl").c_str());
			return;
		}

		m_customPalettes[charIndex][palIndex].palInfo.hasBloom = true;

		g_imGuiLogger->Log(
			"[system] %s: Loaded '%s'\n",
			getCharacterNameByIndexA(charIndex).c_str(),
			fileName.c_str()
		);

		return;
	}

	char fileContents[LEGACY_HPL_HEADER_LEN + LEGACY_HPL_DATALEN];

	if (!utils_ReadFile(fullPath.c_str(), &fileContents, sizeof(fileContents), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", fileName.c_str(), strerror(errno));
		return;
	}

	// Effect file:
	if (fileName.find("_effect0") != std::string::npos)
	{
		std::string palName = fileName.substr(0, fileName.rfind("_effect0"));

		int palIndex = FindCustomPalIndex(charIndex, palName.c_str());

		if (palIndex < 0)
		{
			LOG(2, "ERROR, '%s' has no custom character palette to match with!\n", fileName.c_str());
			g_imGuiLogger->Log("[error] '%s' has no custom character palette to match with! Create a character palette named '%s' to load this effect file on!\n",
				fileName.c_str(), (palName + ".hpl").c_str());

			return;
		}

		int fileIndex = fileName.find("_effect0");
		std::string effectIndex = fileName.substr(fileName.find("_effect0") + 7, 2);

		fileIndex = std::stoi(effectIndex);

		if (fileIndex <= 0 || fileIndex > 7)
		{
			LOG(2, "ERROR, '%s'has wrong index of effect file!\n", fileName.c_str());
			g_imGuiLogger->Log("[error] '%s' has wrong index!\n", fileName.c_str());
			return;
		}

		IMPL_data_t& implData = m_customPalettes[charIndex][palIndex];
		char* pImplEffectFile = (char*)&implData.file0 + fileIndex * IMPL_PALETTE_DATALEN;

		memcpy_s(pImplEffectFile, IMPL_PALETTE_DATALEN, (char*)&fileContents + LEGACY_HPL_HEADER_LEN, LEGACY_HPL_DATALEN);

		g_imGuiLogger->Log(
			"[system] %s: Loaded '%s'\n",
			getCharacterNameByIndexA(charIndex).c_str(),
			fileName.c_str()
		);
	}
	else // Palette file
	{
		IMPL_t implTemplate;

		// Make a copy of template
		memcpy_s(&implTemplate, sizeof(IMPL_t), implTemplates[charIndex], sizeof(IMPL_t));

		// Copy .hpl data into cfpl template
		memcpy_s(&implTemplate.palData.file0, IMPL_PALETTE_DATALEN, (char*)&fileContents + LEGACY_HPL_HEADER_LEN, LEGACY_HPL_DATALEN);

		OverwriteIMPLDataPalName(fileName, implTemplate.palData);
		PushImplFileIntoVector(charIndex, implTemplate.palData);
	}
}

void PaletteManager::LoadPaletteSettingsFile()
{
	InitPaletteSlotsVector();

	LOG(2, "LoadPaletteSettingsFile\n");

	TCHAR pathBuf[MAX_PATH];
	GetModuleFileName(NULL, pathBuf, MAX_PATH);
	std::wstring::size_type pos = std::wstring(pathBuf).find_last_of(L"\\");
	std::wstring wFullPath = std::wstring(pathBuf).substr(0, pos);

	wFullPath += L"\\palettes.ini";

	if (!PathFileExists(wFullPath.c_str()))
	{
		LOG(2, "\t'palettes.ini' file was not found!\n");
		g_imGuiLogger->Log("[error] 'palettes.ini' file was not found!\n");
		return;
	}

	CString strBuffer;
	GetPrivateProfileString(L"General", L"OnlinePalettes", L"1", strBuffer.GetBuffer(MAX_PATH), MAX_PATH, wFullPath.c_str());
	strBuffer.ReleaseBuffer();
	m_loadOnlinePalettes = _ttoi(strBuffer);

	for (int i = 1; i < getCharactersCount(); i++)
	{
		for (int iSlot = 1; iSlot <= MAX_NUM_OF_PAL_SLOTS; iSlot++)
		{
			GetPrivateProfileString(getCharacterNameByIndexW(i).c_str(), std::to_wstring(iSlot).c_str(), L"",
				strBuffer.GetBuffer(MAX_PATH), MAX_PATH, wFullPath.c_str());

			strBuffer.ReleaseBuffer();
			strBuffer.Remove('\"');

			// Delete file extension if found
			int pos = strBuffer.Find(IMPL_FILE_EXTENSION_W, 0);
			if(pos >= 0)
			{ 
				strBuffer.Delete(pos, strBuffer.StringLength(strBuffer));
			}

			CT2CA pszConvertedAnsiString(strBuffer);
			m_paletteSlots[i][iSlot-1] = pszConvertedAnsiString;
		}
	}
}

void PaletteManager::InitPaletteSlotsVector()
{
	LOG(2, "InitPaletteSlotsVector\n");

	m_paletteSlots.clear();
	m_paletteSlots.resize(getCharactersCount());

	for (int i = 1; i < getCharactersCount(); i++)
	{
		for (int iSlot = 0; iSlot < MAX_NUM_OF_PAL_SLOTS; iSlot++)
		{
			m_paletteSlots[i].push_back("");
		}
	}
}

bool PaletteManager::PushImplFileIntoVector(IMPL_t & filledPal)
{
	LOG(7, "PushImplFileIntoVector\n");
	return PushImplFileIntoVector((CharIndex)filledPal.header.charIndex, filledPal.palData);
}

bool PaletteManager::PushImplFileIntoVector(CharIndex charIndex, IMPL_data_t & filledPalData)
{
	LOG(7, "PushImplFileIntoVector <overload>\n");

	if (charIndex >= getCharactersCount())
	{
		g_imGuiLogger->Log("[error] Custom palette couldn't be loaded: CharIndex out of bound.\n");
		LOG(2, "ERROR, CharIndex out of bound\n");
		return false;
	}

	if (FindCustomPalIndex(charIndex, filledPalData.palInfo.palName) > 0)
	{
		g_imGuiLogger->Log(
			"[error] Custom palette couldn't be loaded: a palette with name '%s' is already loaded.\n",
			filledPalData.palInfo.palName
		);
		LOG(2, "ERROR, A custom palette with name '%s' is already loaded.\n", filledPalData.palInfo.palName);
		return false;
	}

	m_customPalettes[charIndex].push_back(filledPalData);

	g_imGuiLogger->Log(
		"[system] %s: Loaded '%s%s'\n",
		getCharacterNameByIndexA(charIndex).c_str(),
		filledPalData.palInfo.palName,
		IMPL_FILE_EXTENSION
	);

	return true;
}

bool PaletteManager::WritePaletteToFile(CharIndex charIndex, IMPL_data_t *filledPalData)
{
	LOG(2, "WritePaletteToFile\n");

	std::string path = std::string("ACPR_IM\\Palettes\\") + getCharacterNameByIndexA(charIndex) + "\\" + filledPalData->palInfo.palName + IMPL_FILE_EXTENSION;

	IMPL_t IMPL_file {};

	IMPL_file.header.headerLen = sizeof(IMPL_header_t);
	IMPL_file.header.dataLen = sizeof(IMPL_data_t);
	IMPL_file.header.charIndex = charIndex;
	IMPL_file.palData = *filledPalData;

	if (!utils_WriteFile(path.c_str(), &IMPL_file, sizeof(IMPL_t), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", path.c_str(), strerror(errno));
		return false;
	}

	return true;
}

bool PaletteManager::WriteOnlinePaletteToFile(std::string fileName, CharIndex charIndex, IMPL_data_t* filledPalData)
{
	LOG(2, "WritePaletteToFile\n");

	std::wstring wpath = L"ACPR_IM\\Download\\" + utf8_to_utf16(getCharacterNameByIndexA(charIndex)) + L"\\" +
		sanitize_filename(utf8_to_utf16(fileName)) + utf8_to_utf16(IMPL_FILE_EXTENSION);

	IMPL_t IMPL_file{};

	IMPL_file.header.headerLen = sizeof(IMPL_header_t);
	IMPL_file.header.dataLen = sizeof(IMPL_data_t);
	IMPL_file.header.charIndex = charIndex;
	IMPL_file.palData = *filledPalData;

	if (!utils_WriteFileW(wpath.c_str(), &IMPL_file, sizeof(IMPL_t), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", utf16_to_utf8(wpath).c_str(), strerror(errno));
		return false;
	}

	return true;
}

void PaletteManager::LoadAllPalettes()
{
	LOG(2, "LoadAllPalettes\n");

	LoadPalettesFromFolder();
	LoadPaletteSettingsFile();

	//if(m_loadOnlinePalettes)
	//	StartAsyncPaletteArchiveDownload();
}

void PaletteManager::ReloadAllPalettes()
{
	LOG(2, "ReloadAllPalettes\n");
	g_imGuiLogger->LogSeparator();
	g_imGuiLogger->Log("[system] Reloading custom palettes...\n");

	LoadAllPalettes();
}

int PaletteManager::GetOnlinePalsStartIndex(CharIndex charIndex)
{
	if (charIndex >= getCharactersCount())
		return MAXINT32;

	return m_onlinePalsStartIndex[charIndex];
}

void PaletteManager::OverwriteIMPLDataPalName(std::string fileName, IMPL_data_t & palData)
{
	// Overwrite palname in data section with the filename, so renaming the file has effect on the actual ingame palname

	int pos = fileName.find('/');

	if(pos != std::string::npos)
		fileName = fileName.substr(pos + 1);

	std::string fileNameWithoutExt = fileName.substr(0, fileName.rfind('.'));
	memset(palData.palInfo.palName, 0, IMPL_PALNAME_LENGTH);
	strncpy(palData.palInfo.palName, fileNameWithoutExt.c_str(), IMPL_PALNAME_LENGTH - 1);
}

// Return values:
// ret > 0, index found
// ret == -1, index not found
// ret == -2, charindex out of bound
// ret == -3, default palette or no name given
int PaletteManager::FindCustomPalIndex(CharIndex charIndex, const char * palNameToFind)
{
	LOG(2, "FindCustomPalIndex\n");

	if (charIndex >= getCharactersCount())
		return -2;

	if (strncmp(palNameToFind, "", IMPL_PALNAME_LENGTH) == 0 ||
		strncmp(palNameToFind, "Default", IMPL_PALNAME_LENGTH) == 0)
		return -3;

	for (int i = 0; i < m_customPalettes[charIndex].size(); i++)
	{
		if (strncmp(palNameToFind, m_customPalettes[charIndex][i].palInfo.palName, IMPL_PALNAME_LENGTH) == 0)
		{
			return i;
		}
	}

	return -1;
}

bool PaletteManager::PaletteArchiveDownloaded()
{
	return m_PaletteArchiveDownloaded;
}

bool PaletteManager::SwitchPalette(CharIndex charIndex, CharPaletteHandle& palHandle, int newCustomPalIndex)
{
	int totalCharPals = m_customPalettes[charIndex].size();

	if (newCustomPalIndex >= totalCharPals)
		return false;

	palHandle.SetSelectedCustomPalIndex(newCustomPalIndex);
	palHandle.ReplacePalData(&m_customPalettes[charIndex][newCustomPalIndex]);

	return true;
}

void PaletteManager::ReplacePaletteFile(const char * newPalData, PaletteFile palFile, CharPaletteHandle& palHandle)
{
	palHandle.ReplaceSinglePalFile(newPalData, palFile);
}

void PaletteManager::RestoreOrigPal(CharPaletteHandle& palHandle)
{
	palHandle.RestoreOrigPal();
}

const char * PaletteManager::GetCurPalFileAddr(PaletteFile palFile, CharPaletteHandle& palHandle)
{
	return palHandle.GetCurPalFileAddr(palFile);
}

const char * PaletteManager::GetCustomPalFile(CharIndex charIndex, int palIndex, PaletteFile palFile, CharPaletteHandle& palHandle)
{
	if (charIndex >= getCharactersCount())
		charIndex = CharIndex_Sol;

	if (palIndex > m_customPalettes[charIndex].size())
		palIndex = 0;

	const char* ptr = 0;

	if (palIndex == 0)
	{
		ptr = palHandle.GetOrigPalFileAddr(palFile);
	}
	else
	{
		ptr = m_customPalettes[charIndex][palIndex].file0;
		ptr += palFile * IMPL_PALETTE_DATALEN;
	}

	return ptr;
}

int PaletteManager::GetCurrentCustomPalIndex(CharPaletteHandle& palHandle) const
{
	return palHandle.GetSelectedCustomPalIndex();
}

const IMPL_info_t& PaletteManager::GetCurrentPalInfo(CharPaletteHandle& palHandle) const
{
	return palHandle.GetCurrentPalInfo();
}

const IMPL_data_t & PaletteManager::GetCurrentPalData(CharPaletteHandle& palHandle)
{
	return palHandle.GetCurrentPalData();
}

void PaletteManager::SetCurrentPalInfo(CharPaletteHandle& palHandle, IMPL_info_t& palInfo)
{
	palHandle.SetCurrentPalInfo(&palInfo);
}

void PaletteManager::OnUpdate(CharPaletteHandle & P1, CharPaletteHandle & P2)
{
	P1.UnlockUpdate();
	P2.UnlockUpdate();
}

void PaletteManager::OnMatchInit(Player& playerOne, Player& playerTwo)
{
	//When in mirror match, two memory regions are allocated for palettes, but only the second one is
	//used by both players.
	if (playerOne.GetData()->charIndex == playerTwo.GetData()->charIndex)
		playerOne.GetPalHandle().m_pPalBaseAddr = playerTwo.GetPalHandle().m_pPalBaseAddr;

	playerOne.GetPalHandle().OnMatchInit();
	playerTwo.GetPalHandle().OnMatchInit();

	if (m_currentGameMode != GameMode_ReplayTheater)
	{
		ApplyDefaultCustomPalette((CharIndex)playerOne.GetData()->charIndex, playerOne.GetPalHandle());
		ApplyDefaultCustomPalette((CharIndex)playerTwo.GetData()->charIndex, playerTwo.GetPalHandle());
	}
	else
	{
		LoadReplayPalettes(playerOne, playerTwo);
	}
	m_replayFile = L"";
}

void PaletteManager::OnMatchRematch(Player& playerOne, Player& playerTwo)
{
	playerOne.GetPalHandle().SetPointerBasePal(nullptr);
	playerTwo.GetPalHandle().SetPointerBasePal(nullptr);
}

void PaletteManager::OnMatchEnd(CharPaletteHandle& playerOne, CharPaletteHandle& playerTwo)
{
	RestoreOrigPal(playerOne);
	RestoreOrigPal(playerTwo);

	playerOne.SetPointerBasePal(nullptr);
	playerTwo.SetPointerBasePal(nullptr);
}

void PaletteManager::SaveReplayPalettes(Player& playerOne, Player& playerTwo)
{
	if (m_replayFile.empty())
		return;

	if (m_currentGameMode != GameMode_Versus && m_currentGameMode != GameMode_Online)
		return;

	std::wstring replayFileName = L"ACPR_IM\\Replays\\" + m_replayFile + L".prep";

	IMPL_t PREP_file[2];

	PREP_file[0].header.headerLen = sizeof(IMPL_header_t);
	PREP_file[0].header.dataLen = sizeof(IMPL_data_t);
	PREP_file[0].header.charIndex = playerOne.GetData()->charIndex;
	PREP_file[0].palData = playerOne.GetPalHandle().GetPalDataAtIndex(playerOne.GetPalHandle().GetOrigPalIndex());
	strcpy(PREP_file[0].palData.palInfo.palName, "Replay");

	PREP_file[1].header.headerLen = sizeof(IMPL_header_t);
	PREP_file[1].header.dataLen = sizeof(IMPL_data_t);
	PREP_file[1].header.charIndex = playerTwo.GetData()->charIndex;
	PREP_file[1].palData = playerTwo.GetPalHandle().GetPalDataAtIndex(playerTwo.GetPalHandle().GetOrigPalIndex());
	strcpy(PREP_file[1].palData.palInfo.palName, "Replay");

	if (!utils_WriteFileW(replayFileName.c_str(), &PREP_file, sizeof(PREP_file), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", utf16_to_utf8(replayFileName), strerror(errno));
		return;
	}

	m_replayFile = L"";

}

std::wstring PaletteManager::FindReplayPaletteFileRecursive(std::wstring& wFolderPath, std::wstring& wReplayName)
{
	LOG(2, "Searching for custom palette file for replay.\n");

	HANDLE hFind;
	WIN32_FIND_DATA data;

	hFind = FindFirstFile(wFolderPath.c_str(), &data);

	std::wstring result = L"";

	if (hFind == INVALID_HANDLE_VALUE)
		return result;

	do {
		// Ignore current and parent directories
		if (_tcscmp(data.cFileName, TEXT(".")) == 0 || _tcscmp(data.cFileName, TEXT("..")) == 0)
			continue;

		// Recursively search subfolders
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			std::wstring wSubfolderPath(wFolderPath.c_str());
			wSubfolderPath.pop_back(); // Delete "*" at the end
			wSubfolderPath += data.cFileName;
			wSubfolderPath += L"\\*";
			result = FindReplayPaletteFileRecursive(wSubfolderPath, wReplayName);
			if (!result.empty())
				break;
			continue;
		}

		std::wstring wFileName(data.cFileName);
		std::wstring wFileNameNoExt = wFileName.substr(0, wFileName.rfind('.'));

		if (wFileNameNoExt != wReplayName)
			continue;

		std::string fileName(wFileName.begin(), wFileName.end());
		std::string fullPath(wFolderPath.begin(), wFolderPath.end());
		fullPath.pop_back(); // Delete "*" at the end
		fullPath += fileName;
		wFolderPath.pop_back();

		LOG(2, "\tFILE: %s", fileName.c_str());
		LOG(2, "\t\tFull path: %s\n", fullPath.c_str());

		if (fileName.find(".prep") != std::string::npos)
		{
			result = std::wstring(fullPath.begin(), fullPath.end());
			break;
		}
		else
		{
			LOG(2, "Unrecognized file format for '%s'\n", fileName.c_str());
			g_imGuiLogger->Log("[error] Unable to open '%s' : not an %s file\n", fileName.c_str(), ".prep");
		}

	} while (FindNextFile(hFind, &data));
	FindClose(hFind);
	return result;
}

bool PaletteManager::LoadPrepFile(const std::wstring& fullPath, CharIndex p1Index, CharIndex p2Index,
	IMPL_data_t& p1PalData, IMPL_data_t& p2PalData)
{
	IMPL_t fileContents[2];
	CharIndex charIndex[2] = { p1Index, p2Index };

	if (!utils_ReadFileW(fullPath.c_str(), &fileContents, sizeof(fileContents), true))
	{
		LOG(2, "\tCouldn't open %s!\n", strerror(errno));
		g_imGuiLogger->Log("[error] Unable to open '%s' : %s\n", utf16_to_utf8(fullPath), strerror(errno));
		return false;
	}

	for (int i = 0; i < 2; i++)
	{// Check for errors
		if (strncmp(fileContents[i].header.fileSig, IMPL_FILESIG, sizeof(fileContents[i].header.fileSig)) != 0)
		{
			LOG(2, "ERROR, unrecognized file format!\n");
			g_imGuiLogger->Log("[error] '%s' unrecognized file format!\n", utf16_to_utf8(fullPath));
			return false;
		}

		if (fileContents[i].header.dataLen != sizeof(IMPL_data_t))
		{
			LOG(2, "ERROR, data size mismatch!\n");
			g_imGuiLogger->Log("[error] '%s' data size mismatch!\n", utf16_to_utf8(fullPath));
			return false;
		}

		if (isCharacterIndexOutOfBound(fileContents[i].header.charIndex))
		{
			LOG(2, "ERROR, '%s' has invalid character index in the header\n", utf16_to_utf8(fullPath));
			g_imGuiLogger->Log("[error] '%s' has invalid character index in the header\n", utf16_to_utf8(fullPath));
			false;
		}
		else if (charIndex[i] != fileContents[i].header.charIndex)
		{
			LOG(2, "ERROR, '%s' belongs to character %s, but is placed in folder %s\n",
				utf16_to_utf8(fullPath), getCharacterNameByIndexA(fileContents[i].header.charIndex).c_str(),
				getCharacterNameByIndexA(charIndex[i]).c_str());

			g_imGuiLogger->Log("[error] '%s' belongs to character '%s', but is placed in folder '%s'\n",
				utf16_to_utf8(fullPath), getCharacterNameByIndexA(fileContents[i].header.charIndex).c_str(),
				getCharacterNameByIndexA(charIndex[i]).c_str());
			return false;
		}
	}
	
	p1PalData = fileContents[0].palData;
	p2PalData = fileContents[1].palData;
	return true;
}

void PaletteManager::LoadReplayPalettes(Player& playerOne, Player& playerTwo)
{
	if (m_replayFile.empty())
		return;

	std::wstring wReplayPalFile = FindReplayPaletteFileRecursive(std::wstring(L"ACPR_IM\\Replays\\*"), m_replayFile);
	if (wReplayPalFile.empty())
		return;

	IMPL_data_t p1PalData, p2PalData;

	if (LoadPrepFile(wReplayPalFile, (CharIndex)playerOne.GetData()->charIndex, (CharIndex)playerTwo.GetData()->charIndex,
		p1PalData, p2PalData))
	{
		CharPaletteHandle palHandle = playerOne.GetPalHandle();
		palHandle.ReplacePalData(&p1PalData);
		palHandle = playerTwo.GetPalHandle();
		palHandle.ReplacePalData(&p2PalData);
	}
}

std::vector<std::vector<IMPL_data_t>>& PaletteManager::GetCustomPalettesVector()
{
	return m_customPalettes;
}

void PaletteManager::SetDisableOnlineCustomPalettes(int player, bool value)
{
	m_disableOnlineCustomPalettes[player] = value;
}

bool PaletteManager::GetDisableOnlineCustomPalettes(int player)
{
	return m_disableOnlineCustomPalettes[player];
}

bool* PaletteManager::GetDisableOnlineCustomPalettesRef(int player)
{
	return &m_disableOnlineCustomPalettes[player];
}

void PaletteManager::SetGameMode(GameMode gameMode)
{
	m_currentGameMode = gameMode;
}

void PaletteManager::SetReplayFileName(wchar_t* lpFileName)
{
	m_replayFile = std::wstring(lpFileName);

	if (!lpFileName)
		return;

	int pos = m_replayFile.rfind(L"\\");

	if (pos != std::wstring::npos)
		m_replayFile = m_replayFile.substr(pos + 1);

	m_replayFile = m_replayFile.substr(0, m_replayFile.rfind('.'));
}