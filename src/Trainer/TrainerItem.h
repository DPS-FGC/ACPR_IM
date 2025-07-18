#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <cassert>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>

enum TrainerItemType_
{
	TrainerItemType_NULL,
	TrainerItemType_Int,
	TrainerItemType_Float,
	TrainerItemType_List,
	TrainerItemType_Bool
};

class TrainerItem {
private:
	friend class boost::serialization::access;

	std::string label = "";
	std::string tag = "";
	std::string tooltip = "";
	std::string character = "";
	uint16_t gamemodes = 0;
	TrainerItemType_ type = TrainerItemType_NULL;
	int nplayers = 2;
	bool disabled = false;

	void (*pUpdateFunctionBool)(uint8_t, bool, int) = NULL;
	void (*pUpdateFunctionInt)(uint8_t, int, int) = NULL;
	void (*pUpdateFunctionFloat)(uint8_t, float, int) = NULL;

	std::array<int, 2> intvalue = { 0, 0 };
	std::array<float, 2> floatvalue = { 0.0f, 0.0f };
	std::array<bool, 2> boolvalue = { false, false };
	bool defaultbool = false;
	bool defaultint = 0;
	float defaultfloat = 0;

	int minint = 0, maxint = 999;
	float minfloat = 0.0f, maxfloat = 1.0;
	std::vector<std::string> itemLabels;

public:
	TrainerItem() {};

	TrainerItem(std::string label, int nplayers, std::string tag, TrainerItemType_ type, std::string tooltip,
		bool default = false, void (*pUpdateFunction)(uint8_t, bool, int) = NULL, std::string character = "",
		uint16_t gamemodes = 0xFFFF) {
		this->label = label;
		this->tag = tag;
		this->type = type;
		this->nplayers = nplayers;
		this->tooltip = tooltip;
		defaultbool = default;
		pUpdateFunctionBool = pUpdateFunction;
		this->character = character;
		this->gamemodes = gamemodes;
		ResetValue();
	}

	TrainerItem(std::string label, int nplayers, std::string tag, TrainerItemType_ type, std::string tooltip,
		int min, int max, int default = 0, void (*pUpdateFunction)(uint8_t, int, int) = NULL, std::string character = "",
		uint16_t gamemodes = 0xFFFF) {
		this->label = label;
		this->tag = tag;
		this->type = type;
		this->nplayers = nplayers;
		this->tooltip = tooltip;
		minint = min;
		maxint = max;
		defaultint = default;
		pUpdateFunctionInt = pUpdateFunction;
		this->character = character;
		this->gamemodes = gamemodes;
		ResetValue();
	}

	TrainerItem(std::string label, int nplayers, std::string tag, TrainerItemType_ type, std::string tooltip,
		float min, float max, float default = 0.0f, void (*pUpdateFunction)(uint8_t, float, int) = NULL, std::string character = "",
		uint16_t gamemodes = 0xFFFF) {
		this->label = label;
		this->tag = tag;
		this->type = type;
		this->nplayers = nplayers;
		this->tooltip = tooltip;
		minfloat = min;
		maxfloat = max;
		defaultfloat = default;
		pUpdateFunctionFloat = pUpdateFunction;
		this->character = character;
		this->gamemodes = gamemodes;
		ResetValue();
	}

	TrainerItem(std::string label, int nplayers, std::string tag, TrainerItemType_ type, std::string tooltip,
		const char* labels[], int nlabels, int default = 0, void (*pUpdateFunction)(uint8_t, int, int) = NULL, std::string character = "",
		uint16_t gamemodes = 0xFFFF) {
		this->label = label;
		this->tag = tag;
		this->type = type;
		this->nplayers = nplayers;
		this->tooltip = tooltip;
		for (int i = 0; i < nlabels; i++)
			itemLabels.push_back(labels[i]);
		minint = 0;
		maxint = nlabels;
		defaultint = default;
		pUpdateFunctionInt = pUpdateFunction;
		this->character = character;
		this->gamemodes = gamemodes;
		ResetValue();
	}

	std::string GetName() { return label; }

	std::string GetTag() { return tag; }

	std::string GetToolTip() { return tooltip; }

	bool ShouldDisplay(std::string name)
	{
		if (character.empty())
			return true;
		return character == name;
	}

