#pragma once
#include<cstdint>
#include<vector>
#include<memory>
#include<unordered_map>
#include "Game/CharData.h"
class StylishComboItem
{ 
private:
	std::vector<uint16_t> m_prevActId;
	std::unordered_map<int, std::vector<std::shared_ptr<StylishComboItem>>> m_possibleCombos;
	std::vector<int> m_input;
	int m_requiredDelay;
	bool m_ignoreDirection;
	bool (*pRequirement)(CharData* p1, CharData* p2) = nullptr;
	int DeconstructInput(int input, bool facingRight);

public:
	StylishComboItem(std::vector<uint16_t> prevActId,
		std::vector<int> input, int required_Delay, bool ignoreDirection);
	std::shared_ptr<StylishComboItem> GetNextItem(int input, CharData* player, CharData* enemy);
	std::vector<int> GetInput();
	void SetRequirement(bool (*requirement)(CharData*, CharData*));
	bool IsRequirementMet(CharData* player, CharData* enemy);
	bool CheckPreviousActId(uint16_t actId);
	void AddItem(int required_input, std::shared_ptr<StylishComboItem> item);
	bool CheckDelay(int framespassed);
};

