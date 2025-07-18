#pragma once
#include <cstdint>

//Addresses taken from GGXXACPROverlay by YouKnow232.
struct Camera
{
	char buffer_0x00[16]; //0x00.
	int CenterXPos; //0x10.
	int BottomEdge; //0x14.
	char buffer_0x18[8]; //0x18.
	int LeftEdge; //0x20.
	char buffer_0x24[4]; //0x24.
	int Width; //0x28.
	int Height; //0x2C.
	char buffer_0x30[20]; //0x30.
	float Zoom; //0x44.
};