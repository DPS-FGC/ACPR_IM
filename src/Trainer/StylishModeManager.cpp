#include "Trainer/StylishModeManager.h"
#include "Core/interfaces.h"

void StylishModeManager::InitializeComboItems(CharIndex charId, int playerNr)
{
	m_actsWithFollowup[playerNr].clear();
	if (m_defaultItem[playerNr] == nullptr)
	{
		switch (charId)
		{
		case CharIndex_Sol:
			m_defaultItem[playerNr] = GetStylishCombosSol();
			m_actsWithFollowup[playerNr] = { 100, 101, 104, 105, 110, 111, 112, 113 , 256 };
			break;
		case CharIndex_Potemkin:
			m_defaultItem[playerNr] = GetStylishCombosPotemkin();
			m_actsWithFollowup[playerNr] = { 71, 107, 108, 114, 116  };
			break;
		}
	}
	ResetNeutral(playerNr);
	m_previousInput[playerNr] = 999;
}

void StylishModeManager::ClearComboItems()
{
	ClearComboItems(0);
	ClearComboItems(1);
}

void StylishModeManager::ClearComboItems(int playerNr)
{
	m_defaultItem[playerNr] = nullptr;
	m_currentItem[playerNr] = nullptr;
}

void StylishModeManager::ResetFrameCounter(int playerNr)
{
	m_frameCounter[playerNr] = 0;
}

void StylishModeManager::ResetNeutral(int playerNr)
{
	m_currentItem[playerNr] = m_defaultItem[playerNr];
	m_lockInput[playerNr] = false;
	ResetFrameCounter(playerNr);
}

int StylishModeManager::InputResolved(int playerNr)
{
	if (m_inputSequence[playerNr].empty())
		return -1;
	for (int i = 0; i < m_inputSequence[playerNr].size(); i++)
	{
		if (m_inputSequence[playerNr][i] > 0)
			return i;
	}
	return -1;
}

int StylishModeManager::ReverseInput(int input, bool facingRight)
{
	if (facingRight)
		return input;
	int direction = input & 0xF;
	if (direction == 3 || direction == 6 || direction == 9)
		direction -= 2;
	else if (direction == 1 || direction == 4 || direction == 7)
		direction += 2;
	return (input & 0xFFFFFFF0) + direction;
}

bool StylishModeManager::HasFollowUp(uint16_t actId, int playerNr)
{
	for (const uint16_t& act : m_actsWithFollowup[playerNr])
	{
		if (act == actId)
			return true;
	}
	return false;
}

#define waitHitstop 0xF000
int StylishModeManager::ComputeNextAction(int input, int playerNr)
{
	CharData* playerdata = g_interfaces.GetPlayer(playerNr)->GetData();
	CharData* enemydata = g_interfaces.GetPlayer(!playerNr)->GetData();

	int iinput = InputResolved(playerNr);
	if (iinput > -1 && !m_inputSequence[playerNr].empty())
	{
		int directional_input = input & 0xF;
		input = m_inputSequence[playerNr][iinput];
		if (input == waitHitstop)
		{
			if (playerdata->hitstopTimer > 0)
				return 5;
			else
				input = 0;
		}
		if ((input & 0xF) == 0)
			input += directional_input;
		m_inputSequence[playerNr][iinput] = 0;
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return ReverseInput(input, playerdata->facingRight);
	}

	if ((playerdata->attackFlags & 0x0800) > 0 || //If in recovery, then cannot connect any new attacks
		playerdata->actId <= 14) //If moving, reset to neutral
	{
		ResetNeutral(playerNr);
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return m_lockInput[playerNr] ? (input & 0xF) : input;
	}

	if (m_currentItem[playerNr] == nullptr)
	{
		//if (!m_lockInput[playerNr])
		ResetNeutral(playerNr);
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return m_lockInput[playerNr] ? (input & 0xF) : input;
	}

	if (!m_currentItem[playerNr]->CheckDelay(m_frameCounter[playerNr]))
	{
		m_frameCounter[playerNr]++;
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return 5;
	}

	if (input <= 9)
	{
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return input;
	}

	if (m_previousInput[playerNr] == (input & 0xFFFFFFF0))
	{
		m_previousInput[playerNr] = input & 0xFFFFFFF0;
		return m_lockInput[playerNr] ? (input & 0xF) : input;
	}
	m_previousInput[playerNr] = input & 0xFFFFFFF0;

	if (!(playerdata->attackFlags & 0x0030) && //If not in gatling window
		!HasFollowUp(playerdata->actId, playerNr))
		return m_lockInput[playerNr] ? (input & 0xF) : input;

	std::shared_ptr<StylishComboItem> nextItem;
	if (!(playerdata->attackFlags & 0x0800))
		nextItem = m_currentItem[playerNr]->GetNextItem(input, playerdata, enemydata);

	if (nextItem != nullptr)
	{
		m_currentItem[playerNr] = nextItem;
		m_inputSequence[playerNr] = m_currentItem[playerNr]->GetInput();
		ResetFrameCounter(playerNr);
		m_lockInput[playerNr] = true;
	}

	return m_lockInput[playerNr] ? (input & 0xF) : input;
}