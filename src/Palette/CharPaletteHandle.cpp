#include "CharPaletteHandle.h"

#include "Core/logger.h"

#define BLOOM_PALETTE_INDEX 99

char* palFileNames[TOTAL_PALETTE_FILES] =
{
	"Character"/*,
	"Effect01",
	"Effect02",
	"Effect03",
	"Effect04",
	"Effect05",
	"Effect06",
	"Effect07"*/
};

char* CharPaletteHandle::GetPalFileAddr(const char* base, uint16_t palIndex, int fileID)
{
	//dereferencing the multi-level pointer:
	// [[[baseaddr + 0x4] + palIndex * 0x20] + fileID * 0x4] + 0x1C

	// Leave for debugging:
	//DWORD* deref1 = (DWORD*)base + 1;
	//DWORD* deref2 = (DWORD*)*deref1 + (palIndex * 8);
	//DWORD* deref2Offset = deref2 + + fileID;
	//DWORD* finalAddr = (DWORD*)*deref2Offset + 7;

	DWORD* paletteAddress = (DWORD*)(*(DWORD*)(base + 4 * palIndex)) + 4;
	
	return (char*)paletteAddress;
}

void CharPaletteHandle::SetPointerPalIndex(uint16_t* pPalIndex)
{
	m_pCurPalIndex = pPalIndex;
}

void CharPaletteHandle::SetPointerBasePal(char* pPalBaseAddr)
{
	m_pPalBaseAddr = pPalBaseAddr;
}

void CharPaletteHandle::SetPaletteIndex(uint16_t palIndex)
{
	if (palIndex < 0 || palIndex > MAX_PAL_INDEX)
		return;

	*m_pCurPalIndex = palIndex;
}

uint16_t CharPaletteHandle::GetOrigPalIndex() const
{
	return m_origPalIndex;
}

bool CharPaletteHandle::IsNullPointerPalBasePtr()
{
	return m_pPalBaseAddr == nullptr;
}

bool CharPaletteHandle::IsNullPointerPalIndex()
{
	return m_pCurPalIndex == nullptr;
}

uint16_t& CharPaletteHandle::GetPalIndexRef()
{
	return *m_pCurPalIndex;
}

void CharPaletteHandle::ReplacePalData(IMPL_data_t* newPaletteData)
{
	SetCurrentPalInfo(&newPaletteData->palInfo);
	ReplaceAllPalFiles(newPaletteData, *m_pCurPalIndex);
	//ReplaceAllPalFiles(newPaletteData, m_switchPalIndex2);

	UpdatePalette();
}

void CharPaletteHandle::ReplaceSinglePalFile(const char* newPalData, PaletteFile palFile)
{
	char* pDst1 = GetPalFileAddr(m_pPalBaseAddr, *m_pCurPalIndex, (int)palFile);
	//char* pDst2 = GetPalFileAddr(m_pPalBaseAddr, m_switchPalIndex2);
	ReplacePalArrayInMemory(pDst1, newPalData);
	//ReplacePalArrayInMemory(pDst2, newPalData);

	UpdatePalette();
}

void CharPaletteHandle::OnMatchInit()
{
	m_origPalIndex = *m_pCurPalIndex;

	BackupOrigPal();

	m_selectedCustomPalIndex = 0;

	m_switchPalIndex1 = *m_pCurPalIndex;
	
	m_switchPalIndex2 = m_switchPalIndex1 == MAX_PAL_INDEX
		? m_switchPalIndex1 - 1
		: m_switchPalIndex1 + 1;

	// Clear palette info
	IMPL_info_t palInfo = { "Default" };
	SetCurrentPalInfo(&palInfo);
}

void CharPaletteHandle::OnMatchRematch()
{
	// In case of a rematch we want to start with the original palindex
	*m_pCurPalIndex = m_origPalIndex;
}

void CharPaletteHandle::UnlockUpdate()
{
	m_updateLocked = false;
}

uint16_t CharPaletteHandle::GetSelectedCustomPalIndex()
{
	return m_selectedCustomPalIndex;
}

void CharPaletteHandle::SetSelectedCustomPalIndex(uint16_t index)
{
	m_selectedCustomPalIndex = index;
}

const char* CharPaletteHandle::GetCurPalFileAddr(PaletteFile palFile)
{
	return GetPalFileAddr(m_pPalBaseAddr, *m_pCurPalIndex, (int)palFile);
}

const char* CharPaletteHandle::GetOrigPalFileAddr(PaletteFile palFile)
{
	const char* ret = m_origPalBackup.file0;
	ret += palFile * IMPL_PALETTE_DATALEN;
	return ret;
}

const IMPL_info_t& CharPaletteHandle::GetCurrentPalInfo() const
{
	return m_currentPalData.palInfo;
}

void CharPaletteHandle::SetCurrentPalInfo(IMPL_info_t* pPalInfo)
{
	memcpy_s(&m_currentPalData.palInfo, sizeof(IMPL_info_t), pPalInfo, sizeof(IMPL_info_t));
}

