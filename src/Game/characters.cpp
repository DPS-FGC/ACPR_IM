#include "characters.h"

#include <vector>

const std::vector<std::string> charNames
{
	"None",         //00
	"Sol",			//01
	"Ky",			//02
	"May",			//03
	"Millia",		//04
	"Axl",			//05
	"Potemkin",		//06
	"Chipp",		//07
	"Eddie",		//08
	"Baiken",		//09
	"Faust",		//10
	"Testament",	//11
	"Jam",			//12
	"Anji",			//13
	"Johnny",		//14
	"Venom",		//15
	"Dizzy",		//16
	"Slayer",		//17
	"I-No",			//18
	"Zappa",		//19
	"Bridget",		//20
	"Robo-Ky",		//21
	"A.B.A",		//22
	"Order-Sol",	//23
	"Kliff",		//24
	"Justice"		//25
};

int getCharactersCount()
{
	return charNames.size();
}

const std::string& getCharacterNameByIndexA(int charIndex)
{
	const static std::string UNKNOWN = "Unknown";

	if (charIndex < getCharactersCount())
		return charNames[charIndex];

	return UNKNOWN;
}

std::wstring getCharacterNameByIndexW(int charIndex)
{
	const std::string& charName = getCharacterNameByIndexA(charIndex);
	return std::wstring(charName.begin(), charName.end());
}

bool isCharacterIndexOutOfBound(int charIndex)
{
	return getCharactersCount() < charIndex;
}
