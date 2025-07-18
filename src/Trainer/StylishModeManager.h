#pragma once
#include<vector>
#include<memory>
#include"Trainer/StylishComboItem.h"
#include "Game/characters.h"

class StylishModeManager
{
public:
	void InitializeComboItems(CharIndex charId, int playerNr);
	void ClearComboItems();
	void ClearComboItems(int playerNr);
	void ResetNeutral(int playerNr);
	int ComputeNextAction(int input, int playerNr);

private:
	std::shared_ptr<StylishComboItem> m_defaultItem[2];
	std::shared_ptr<StylishComboItem> m_currentItem[2];
	int m_frameCounter[2];
	std::vector<int> m_inputSequence[2];
	bool m_lockInput[2];
	int m_previousInput[2];
	std::vector<uint16_t> m_actsWithFollowup[2];

	void ResetFrameCounter(int playerNr);
	int InputResolved(int playerNr);
	int ReverseInput(int input, bool facingRight);
	bool HasFollowUp(uint16_t actId, int playerNr);

	std::shared_ptr<StylishComboItem> GetStylishCombosSol();
	std::shared_ptr<StylishComboItem> GetStylishCombosPotemkin();
};

