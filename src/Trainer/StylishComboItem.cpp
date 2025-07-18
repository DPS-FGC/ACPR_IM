#include "Trainer/StylishComboItem.h"

StylishComboItem::StylishComboItem(std::vector<uint16_t> prevActId,
	std::vector<int> input, int required_delay, bool ignoreDirection)
{
	m_prevActId = prevActId;
	m_input = input;
	m_requiredDelay = required_delay;
    m_ignoreDirection = ignoreDirection;
}

std::shared_ptr<StylishComboItem> StylishComboItem::GetNextItem(int input, CharData* player, CharData* enemy)
{
    input = DeconstructInput(input, player->facingRight);
    if (m_ignoreDirection)
        input &= 0xFFFFFFF0;
	std::vector<std::shared_ptr<StylishComboItem>> possibleCombos = m_possibleCombos[input];
	for (int i = 0; i < possibleCombos.size(); i++)
	{
		if (possibleCombos[i]->CheckPreviousActId(player->actId) && possibleCombos[i]->IsRequirementMet(player, enemy))
			return possibleCombos[i];
	}
	return nullptr;
}

#define T 512
#define D 256
#define H 128
#define S 64
#define K 32
#define P 16

int StylishComboItem::DeconstructInput(int input, bool facingRight)
{
    auto buffer = input;
    auto test = buffer - T;
    if (test > 0) {
        buffer = test;
        input = T;
    }
    test = buffer - D;
    if (test > 0) {
        buffer = test;
        input = D;
    }
    test = buffer - H;
    if (test > 0) {
        buffer = test;
        input = H;
    }
    test = buffer - S;
    if (test > 0) {
        buffer = test;
        input = S;
    }
    test = buffer - K;
    if (test > 0) {
        buffer = test;
        input = K;
    }
    test = buffer - P;
    if (test > 0) {
        buffer = test;
        input = P;
    }

    if (buffer != 5)
    {
        if (!facingRight)
        {
            if (buffer == 3 || buffer == 6 || buffer == 9)
                buffer -= 2;
            else if (buffer == 1 || buffer == 4 || buffer == 7)
                buffer += 2;
        }
        input += buffer;
    }
    return input;
}

void StylishComboItem::SetRequirement(bool (*requirement)(CharData*, CharData*))
{
    pRequirement = requirement;
}

std::vector<int> StylishComboItem::GetInput()
{
	return m_input;
}

bool StylishComboItem::IsRequirementMet(CharData* player, CharData* enemy)
{
    if (pRequirement == nullptr)
        return true;

    return pRequirement(player, enemy);
}

bool StylishComboItem::CheckPreviousActId(uint16_t actId)
{
	if (m_prevActId.empty())
		return true;
	for (int i = 0; i < m_prevActId.size(); i++)
	{
		if (m_prevActId[i] == actId)
			return true;
	}
	return false;
}

void StylishComboItem::AddItem(int required_input, std::shared_ptr<StylishComboItem> item)
{
	m_possibleCombos[required_input].push_back(item);
}

bool StylishComboItem::CheckDelay(int framespassed)
{
	return framespassed >= m_requiredDelay;
}