//#include "RoomManager.h"
#include "Core/interfaces.h"
#include "Core/logger.h"

RoomManager::RoomManager(NetworkManager* pNetworkManager, CSteamID steamID)
	: m_pNetworkManager(pNetworkManager),
	m_thisPlayerSteamID(steamID), m_pFFAThisPlayerIndex(nullptr), m_pRoom(nullptr)
{
	m_imPlayers.resize(250);
	m_imSpectators.resize(250);
}

RoomManager::~RoomManager() {}

void RoomManager::SendAnnounce()
{
	LOG(2, "RoomManager::SendAnnounce\n");
	char name[32];
	strcpy(name, GetThisPlayerSteamName());
	Packet packet = Packet((void*)name, strlen(name), PacketType_IMID_Announce, m_thisPlayerSteamID.ConvertToUint64());

	// Send to every other player in the room
	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (pRoomMemberEntry && !IsThisPlayer(pRoomMemberEntry->steamId))
		{
			m_pNetworkManager->SendPacket(&CSteamID(pRoomMemberEntry->steamId), &packet);
		}
	}
}

void RoomManager::AnnounceLeave()
{
	LOG(2, "RoomManager::AnnounceLeave\n");

	Packet packet = Packet(NULL, NULL, PacketType_IMID_LeaveMatch, m_thisPlayerSteamID.ConvertToUint64());

	for (IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		m_pNetworkManager->SendPacket(&imPlayer.steamID, &packet);
	}
}

void RoomManager::SendAcknowledge(Packet* packet)
{
	LOG(2, "RoomManager::SendAcknowledge\n");

	IMPlayer otherPlayer = IMPlayer(packet->steamID, (const char*)packet->data, packet->dataSize);
	AddIMPlayerToRoom(otherPlayer);

	char name[32];
	strcpy(name, GetThisPlayerSteamName());
	Packet ackPacket = Packet((void*)name, strlen(name), PacketType_IMID_Acknowledge, m_thisPlayerSteamID.ConvertToUint64());
	m_pNetworkManager->SendPacket(&otherPlayer.steamID, &ackPacket);
}

void RoomManager::AcceptAcknowledge(Packet* packet)
{
	LOG(2, "RoomManager::AcceptAcknowledge\n");

	IMPlayer otherPlayer = IMPlayer(packet->steamID, (const char*)packet->data, packet->dataSize);
	AddIMPlayerToRoom(otherPlayer);
}

void RoomManager::AcknowledgeSpectate(Packet* packet)
{
	LOG(2, "RoomManager::AcknowledgeSpectate\n");

	if (IsThisPlayer(packet->steamID))
		return;

	IMPlayer otherPlayer = IMPlayer(packet->steamID, (const char*)packet->data, packet->dataSize);

	char name[32];
	strcpy(name, GetThisPlayerSteamName());
	Packet ackPacket = Packet((void*)name, strlen(name), PacketType_IMID_PlayerInfo, m_thisPlayerSteamID.ConvertToUint64(), 0,
		GetThisPlayerMatchPlayerIndex());
	m_pNetworkManager->SendPacket(&otherPlayer.steamID, &ackPacket);

	uint64_t opponentID = GetOpponentRoomMemberEntry()->steamId;
	for (const IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		if (!imPlayer.active || IsThisPlayer(imPlayer.steamID.ConvertToUint64()))
			continue;

		if (imPlayer.steamID == opponentID)
		{
			strcpy(name, GetPlayerSteamName(GetOpponentRoomMemberEntry()->steamId));
			ackPacket = Packet((void*)name, strlen(name), PacketType_IMID_PlayerInfo, GetOpponentRoomMemberEntry()->steamId, 1,
				GetOpponentRoomMemberEntry()->matchPlayerIndex);
			m_pNetworkManager->SendPacket(&otherPlayer.steamID, &ackPacket);
		}
	}

	int i = 2;
	for (const IMPlayer& imPlayer : m_imSpectators)
	{
		if (!imPlayer.active)
			continue;
		strcpy(name, imPlayer.steamName.c_str());
		ackPacket = Packet((void*)name, strlen(name), PacketType_IMID_PlayerInfo, imPlayer.steamID.ConvertToUint64(), i++, 2);
		m_pNetworkManager->SendPacket(&otherPlayer.steamID, &ackPacket);
	}

	for (IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		if (!imPlayer.active || IsThisPlayer(imPlayer.steamID.ConvertToUint64()) || imPlayer.steamID == opponentID)
			continue;
		strcpy(name, imPlayer.steamName.c_str());
		ackPacket = Packet((void*)name, strlen(name), PacketType_IMID_PlayerInfo, otherPlayer.steamID.ConvertToUint64(), 0, 2);
		m_pNetworkManager->SendPacket(&imPlayer.steamID, &ackPacket);
	}

	AddIMPlayerToSpectators(otherPlayer);
	g_interfaces.pOnlinePaletteManager->SendPalettePackets(otherPlayer.steamID);
}

