#include "OnlinePaletteManager.h"

#include "Palette/impl_format.h"

#include "Core/logger.h"

OnlinePaletteManager::OnlinePaletteManager(PaletteManager* pPaletteManager, CharPaletteHandle* pP1CharPalHandle,
	CharPaletteHandle* pP2CharPalHandle, RoomManager* pRoomManager)
	: m_pPaletteManager(pPaletteManager), m_pP1CharPalHandle(pP1CharPalHandle), 
	m_pP2CharPalHandle(pP2CharPalHandle), m_pRoomManager(pRoomManager)
{
}

void OnlinePaletteManager::SendPalettePackets()
{
	LOG(2, "OnlinePaletteManager::SendPalettePackets\n");

	if (m_pRoomManager->IsThisPlayerSpectator() || !m_sharePalettesOnline)
		return;

	uint16_t thisPlayerMatchPlayerIndex = m_pRoomManager->GetThisPlayerMatchPlayerIndex();
	CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(thisPlayerMatchPlayerIndex);

	SendPaletteInfoPacket(charPalHandle, m_pRoomManager->GetThisPlayerSteamID());
	SendPaletteDataPackets(charPalHandle, m_pRoomManager->GetThisPlayerSteamID());
}

void OnlinePaletteManager::SendPalettePackets(CSteamID id)
{
	LOG(2, "OnlinePaletteManager::SendPalettePackets\n");

	if (m_pRoomManager->IsThisPlayerSpectator() || !m_sharePalettesOnline)
		return;

	uint16_t thisPlayerMatchPlayerIndex = m_pRoomManager->GetThisPlayerMatchPlayerIndex();
	CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(thisPlayerMatchPlayerIndex);

	SendPaletteInfoPacket(charPalHandle, m_pRoomManager->GetThisPlayerSteamID(), id);
	SendPaletteDataPackets(charPalHandle, m_pRoomManager->GetThisPlayerSteamID(), id);
}

void OnlinePaletteManager::RecvPaletteDataPacket(Packet* packet)
{
	LOG(2, "OnlinePaletteManager::RecvPaletteDataPacket\n");

	uint16_t matchPlayerIndex = m_pRoomManager->GetPlayerMatchPlayerIndexBySteamID(packet->steamID);
	CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(matchPlayerIndex);

	if (charPalHandle.IsNullPointerPalBasePtr())
	{
		m_unprocessedPaletteFiles.push(UnprocessedPaletteFile(matchPlayerIndex, (PaletteFile)packet->part, (char*)packet->data));
		return;
	}

	if (!m_pPaletteManager->GetDisableOnlineCustomPalettes(matchPlayerIndex))
	{
		m_pPaletteManager->ReplacePaletteFile((const char*)packet->data, (PaletteFile)packet->part, charPalHandle);
		SaveRecievedPaletteToFile(charPalHandle, m_charIndices[matchPlayerIndex], packet->steamID);
	}
}

void OnlinePaletteManager::RecvPaletteInfoPacket(Packet* packet)
{
	LOG(2, "OnlinePaletteManager::RecvPaletteInfoPacket\n");

	uint16_t matchPlayerIndex = m_pRoomManager->GetPlayerMatchPlayerIndexBySteamID(packet->steamID);
	CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(matchPlayerIndex);

	if (charPalHandle.IsNullPointerPalBasePtr())
	{
		m_unprocessedPaletteInfos.push(UnprocessedPaletteInfo(matchPlayerIndex, (IMPL_info_t*)packet->data));
		return;
	}

	if (!m_pPaletteManager->GetDisableOnlineCustomPalettes(matchPlayerIndex))
		m_pPaletteManager->SetCurrentPalInfo(charPalHandle, *(IMPL_info_t*)packet->data);
}

void OnlinePaletteManager::ProcessSavedPalettePackets()
{
	LOG(2, "OnlinePaletteManager::ProcessSavedPalettePackets\n");

	if (!m_pRoomManager->IsRoomFunctional())
		return;

	ProcessSavedPaletteInfoPackets();
	ProcessSavedPaletteDataPackets();
}

void OnlinePaletteManager::ClearSavedPalettePacketQueues()
{
	LOG(2, "OnlinePaletteManager::ClearSavedPalettePacketQueues\n");

	m_unprocessedPaletteInfos = {};
	m_unprocessedPaletteFiles = {};
}

void OnlinePaletteManager::OnMatchInit(CharIndex p1Idx, CharIndex p2Idx)
{
	LOG(2, "OnlinePaletteManager::OnMatchInit\n");

	m_charIndices[0] = p1Idx;
	m_charIndices[1] = p2Idx;

	SendPalettePackets();
	ProcessSavedPalettePackets();
}

void OnlinePaletteManager::SendPaletteInfoPacket(CharPaletteHandle& charPalHandle, uint64_t steamID)
{
	LOG(2, "OnlinePaletteManager::SendPaletteInfoPacket\n");

	Packet packet = Packet(
		(char*)&m_pPaletteManager->GetCurrentPalInfo(charPalHandle),
		(uint16_t)sizeof(IMPL_info_t),
		PacketType_PaletteInfo,
		steamID
	);

	m_pRoomManager->SendPacketToSameMatchIMPlayers(&packet);
}

void OnlinePaletteManager::SendPaletteInfoPacket(CharPaletteHandle& charPalHandle, uint64_t steamID, CSteamID otherID)
{
	LOG(2, "OnlinePaletteManager::SendPaletteInfoPacket\n");

	Packet packet = Packet(
		(char*)&m_pPaletteManager->GetCurrentPalInfo(charPalHandle),
		(uint16_t)sizeof(IMPL_info_t),
		PacketType_PaletteInfo,
		steamID
	);

	m_pRoomManager->SendPacketToPlayerBySteamID(&packet, otherID);
}

