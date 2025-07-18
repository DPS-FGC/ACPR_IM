#include "hooks_cbr.h"

#include "HookManager.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Game/gamestates.h"

DWORD PInputJmpBackAddr = 0;
void __declspec(naked)PInput()
{
	LOG_ASM(7, "PInput\n");
	static uint16 input = 0;
	static int playerNr = -1;
	__asm
	{
		mov input, ax
		mov playerNr, edi
		pushad
	}

	if (g_gameVals.GetGameMode() == GameMode_ReplayTheater)
	{
		if (g_interfaces.cbrInterface.autoScanReplayFolder && playerNr == 0)
		{
			if (g_gameVals.GetGameState() == GameState_ReplayMenu)
			{
				if (g_interfaces.cbrInterface.currentReplayItem == -1)
				{
					if (g_interfaces.cbrInterface.replayRangeStart - 1 < 1)
						*g_gameVals.pCurrentReplayItem = 1;
					else
						*g_gameVals.pCurrentReplayItem = g_interfaces.cbrInterface.replayRangeStart - 1;
				}
				else if (g_interfaces.cbrInterface.currentReplayItem == 0 || g_interfaces.cbrInterface.stopAutoScan ||
					(g_interfaces.cbrInterface.replayRangeEnd - 1 > 0 && g_interfaces.cbrInterface.currentReplayItem > g_interfaces.cbrInterface.replayRangeEnd - 1))
					g_interfaces.cbrInterface.autoScanReplayFolder = false; //Finished scanning
				else if ((g_interfaces.cbrInterface.replayRangeEnd - 1 <= 0 && g_interfaces.cbrInterface.currentReplayItem > 0) ||
					     (g_interfaces.cbrInterface.replayRangeEnd - 1 >  0 && g_interfaces.cbrInterface.currentReplayItem <= g_interfaces.cbrInterface.replayRangeEnd - 1))
				{
					if (g_interfaces.cbrInterface.currentReplayItem == *g_gameVals.pCurrentReplayItem)
					{
						if (!g_interfaces.cbrInterface.currentReplayWatched)
							input = 0x4000; //Press A button to start replay
						else
						{
							if (!g_interfaces.cbrInterface.threadActiveCheck())  //Make sure save file was written
								input = 0x0040; //Press down to go to next replay.
						}
					}
					else
						g_interfaces.cbrInterface.currentReplayWatched = false;
				}
			}
		}
		g_interfaces.cbrInterface.currentReplayItem = *g_gameVals.pCurrentReplayItem;
	}
	
	CBRLogic(&input, 5, true, true, playerNr);
	UpdateFrameMeter(playerNr);

	__asm
	{
		popad
		movzx eax, input
		mov[esi - 04h], eax
		not edx
		jmp[PInputJmpBackAddr]
	}
}

DWORD UnlockFPSInReplayJmpBackAddr = 0;
void __declspec(naked)UnlockFPSInReplay()
{
	if (g_interfaces.cbrInterface.autoScanReplayFolder)
		_asm test ecx, ecx
	else
		_asm test[ebx + esi * 2h + 04h], cx
	_asm jmp[UnlockFPSInReplayJmpBackAddr]
}

DWORD PInputReplayJmpBackAddr = 0;
void __declspec(naked)PInputReplay()
{
	static uint16 input = 0;
	static int playerNr = -1;
	
	__asm
	{
		pushad
		mov input, cx
		mov playerNr, edi  //Read player Nr (2 - P1, 1 - P2)
	}

	if (g_interfaces.cbrInterface.autoScanReplayFolder && playerNr == 2)
	{
		if (g_gameVals.GetGameState() == GameState_VersusScreen)
		{
			if (!g_interfaces.cbrInterface.currentReplayWatched)
			{
				input = 0x0000; //Simulate key up, so that another key press can be detected.
				g_interfaces.cbrInterface.replayInputCooldown = 10;
				g_interfaces.cbrInterface.currentReplayWatched = true;
			}
			else
			{
				if (g_interfaces.cbrInterface.replayInputCooldown > 0)
				{
					input = 0x0000;
					g_interfaces.cbrInterface.replayInputCooldown--;
				}
				else
				{
					input = 0x4000; //Press A button to skip versus screen
					g_interfaces.cbrInterface.replayInputCooldown = 10;
				}
			}
		}
		else if (*g_gameVals.pInGameFlag)
		{
			if (g_interfaces.cbrInterface.replayInputCooldown > 0)
			{
				input = 0x0020; //Simulate press forward to activate fast forward
				g_interfaces.cbrInterface.replayInputCooldown--;
			}
			else
			{
				input = 0x0000;
				g_interfaces.cbrInterface.replayInputCooldown = 0;
			}
		}
	}
	__asm
	{
		popad
		movzx ecx, input
		mov [eax], cx
		add eax, 02
		jmp[PInputReplayJmpBackAddr]
	}
}

bool CheckRoundStart(int round)
{
	if (g_gameVals.pRoundCounter == NULL || *g_gameVals.pRoundCounter != round)
		return false;
	return (g_gameVals.GetMatchState() != MatchState_BeforeBattle && g_interfaces.cbrInterface.pMatchState == MatchState_BeforeBattle) ||
		(g_gameVals.GetMatchState() != MatchState_LetsRock && g_interfaces.cbrInterface.pMatchState == MatchState_LetsRock);
}

bool CheckRoundEnd(int round)
{
	if (g_gameVals.GetGameMode() != GameMode_Training && //Round counter doesn't change in training so skip this check
		(g_gameVals.pRoundCounter == NULL || *g_gameVals.pRoundCounter == round))
		return false;
	return (g_gameVals.GetMatchState() != MatchState_RoundEnd && g_interfaces.cbrInterface.pMatchState == MatchState_RoundEnd) ||
		(g_gameVals.GetMatchState() != MatchState_FadeOut && g_interfaces.cbrInterface.pMatchState == MatchState_FadeOut) ||
		(g_gameVals.GetMatchState() == MatchState_ExitBattle && g_interfaces.cbrInterface.pMatchState != MatchState_ExitBattle);
}

bool CheckStylishMode(int playerNr)
{
	return g_interfaces.trainerInterface.GetItem("sm:StylishMode")->GetBool(playerNr);
}

