#pragma once
#include <array>
#include <vector>
#include "TrainerItem.h"
#include <filesystem>

enum GuardFrame_
{
	GuardFrame_None,
	GuardFrame_IB,
	GuardFrame_FD,
	GuardFrame_SB
};

enum AttackLevel_
{
	AttackLevel_None,
	AttackLevel_Lv1,
	AttackLevel_Lv2,
	AttackLevel_Lv3
};

class TrainerInterface
{
private:
	bool disabled = false;
public:
	std::array<bool, 2> settingUpNightmare = { false, false };
	std::array<int, 2>  nightmareFrameCount = { 0, 0 };
	uint8_t change_flags = 0x00;

	std::vector<TrainerItem> trainerItems;
	std::vector<std::string> settingFiles;
	std::string currentSettingsFile;
	char settingsFile[128];
	bool disableInReplayAndOnline = true;

	void initializeTrainer();
	void resetTrainer();
	void updateGameValues(uint8_t);
	size_t NumberOfItems() {
		return trainerItems.size();
	}
	TrainerItem* GetItem(int id);
	TrainerItem* GetItem(std::string tag);
	void OnMatchInit();

	void saveSettings(std::string filename);
	void loadSettings(std::string filename);
	void refreshSettingsFiles();
	void disableTrainer();
	void enableTrainer();
};