void RoomManager::RecvPlayerInfo(Packet* packet)
{
	LOG(2, "RoomManager::RecvPlayerInfo\n");

	IMPlayer otherPlayer = IMPlayer(packet->steamID, (const char*)packet->data, packet->dataSize);
	otherPlayer.matchPlayerIndex = packet->matchIndex;

	AddIMPlayerToSpectators(otherPlayer);
	g_interfaces.pOnlinePaletteManager->SendPalettePackets(otherPlayer.steamID);
}

void RoomManager::AcceptLeave(Packet* packet)
{
	LOG(2, "RoomManager::AcceptLeave\n");
	RemoveIMPlayerFromSpectators(packet->steamID);
}

void RoomManager::JoinRoom()
{
	LOG(2, "RoomManager::JoinRoom\n");

	//m_pRoom = pRoom;

	m_imPlayers.clear();
	m_imPlayers.resize(g_gameVals.pRoom->capacity);

	IMPlayer thisPlayer = IMPlayer(m_thisPlayerSteamID.ConvertToUint64(), GetPlayerSteamName(m_thisPlayerSteamID.ConvertToUint64()), 32);
	AddIMPlayerToRoom(thisPlayer);

	SendAnnounce();
}

bool RoomManager::IsRoomFunctional() const
{
	LOG(7, "RoomManager::IsRoomFunctional\n");

	bool result = g_gameVals.pRoom != nullptr && g_gameVals.pRoom->status != 0;
	//return m_pRoom != nullptr && m_pRoom->roomStatus == RoomStatus_Functional;

	return result;
}

void RoomManager::OnMatchInit()
{
	m_imSpectators.clear();
	m_imSpectators.resize(g_gameVals.pRoom->capacity);
}

void RoomManager::OnMatchEnd()
{
	m_imSpectators.clear();
	m_imSpectators.resize(g_gameVals.pRoom->capacity);
}

void RoomManager::SendPacketToSameMatchIMPlayers(Packet* packet)
{
	LOG(2, "RoomManager::SendPacketToSameMatchIMPlayers\n");

	//packet->roomMemberIndex = GetThisPlayerRoomMemberIndex();

	for (IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		// Remove from IM users list if player has left the room
		/*if (!IsPlayerInRoom(imPlayer))
		{
			RemoveIMPlayerFromRoom(imPlayer.steamID.ConvertToUint64());
			continue;
		}*/

		// Send to all other IM players
		if (!IsThisPlayer(imPlayer.steamID.ConvertToUint64()))
		{
			m_pNetworkManager->SendPacket(&imPlayer.steamID, packet);
		}
	}
}

void RoomManager::SendPacketToPlayerBySteamID(Packet* packet, CSteamID id)
{
	LOG(2, "RoomManager::SendPacketToPlayerBySteamID\n");

	for (IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		// Send to all other IM players
		if (!IsThisPlayer(imPlayer.steamID.ConvertToUint64()) &&
			imPlayer.steamID == id)
		{
			m_pNetworkManager->SendPacket(&imPlayer.steamID, packet);
		}
	}
}

void RoomManager::RequestPaletteResend()
{
	LOG(2, "RoomManager::RequestPacketResend\n");

	Packet packet = Packet(NULL, NULL, PacketType_RequestPalette, GetThisPlayerSteamID());

	for (IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		// Send to all other IM players
		if (!IsThisPlayer(imPlayer.steamID.ConvertToUint64()) && imPlayer.matchPlayerIndex < 2)
		{
			m_pNetworkManager->SendPacket(&imPlayer.steamID, &packet);
		}
	}
}