bool CBRLogic(int* input, int hirarchy, int playerNr, int controllerNr, bool read, bool write, bool netplayMemory) {
	bool result = false; //Returns true if AI wrote a new input, otherwise false.
	if (playerNr < 0 || playerNr > 1) {
		return result;
	}
#ifdef DEBUG
	std::string s = "CBRLogic: Input: " + std::to_string(input) + " -Hirarchy: " + std::to_string(hirarchy) +
		" -playerNr: " + std::to_string(playerNr) + " -controllerNr: " + std::to_string(controllerNr) +
		" -read: " + std::to_string(read) + " -write: " + std::to_string(write) + "\n";
	LOG(2, s.c_str());
#endif

	//Resetting past inputs in first input function.
	if (g_interfaces.cbrInterface.resetDepth == hirarchy && g_interfaces.cbrInterface.resetPlayer == playerNr) {
#ifdef DEBUG
		std::string s = "Reset: \n";
		LOG(2, s.c_str());
#endif
		bool roundEnd = CheckRoundEnd(g_interfaces.cbrInterface.lastRecordedRound);
		if (CheckRoundStart(1) || roundEnd) {
			g_interfaces.cbrInterface.RestartCbrActivities(g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetData()->charIndex, g_interfaces.Player2.GetData()->charIndex);
			if (roundEnd)
				g_interfaces.cbrInterface.lastRecordedRound = *g_gameVals.pRoundCounter;
		}
		
		g_interfaces.cbrInterface.pMatchState = g_gameVals.GetMatchState();
		g_interfaces.cbrInterface.inputMemory = { 5,5 };
		g_interfaces.cbrInterface.writeMemory = { -1,-1 };
		g_interfaces.cbrInterface.inputMemoryHirarchy = { -1,-1 };
		g_interfaces.cbrInterface.executionOrder = {};
		g_interfaces.cbrInterface.reversalInput = 5;

	}

	auto i = g_interfaces.cbrInterface.executionOrder.find(hirarchy * 10 + playerNr);
	if (true == false && i != g_interfaces.cbrInterface.executionOrder.end()) {
		//g_interfaces.cbrInterface.resetCbrInterface();
		g_interfaces.cbrInterface.resetDepth = -1;
		g_interfaces.cbrInterface.resetPlayer = -1;
		//g_interfaces.cbrInterface.pMatchState = *g_gameVals.pMatchState;
		g_interfaces.cbrInterface.inputMemory = { 5,5 };
		g_interfaces.cbrInterface.writeMemory = { -1,-1 };
		g_interfaces.cbrInterface.inputMemoryHirarchy = { -1,-1 };
		g_interfaces.cbrInterface.executionOrder = {};
		g_interfaces.cbrInterface.reversalInput = 5;
		g_interfaces.cbrInterface.readDepth[0] = 99;
		g_interfaces.cbrInterface.readDepth[1] = 99;
		g_interfaces.cbrInterface.writeDepth[0] = -1;
		g_interfaces.cbrInterface.writeDepth[1] = -1;
	}
	g_interfaces.cbrInterface.executionOrder[hirarchy * 10 + playerNr] = true;
	if (write && g_interfaces.cbrInterface.writeMemory[playerNr] != -1 && *input == 5) {
		*input = g_interfaces.cbrInterface.writeMemory[playerNr];
	}
	bool readHere = g_interfaces.cbrInterface.readDepth[playerNr] == hirarchy && read;
	bool writeHere = g_interfaces.cbrInterface.writeDepth[playerNr] == hirarchy && write;

	if (*input != 5 && *input != 0) {
		if (g_interfaces.cbrInterface.inputMemory[playerNr] != 5 && g_interfaces.cbrInterface.inputMemory[playerNr] != 0 && *input != g_interfaces.cbrInterface.inputMemory[playerNr]) {
			g_interfaces.cbrInterface.inputMemory[playerNr] = *input;
		}
		g_interfaces.cbrInterface.inputMemory[playerNr] = *input;
		g_interfaces.cbrInterface.inputMemoryHirarchy[playerNr] = hirarchy;
	}
	if (controllerNr >= 0) {
		g_interfaces.cbrInterface.controllerMemory[playerNr] = controllerNr;
	}
	if (*input != 0 && readHere || writeHere) {
		if (!netplayMemory) {
			*input = g_interfaces.cbrInterface.inputMemory[playerNr];
		}
		auto controllerNr = g_interfaces.cbrInterface.controllerMemory[playerNr];
#ifdef DEBUG
		std::string s = "CBRTrigger: readHere: " + std::to_string(readHere) + " -writeHere: " + std::to_string(writeHere)
			+ " -playerNr: " + std::to_string(playerNr) + " -input: " + std::to_string(input) + " -controllerNr: " + std::to_string(controllerNr) + "\n";
		LOG(2, s.c_str());
#endif
		result = CBRLogic(input, playerNr, controllerNr, readHere, writeHere, netplayMemory);
		g_interfaces.cbrInterface.inputMemory[playerNr] = *input;
		if (writeHere) {
			g_interfaces.cbrInterface.writeMemory[playerNr] = *input;
		}
	}
	else {
		if (g_interfaces.cbrInterface.resetDepth < hirarchy) {
			g_interfaces.cbrInterface.resetDepth = hirarchy;
			g_interfaces.cbrInterface.resetPlayer = playerNr;
		}
		if (g_interfaces.cbrInterface.readDepth[playerNr] > hirarchy && read) {
			g_interfaces.cbrInterface.readDepth[playerNr] = hirarchy;
		}
		if (g_interfaces.cbrInterface.writeDepth[playerNr] < hirarchy && write) {
			g_interfaces.cbrInterface.writeDepth[playerNr] = hirarchy;
		}
		//input = g_interfaces.cbrInterface.inputMemory[playerNr];
	}
	//input = reversalLogic(input, playerNr, read, writeHere);
	if (playerNr == 0) {
		g_interfaces.cbrInterface.input = *input;
	}
	if (playerNr == 1) {
		g_interfaces.cbrInterface.inputP2 = *input;
	}

	return result;
}

