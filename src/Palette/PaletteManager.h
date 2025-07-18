#pragma once
#include "impl_format.h"

#include "CharPaletteHandle.h"

#include "Game/characters.h"
#include "Game/Player.h"
#include "Game/gamestates.h"

#include <array>
#include <vector>

class PaletteManager
{
public:
	PaletteManager();
	~PaletteManager();
	std::vector<std::vector<IMPL_data_t>> &GetCustomPalettesVector();

	bool PushImplFileIntoVector(IMPL_t &filledPal);
	bool PushImplFileIntoVector(CharIndex charIndex, IMPL_data_t &filledPalData);
	bool WritePaletteToFile(CharIndex charIndex, IMPL_data_t *filledPalData);
	bool WriteOnlinePaletteToFile(std::string fileName, CharIndex charIndex, IMPL_data_t* filledPalData);

	void LoadAllPalettes();
	void ReloadAllPalettes();

	int GetOnlinePalsStartIndex(CharIndex charIndex);
	void OverwriteIMPLDataPalName(std::string fileName, IMPL_data_t& palData);

	// Return values:
	// ret > 0, index found
	// ret == -1, index not found
	// ret == -2, charindex out of bound
	// ret == -3, default palette or no name given
	int FindCustomPalIndex(CharIndex charIndex, const char* palNameToFind);
	bool PaletteArchiveDownloaded();
	bool SwitchPalette(CharIndex charIndex, CharPaletteHandle& palHandle, int newCustomPalIndex);
	void ReplacePaletteFile(const char* newPalData, PaletteFile palFile, CharPaletteHandle& palHandle);
	void RestoreOrigPal(CharPaletteHandle& palHandle);
	const char* GetCurPalFileAddr(PaletteFile palFile, CharPaletteHandle& palHandle);
	const char* GetCustomPalFile(CharIndex charIndex, int palIndex, PaletteFile palFile, CharPaletteHandle& palHandle);
	int GetCurrentCustomPalIndex(CharPaletteHandle& palHandle) const;
	const IMPL_info_t& GetCurrentPalInfo(CharPaletteHandle& palHandle) const;
	void SetCurrentPalInfo(CharPaletteHandle& palHandle, IMPL_info_t& palInfo);
	const IMPL_data_t& GetCurrentPalData(CharPaletteHandle& palHandle);
	void LoadPaletteSettingsFile();

	// Call it ONCE per frame
	void OnUpdate(CharPaletteHandle& P1, CharPaletteHandle& P2);

	// Call it ONCE upon match start
	void OnMatchInit(Player& playerOne, Player& playerTwo);

	void OnMatchRematch(Player& playerOne, Player& playerTwo);
	void OnMatchEnd(CharPaletteHandle& playerOne, CharPaletteHandle& playerTwo);

	void SaveReplayPalettes(Player& playerOne, Player& playerTwo);
	void LoadReplayPalettes(Player& playerOne, Player& playerTwo);

	void SetDisableOnlineCustomPalettes(int player, bool value);
	bool GetDisableOnlineCustomPalettes(int player);
	bool* GetDisableOnlineCustomPalettesRef(int player);

	void SetGameMode(GameMode gameMode);
	void SetReplayFileName(wchar_t* lpFileName);

private:
	std::vector<std::vector<IMPL_data_t>> m_customPalettes;
	std::vector<std::vector<std::string>> m_paletteSlots;
	std::vector<int> m_onlinePalsStartIndex;
	bool m_loadOnlinePalettes = false;
	bool m_PaletteArchiveDownloaded = false;
	std::array<bool, 2> m_disableOnlineCustomPalettes = { false, false };
	GameMode m_currentGameMode;
	std::wstring m_replayFile;

	void CreatePaletteFolders();
	void InitCustomPaletteVector();
	void LoadPalettesIntoVector(CharIndex charIndex, std::wstring& wFolderPath);
	void LoadPalettesFromFolder();
	void LoadImplFile(const std::string& fullPath, const std::string& fileName, CharIndex charIndex);
	void LoadHplFile(const std::string& fullPath, const std::string& fileName, CharIndex charIndex);
	void InitPaletteSlotsVector();
	void InitOnlinePalsIndexVector();
	void ApplyDefaultCustomPalette(CharIndex charIndex, CharPaletteHandle& charPalHandle);
	std::wstring FindReplayPaletteFileRecursive(std::wstring& wFolderPath, std::wstring& wReplayName);
	bool LoadPrepFile(const std::wstring& fullPath, CharIndex p1Index, CharIndex p2Index, IMPL_data_t& p1PalData, IMPL_data_t& p2PalData);
};