const IMPL_data_t& CharPaletteHandle::GetCurrentPalData()
{
	for (int i = 0; i < TOTAL_PALETTE_FILES; i++)
	{
		char* pDst = m_currentPalData.file0 + (i * IMPL_PALETTE_DATALEN);
		const char* pSrc = GetPalFileAddr(m_pPalBaseAddr, *m_pCurPalIndex, i);
		memcpy(pDst, pSrc, IMPL_PALETTE_DATALEN);
		pDst += IMPL_PALETTE_DATALEN;
	}

	return m_currentPalData;
}

const IMPL_data_t& CharPaletteHandle::GetPalDataAtIndex(int index)
{
	IMPL_data_t palData = {};
	for (int i = 0; i < TOTAL_PALETTE_FILES; i++)
	{
		char* pDst = palData.file0 + (i * IMPL_PALETTE_DATALEN);
		const char* pSrc = GetPalFileAddr(m_pPalBaseAddr, index, i);
		memcpy(pDst, pSrc, IMPL_PALETTE_DATALEN);
		pDst += IMPL_PALETTE_DATALEN;
	}
	return palData;
}

bool CharPaletteHandle::IsCurrentPalWithBloom() const
{
	return m_origPalIndex == BLOOM_PALETTE_INDEX ||
		m_currentPalData.palInfo.hasBloom;
}

void CharPaletteHandle::ReplacePalArrayInMemory(char* Dst, const void* Src)
{
	// The palette datas are duplicated in the memory at offset 0x800
	// We have to overwrite both, as some characters use the duplication's address instead
	memcpy(Dst, Src, IMPL_PALETTE_DATALEN);
	//memcpy(Dst + 0x800, Src, IMPL_PALETTE_DATALEN);
}

void CharPaletteHandle::ReplaceAllPalFiles(IMPL_data_t* newPaletteData, uint16_t palIndex)
{
	static const char NULLBLOCK[IMPL_PALETTE_DATALEN]{ 0 };

	// If palname is "Default" then we load the original palette from backup
	if (strncmp(newPaletteData->palInfo.palName, "Default", IMPL_PALNAME_LENGTH) == 0)
		newPaletteData = &m_origPalBackup;

	for (int i = 0; i < TOTAL_PALETTE_FILES; i++)
	{
		const char* pSrc = newPaletteData->file0 + (i * IMPL_PALETTE_DATALEN);

		if (!memcmp(NULLBLOCK, pSrc, IMPL_PALETTE_DATALEN))
			continue;

		char* pDst = GetPalFileAddr(m_pPalBaseAddr, palIndex, i);
		ReplacePalArrayInMemory(pDst, pSrc);
	}
}

void CharPaletteHandle::BackupOrigPal()
{
	LOG(2, "CharPaletteHandle::BackupOrigPal\n");

	const char* pSrc = 0;
	char* pDst = m_origPalBackup.file0;

	for (int i = 0; i < TOTAL_PALETTE_FILES; i++)
	{
		pSrc = GetPalFileAddr(m_pPalBaseAddr, *m_pCurPalIndex, i);
		memcpy(pDst, pSrc, IMPL_PALETTE_DATALEN);
		pDst += IMPL_PALETTE_DATALEN;
	}

	strncpy(m_origPalBackup.palInfo.palName, "Default", IMPL_PALNAME_LENGTH);
}

void CharPaletteHandle::RestoreOrigPal()
{
	LOG(2, "CharPaletteHandle::RestoreOrigPalette\n");

	//ReplacePalData(&m_origPalBackup);
	IMPL_data_t* newPaletteData = &m_origPalBackup;

	SetCurrentPalInfo(&newPaletteData->palInfo);
	ReplaceAllPalFiles(newPaletteData, m_origPalIndex);
	//ReplaceAllPalFiles(newPaletteData, m_switchPalIndex2);

	UpdatePalette();
}

void CharPaletteHandle::UpdatePalette()
{
	// Must not switch more than once per frame, or palette doesn't get updated!
	//if (m_updateLocked)
	//	return;

	/*m_pCurPalIndex = *m_pCurPalIndex == m_switchPalIndex1
		? m_switchPalIndex2
		: m_switchPalIndex1;*/

	m_requiresUpdate = true;

	//LockUpdate();
}

void CharPaletteHandle::LockUpdate()
{
	m_updateLocked = true;
}

bool CharPaletteHandle::RequiresUpdate()
{
	return m_requiresUpdate;
}

void CharPaletteHandle::PaletteUpdated()
{
	m_requiresUpdate = false;
}

bool CharPaletteHandle::IsCustomPalette()
{
	for (int i = 0; i < IMPL_PALETTE_DATALEN; i++)
	{
		if (m_currentPalData.file0[i] != m_origPalBackup.file0[i])
			return true;
	}
	return false;
}