bool RoomManager::IsPacketFromSameRoom(Packet* packet) const
{
	LOG(7, "RoomManager::IsPacketFromSameRoom\n");

	if (!IsRoomFunctional())
		return false;

	return IsPlayerInRoom(IMPlayer(packet->steamID, GetPlayerSteamName(packet->steamID), 32));
}

bool RoomManager::IsPacketFromSameMatchNonSpectator(Packet* packet) const
{
	LOG(7, "RoomManager::IsPacketFromSameMatchNonSpectator\n");

	return IsPacketFromSameMatch(packet) && !IsPacketFromSpectator(packet);
}

bool RoomManager::IsThisPlayerSpectator() const
{
	LOG(7, "RoomManager::IsThisPlayerSpectator\n");

	const RoomMemberEntry* thisPlayerMemberEntry = GetThisPlayerRoomMemberEntry();

	if (!thisPlayerMemberEntry)
		return false;

	return *g_gameVals.pSpectatingTest1 | *g_gameVals.pSpectatingTest2;
}

bool RoomManager::IsThisPlayerAutoSpectator() const
{
	LOG(7, "RoomManager::IsThisPlayerAutoSpectator\n");

	const RoomMemberEntry* thisPlayerMemberEntry = GetThisPlayerRoomMemberEntry();

	if (!thisPlayerMemberEntry)
		return true;

	return thisPlayerMemberEntry->memberIndex == 0;
}

bool RoomManager::IsThisPlayerInMatch() const
{
	LOG(7, "RoomManager::IsThisPlayerInMatch\n");

	const RoomMemberEntry* thisPlayerMemberEntry = GetThisPlayerRoomMemberEntry();
	//int matchId = thisPlayerMemberEntry ? thisPlayerMemberEntry->matchId : 0;

	if (!thisPlayerMemberEntry)
		return false;
	return thisPlayerMemberEntry->status == 3 || IsThisPlayerSpectator(); //matchId != 0;
}

void RoomManager::SetFFAThisPlayerIndex(int* pFFAThisPlayerIndex)
{
	LOG(7, "RoomManager::SetFFAThisPlayerIndex\n");

	m_pFFAThisPlayerIndex = pFFAThisPlayerIndex;
}

RoomPlayerMode RoomManager::GetThisPlayerGameMode() const
{
	LOG(7, "RoomManager::GetThisPlayerGameMode\n");

	const RoomMemberEntry* pRoomMemberEntry = GetThisPlayerRoomMemberEntry();
	if (!pRoomMemberEntry)
		return RoomPlayerMode_None;

	return pRoomMemberEntry->mode;
}

uint16_t RoomManager::GetThisPlayerMatchPlayerIndex() const
{
	LOG(7, "RoomManager::GetThisPlayerMatchPlayerIndex\n");

	const RoomMemberEntry* pRoomMemberEntry = GetThisPlayerRoomMemberEntry();
	if (!pRoomMemberEntry)
		return 0xFFFF;

	return pRoomMemberEntry->matchPlayerIndex;
}

uint16_t RoomManager::GetPlayerMatchPlayerIndexBySteamID(uint64_t steamID) const
{
	LOG(7, "RoomManager::GetPlayerMatchPlayerIndexBySteamID\n");

	const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryBySteamID(steamID);
	if (!pRoomMemberEntry)
		return 0xFFFF;

	return pRoomMemberEntry->matchPlayerIndex;
}

const char* RoomManager::GetPlayerSteamName(uint64_t steamID) const
{
	LOG(7, "RoomManager::GetPlayerSteamName\n");

	for (const IMPlayer& imPlayer : m_imPlayers)
	{
		if (!imPlayer.active)
			continue;
		if (steamID == imPlayer.steamID.ConvertToUint64())
			return imPlayer.steamName.c_str();
	}

	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (pRoomMemberEntry && pRoomMemberEntry->steamId == steamID)
		{
			return pRoomMemberEntry->playerName;
		}
	}
	return "Unknown User";
}

