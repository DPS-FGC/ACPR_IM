#pragma once
//#include "Packet.h"
#include "Packet.h"
#include "SteamApiWrapper/SteamNetworkingWrapper.h"

#include <steam_api.h>

class NetworkManager
{
public:
	NetworkManager(SteamNetworkingWrapper* SteamNetworking, CSteamID steamID);
	~NetworkManager();
	bool SendPacket(CSteamID* steamID, Packet* packet, int nChannel = 0);
	void RecvPacket(Packet* packet);
	bool IsIMPacket(Packet* packet);

private:

	SteamNetworkingWrapper* m_pSteamNetworking;
	CSteamID m_steamID;
};