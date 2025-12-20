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

std::vector<Hitbox> HitboxReader::getExtraHitboxes(const CharData* charObj)
{
	std::vector<Hitbox> hitboxes;

	if (charObj == NULL)
		return hitboxes;

	const int entriesCount = charObj->numHitbox;
	Hitbox* pEntry = reinterpret_cast<Hitbox*>(charObj->hitboxArray);
	Hitbox* pExtraEntry = reinterpret_cast<Hitbox*>(charObj->hitboxExtraArray);

	if (pEntry == NULL || pExtraEntry == NULL) //Fixes crash when loading EX ky
		return hitboxes;

	for (int i = 0; i < entriesCount; i++)
	{
		if (pEntry->type == HitboxType_Extra)
			hitboxes.push_back(*pExtraEntry);
		pEntry++;
		pExtraEntry++;
	}

	return hitboxes;
}