	bool ShouldDisplay(std::string P1, std::string P2)
	{
		if (character.empty())
			return true;
		return character == P1 || character == P2;
	}

	bool ShouldDisplay(uint16_t gamemode)
	{
		return (this->gamemodes & (1 << gamemode)) > 0;
	}

	size_t GetNplayers() {
		return nplayers;
	}

	TrainerItemType_ GetType() {
		return type;
	}

	bool GetBool(int playerNr) {
		assert(playerNr < nplayers);
		return disabled ? defaultbool : boolvalue[playerNr];
	}

	bool* GetBoolPtr(int playerNr) {
		assert(playerNr < nplayers);
		return &boolvalue[playerNr];
	}

	int GetInt(int playerNr) {
		assert(playerNr < nplayers);
		return disabled ? defaultint : intvalue[playerNr];
	}

	int* GetIntPtr(int playerNr) {
		assert(playerNr < nplayers);
		return &intvalue[playerNr];
	}

	int GetMinInt() {
		return minint;
	}

	int GetMaxInt() {
		return maxint;
	}

	float GetFloat(int playerNr) {
		assert(playerNr < nplayers);
		return disabled ? defaultfloat : floatvalue[playerNr];
	}

	float* GetFloatPtr(int playerNr) {
		assert(playerNr < nplayers);
		return &floatvalue[playerNr];
	}

	float GetMinFloat() {
		return minfloat;
	}

	float GetMaxFloat() {
		return maxfloat;
	}

	std::string GetCurrentLabel(int playerNr) {
		assert(playerNr < nplayers);
		return itemLabels[intvalue[playerNr]];
	}

	std::string GetLabel(int index) {
		return itemLabels[index];
	}

	int NumberOfItems() {
		return maxint;
	}

	void CheckInt(int playerNr) {
		assert(playerNr < nplayers);
		if (intvalue[playerNr] < minint)
			intvalue[playerNr] = minint;
		else if (intvalue[playerNr] > maxint)
			intvalue[playerNr] = maxint;
	}

	void SetInt(int newval, int playerNr) {
		assert(playerNr < nplayers);
		intvalue[playerNr] = newval;
		CheckInt(playerNr);
	}

	void CheckFloat(int playerNr) {
		assert(playerNr < nplayers);
		if (floatvalue[playerNr] < minfloat)
			floatvalue[playerNr] = minfloat;
		else if (floatvalue[playerNr] > maxfloat)
			floatvalue[playerNr] = maxfloat;
	}

	void SetFloat(float newval, int playerNr) {
		assert(playerNr < nplayers);
		floatvalue[playerNr] = newval;
		CheckFloat(playerNr);
	}

	void SetBool(bool newval, int playerNr) {
		assert(playerNr < nplayers);
		boolvalue[playerNr] = newval;
	}

	void UpdateGameValue(uint8_t flags) {
		for (int i = 0; i < nplayers; i++)
		{
			switch (type)
			{
			case TrainerItemType_Bool:
				if (pUpdateFunctionBool)
					pUpdateFunctionBool(flags, boolvalue[i], i);
				break;
			case TrainerItemType_Int:
			case TrainerItemType_List:
				if (pUpdateFunctionInt)
					pUpdateFunctionInt(flags, intvalue[i], i);
				break;
			case TrainerItemType_Float:
				if (pUpdateFunctionFloat)
					pUpdateFunctionFloat(flags, floatvalue[i], i);
				break;

			}
		}
	}

	void ResetValue() {
		for (int i = 0; i < nplayers; i++)
		{
			switch (type)
			{
			case TrainerItemType_Bool:
				boolvalue[i] = defaultbool;
				break;
			case TrainerItemType_Int:
			case TrainerItemType_List:
				intvalue[i] = defaultint;
				break;
			case TrainerItemType_Float:
				floatvalue[i] = defaultfloat;
				break;

			}
		}
	}

	void Disable() { disabled = true; }

	void Enable() { disabled = false; }

	template<class Archive>
	void serialize(Archive& a, const unsigned version) {
		a& tag;
		a& type;
		a& nplayers;
		a& boolvalue;
		a& intvalue;
		a& floatvalue;
	}
};