void CBRLogic(uint16* input, int hirarchy, bool readOnly, bool writeOnly, int playerNr, bool netplayMemory) {
	if (playerNr < 0 || playerNr > 1) {
		return;
	}
	if (!*g_gameVals.pInGameFlag || *g_gameVals.pInPauseMenu)
		return;

	if (g_interfaces.Player1.GetData()->charIndex == 0 || g_interfaces.Player2.GetData()->charIndex == 0) //characters weren't initialized yet
		return;

	int input_bb = GGtoBBInput(input, playerNr);
	if (input_bb >= 0)
	{
		/*if (CheckStylishMode(playerNr))
		{
			input_bb = g_interfaces.stylishModeManager.ComputeNextAction(input_bb, playerNr);
			uint16 input_gg = BBtoGGInput(input_bb, playerNr);
			*input = input_gg;
		}
		else */if (CBRLogic(&input_bb, hirarchy, playerNr, playerNr, readOnly, writeOnly, netplayMemory))
		{
			uint16 input_gg = BBtoGGInput(input_bb, playerNr);
			*input = input_gg;
		}
	}
}

bool CBRLogic(int* input, int playerNr, int controllerNr, bool readOnly, bool writeOnly, bool netplayMemory) {
	static std::array<int, 2> oldInput{ 5,5 };
	bool result = false;
	g_interfaces.cbrInterface.debugNr = playerNr;
	if (playerNr == 0) {

		if (g_interfaces.cbrInterface.instantLearningP2 == true) {
			auto& err = g_interfaces.cbrInterface.getCbrData(0)->getInstantLearnReplay()->instantLearning(g_interfaces.cbrInterface.getAnnotatedReplay(1), g_interfaces.cbrInterface.getAnnotatedReplay(1)->getFocusCharName());
			g_interfaces.cbrInterface.debugErrorCounter[playerNr] += err.errorCount;
		}
		if (g_interfaces.cbrInterface.netaPlaying || g_interfaces.cbrInterface.netaRecording || g_interfaces.cbrInterface.autoRecordActive || g_interfaces.cbrInterface.Replaying || g_interfaces.cbrInterface.Recording || g_interfaces.cbrInterface.instantLearning == true || g_interfaces.cbrInterface.instantLearningP2 == true) {

			auto meta = RecordCbrMetaData(0, oldInput[playerNr]);
			meta->computeMetaData();
			RecordCbrHelperData(meta, 0);

			result = netaLogic(input, meta, 0, readOnly, writeOnly);
			if (writeOnly && controllerNr >= 0 && (g_interfaces.cbrInterface.Replaying || g_interfaces.cbrInterface.instantLearningP2 == true)) {
				*input = g_interfaces.cbrInterface.getCbrData(0)->CBRcomputeNextAction(meta.get());
				result = true;
			}
			if (readOnly && ((g_interfaces.cbrInterface.autoRecordActive && ((g_interfaces.cbrInterface.autoRecordGameOwner && g_interfaces.cbrInterface.CheckGameOwner(playerNr)) || (g_interfaces.cbrInterface.autoRecordAllOtherPlayers && !g_interfaces.cbrInterface.CheckGameOwner(playerNr)))) || g_interfaces.cbrInterface.Recording || g_interfaces.cbrInterface.instantLearning == true)) {
				if (netplayMemory) {
					auto meta2 = g_interfaces.cbrInterface.netplayMemory[0];
					g_interfaces.cbrInterface.netplayMemory[0] = meta;
					meta = meta2;
				}
				if (input != 0 && meta != nullptr) {
					g_interfaces.cbrInterface.getAnnotatedReplay(0)->AddFrame(meta, *input);
				}

			}

		}
		else {
			InactiveMetaDataUpdate(0, oldInput[playerNr]);
		}

	}
	if (playerNr == 1) {

		if (g_interfaces.cbrInterface.instantLearning == true) {
			auto& err = g_interfaces.cbrInterface.getCbrData(1)->getInstantLearnReplay()->instantLearning(g_interfaces.cbrInterface.getAnnotatedReplay(0), g_interfaces.cbrInterface.getAnnotatedReplay(0)->getFocusCharName());
			g_interfaces.cbrInterface.debugErrorCounter[playerNr] += err.errorCount;
		}
		LOG_ASM(7, "ActiveCheckP2\n");
		if (g_interfaces.cbrInterface.netaPlaying || g_interfaces.cbrInterface.netaRecording || g_interfaces.cbrInterface.autoRecordActive || g_interfaces.cbrInterface.ReplayingP2 || g_interfaces.cbrInterface.RecordingP2 || g_interfaces.cbrInterface.instantLearning == true || g_interfaces.cbrInterface.instantLearningP2 == true) {

			auto meta = RecordCbrMetaData(1, oldInput[playerNr]);
			meta->computeMetaData();
			RecordCbrHelperData(meta, 1);
			//input = reversalLogic(input, meta, 2, readOnly, writeOnly);
			result = netaLogic(input, meta, 1, readOnly, writeOnly);
			if (writeOnly && controllerNr >= 0 && (g_interfaces.cbrInterface.ReplayingP2 || g_interfaces.cbrInterface.instantLearning == true)) {
				*input = g_interfaces.cbrInterface.getCbrData(1)->CBRcomputeNextAction(meta.get());
				result = true;
			}
			LOG_ASM(6, "PreReadP2\n");
			if (readOnly && ((g_interfaces.cbrInterface.autoRecordActive && ((g_interfaces.cbrInterface.autoRecordGameOwner && g_interfaces.cbrInterface.CheckGameOwner(playerNr)) || (g_interfaces.cbrInterface.autoRecordAllOtherPlayers && !g_interfaces.cbrInterface.CheckGameOwner(playerNr)))) || g_interfaces.cbrInterface.RecordingP2 || g_interfaces.cbrInterface.instantLearningP2 == true)) {
				LOG_ASM(6, "ReadP2\n");
				if (netplayMemory) {
					auto meta2 = g_interfaces.cbrInterface.netplayMemory[1];
					g_interfaces.cbrInterface.netplayMemory[1] = meta;
					meta = meta2;
				}
				if (input != 0 && meta != nullptr) {
					g_interfaces.cbrInterface.getAnnotatedReplay(1)->AddFrame(meta, *input);
				}
			}

		}
		else {
			InactiveMetaDataUpdate(1, oldInput[playerNr]);
		}
	}
	if (writeOnly || readOnly) {
		oldInput[playerNr] = *input;
	}
	return result;
}

