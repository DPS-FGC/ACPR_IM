#include "gamestates.h"

#include "Core/interfaces.h"

bool isPaletteEditingEnabledInCurrentState()
{
	bool isEnabledInCurrentState =
		g_gameVals.GetGameState() == GameState_InMatch;

	bool isEnabledInCurrentMode =
		g_gameVals.GetGameMode() == GameMode_Training ||
		g_gameVals.GetGameMode() == GameMode_Versus;

	return isEnabledInCurrentState && isEnabledInCurrentMode;
}

bool isHitboxOverlayEnabledInCurrentState()
{
	bool isEnabledInCurrentState =
		(g_gameVals.GetGameState() == GameState_InMatch) &&
		((g_gameVals.GetGameMode() != GameMode_ReplayTheater &&
			(*g_gameVals.pInPauseMenu == 0 || *g_gameVals.pIsPauseMenuDisplayed == 0)) ||
		(g_gameVals.GetGameMode() == GameMode_ReplayTheater && *g_gameVals.pInReplayPauseMenu == 0));

	bool isEnabledInCurrentMode =
		g_gameVals.GetGameMode() == GameMode_Training ||
		g_gameVals.GetGameMode() == GameMode_Versus ||
		g_gameVals.GetGameMode() == GameMode_ReplayTheater;

	return isEnabledInCurrentState && isEnabledInCurrentMode;
}

bool isGameModeSelectorEnabledInCurrentState()
{
	bool isEnabledInCurrentState =
		g_gameVals.GetGameState() == GameState_CharacterSelectionScreen ||
		g_gameVals.GetGameState() == GameState_ReplayMenu;

	bool isEnabledInCurrentMode =
		g_gameVals.GetGameMode() == GameMode_Training ||
		g_gameVals.GetGameMode() == GameMode_Versus ||
		g_gameVals.GetGameMode() == GameMode_Online ||
		g_gameVals.GetGameState() == GameState_ReplayMenu;

	return isEnabledInCurrentState && isEnabledInCurrentMode;
}

bool isStageSelectorEnabledInCurrentState()
{
	return g_gameVals.GetGameState() == GameState_CharacterSelectionScreen;
}

bool isInMatch()
{
	return g_gameVals.GetGameState() == GameState_InMatch;
}

bool isInMenu()
{
	return g_gameVals.GetGameState() == GameState_MainMenu;
}

bool isOnVersusScreen()
{
	return g_gameVals.GetGameState() == GameState_VersusScreen;
}

bool isOnReplayMenuScreen()
{
	return g_gameVals.GetGameState() == GameState_ReplayMenu;
}

bool isOnCharacterSelectionScreen()
{
	return g_gameVals.GetGameState() == GameState_CharacterSelectionScreen;
}