void OnlinePaletteManager::SendPaletteDataPackets(CharPaletteHandle& charPalHandle, uint64_t steamID)
{
	LOG(2, "OnlinePaletteManager::SendPaletteDataPackets\n");

	for (int palFileIndex = 0; palFileIndex < IMPL_PALETTE_FILES_COUNT; palFileIndex++)
	{
		const char* palAddr = m_pPaletteManager->GetCurPalFileAddr((PaletteFile)palFileIndex, charPalHandle);

		Packet packet = Packet(
			(char*)palAddr,
			(uint16_t)IMPL_PALETTE_DATALEN,
			PacketType_PaletteData,
			steamID,
			palFileIndex
		);

		m_pRoomManager->SendPacketToSameMatchIMPlayers(&packet);
	}
}

void OnlinePaletteManager::SendPaletteDataPackets(CharPaletteHandle& charPalHandle, uint64_t steamID, CSteamID otherID)
{
	LOG(2, "OnlinePaletteManager::SendPaletteDataPackets\n");

	for (int palFileIndex = 0; palFileIndex < IMPL_PALETTE_FILES_COUNT; palFileIndex++)
	{
		const char* palAddr = m_pPaletteManager->GetCurPalFileAddr((PaletteFile)palFileIndex, charPalHandle);

		Packet packet = Packet(
			(char*)palAddr,
			(uint16_t)IMPL_PALETTE_DATALEN,
			PacketType_PaletteData,
			steamID,
			palFileIndex
		);

		m_pRoomManager->SendPacketToPlayerBySteamID(&packet, otherID);
	}
}

void OnlinePaletteManager::ProcessSavedPaletteInfoPackets()
{
	LOG(2, "OnlinePaletteManager::ProcessSavedPaletteInfoPackets\n");

	for (int i = 0; i < m_unprocessedPaletteInfos.size(); i++)
	{
		UnprocessedPaletteInfo& palInfo = m_unprocessedPaletteInfos.front();

		CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(palInfo.matchPlayerIndex);

		if (!m_pPaletteManager->GetDisableOnlineCustomPalettes(palInfo.matchPlayerIndex))
			m_pPaletteManager->SetCurrentPalInfo(charPalHandle, palInfo.palInfo);

		m_unprocessedPaletteInfos.pop();
	}
}

void OnlinePaletteManager::ProcessSavedPaletteDataPackets()
{
	LOG(2, "OnlinePaletteManager::ProcessSavedPaletteDataPackets\n");

	for (int i = 0; i < m_unprocessedPaletteFiles.size(); i++)
	{
		UnprocessedPaletteFile& palfile = m_unprocessedPaletteFiles.front();

		CharPaletteHandle& charPalHandle = GetPlayerCharPaletteHandle(palfile.matchPlayerIndex);

		if (!m_pPaletteManager->GetDisableOnlineCustomPalettes(palfile.matchPlayerIndex))
			m_pPaletteManager->ReplacePaletteFile(palfile.palData, palfile.palFile, charPalHandle);

		m_unprocessedPaletteFiles.pop();
	}
}

CharPaletteHandle& OnlinePaletteManager::GetPlayerCharPaletteHandle(uint16_t matchPlayerIndex)
{
	return matchPlayerIndex == 0 ? *m_pP1CharPalHandle : *m_pP2CharPalHandle;
}

std::string PalIndexToStr(int idx)
{
	switch (idx)
	{
	case 0:
		return "P";
	case 1:
		return "K";
	case 2:
		return "S";
	case 3:
		return "H";
	case 4:
		return "D";
	case 5:
		return "ExP";
	case 6:
		return "ExK";
	case 7:
		return "ExS";
	case 8:
		return "ExH";
	case 9:
		return "ExD";
	case 10:
		return "SlashP";
	case 11:
		return "SlashK";
	case 12:
		return "SlashS";
	case 13:
		return "SlashH";
	case 14:
		return "Gold";
	case 15:
		return "ReloadP";
	case 16:
		return "ReloadK";
	case 17:
		return "ReloadS";
	case 18:
		return "ReloadH";
	case 19:
		return "Shadow";
	case 20:
		return "SlashD";
	case 21:
		return "ReloadD";
	default:
		return "Unknown";
	}
}

void OnlinePaletteManager::SaveRecievedPaletteToFile(CharPaletteHandle& charPalHandle, CharIndex charIndex, uint64_t steamID)
{
	IMPL_data_t curPalData = m_pPaletteManager->GetCurrentPalData(charPalHandle);

	std::string palName(curPalData.palInfo.palName);
	bool palmod = false;
	if (palName == "Default")
	{
		if (!charPalHandle.IsCustomPalette())
			return;
		palName += PalIndexToStr(charPalHandle.GetOrigPalIndex());
		palmod = true;
	}

	std::string palCreator(curPalData.palInfo.creator);
	if (palCreator == "")
		palCreator = std::string(m_pRoomManager->GetPlayerSteamName(steamID));
	std::string fileName = palmod ? palCreator + "-" + palName : palName;

	sprintf(curPalData.palInfo.creator, palCreator.c_str());


	m_pPaletteManager->WriteOnlinePaletteToFile(fileName, charIndex , &curPalData);
	
}

bool* OnlinePaletteManager::GetSharePalettesOnlineRef()
{
	return &m_sharePalettesOnline;
}