#include "HitboxReader.h"

std::vector<Hitbox> HitboxReader::getHitboxes(const CharData* charObj)
{
	std::vector<Hitbox> hitboxes;

	if (charObj == NULL)
		return hitboxes;

	const int entriesCount = charObj->numHitbox;
	Hitbox* pEntry = reinterpret_cast<Hitbox*>(charObj->hitboxArray);

	for (int i = 0; i < entriesCount; i++)
	{
		hitboxes.push_back(*pEntry);
		pEntry++;
	}

	return hitboxes;
}
