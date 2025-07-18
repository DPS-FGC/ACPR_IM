#pragma once

enum MatchState_
{
	MatchState_NotStarted = 0,
	MatchState_GetInGame = 1,
	MatchState_RebelActionRoundSign = 2,
	MatchState_Fight = 3,
	MatchState_FinishSign = 4,
	MatchState_WinLoseSign = 5,
	MatchState_VictoryScreen = 7,
	MatchState_FadeIn = 8,
	MatchState_FadeOut = 9,
	MatchState_ExitBattle = 10,
	MatchState_RoundEnd = 11,
	MatchState_BeforeBattle = 12,
	MatchState_LetsRock = 13
};

enum GameState
{
	GameState_ArcsysLogo = 2,
	GameState_IntroVideoPlaying = 3,
	GameState_TitleScreen = 4,
	GameState_CharacterSelectionScreen = 6,
	GameState_ArcadeActSelectScreen = 11,
	GameState_ScoreAttackModeSelectScreen = 11,
	GameState_SpeedStarModeSelectScreen = 11,
	GameState_ArcadeCharInfoScreen = 12,
	GameState_ArcadeStageSelectScreen = 13,
	GameState_VersusScreen = 14,
	GameState_InMatch = 15,
	GameState_VictoryScreen = 16,
	GameState_StoryMenu = 24,
	GameState_GalleryMenu = 25,
	GameState_ItemMenu = 25,
	GameState_ReplayMenu = 26,
	GameState_MainMenu = 27,
	GameState_TutorialMenu = 28,
	GameState_LibraryMenu = 28,
	GameState_Lobby = 31,
	GameState_StoryPlaying = 33,
	GameState_AbyssMenu = 34,
	GameState_DCodeEdit = 39,
};

enum GameMode : unsigned char
{
	GameMode_Arcade = 0,
	GameMode_MOM = 1,
	GameMode_Versus = 2,
	GameMode_Online = 3,
	GameMode_VSCPU = 4,
	GameMode_TeamVersus = 5,
	GameMode_TeamVSCPU = 6,
	GameMode_Training = 7,
	GameMode_Survival = 8,
	GameMode_Mission = 9,
	GameMode_Story = 10,
	GameMode_Gallery = 11,
	GameMode_ReplayTheater = 12,
	GameMode_Sound = 13
};

bool isPaletteEditingEnabledInCurrentState();
bool isHitboxOverlayEnabledInCurrentState();
bool isGameModeSelectorEnabledInCurrentState();
bool isStageSelectorEnabledInCurrentState();

bool isInMatch();
bool isInMenu();
bool isOnVersusScreen();
bool isOnReplayMenuScreen();
bool isOnCharacterSelectionScreen();