bool netaLogic(int* input, std::shared_ptr<Metadata> meta, int playerNR, bool readOnly, bool writeOnly) {
	bool result = false;
	if (g_interfaces.cbrInterface.netaRecording && readOnly) {
		g_interfaces.cbrInterface.getAnnotatedReplay(playerNR)->AddFrame(meta, *input);
	}

	if (g_interfaces.cbrInterface.netaPlaying && writeOnly) {
		if (g_interfaces.cbrInterface.getAnnotatedReplay(playerNR)->getInput().size() > g_interfaces.cbrInterface.netaReplayCounter) {
			*input = g_interfaces.cbrInterface.getAnnotatedReplay(playerNR)->getInput()[g_interfaces.cbrInterface.netaReplayCounter];
			result = true;
			if (playerNR == 1) {
				g_interfaces.cbrInterface.netaReplayCounter++;
			}

		}
		else {
			g_interfaces.cbrInterface.netaReplayCounter = 0;
			g_interfaces.cbrInterface.netaPlaying = false;
		}

	}
	return result;
}

#define TButton 512 //Taunt
#define DButton 256 //Dust
#define HButton 128 //Heavy slash
#define SButton 64 //Slash
#define KButton 32 //Kick
#define PButton 16 //Punch

#define UpDirection 16
#define RightDirection 32
#define DownDirection 64
#define LeftDirection 128

//Convert guilty gear input to blazblue input for better compatibility with existing code
int GGtoBBInput(uint16* input_gg, int playerNr)
{
	char* pInput = reinterpret_cast<char*>(input_gg);
	Player currentPlayer;
	if (playerNr == 0)
		currentPlayer = g_interfaces.Player1;
	else if (playerNr == 1)
		currentPlayer = g_interfaces.Player2;
	else
		return -1;
	byte button_flags = *(pInput + 1);
	byte direction_flags = *pInput;

	int direction_bb = 5, button_bb = 0;
	byte test;

	if (direction_flags)
	{
		test = UpDirection | LeftDirection;
		if ((direction_flags & test) == test)
			direction_bb = 7;
		else {
			test = UpDirection | RightDirection;
			if ((direction_flags & test) == test)
				direction_bb = 9;
			else {
				test = DownDirection | LeftDirection;
				if ((direction_flags & test) == test)
					direction_bb = 1;
				else {
					test = DownDirection | RightDirection;
					if ((direction_flags & test) == test)
						direction_bb = 3;
					else {
						if ((direction_flags & UpDirection) == UpDirection)
							direction_bb = 8;
						else {
							if ((direction_flags & RightDirection) == RightDirection)
								direction_bb = 6;
							else {
								if ((direction_flags & DownDirection) == DownDirection)
									direction_bb = 2;
								else {
									if ((direction_flags & LeftDirection) == LeftDirection)
										direction_bb = 4;
									else
										return -1; //Used for start, select, LS and RS.
								}
							}
						}
					}
				}
			}
		}
	}

	if (button_flags)
	{
		if ((*input_gg & currentPlayer.GetData()->extraData->pButton) > 0)
			button_bb += PButton;
		if ((*input_gg & currentPlayer.GetData()->extraData->kButton) > 0)
			button_bb += KButton;
		if ((*input_gg & currentPlayer.GetData()->extraData->sButton) > 0)
			button_bb += SButton;
		if ((*input_gg & currentPlayer.GetData()->extraData->hButton) > 0)
			button_bb += HButton;
		if ((*input_gg & currentPlayer.GetData()->extraData->dButton) > 0)
			button_bb += DButton;
		if ((*input_gg & currentPlayer.GetData()->extraData->tButton) > 0)
			button_bb += TButton;

		if (!button_bb)
			return -1; //If a button that is not bound to an attack button was pressed.
	}
	
	return direction_bb + button_bb;
}

uint16 BBtoGGInput(int input_bb, int playerNr)
{
	uint16 input_gg = 0;
	Player currentPlayer;
	if (playerNr == 0)
		currentPlayer = g_interfaces.Player1;
	else if (playerNr == 1)
		currentPlayer = g_interfaces.Player2;

	auto test = input_bb - TButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->tButton;
	}
	test = input_bb - DButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->dButton;
	}
	test = input_bb - HButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->hButton;
	}
	test = input_bb - SButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->sButton;
	}
	test = input_bb - KButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->kButton;
	}
	test = input_bb - PButton;
	if (test > 0) {
		input_bb = test;
		input_gg = input_gg | currentPlayer.GetData()->extraData->pButton;
	}

	switch (input_bb)
	{
	default:
		break;
	case 1:
		input_gg = input_gg | (DownDirection | LeftDirection);
		break;
	case 2:
		input_gg = input_gg | DownDirection;
		break;
	case 3:
		input_gg = input_gg | (DownDirection | RightDirection);
		break;
	case 4:
		input_gg = input_gg | LeftDirection;
		break;
	case 5:
		break;
	case 6:
		input_gg = input_gg | RightDirection;
		break;
	case 7:
		input_gg = input_gg | (UpDirection | LeftDirection);
		break;
	case 8:
		input_gg = input_gg | UpDirection;
		break;
	case 9:
		input_gg = input_gg | (UpDirection | RightDirection);
		break;
	}

	return input_gg;
}

std::array<int, 2> oldPosX;
std::array<int, 2> oldPosY;
std::array<uint8_t, 2> lastActId;
std::array<uint32_t, 2> lastState;
std::array<uint32_t, 2> lastAttackFlags;

void InactiveMetaDataUpdate(bool PlayerIndex, int input) {
	CharData* focusCharData;
	CharData* enemyCharData;
	if (PlayerIndex == 0) {
		focusCharData = g_interfaces.Player1.GetData();
		enemyCharData = g_interfaces.Player2.GetData();

		lastAttackFlags[0] = focusCharData->attackFlags;
		lastAttackFlags[1] = focusCharData->attackFlags;
		lastActId[0] = focusCharData->actId;
		lastActId[1] = enemyCharData->actId;
		lastState[0] = focusCharData->status;
		lastState[1] = enemyCharData->status;
	}
	else {
		focusCharData = g_interfaces.Player2.GetData();
		enemyCharData = g_interfaces.Player1.GetData();
	}
	//Velocity
	oldPosX[0] = focusCharData->posX;
	oldPosX[1] = enemyCharData->posX;
	oldPosY[0] = focusCharData->posY;
	oldPosY[1] = enemyCharData->posY;
}

