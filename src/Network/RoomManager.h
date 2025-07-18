#pragma once
#include "NetworkManager.h"

#include "Game/Room/Room.h"

#include <isteamfriends.h>
#include <string>
#include <vector>

#define MAX_PLAYERS_IN_ROOM 8

struct IMPlayer
{
	bool active = false;
	CSteamID steamID;
	std::string steamName;
	int matchPlayerIndex;

	IMPlayer() : matchPlayerIndex(-1)
	{
	}

	IMPlayer(uint64_t steamID, const char* steamName)
		: steamID(steamID),
		steamName(steamName), matchPlayerIndex(-1)
	{
		active = true;
	}

	IMPlayer(IMPlayer other, int matchPlayerIndex)
		: steamID(other.steamID),
		steamName(other.steamName), matchPlayerIndex(matchPlayerIndex)
	{
		active = true;
	}
};

class RoomManager
{
public:
	RoomManager(NetworkManager* pNetworkManager, CSteamID steamID);
	~RoomManager();

	void SendAcknowledge(Packet* packet);
	void AcceptAcknowledge(Packet* packet);
	void AcknowledgeSpectate(Packet* packet);
	void RecvPlayerInfo(Packet* packet);
	void AcceptLeave(Packet* packet);
	void AnnounceLeave();
	void OnMatchInit();
	void OnMatchEnd();
	void JoinRoom();
	bool IsRoomFunctional() const;
	void SendPacketToSameMatchIMPlayers(Packet* packet);
	void SendPacketToPlayerBySteamID(Packet* packet, CSteamID id);
	bool IsPacketFromSameRoom(Packet* packet) const;
	bool IsPacketFromSameMatchNonSpectator(Packet* packet) const;
	bool IsThisPlayerSpectator() const;
	bool IsThisPlayerAutoSpectator() const;
	bool IsThisPlayerInMatch() const;
	void SetFFAThisPlayerIndex(int* pFFAThisPlayerIndex);
	RoomPlayerMode GetThisPlayerGameMode() const;
	const char* GetThisPlayerSteamName() const;
	uint64_t GetThisPlayerSteamID() const;
	uint16_t GetThisPlayerMatchPlayerIndex() const;
	uint16_t GetPlayerMatchPlayerIndexBySteamID(uint64_t steamID) const;
	const std::string GetRoomTypeName() const;
	std::vector<IMPlayer> GetIMPlayersInCurrentMatch() const;
	std::vector<IMPlayer> GetIMPlayersInCurrentRoom() const;
	std::vector<const RoomMemberEntry*> GetOtherRoomMemberEntriesInCurrentMatch() const;
	RoomMemberEntry* GetSelectedRoomMember() const;
	void RequestPaletteResend();
	const char* GetPlayerSteamName(uint64_t steamID) const;
	bool IsPlayerIMUser(uint64_t steamID) const;

private:
	void SendAnnounce();
	void AddIMPlayerToRoom(const IMPlayer& player);
	void AddIMPlayerToSpectators(const IMPlayer& player);
	void RemoveIMPlayerFromRoom(uint64_t steamID);
	void RemoveIMPlayerFromSpectators(uint64_t steamID);
	bool IsPacketFromSameMatch(Packet* packet) const;
	bool IsPacketFromSpectator(Packet* packet) const;
	//uint16_t GetThisPlayerRoomMemberIndex() const;
	const RoomMemberEntry* GetThisPlayerRoomMemberEntry() const;
	const RoomMemberEntry* GetOpponentRoomMemberEntry() const;
	const RoomMemberEntry* GetRoomMemberEntryByIndex(uint16_t index) const;
	//const RoomMemberEntry* RoomManager::GetRoomMemberEntryByRoomIndex(uint32_t index) const;
	const RoomMemberEntry* RoomManager::GetRoomMemberEntryBySteamID(uint64_t steamID) const;
	bool IsPlayerInRoom(const IMPlayer& player) const;
	bool IsThisPlayer(const uint64_t otherSteamID) const;

	std::vector<IMPlayer> m_imPlayers;
	std::vector<IMPlayer> m_imSpectators;
	CSteamID m_thisPlayerSteamID;

	// Free-for-All fix
	int* m_pFFAThisPlayerIndex;

	// Interfaces
	NetworkManager* m_pNetworkManager;
	Room* m_pRoom;

	// For debug purposes
	friend class DebugWindow;
};
