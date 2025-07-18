#pragma once
#include "Palette/impl_format.h"
#include "Game/characters.h"
#include <cstdint>

#define MAX_PAL_INDEX 21
#define TOTAL_PALETTE_FILES 1

extern char* palFileNames[TOTAL_PALETTE_FILES];

enum PaletteFile
{
	PaletteFile_Character,
	PaletteFile_Effect1,
	PaletteFile_Effect2,
	PaletteFile_Effect3,
	PaletteFile_Effect4,
	PaletteFile_Effect5,
	PaletteFile_Effect6,
	PaletteFile_Effect7
};

class CharPaletteHandle
{
	friend class PaletteManager;

	uint16_t* m_pCurPalIndex;
	uint16_t m_origPalIndex;
	const char* m_pPalBaseAddr;
	IMPL_data_t m_origPalBackup;
	IMPL_data_t m_currentPalData;
	int m_switchPalIndex1;
	int m_switchPalIndex2;
	uint16_t m_selectedCustomPalIndex;
	bool m_updateLocked;
	bool m_requiresUpdate = false;

public:
	void SetPointerPalIndex(uint16_t* pPalIdx);
	void SetPointerBasePal(char* pPalBaseAddr);
	bool IsNullPointerPalBasePtr();
	bool IsNullPointerPalIndex();
	uint16_t& GetPalIndexRef();
	uint16_t GetOrigPalIndex() const;
	bool IsCurrentPalWithBloom() const;
	bool RequiresUpdate();
	void PaletteUpdated();
	bool IsCustomPalette();

private:
	void SetPaletteIndex(uint16_t palIndex);
	void ReplacePalData(IMPL_data_t* newPaletteData);
	void OnMatchInit();
	void OnMatchRematch();
	void LockUpdate();
	void UnlockUpdate();
	uint16_t GetSelectedCustomPalIndex();
	void SetSelectedCustomPalIndex(uint16_t index);
	const char* GetCurPalFileAddr(PaletteFile palFile);
	const char* GetOrigPalFileAddr(PaletteFile palFile);
	const IMPL_info_t& GetCurrentPalInfo() const;
	void SetCurrentPalInfo(IMPL_info_t* pPalInfo);
	const IMPL_data_t& GetCurrentPalData();
	const IMPL_data_t& GetPalDataAtIndex(int index);
	char* GetPalFileAddr(const char* base, uint16_t palIdx, int fileID);
	void ReplacePalArrayInMemory(char* Dst, const void* Src);
	void ReplaceSinglePalFile(const char* newPalData, PaletteFile palFile);
	void ReplaceAllPalFiles(IMPL_data_t* newPaletteData, uint16_t palIdx);
	void BackupOrigPal();
	void RestoreOrigPal();
	void UpdatePalette();
};