std::shared_ptr<Metadata> RecordCbrMetaData(bool PlayerIndex, int input) {
	//static std::array<FixedQueue<int, 5>, 2> inputBuffer;

	CharData* focusCharData;
	CharData* enemyCharData;
	char* lastact = "CmnActStand"; //To be removed
	if (PlayerIndex == 0) {
		focusCharData = g_interfaces.Player1.GetData();
		enemyCharData = g_interfaces.Player2.GetData();
	}
	else {
		focusCharData = g_interfaces.Player2.GetData();
		enemyCharData = g_interfaces.Player1.GetData();
	}

	auto p1X = focusCharData->posX;
	auto p1Y = focusCharData->posY;
	auto p2X = enemyCharData->posX;
	auto p2Y = enemyCharData->posY;
	auto facing = !focusCharData->facingRight; //original code used facing left
	auto p1State = std::string("CmnActStand"); //To be removed
	auto p2State = std::string("CmnActStand");
	auto p1actId = focusCharData->actId;
	auto p2actId = enemyCharData->actId;
	auto p1StateFlags = focusCharData->status;
	auto p2StateFlags = enemyCharData->status;
	auto p1Block = p1StateFlags & 0x0200 ? focusCharData->totalBlockstun - focusCharData->frameCounter + 2 : 0; //If character is in blockstun, the remaining time until he returns to neutral is given by totalBlockstun - frameCounter. Add 2 so that blockstun ends at 0.
	auto p2Block = p2StateFlags & 0x0200 ? enemyCharData->totalBlockstun - enemyCharData->frameCounter + 2 : 0;
	auto p1Hit = p1StateFlags & 0x0020 ? focusCharData->frameCounter : 0; //Since there is no counter for hitstun in memory, use number of frames in current animation when in hitstun to determine hitstun frames.
	auto p2Hit = p2StateFlags & 0x0020 ? enemyCharData->frameCounter : 0;
	auto p1atkType = focusCharData->actFlags & 0x0400 ? 3 : (focusCharData->actFlags & 0x0200 ? 2 : (focusCharData->actFlags & 0x0008 ? 1 : 0)); //Tests flags for animation id. Overdrives have 0x0400 in them, specials have 0x0200 and all attacks have 0x0008. Use same numbering scheme as BB.
	auto p2atkType = enemyCharData->actFlags & 0x0400 ? 3 : (enemyCharData->actFlags & 0x0200 ? 2 : (enemyCharData->actFlags & 0x0008 ? 1 : 0));
	auto p1hitstop = focusCharData->hitstopTimer; //For some reason, +R counts hitstop in steps of 2 and not 1. Probably shouldn't affect the algorithm.
	auto p2hitstop = enemyCharData->hitstopTimer;
	auto p1actionTimeNHS = focusCharData->frameCounter; //Number of frames in current action (counts while hitstop is active as well)
	auto p2actionTimeNHS = enemyCharData->frameCounter; //In +R this is equivalent to BB actionTime which does not count while in hitstun.
	auto p1lastAction = lastact;
	auto p2lastAction = lastact;
	auto p1lastActId = lastActId[0];
	auto p2lastActId = lastActId[1];
	auto frameCount = *g_gameVals.pframe_count_minus_1_P1;
	auto meta = std::make_shared<Metadata>(p1X, p2X, p1Y, p2Y, facing, p1State, p2State, p1actId, p2actId, p1StateFlags, p2StateFlags, lastState[PlayerIndex], lastState[!PlayerIndex], p1Block, p2Block, p1Hit, p2Hit, p1atkType, p2atkType, p1hitstop, p2hitstop, p1actionTimeNHS, p2actionTimeNHS, p1lastAction, p2lastAction, p1lastActId, p2lastActId);
	auto p1comboProration = focusCharData->extraData->baseComboDamage;
	auto p2comboProration = enemyCharData->extraData->baseComboDamage;
	auto p1starterRating = 0; //Does not exist in +R.
	auto p2starterRating = 0;
	auto p1comboTime = focusCharData->extraData->damageTime;
	auto p2comboTime = enemyCharData->extraData->damageTime;
	meta->SetComboVariables(p1comboProration, p2comboProration, p1starterRating, p2starterRating, p1comboTime, p2comboTime);
	meta->SetFrameCount(frameCount);
	meta->timeAfterRecovery[0] = -1; //Not used
	meta->timeAfterRecovery[1] = -1;
	meta->matchState = g_gameVals.GetMatchState();
	meta->heatMeter[0] = focusCharData->extraData->tension;
	meta->heatMeter[1] = enemyCharData->extraData->tension;
	meta->overdriveMeter[0] = focusCharData->extraData->burstGauge;
	meta->overdriveMeter[1] = enemyCharData->extraData->burstGauge;
	meta->overdriveTimeleft[0] = 0;
	meta->overdriveTimeleft[1] = 0;
	meta->healthMeter[0] = focusCharData->currentHP;
	meta->healthMeter[1] = enemyCharData->currentHP;
	meta->opponentId = enemyCharData->charIndex;

	lastActId[PlayerIndex] = focusCharData->actId;
	lastActId[!PlayerIndex] = enemyCharData->actId;
	lastState[PlayerIndex] = focusCharData->status;
	lastState[!PlayerIndex] = enemyCharData->status;

	meta->hitMinX = 0;
	meta->hitMinY = 0;
	if ((focusCharData->attackFlags & 0x00080000) && !(lastAttackFlags[PlayerIndex] & 0x00080000)) {
		//When hitbox intersects opponent add a min distance to the case
		auto minDist = g_interfaces.cbrInterface.GetMinDistance(focusCharData, enemyCharData);
		meta->hitMinX = minDist.x;
		meta->hitMinY = minDist.y;
	}
	lastAttackFlags[PlayerIndex] = focusCharData->attackFlags;
	lastAttackFlags[!PlayerIndex] = enemyCharData->attackFlags;
	//Preprocessing inputs
	
	static int oldBuffer = 5;
	auto buffer = oldBuffer;
	oldBuffer = input;

	auto test = buffer - TButton;
	if (test > 0) {
		buffer = test;
		meta->inputT = true;
	}
	test = buffer - DButton;
	if (test > 0) {
		buffer = test;
		meta->inputD = true;
	}
	test = buffer - HButton;
	if (test > 0) {
		buffer = test;
		meta->inputH = true;
	}
	test = buffer - SButton;
	if (test > 0) {
		buffer = test;
		meta->inputS = true;
	}
	test = buffer - KButton;
	if (test > 0) {
		buffer = test;
		meta->inputK = true;
	}
	test = buffer - PButton;
	if (test > 0) {
		buffer = test;
		meta->inputP = true;
	}
	if (facing) {
		if (buffer == 6 || buffer == 3 || buffer == 9) {
			buffer = buffer - 2;
		}
		else {
			if (buffer == 4 || buffer == 7 || buffer == 1) {
				buffer = buffer + 2;
			}
		}
	}
	//inputBuffer[PlayerIndex].push(buffer);
	switch (buffer)
	{
	default:
		break;
	case 1:
		meta->inputFwd = false, meta->inputUp = false, meta->inputDown = true, meta->inputBack = true;
		break;
	case 2:
		meta->inputFwd = false, meta->inputUp = false, meta->inputDown = true, meta->inputBack = false;
		break;
	case 3:
		meta->inputFwd = true, meta->inputUp = false, meta->inputDown = true, meta->inputBack = false;
		break;
	case 4:
		meta->inputFwd = false, meta->inputUp = false, meta->inputDown = false, meta->inputBack = true;
		break;
	case 5:
		meta->inputFwd = false, meta->inputUp = false, meta->inputDown = false, meta->inputBack = false;
		break;
	case 6:
		meta->inputFwd = true, meta->inputUp = false, meta->inputDown = false, meta->inputBack = false;
		break;
	case 7:
		meta->inputFwd = false, meta->inputUp = true, meta->inputDown = false, meta->inputBack = true;
		break;
	case 8:
		meta->inputFwd = false, meta->inputUp = true, meta->inputDown = false, meta->inputBack = false;
		break;
	case 9:
		meta->inputFwd = true, meta->inputUp = true, meta->inputDown = false, meta->inputBack = false;
		break;
	}

	//Velocity
	oldPosX[0] = focusCharData->posX;
	oldPosX[1] = enemyCharData->posX;
	oldPosY[0] = focusCharData->posY;// _dupe;
	oldPosY[1] = enemyCharData->posY;// _dupe;
	meta->velocity[0][0] = focusCharData->posX - oldPosX[0];
	meta->velocity[0][1] = focusCharData->posY - oldPosY[0];
	meta->velocity[1][0] = enemyCharData->posX - oldPosX[1];
	meta->velocity[1][1] = enemyCharData->posY - oldPosY[1];
	oldPosX[0] = focusCharData->posX;
	oldPosY[0] = focusCharData->posY;// _dupe;
	oldPosX[1] = enemyCharData->posX;
	oldPosY[1] = enemyCharData->posY;// _dupe;


	int buffVal = 0;
	int testVal = 0;
	CharData* pEntity;
	switch (focusCharData->charIndex)
	{
	case 1: //Sol
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1; //Sol dragon install timer (max 360)
		break;
	case 4: //Millia
		meta->CharSpecific1[0] = focusCharData->extraData->projectileFlag; //Millia silent force active
		break;
	case 7: //Chipp
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT4; //Chipp shuriken type
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT3; //Chipp invisibility timer (max 480)
		break;
	case 8: //Eddie
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT3; //Eddie gauge (max 8000, decreases by 1200 each time Eddie attacks).
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT1 >> 8; //Eddie gauge state (0 = recovering, 1 = can summon)
		meta->CharSpecific3[0] = focusCharData->extraData->characterSLOT4 >> 8; //Eddie type (0 = normal, 1 = vice)
		meta->CharSpecific4[0] = ((~focusCharData->extraData->projectileFlag) & 0x04) == 0; //Eddie active (this test is used in the game code)
		break;
	case 9: //Baiken
		meta->CharSpecific1[0] = enemyCharData->extraData->dashSealTimer; //(max 210)
		meta->CharSpecific2[0] = enemyCharData->extraData->allCounterTimer; //(max 140)
		meta->CharSpecific3[0] = enemyCharData->extraData->guardSealTimer; //(max 90)
		meta->CharSpecific4[0] = enemyCharData->extraData->jumpSealTimer; //(max 210)
		meta->CharSpecific5[0] = enemyCharData->extraData->pSealTimer;
		meta->CharSpecific6[0] = enemyCharData->extraData->kSealTimer;
		meta->CharSpecific7[0] = enemyCharData->extraData->sSealTimer;
		meta->CharSpecific8[0] = enemyCharData->extraData->hSealTimer;
		meta->CharSpecific9[0] = enemyCharData->extraData->dSealTimer;
		break;
	case 10: //Faust
		meta->CharSpecific1[0] = focusCharData->extraData->projectileFlag; //Is an item active?
		break;
	case 11: //Testament
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT3; //Testament puppets
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT5; //Number of times crow attacked
		meta->CharSpecific3[0] = focusCharData->extraData->characterSLOT1 == 5; //Is crow attacking?
		meta->CharSpecific4[0] = focusCharData->extraData->characterSLOT6; //Crow pattern
		break;
	case 12: //Jam
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT2; //Asanagi-K
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT3 >> 8; //Asangi-S
		meta->CharSpecific3[0] = focusCharData->extraData->characterSLOT3 & 0x00FF; //Asanagi-H
		break;
	case 14: //Johnny
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1; //MF level
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT3; //Number of coins used
		meta->CharSpecific3[0] = focusCharData->actId == 202;              //In mist stance
		break;
	case 15: //Venom
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1 >> 8; //Stinger charge level (max 100, lv2 above 22, lv3 above 60).
		if (*g_gameVals.entityCount > 0)
		{
			pEntity = reinterpret_cast<CharData*>(*g_gameVals.pEntityStartAddress);

			bool isEntityActive = pEntity->charIndex > 0;
			while (isEntityActive)
			{
				if (pEntity->charIndex == 34 && pEntity->parent->playerID == PlayerIndex)
				{
					meta->CharSpecific2[0] = pEntity->actId == 6; //P-ball active
					meta->CharSpecific3[0] = pEntity->actId == 7; //K-ball active
					meta->CharSpecific4[0] = pEntity->actId == 8; //S-ball active
					meta->CharSpecific5[0] = pEntity->actId == 9; //H-ball active
					meta->CharSpecific6[0] = pEntity->actId == 30; //D-ball active
				}
				pEntity = pEntity->nextEntity;
				isEntityActive = pEntity->charIndex > 0;
			}
		}
		break;
	case 19: //Zappa
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1; //Summon type
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT3; //Raou gauge (max 1000)
		break;
	case 20: //Bridget
		meta->CharSpecific1[0] = focusCharData->extraData->projectileFlag; //Yo-yo state
		break;
	case 21: //Robo-ky
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT3; //Heat gauge (max 10000, burning after 8000)
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT4; //Self-destruct timer (max 600)
		break;
	case 22: //A.B.A
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1; //A.B.A mode (moroha, goku-moroha)
		meta->CharSpecific2[0] = focusCharData->extraData->characterSLOT2; //A.B.A blood packs
		meta->CharSpecific3[0] = focusCharData->extraData->characterSLOT5; //A.B.A gauge (max 10000, lose 3000 when down)
		break;
	case 23: //Order-Sol
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT7; //Order-Sol gauge (max 5999, changes level each 2000)
		break;
	case 25: //Justice
		meta->CharSpecific1[0] = focusCharData->extraData->characterSLOT1; //Install counter (max 720).
		break;
	default:
		break;
	}
	switch (enemyCharData->charIndex)
	{
	case 1: //Sol
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1; //Sol dragon install timer
		break;
	case 4: //Millia
		meta->CharSpecific1[1] = enemyCharData->extraData->projectileFlag; //Millia silent force active
		break;
	case 7: //Chipp
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT4; //Chipp shuriken type
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT3; //Chipp invisibility timer
		break;
	case 8: //Eddie
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT3; //Eddie gauge
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT1 >> 8; //Eddie gauge state (0 = recovering, 1 = can summon)
		meta->CharSpecific3[1] = enemyCharData->extraData->characterSLOT4 >> 8; //Eddie type (0 = normal, 1 = vice).
		meta->CharSpecific4[0] = ((~focusCharData->extraData->projectileFlag) & 0x04) == 0; //Eddie active (this test is used in the game code)
		break;
	case 9: //Baiken
		meta->CharSpecific1[1] = focusCharData->extraData->dashSealTimer;
		meta->CharSpecific2[1] = focusCharData->extraData->allCounterTimer;
		meta->CharSpecific3[1] = focusCharData->extraData->guardSealTimer;
		meta->CharSpecific4[1] = focusCharData->extraData->jumpSealTimer;
		meta->CharSpecific5[1] = focusCharData->extraData->pSealTimer;
		meta->CharSpecific6[1] = focusCharData->extraData->kSealTimer;
		meta->CharSpecific7[1] = focusCharData->extraData->sSealTimer;
		meta->CharSpecific8[1] = focusCharData->extraData->hSealTimer;
		meta->CharSpecific9[1] = focusCharData->extraData->dSealTimer;
		break;
	case 10: //Faust
		meta->CharSpecific1[1] = enemyCharData->extraData->projectileFlag; //Is an item active?
		break;
	case 11: //Testament
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT3; //Testament puppets
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT5; //Number of times crow attacked
		meta->CharSpecific3[1] = enemyCharData->extraData->characterSLOT1 == 5; //Is crow attacking?
		meta->CharSpecific4[1] = enemyCharData->extraData->characterSLOT6; //Crow pattern
		break;
	case 12: //Jam
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT2; //Asanagi-K
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT3 >> 8; //Asangi-S
		meta->CharSpecific3[1] = enemyCharData->extraData->characterSLOT3 & 0x00FF; //Asanagi-H
		break;
	case 14: //Johnny
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1; //MF level
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT3; //Number of coins used
		meta->CharSpecific3[1] = enemyCharData->actId == 202;              //In mist stance
		break;
	case 15: //Venom
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1 >> 8; //Stinger charge level
		if (*g_gameVals.entityCount > 0)
		{
			pEntity = reinterpret_cast<CharData*>(*g_gameVals.pEntityStartAddress);

			bool isEntityActive = pEntity->charIndex > 0;
			while (isEntityActive)
			{
				if (pEntity->charIndex == 34 && pEntity->parent->playerID != PlayerIndex)
				{
					meta->CharSpecific2[1] = pEntity->actId == 6; //P-ball active
					meta->CharSpecific3[1] = pEntity->actId == 7; //K-ball active
					meta->CharSpecific4[1] = pEntity->actId == 8; //S-ball active
					meta->CharSpecific5[1] = pEntity->actId == 9; //H-ball active
					meta->CharSpecific6[1] = pEntity->actId == 30; //D-ball active
				}
				pEntity = pEntity->nextEntity;
				isEntityActive = pEntity->charIndex > 0;
			}
		}
		break;
	case 19: //Zappa
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1; //Summon type
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT3; //Raou gauge
		break;
	case 20:
		meta->CharSpecific1[1] = enemyCharData->extraData->projectileFlag; //Yo-yo state
		break;
	case 21: //Robo-ky
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT3; //Heat gauge
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT4; //Self-destruct timer
		break;
	case 22: //A.B.A
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1; //A.B.A mode (moroha, goku-moroha)
		meta->CharSpecific2[1] = enemyCharData->extraData->characterSLOT2; //A.B.A blood packs
		meta->CharSpecific3[1] = enemyCharData->extraData->characterSLOT5; //A.B.A gauge
		break;
	case 23: //Order-Sol
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT7; //Order-Sol gauge
		break;
	case 25: //Justice
		meta->CharSpecific1[1] = enemyCharData->extraData->characterSLOT1; //Install counter
		break;
	default:
		break;
	}
	return meta;



}

