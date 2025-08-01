#pragma once

#include "D3D9Wrapper/d3d9.h"
#include "D3D9Wrapper/ID3D9Wrapper_Device.h"
#include "Game/Player.h"
#include "Game/Room/Room.h"
#include "Game/Room/RoomMemberEntry.h"
#include "Game/gamestates.h"
#include "Game/Camera.h"
#include "Network/NetworkManager.h"
#include "Network/OnlinePaletteManager.h"
#include "Network/RoomManager.h"
#include "Palette/PaletteManager.h"
#include "SteamApiWrapper/SteamApiHelper.h"
#include "SteamApiWrapper/SteamNetworkingWrapper.h"
#include "SteamApiWrapper/SteamUserStatsWrapper.h"
#include "Cbr/CbrInterface.h"
#include "Trainer/TrainerInterface.h"
#include "Overlay/FrameMeter/FrameMeterInterface.h"
#include "Trainer/ExtraMenu.h"
#include "Trainer/StylishModeManager.h"
#include <string.h>

struct interfaces_t
{
	SteamNetworkingWrapper* pSteamNetworkingWrapper;
	SteamUserStatsWrapper* pSteamUserStatsWrapper;

	IDirect3DDevice9Ex* pD3D9ExWrapper;
	IDirect3DDevice9* pD3D9Wrapper;

	NetworkManager* pNetworkManager;
	RoomManager* pRoomManager;
	SteamApiHelper* pSteamApiHelper;

	PaletteManager* pPaletteManager;
	OnlinePaletteManager* pOnlinePaletteManager;

	Player Player1;
	Player Player2;
	CbrInterface cbrInterface;

	Player* GetPlayer(int playerNr)
	{
		if (!playerNr)
			return &Player1;
		return &Player2;
	}

	TrainerInterface trainerInterface;
	FrameMeterInterface frameMeterInterface;
	//StylishModeManager stylishModeManager;
};

extern interfaces_t g_interfaces;

struct gameVals_t
{
	char* pGameScreen;
	byte* pInGameFlag;
	int* pInPauseMenu;
	byte* pInReplayPauseMenu;
	int* pIsPauseMenuDisplayed;
	int* pGameMode;
	int* pMatchState;
	int* pMatchTimer;
	int* pframe_count_minus_1_P1;
	int* pFPSCounter;
	int* pMatchRounds;
	int* pRoundCounter;
	int* pCurrentReplayItem;
	uint32_t* pFirstReplayItemAddr;
	uint32_t* pLastReplayItemAddr;
	byte* pGlobalThrowFlags;
	uint32_t *pSpectatingTest1, *pSpectatingTest2;
	uint64_t* pSteamID;
	bool loadingReplay = false;

	uint8_t GetGameMode()
	{
		if ((uint8_t)*pGameMode == GameMode_Online)
		{
			if (g_interfaces.pRoomManager->IsRoomFunctional())
			{
				switch (g_interfaces.pRoomManager->GetThisPlayerGameMode())
				{
				case RoomPlayerMode_MOM:
					return GameMode_MOM;
					break;
				case RoomPlayerMode_Survival:
					return GameMode_Survival;
					break;
				case RoomPlayerMode_Training:
					return GameMode_Training;
					break;
				case RoomPlayerMode_Replay:
					return GameMode_ReplayTheater;
					break;
				}
			}
		}
		//Check for special case when loading replay from .ggr file directly
		if (*pGameMode == 0 && pGameScreen != NULL && !strcmp(pGameScreen, "RPPB"))
			return GameMode_ReplayTheater;
		return (uint8_t)*pGameMode;
	}

