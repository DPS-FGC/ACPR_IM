#pragma once
#include <cstdint>

struct ExtraMenu
{
	int mode; //0x00.
	int attack; //0x04.
	int defense; //0x08.
	int speed; //0x0C.
	int tension; //0x10.
	int tensionrate; //0x14.
	int starttension; //0x18.
	char pad_1C[4]; //0x1C.
	int life; //0x20.
	int burst; //0x24.
	int burstrate; //0x28.
	int guard; //0x2C.
	int guardbalance; //0x30.
	int kizetsu; //0x34.
	int airdash; //0x38.
	int airjump; //0x3C.
	int inputeasy; //0x40.
	int easyfrc; //0x44.
	int thunderattack; //0x48.
	int flameattack; //0x4C.
	int vampireattack; //0x50.
	int guardthunder; //0x54.
	int guardflame; //0x58.
	int guardmistfiner; //0x5C.
	int guardbeat; //0x60.
	int chaosrc; //0x64.
	char pad_68[8]; //0x68.
	int yellowrc; //0x70.
	int allwbound; //0x74.
	int allgbound; //0x78.
	char pad_7C[28];
};