std::array<uint16_t, 10> DizzyBitIds = {
	0, //Bit summoned
	6, //Bit fly
	7, //Bit S lazer
	8, //Bit H lazer
	9, //Bit finish lazer
	10, //Bit destroyed
	60, //Bit P bite
	61, //Bit K first bite
	62, //Bit K second bite
	94 //Bit D bite
};

std::array<uint16_t, 6> DizzyBubbleIds = {
	50, //P bubble fly
	51, //K bubble fly
	53, //Bubble burst
	55, //Bubble S fly
	56, //Bubble D fly
	59, //Bubble D burst
};

void RecordCbrHelperData(std::shared_ptr<Metadata> me, bool PlayerIndex) {
	if (*g_gameVals.entityCount == 0)
		return;

	CharData* pEntity = reinterpret_cast<CharData*>(*g_gameVals.pEntityStartAddress);

	bool isEntityActive = pEntity->charIndex > 0;
	while (isEntityActive)
	{
		bool attacking = false;
		//Check if an active hitbox exists to determine if entity is attacking
		if (!(pEntity->status & 0x0040)) //If hitboxes are disabled, don't check them
		{
			std::vector<Hitbox> entries = HitboxReader::getHitboxes(pEntity);
			for (const Hitbox& entry : entries)
			{
				if (entry.type == HitboxType_Hitbox)
				{
					attacking = true;
					break;
				}
			}
		}

		bool proximityScale = false;


		auto pIndex = 0;
		uint16_t pid = 0;

		if (pEntity->parent == g_interfaces.Player1.GetData()) {
			if (PlayerIndex == 0) {
				pIndex = 0;
			}
			else {
				pIndex = 1;
			}
			pid = g_interfaces.Player1.GetData()->charIndex;
		}
		else {
			if (PlayerIndex == 0) {
				pIndex = 1;
			}
			else {
				pIndex = 0;
			}
			pid = g_interfaces.Player2.GetData()->charIndex;
		}

		//character specific projectiles like dolls that are more complex need special treatment
		bool specialProjectile = false;
		proximityScale = false;
		std::string specialName = "";
		std::size_t found;

		switch (pid)
		{
		default:
			break;
		case 3: //May
			if (pEntity->charIndex == 72) //Dolphin hoop
			{
				specialProjectile = true;
				specialName = "Hoop";
			}
			break;
		case 4: //Millia
			if (pEntity->charIndex == 46) //Pin
			{
				specialProjectile = true;
				proximityScale = true;
				specialName = "Pin";
			}
			break;
		case 8: //Eddie
			if (pEntity->charIndex == 35) //Eddie
			{
				specialProjectile = true;
				specialName = "Eddie";
			}
			break;
		case 11: //Testament
			if (pEntity->charIndex == 80) //Tree or exe-beast
			{
				specialProjectile = true;
				specialName = "TreeBeast";
			}
			break;
		case 15: //Venom
			if (pEntity->charIndex == 34) //Balls
			{
				specialProjectile = true;
				proximityScale = true;
				specialName = "Poolball";
			}
			break;
		case 16:
			if (pEntity->charIndex == 67) //Bit, bubble
			{
				for (int i = 0; i < DizzyBitIds.size(); i++)
				{
					if (pEntity->actId == DizzyBitIds[i])
					{
						specialProjectile = true;
						specialName = "Bit";
						break;
					}
				}
				for (int i = 0; i < DizzyBubbleIds.size(); i++)
				{
					if (pEntity->actId == DizzyBubbleIds[i])
					{
						specialProjectile = true;
						specialName = "Bubble";
						break;
					}
				};
			}
			break;
		case 19: //Zappa
			if (pEntity->charIndex == 87) //Raou
			{
				specialProjectile = true;
				specialName = "Raou";
			}
			else if (pEntity->charIndex == 91) //Triplets
			{
				specialProjectile = true;
				specialName = "Triplets";
			}
			else if (pEntity->charIndex == 93) //Sword
			{
				specialProjectile = true;
				specialName = "Sword";
			}
			else if (pEntity->charIndex == 96) //Dog
			{
				specialProjectile = true;
				specialName = "Dog";
			}
			break;
		case 20: //Bridget
			if (pEntity->charIndex == 92) //Roger
			{
				specialProjectile = true;
				specialName = "Roger";
			}
			break;
		case 21: //Robo-ky
			if (pEntity->charIndex == 98 && pEntity->actId == 13) //Matt
			{
				specialProjectile = true;
				proximityScale = true;
				specialName = "Matt";
			}
			break;
		case 25: //Justice
			if (pEntity->charIndex == 111) //Nukes
			{
				specialProjectile = true;
				specialName = "Nuke";
			}
			break;
		}

		if (attacking || specialProjectile) {

			auto p1id = pEntity->charIndex;
			auto p1X = pEntity->posX;
			auto p1Y = pEntity->posY;
			auto facing = !pEntity->facingRight;
			auto p2Hit = 0;
			auto p1atkType = attacking;
			auto p1hitstop = pEntity->hitstopTimer;
			auto p1actionTimeNHS = pEntity->frameCounter;
			auto p1State = "CmnActStand";
			auto p1actId = pEntity->actId;
			auto p1attackFlags = pEntity->attackFlags;
			auto helper = std::make_shared<Helper>(p1id, p1X, p1Y, facing, p1State, p1actId, p2Hit, p1atkType, p1hitstop, p1actionTimeNHS, p1attackFlags);
			if (specialProjectile) {
				helper->type = specialName;
			}
			else {
				helper->type = "-";
			}
			helper->computeMetaData();
			helper->proximityScale = proximityScale;

			me->addHelper(helper, pIndex);
		}

		pEntity = pEntity->nextEntity;
		isEntityActive = pEntity->charIndex > 0;
	}
}

