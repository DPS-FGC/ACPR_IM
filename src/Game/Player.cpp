#include "Player.h"
#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

void Player::SetCharDataPtr(const void* addr)
{
	m_charData = (CharData**)addr;
}

bool Player::IsCharDataNullPtr() const
{
	if (m_charData == 0)
		return true;

	return *m_charData == 0;
}

CharData* Player::GetData() const
{
	return *m_charData;
}

char* Player::GetCharAbbr()
{
	if (IsCharDataNullPtr())
		return NULL;

	return (char*) & abbr[3 * ((*m_charData)->charIndex - 1)];
}

CharPaletteHandle& Player::GetPalHandle()
{
	return m_charPalHandle;
}
