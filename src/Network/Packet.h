#pragma once
#include <cstdint>
#include <cstring>

#define HEADER_SIZE 24
constexpr int MAX_DATA_SIZE = 4096;

enum PacketType : uint8_t
{
	PacketType_IMID_Announce = 0xFF,
	PacketType_IMID_Acknowledge = 0xFE,
	PacketType_PaletteInfo = 0xFD,
	PacketType_PaletteData = 0xFC,
	PacketType_IMID_AnnounceSpectate = 0xFB,
	PacketType_IMID_LeaveMatch = 0xFA,
	PacketType_IMID_PlayerInfo = 0xF9,
	PacketType_RequestPalette = 0xF8,
	PacketType_GameMode = 0xF0
};

// BBCF packets' first two fields must be the packet size
struct Packet
{
	uint16_t checksum = 0;
	int16_t matchIndex = 0; //0x02
	uint16_t part; //0x04.
	PacketType packetType; //0x06
	uint64_t steamID;
	uint16_t packetSize;
	//uint16_t roomMemberIndex;
	uint16_t dataSize;
	unsigned char data[MAX_DATA_SIZE - HEADER_SIZE];

	Packet(void* dataSrc, uint16_t dataSize, PacketType packetType, uint64_t steamID, uint16_t part = 0, int16_t matchIndex = -1)
		: dataSize(dataSize), packetType(packetType), steamID(steamID), part(part), matchIndex(matchIndex)
	{
		packetSize = HEADER_SIZE + dataSize;
		if (dataSrc && dataSize)
		{
			memcpy_s(data, MAX_DATA_SIZE - HEADER_SIZE, dataSrc, dataSize);
		}
	}
};