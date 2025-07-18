#pragma once

#include "RoomManager.h"

#include "Palette/PaletteManager.h"

#include <queue>
#include <array>

class OnlinePaletteManager
{
public:
	OnlinePaletteManager(PaletteManager* pPaletteManager, CharPaletteHandle* pP1CharPalHandle,
		CharPaletteHandle* pP2CharPalHandle, RoomManager* pRoomManager);
	void SendPalettePackets();
	void SendPalettePackets(CSteamID steamID);
	void RecvPaletteDataPacket(Packet* packet);
	void RecvPaletteInfoPacket(Packet* packet);
	void ProcessSavedPalettePackets();
	void ClearSavedPalettePacketQueues();
	void OnMatchInit(CharIndex p1Idx, CharIndex p2Idx);
	bool* GetSharePalettesOnlineRef();

private:
	void SendPaletteInfoPacket(CharPaletteHandle& charPalHandle, uint64_t steamID);
	void SendPaletteDataPackets(CharPaletteHandle& charPalHandle, uint64_t steamID);
	void SendPaletteInfoPacket(CharPaletteHandle& charPalHandle, uint64_t steamID, CSteamID otherID);
	void SendPaletteDataPackets(CharPaletteHandle& charPalHandle, uint64_t steamID, CSteamID otherID);
	void ProcessSavedPaletteInfoPackets();
	void ProcessSavedPaletteDataPackets();
	CharPaletteHandle& GetPlayerCharPaletteHandle(uint16_t matchPlayerIndex);
	void SaveRecievedPaletteToFile(CharPaletteHandle& charPalHandle, CharIndex charIndex, uint64_t steamID);

	struct UnprocessedPaletteInfo
	{
		uint16_t matchPlayerIndex;
		IMPL_info_t palInfo;

		UnprocessedPaletteInfo(uint16_t matchPlayerIndex_, IMPL_info_t* pPalInfo)
			: matchPlayerIndex(matchPlayerIndex_)
		{
			memcpy_s(&palInfo, sizeof(IMPL_info_t), pPalInfo, sizeof(IMPL_info_t));
		}
	};

	struct UnprocessedPaletteFile
	{
		uint16_t matchPlayerIndex;
		PaletteFile palFile;
		char palData[IMPL_PALETTE_DATALEN];

		UnprocessedPaletteFile(uint16_t matchPlayerIndex_, PaletteFile palFile_, char* pPalSrc)
			: matchPlayerIndex(matchPlayerIndex_), palFile(palFile_)
		{
			memcpy_s(palData, IMPL_PALETTE_DATALEN, pPalSrc, IMPL_PALETTE_DATALEN);
		}
	};

	std::queue<UnprocessedPaletteInfo> m_unprocessedPaletteInfos;
	std::queue<UnprocessedPaletteFile> m_unprocessedPaletteFiles;
	std::array<CharIndex, 2> m_charIndices;
	bool m_sharePalettesOnline = true;

	CharPaletteHandle* m_pP1CharPalHandle;
	CharPaletteHandle* m_pP2CharPalHandle;

	// Interfaces
	PaletteManager* m_pPaletteManager;
	RoomManager* m_pRoomManager;
};