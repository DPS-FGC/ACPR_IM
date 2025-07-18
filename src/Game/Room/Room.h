#pragma once

#include <cstdint>
#include <Game/Room/RoomMemberEntry.h>
class Room
{
public:
	int status; //0x00.
	char pad_04[8]; //0x04.
	char roomName[32]; //0x0C.
	char pad_2C[12]; //0x2C.
	bool enableKliffJustice; //0x38.
	bool enableSP; //0x39.
	bool enableEx; //0x3A.
	bool enableGGMode; //0x3B.
	uint32_t roundsToWin; //0x3C. 0 = 1, 1 = 3, 2 = 5, 3 = 7, 4 = 9
	uint32_t rountTime; //0x40. 0 = 30, 1 = 60, 2 = 99.
	uint32_t capacity; //0x44. 
	char pad_48[4]; //0x48.
	uint32_t gameMode; //0x4C. 1 = Single, 2 = Team
};