#include "HitboxReader.h"

std::vector<Hitbox> HitboxReader::getHitboxes(const CharData* charObj)
{
	std::vector<Hitbox> hitboxes;

	if (charObj == NULL)
		return hitboxes;

	const int entriesCount = charObj->numHitbox;
	Hitbox* pEntry = reinterpret_cast<Hitbox*>(charObj->hitboxArray);

	if (pEntry == NULL) //Fixes crash when loading EX ky
		return hitboxes;

	for (int i = 0; i < entriesCount; i++)
	{
		hitboxes.push_back(*pEntry);
		pEntry++;
	}

	return hitboxes;
}