const char* RoomManager::GetThisPlayerSteamName() const
{
	return GetPlayerSteamName(m_thisPlayerSteamID.ConvertToUint64());
}

const std::string RoomManager::GetRoomTypeName() const
{
	return "Free-for-All Room";
}

std::vector<IMPlayer> RoomManager::GetIMPlayersInCurrentMatch() const
{
	LOG(7, "RoomManager::GetIMPlayersInCurrentMatch\n");

	std::vector<IMPlayer> currentMatchIMPlayers;
	//uint32_t matchId = GetThisPlayerRoomMemberEntry()->matchId;

	if (!IsThisPlayerSpectator())
	{
		const RoomMemberEntry* thisPlayerEntry = GetThisPlayerRoomMemberEntry();
		const RoomMemberEntry* opponentEntry = GetOpponentRoomMemberEntry();

		for (const IMPlayer& imPlayer : m_imPlayers)
		{
			if (!imPlayer.active)
				continue;

			if (thisPlayerEntry && imPlayer.steamID.ConvertToUint64() == thisPlayerEntry->steamId)
				currentMatchIMPlayers.push_back(
					IMPlayer(imPlayer, GetPlayerMatchPlayerIndexBySteamID(thisPlayerEntry->steamId)));
			if (opponentEntry && imPlayer.steamID.ConvertToUint64() == opponentEntry->steamId)
				currentMatchIMPlayers.push_back(
					IMPlayer(imPlayer, GetPlayerMatchPlayerIndexBySteamID(opponentEntry->steamId)));
		}
	}
	for (const IMPlayer& imPlayer : m_imSpectators)
	{
		if (!imPlayer.active)
			continue;
		currentMatchIMPlayers.push_back(IMPlayer(imPlayer, imPlayer.matchPlayerIndex));
	}

	return currentMatchIMPlayers;
}

std::vector<IMPlayer> RoomManager::GetIMPlayersInCurrentRoom() const
{
	LOG(7, "RoomManager::GetIMPlayersInCurrentRoom\n");

	std::vector<IMPlayer> currentRoomIMPlayers;

	for (const IMPlayer& imPlayer : m_imPlayers)
	{
		if (!imPlayer.active)
			continue;
		
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryBySteamID(imPlayer.steamID.ConvertToUint64());

		if (!pRoomMemberEntry)
			continue;

		currentRoomIMPlayers.push_back(imPlayer);
	}

	return currentRoomIMPlayers;
}

std::vector<const RoomMemberEntry*> RoomManager::GetOtherRoomMemberEntriesInCurrentMatch() const
{
	std::vector<const RoomMemberEntry*> currentMatchRoomMembers;

	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (pRoomMemberEntry && !IsThisPlayer(pRoomMemberEntry->steamId))
		{
			currentMatchRoomMembers.push_back(pRoomMemberEntry);
		}
	}

	return currentMatchRoomMembers;
}

void RoomManager::AddIMPlayerToRoom(const IMPlayer& imPlayer)
{
	int first_free_idx = -1;
	bool in_list = false;
	for (int i = 0; i < m_imPlayers.size(); i++)
	{
		if (!m_imPlayers[i].active && first_free_idx == -1 && !in_list)
			first_free_idx = i;
		if (m_imPlayers[i].steamID == imPlayer.steamID)
		{
			first_free_idx = i;
			in_list = true;
		}
	}
	
	if (first_free_idx >= 0)
		m_imPlayers[first_free_idx] = imPlayer;
	LOG(3, "Add new IMPlayer at %d: %s\n", first_free_idx, imPlayer.steamName.c_str());
}

void RoomManager::AddIMPlayerToSpectators(const IMPlayer& imPlayer)
{
	int first_free_idx = -1;
	bool in_list = false;
	for (int i = 0; i < m_imSpectators.size(); i++)
	{
		if (!m_imSpectators[i].active && first_free_idx == -1 && !in_list)
			first_free_idx = i;
		if (m_imSpectators[i].steamID == imPlayer.steamID)
		{
			first_free_idx = i;
			in_list = true;
		}
	}

	if (first_free_idx >= 0)
		m_imSpectators[first_free_idx] = imPlayer;
}

