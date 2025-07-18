#pragma once
#include "CharData.h"
#include "Palette/CharPaletteHandle.h"
#include <deque>

static const char abbr[75] = { "so\0ky\0ma\0mi\0ax\0po\0ch\0ed\0ba\0fa\0te\0ja\0an\0jo\0ve\0di\0sl\0in\0za\0br\0ro\0ab\0os\0kl\0ju" };

class Player
{
public:
	CharData* GetData() const;
	CharPaletteHandle& GetPalHandle();

	void SetCharDataPtr(const void* addr);
	bool IsCharDataNullPtr() const;
	char* GetCharAbbr();
private:
	CharData** m_charData;
	CharPaletteHandle m_charPalHandle;
};