	int GetGameState()
	{
		if (pGameScreen != NULL)
		{
			if (!strcmp(pGameScreen, "TITL"))
				return GameState_TitleScreen;
			if (!strcmp(pGameScreen, "OPTION"))
				return GameState_MainMenu;
			if (!strcmp(pGameScreen, "CHRSLCT") || !strcmp(pGameScreen, "1P_SLC") || !strcmp(pGameScreen, "2P_SLC"))
				return GameState_CharacterSelectionScreen;
			if (!strcmp(pGameScreen, "RPSL"))
				return GameState_ReplayMenu;
			if (!strcmp(pGameScreen, "VS  "))
				return GameState_VersusScreen;
		}
		if (pInGameFlag != NULL && *pInGameFlag)
			return GameState_InMatch;
		return 0;
	}

	int GetMatchState()
	{
		if (pGameScreen != NULL)
		{
			if (!strcmp(pGameScreen, "GINI"))
				return MatchState_GetInGame;
			if (!strcmp(pGameScreen, "FIN"))
				return MatchState_FadeIn;
			if (!strcmp(pGameScreen, "BFBT"))
				return MatchState_BeforeBattle;
			if (!strcmp(pGameScreen, "HVORHL") || !strcmp(pGameScreen, "FIGMK1"))
				return MatchState_RebelActionRoundSign;
			if (!strcmp(pGameScreen, "LETS"))
				return MatchState_LetsRock;
			if (!strcmp(pGameScreen, "") && pInGameFlag != NULL && *pInGameFlag)
				return MatchState_Fight;
			if (!strcmp(pGameScreen, "AFBT"))
				return MatchState_FinishSign;
			if (!strcmp(pGameScreen, "WIN") || !strcmp(pGameScreen, "LOSE") || !strcmp(pGameScreen, "DRAW") || !strcmp(pGameScreen, "WNDS"))
				return MatchState_WinLoseSign;
			if (!strcmp(pGameScreen, "FOUT"))
				return MatchState_FadeOut;
			if (!strcmp(pGameScreen, "RDED"))
				return MatchState_RoundEnd;
			if (!strcmp(pGameScreen, "EXBT"))
				return MatchState_ExitBattle;
		}
		return MatchState_NotStarted;
	}

	byte* pSpecialMode[2];
	byte* pExMode[2];
	byte* pMegalomaniaType[2];
	ExtraMenu* pExtraMenu[2];
	uint16_t* pArcadeLevel_minus_1[2];
	uint16_t* pDaredevilCounter;
	uint16_t* pSurvivalLevel;

	char* pPlayerNameReplay[2];
	char* pPlayerNameOnline[2];

	uint16_t* pThrowRangeLookupTable;
	int* pCommandThrowId[2];

	uint16_t* pPushboxDimensions[3][2];
	uint16_t* pPushboxJumpOffset[2];

	int16_t* pGroundThrowRange[2]; //AC and +R
	int16_t* pAirHorizontalThrowRange[2]; //AC and +R
	int16_t* pAirVerticalThrowRange[2]; //lower and upper ranges
	int16_t* pCommandGrabThrowRanges;

	byte* pGameVerFlag; //0 = AC, 1 = +R

	bool isFrameFrozen, isFrameFrozenPrev = false;
	unsigned framesToReach;
	unsigned* pFrameCount;

	Camera* pCamera;

	int* pEntityList;
	void** pEntityStartAddress;
	uint16_t* entityCount;

	RoomMemberEntry** ppFirstRoomMemberStatic;   //Static list of players built upon their order of arrival
	RoomMemberEntry** ppFirstRoomMemberDynamic;  //Dynamic list that is determined by the sorting order in the room
	Room* pRoom;
	int* pSelectedRoomMember;
	int* pNumberOfPlayersInRoom;
};

struct gameProc_t
{
	HWND hWndGameWindow;
};

//temporary placeholders until wrappers are created / final addresses updated
struct temps_t
{
	ISteamNetworking** ppSteamNetworking;
	ISteamUserStats** ppSteamUserStats;
};
extern gameProc_t g_gameProc;
extern gameVals_t g_gameVals;
extern temps_t g_tempVals;

void InitManagers();
void CleanupInterfaces();
