#pragma once
#include <cstdint>

enum RoomPlayerMode
{
	RoomPlayerMode_None,
	RoomPlayerMode_InMatch,
	RoomPlayerMode_Menu,
	RoomPlayerMode_Training,
	RoomPlayerMode_Survival,
	RoomPlayerMode_MOM,
	RoomPlayerMode_Replay,
	RoomPlayerMode_InMatchOther,
	RoomPlayerMode_Spectate
};

class RoomMemberEntry
{
public:
	uint32_t status; //0x00. 0 - Not ready, 1 - Ready, 2 - Requesting match, 3 - In match, 4 - In menu
	RoomPlayerMode mode; //0x04.
	uint64_t opponentSteamID; //0x08.
	char pad_10[4]; //0x10.
	uint32_t matchPlayerIndex; //0x14.
	char pad_18[24]; //0x18.
	uint64_t steamId; //0x30.
	char playerName[32]; //0x38.
	char pad_58[108]; //0x58.
	uint32_t charIndex; //0xC4. Index of character selected in waiting mode.
	char pad_C8[64]; //0xC8.
	uint32_t messageIndex; //0x108.
	char pad_10C[16]; //0x10C.
	int memberIndex; //0x11C.
	int pad_120; //0x120. Has value -1, when in spectator mode.
	int winStreak; //0x124.
	int numberOfWins; //0x128.
	int totalMatches; //0x12C. Number of losses is calculated as Total games - Wins
	char pad_130[32]; //0x130.
}; //Size: 0x150