void RoomManager::RemoveIMPlayerFromRoom(uint64_t steamID)
{
	for (int i = 0; i < m_imPlayers.size(); i++)
	{
		if (m_imPlayers[i].steamID.ConvertToUint64() == steamID)
			m_imPlayers[i] = IMPlayer();
	}
}

void RoomManager::RemoveIMPlayerFromSpectators(uint64_t steamID)
{
	for (int i = 0; i < m_imSpectators.size(); i++)
	{
		if (m_imSpectators[i].steamID.ConvertToUint64() == steamID)
			m_imSpectators[i] = IMPlayer();
	}
}

bool RoomManager::IsPacketFromSameMatch(Packet* packet) const
{
	/*const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryBySteamID(packet->steamID);

	if (!pRoomMemberEntry)
		return false;*/

	return true; // pRoomMemberEntry->matchId == GetThisPlayerRoomMemberEntry()->matchId;
}

// IsPacketFromSameMatch should be called beforehand
bool RoomManager::IsPacketFromSpectator(Packet* packet) const
{
	for (const IMPlayer& imPlayer : GetIMPlayersInCurrentMatch())
	{
		if (!imPlayer.active)
			continue;
		if (imPlayer.steamID.ConvertToUint64() == packet->steamID)
		{
			if (imPlayer.matchPlayerIndex == 2)
				return true;
		}
	}

	return false;
}

const RoomMemberEntry* RoomManager::GetThisPlayerRoomMemberEntry() const
{
	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (pRoomMemberEntry && IsThisPlayer(pRoomMemberEntry->steamId))
		{
			return pRoomMemberEntry;
		}
	}

	return nullptr;
}

const RoomMemberEntry* RoomManager::GetOpponentRoomMemberEntry() const
{
	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (pRoomMemberEntry && pRoomMemberEntry->opponentSteamID == m_thisPlayerSteamID.ConvertToUint64())
		{
			return pRoomMemberEntry;
		}
	}

	return nullptr;
}

const RoomMemberEntry* RoomManager::GetRoomMemberEntryByIndex(uint16_t index) const
{
	char* deref = (char*)(g_gameVals.ppFirstRoomMemberStatic) + 4 * index;
	RoomMemberEntry* roomMemberEntry = *(RoomMemberEntry**)deref;

	if (!roomMemberEntry || roomMemberEntry->steamId == 0)
		return nullptr;

	return roomMemberEntry;
}

const RoomMemberEntry* RoomManager::GetRoomMemberEntryBySteamID(uint64_t steamID) const
{
	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (!pRoomMemberEntry)
			continue;

		if (pRoomMemberEntry->steamId == steamID)
		{
			return pRoomMemberEntry;
		}
	}

	return nullptr;
}

bool RoomManager::IsPlayerInRoom(const IMPlayer& player) const
{
	for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
	{
		const RoomMemberEntry* pRoomMemberEntry = GetRoomMemberEntryByIndex(i);

		if (!pRoomMemberEntry)
			continue;

		if (pRoomMemberEntry->steamId == player.steamID.ConvertToUint64())
		{
			return true;
		}
	}

	return false;
}

bool RoomManager::IsThisPlayer(const uint64_t otherSteamID) const
{
	return otherSteamID == m_thisPlayerSteamID.ConvertToUint64();
}

uint64_t RoomManager::GetThisPlayerSteamID() const
{
	return m_thisPlayerSteamID.ConvertToUint64();
}

RoomMemberEntry* RoomManager::GetSelectedRoomMember() const
{
	char* deref = (char*)(*g_gameVals.ppFirstRoomMemberDynamic) + 0x150 * *g_gameVals.pSelectedRoomMember;
	RoomMemberEntry* roomMemberEntry = (RoomMemberEntry*)deref;

	if (!roomMemberEntry || roomMemberEntry->steamId == 0)
		return nullptr;

	return roomMemberEntry;
}

bool RoomManager::IsPlayerIMUser(uint64_t steamID) const
{
	for (const IMPlayer& imPlayer : GetIMPlayersInCurrentRoom())
	{
		if (!imPlayer.active)
			continue;
		if (imPlayer.steamID.ConvertToUint64() == steamID)
			return true;
	}

	return false;
}