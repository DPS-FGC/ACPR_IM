#pragma once
#include <string>

enum CharIndex
{
	CharIndex_Sol = 1,
	CharIndex_Ky,
	CharIndex_May,
	CharIndex_Millia,
	CharIndex_Axl,
	CharIndex_Potemkin,
	CharIndex_Chipp,
	CharIndex_Eddie,
	CharIndex_Baiken,
	CharIndex_Faust,
	CharIndex_Testament,
	CharIndex_Jam,
	CharIndex_Anji,
	CharIndex_Johnny,
	CharIndex_Venom,
	CharIndex_Dizzy,
	CharIndex_Slayer,
	CharIndex_Ino,
	CharIndex_Zappa,
	CharIndex_Bridget,
	CharIndex_RoboKy,
	CharIndex_ABA,
	CharIndex_OrderSol,
	CharIndex_Kliff,
	CharIndex_Justice
};

int getCharactersCount();

const std::string& getCharacterNameByIndexA(int charIndex);

std::wstring getCharacterNameByIndexW(int charIndex);

bool isCharacterIndexOutOfBound(int charIndex);
