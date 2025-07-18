#pragma once

enum HitboxType_ : uint16_t
{
	HitboxType_Hitbox = 1,
	HitboxType_Hurtbox = 2,
	HitboxType_Pushbox = 4
};

class Hitbox
{
public:
	int16_t offsetX;
	int16_t offsetY;
	int16_t width;
	int16_t height;
	HitboxType_ type; //1 - hit, 2 - hurt
	uint16_t flags;
};