#include "MatchState.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Overlay/Window/PaletteEditorWindow.h"
#include "Overlay/WindowContainer/WindowType.h"
#include "Overlay/WindowManager.h"
#include "Game/characters.h"

void MatchState::OnMatchInit()
{
	if (!WindowManager::GetInstance().IsInitialized())
	{
		return;
	}

	LOG(2, "MatchState::OnMatchInit\n");

	//This function is called twice at the start of each round, so ignore the second call.
	static int counter = 0;

	if (counter == 0)
	{
		counter = 1;
		return;
	}

	g_interfaces.pPaletteManager->LoadPaletteSettingsFile();
	g_interfaces.pPaletteManager->SetGameMode((GameMode)g_gameVals.GetGameMode());
	g_interfaces.pPaletteManager->OnMatchInit(g_interfaces.Player1, g_interfaces.Player2);
	g_interfaces.trainerInterface.OnMatchInit();
	g_interfaces.frameMeterInterface.Reset();
	g_interfaces.cbrInterface.OnMatchInit(*g_interfaces.Player1.GetData(), *g_interfaces.Player2.GetData(), g_gameVals.GetGameMode());
	if (g_gameVals.GetGameMode() == GameMode_Online && g_interfaces.pRoomManager->IsRoomFunctional())
	{
		g_interfaces.pRoomManager->OnMatchInit();
		// Prevent loading palettes.ini custom palette on opponent

		if (g_gameVals.pRoom->gameMode == 2)
			g_interfaces.pPaletteManager->SetGameMode(GameMode_TeamVersus);

		if (!g_interfaces.pRoomManager->IsThisPlayerSpectator())
		{
			uint16_t thisPlayerMatchPlayerIndex = g_interfaces.pRoomManager->GetThisPlayerMatchPlayerIndex();

			if (thisPlayerMatchPlayerIndex != 0xFFFF) //Rare case, but can happen when using spectate mode.
			{
				if (thisPlayerMatchPlayerIndex != 0)
				{
					g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player1.GetPalHandle());
				}

				if (thisPlayerMatchPlayerIndex != 1)
				{
					g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player2.GetPalHandle());
				}
			}
		}
		else if (!g_interfaces.pRoomManager->IsThisPlayerAutoSpectator())
		{
			bool spectatingIMPlayer = false;
			uint64_t selectedPlayerID = g_interfaces.pRoomManager->GetSelectedRoomMember()->steamId;

			if (g_interfaces.pRoomManager->IsPlayerIMUser(selectedPlayerID))
				spectatingIMPlayer = true;
			else
			{
				selectedPlayerID = g_interfaces.pRoomManager->GetSelectedRoomMember()->opponentSteamID;
				if (g_interfaces.pRoomManager->IsPlayerIMUser(selectedPlayerID))
					spectatingIMPlayer = true;
			}

			if (spectatingIMPlayer)
			{
				g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player1.GetPalHandle());
				g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player2.GetPalHandle());

				char name[32];
				strcpy(name, g_interfaces.pRoomManager->GetThisPlayerSteamName());
				Packet packet = Packet(
					(void*)name, strlen(name), PacketType_IMID_AnnounceSpectate, g_interfaces.pRoomManager->GetThisPlayerSteamID()
				);
				g_interfaces.pNetworkManager->SendPacket(&CSteamID(selectedPlayerID),
					&packet);
			}
		}
		// Send our custom palette and load opponent's
		g_interfaces.pOnlinePaletteManager->OnMatchInit((CharIndex)g_interfaces.Player1.GetData()->charIndex,
			                                            (CharIndex)g_interfaces.Player2.GetData()->charIndex);

		// Activate settled game mode
		//g_interfaces.pOnlineGameModeManager->OnMatchInit();
	}

	g_gameVals.isFrameFrozen = false;

	WindowManager::GetInstance().GetWindowContainer()->GetWindow<PaletteEditorWindow>(WindowType_PaletteEditor)->OnMatchInit();

	counter = 0;
}

void MatchState::OnMatchRematch()
{
	LOG(2, "MatchState::OnMatchRematch\n");

	g_interfaces.pPaletteManager->OnMatchRematch(
		g_interfaces.Player1,
		g_interfaces.Player2
	);

	g_interfaces.pOnlinePaletteManager->ClearSavedPalettePacketQueues();
}

void MatchState::OnMatchEnd()
{
	LOG(2, "MatchState::OnMatchEnd\n");

	//g_interfaces.pGameModeManager->EndGameMode();

	if (!isInMatch())
		return;

	g_interfaces.pPaletteManager->SaveReplayPalettes(g_interfaces.Player1, g_interfaces.Player2);

	if (g_gameVals.GetGameMode() == GameMode_Online && g_interfaces.pRoomManager->IsRoomFunctional())
	{
		//g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player1.GetPalHandle());
		//g_interfaces.pPaletteManager->RestoreOrigPal(g_interfaces.Player2.GetPalHandle());

		g_interfaces.pRoomManager->AnnounceLeave();
		g_interfaces.pRoomManager->OnMatchEnd();
		g_interfaces.pOnlinePaletteManager->OnMatchEnd();
	}

	g_interfaces.pPaletteManager->OnMatchEnd(
		g_interfaces.Player1.GetPalHandle(),
		g_interfaces.Player2.GetPalHandle()
	);

	g_interfaces.pOnlinePaletteManager->ClearSavedPalettePacketQueues();
	//g_interfaces.pOnlineGameModeManager->ClearPlayerGameModeChoices();
}

void MatchState::OnUpdate()
{
	LOG(7, "MatchState::OnUpdate\n");

	if (*g_gameVals.pInGameFlag)
	{
		g_interfaces.trainerInterface.updateGameValues(g_interfaces.trainerInterface.change_flags);
		g_interfaces.trainerInterface.change_flags = 0x00;
		if (!g_gameVals.isFrameFrozen && !*g_gameVals.pInPauseMenu)
			g_interfaces.frameMeterInterface.Update();
	}
	if (g_gameVals.isFrameFrozen)
	{
		if (g_gameVals.framesToReach > *g_gameVals.pframe_count_minus_1_P1)
		{
			*g_gameVals.pInPauseMenu = 0;
			g_interfaces.frameMeterInterface.Update();
		}
		else
		{
			if (g_gameVals.GetGameMode() == GameMode_Training)
				*g_gameVals.pInPauseMenu = 2;
			else
				*g_gameVals.pInPauseMenu = 1;
			*g_gameVals.pIsPauseMenuDisplayed = 0;
		}
	}
	else if (g_gameVals.isFrameFrozenPrev)
	{
		*g_gameVals.pInPauseMenu = 0;
	}
	g_gameVals.isFrameFrozenPrev = g_gameVals.isFrameFrozen;

	
	g_interfaces.pPaletteManager->OnUpdate(
		g_interfaces.Player1.GetPalHandle(),
		g_interfaces.Player2.GetPalHandle()
	);
}