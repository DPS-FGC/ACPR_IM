#pragma once
#include "../CharData.h"
#include "Hitbox.h"

#include <vector>

class HitboxReader
{
public:
	static std::vector<Hitbox> getHitboxes(const CharData* charObj);
};