bool placeHooks_cbr()
{
	PInputJmpBackAddr = HookManager::SetHook("PInput", "\x89\x46\xFC\xF7\xD2", "xxxxx", 5, PInput);

	UnlockFPSInReplayJmpBackAddr = HookManager::SetHook("UnlockFPSInReplay", "\x66\x85\x4C\x73\x04\x74\x32", "xxxxxxx", 5, UnlockFPSInReplay);

	HookManager::RegisterHook("GetCurrentReplayItem", "\x3B\x0D\00\00\00\00\x75\x05", "xx????xx", 6);
	g_gameVals.pCurrentReplayItem = (int*)HookManager::GetBytesFromAddr("GetCurrentReplayItem", 2, 4);

	HookManager::RegisterHook("GetReplayMenuItemAddresses", "\x8B\x0D\x00\x00\x00\x00\x2B\x0D\x00\x00\x00\x00\xB8\x55\x3A\xB2\x67\xF7\xE9\x8B\x0D\x00\x00\x00\x00",
		"xx????xx????xxxxxxxxx????", 6);
	g_gameVals.pLastReplayItemAddr = (uint32_t*)HookManager::GetBytesFromAddr("GetReplayMenuItemAddresses", 2, 4);
	g_gameVals.pFirstReplayItemAddr = (uint32_t*)HookManager::GetBytesFromAddr("GetReplayMenuItemAddresses", 8, 4);

	PInputReplayJmpBackAddr = HookManager::SetHook("PInputReplay", "\x66\x89\x08\x83\xC0\x02\x4F", "xxxxxxx", 6, PInputReplay);

	return true;
}

void UpdateFrameMeter(int playerNr)
{
	if (playerNr != 0)
		return;

	if (*g_gameVals.pInGameFlag)
	{
		if (!g_gameVals.isFrameFrozen && !*g_gameVals.pInPauseMenu)
			g_interfaces.frameMeterInterface.UpdateFastForward();
	}
	if (g_gameVals.isFrameFrozen)
	{
		if (g_gameVals.framesToReach > *g_gameVals.pframe_count_minus_1_P1)
			g_interfaces.frameMeterInterface.UpdateFastForward();
	}
}