#include "DebugWindow.h"

#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Game/gamestates.h"
#include "Overlay/NotificationBar/NotificationBar.h"
#include "Overlay/WindowManager.h"
#include "Overlay/Window/HitboxOverlay.h"


void DebugWindow::Draw()
{
	if (m_showDemoWindow)
	{
		ImGui::ShowDemoWindow(&m_showDemoWindow);
	}

	DrawImGuiSection();

	DrawGameValuesSection();

	DrawRoomSection();

	DrawNotificationSection();
}

void DebugWindow::DrawImGuiSection()
{
	if (!ImGui::CollapsingHeader("ImGui"))
		return;

	if (ImGui::TreeNode("Display"))
	{
		static ImVec2 newDisplaySize = ImVec2(Settings::settingsIni.renderwidth, Settings::settingsIni.renderheight);

		ImVec2 curDisplaySize = ImGui::GetIO().DisplaySize;

		ImGui::Text("Current display size: %.f %.f", curDisplaySize.x, curDisplaySize.y);

		ImGui::InputFloat2("Display size", (float*)&newDisplaySize, 0);

		static bool isNewDisplaySet = false;
		ImGui::Checkbox("Set value", &isNewDisplaySet);

		if (isNewDisplaySet)
			ImGui::GetIO().DisplaySize = newDisplaySize;

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Unicode text"))
	{
		ImGui::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)");
		ImGui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Frame data"))
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("DeltaTime: %f", ImGui::GetIO().DeltaTime);
		ImGui::Text("FrameCount: %d", ImGui::GetFrameCount());

		ImGui::TreePop();
	}

	if (ImGui::Button("Demo"))
	{
		m_showDemoWindow = !m_showDemoWindow;
	}
}

void DebugWindow::DrawGameValuesSection()
{
	if (!ImGui::CollapsingHeader("Game values"))
		return;

	if (ImGui::TreeNode("Character data"))
	{
		

		//g_interfaces.player1.GetInputData()->Inputs = 6;
		
		//ImGui::Text("pP1InputData 0x%p", g_interfaces.player1.GetInputData()->Inputs);
		


		if (!g_interfaces.Player1.IsCharDataNullPtr())
			ImGui::Text("pP1CharData 0x%p", g_interfaces.Player1.GetData());

		if (!g_interfaces.Player2.IsCharDataNullPtr())
			ImGui::Text("pP2CharData 0x%p", g_interfaces.Player2.GetData());

		if (!g_interfaces.Player1.IsCharDataNullPtr())
			ImGui::Text("P1CharIndex %d", g_interfaces.Player1.GetData()->charIndex);

		if (!g_interfaces.Player2.IsCharDataNullPtr())
			ImGui::Text("P2CharIndex %d", g_interfaces.Player2.GetData()->charIndex);


		ImGui::Separator();
		ImGui::Text("pP1PalIndex 0x%p", &(g_interfaces.Player1.GetPalHandle().GetPalIndexRef()));
		ImGui::Text("pP2PalIndex 0x%p", &(g_interfaces.Player2.GetPalHandle().GetPalIndexRef()));

		const int PAL_INDEX_MIN = 0;
		const int PAL_INDEX_MAX = 21;
		/*if (!g_interfaces.Player1.GetPalHandle().IsNullPointerPalIndex())
			ImGui::SliderInt("P1PalIndex", &g_interfaces.Player1.GetPalHandle().GetPalIndexRef(), PAL_INDEX_MIN, PAL_INDEX_MAX);

		if (!g_interfaces.Player2.GetPalHandle().IsNullPointerPalIndex())
			ImGui::SliderInt("P2PalIndex", &g_interfaces.Player2.GetPalHandle().GetPalIndexRef(), PAL_INDEX_MIN, PAL_INDEX_MAX);*/

		if (!g_interfaces.Player1.GetPalHandle().IsNullPointerPalIndex())
			ImGui::Text("P1OrigPalIndex %d", g_interfaces.Player1.GetPalHandle().GetOrigPalIndex());

		if (!g_interfaces.Player2.GetPalHandle().IsNullPointerPalIndex())
			ImGui::Text("P2OrigPalIndex %d", g_interfaces.Player2.GetPalHandle().GetOrigPalIndex());

		ImGui::TreePop();
	}


	if (ImGui::TreeNode("Game and Match variables"))
	{
		//ImGui::Text("pGameState: 0x%p : %d", g_gameVals.GetGameState(), SafeDereferencePtr(g_gameVals.pGameState));
		ImGui::Text("pGameMode: 0x%p : %d", g_gameVals.pGameMode, SafeDereferencePtr(g_gameVals.pGameMode));
		//ImGui::Text("pMatchState: 0x%p : %d", g_gameVals.pMatchState, SafeDereferencePtr(g_gameVals.pMatchState));
		ImGui::Text("pMatchTimer: 0x%p : %d", g_gameVals.pMatchTimer, SafeDereferencePtr(g_gameVals.pMatchTimer));
		ImGui::Text("pMatchRounds: 0x%p : %d", g_gameVals.pMatchRounds, SafeDereferencePtr(g_gameVals.pMatchRounds));
		ImGui::Text("pFrameCount: 0x%p : %d", g_gameVals.pFrameCount, SafeDereferencePtr((int*)g_gameVals.pframe_count_minus_1_P1));
		ImGui::Text("entityCount: %d", *g_gameVals.entityCount);
		ImGui::Text("pEntityList: 0x%p", g_gameVals.pEntityList);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Hitbox overlay"))
	{
		float& scale = WindowManager::GetInstance().GetWindowContainer()->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->GetScale();
		ImGui::SliderFloat("Hitbox overlay projection scale", &scale, 0.3f, 0.4f);

		ImGui::TreePop();
	}
}

void DebugWindow::DrawRoomSection()
{
	if (!ImGui::CollapsingHeader("Room"))
		return;

	if (!g_gameVals.pRoom || g_gameVals.pRoom->status == 0)
	{
		ImGui::TextUnformatted("Room is not available!");
		return;
	}

	if (ImGui::TreeNode("Room info"))
	{
		ImGui::Text("Status: %d", g_gameVals.pRoom->status);
		ImGui::Text("Name: %s", g_gameVals.pRoom->roomName);
		ImGui::Text("Mode: %d", g_gameVals.pRoom->gameMode);
		ImGui::Text("Capacity: %d", g_gameVals.pRoom->capacity);
		ImGui::Text("Member count: %d", *g_gameVals.pNumberOfPlayersInRoom);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Room members"))
	{
		for (int i = 0; i < MAX_PLAYERS_IN_ROOM; i++)
		{
			const RoomMemberEntry* member = g_interfaces.pRoomManager->GetRoomMemberEntryByIndex(i);

			if (!member)
				continue;

			const char* name = g_interfaces.pRoomManager->GetPlayerSteamName(member->steamId);

			char buf[128];
			sprintf_s(buf, "%s##%d", name, i + 1);

			if (ImGui::TreeNode(buf))
			{
				ImGui::Text("MemberIndex: %d", member->memberIndex);
				//ImGui::Text("MatchId: %d", member->matchId);
				ImGui::Text("MatchPlayerIndex: %d", member->matchPlayerIndex);
				ImGui::Text("SteamID: %llu", member->steamId);

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Room IM users"))
	{
		for (int i = 0; i < *g_gameVals.pNumberOfPlayersInRoom; i++)
		{
			const IMPlayer& player = g_interfaces.pRoomManager->m_imPlayers[i];
			const RoomMemberEntry* member = g_interfaces.pRoomManager->GetRoomMemberEntryByIndex(i);

			if (!member || player.steamID.ConvertToUint64() == -1)
				continue;

			const char* name = g_interfaces.pRoomManager->GetPlayerSteamName(member->steamId);

			char buf[128];
			sprintf_s(buf, "%s##%d", player.steamName.c_str(), i);

			if (ImGui::TreeNode(buf))
			{
				ImGui::Text("MemberIndex: %d", i);
				ImGui::Text("SteamID: %llu", player.steamID.ConvertToUint64());

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("IM users in current match"))
	{
		if (g_interfaces.pRoomManager->IsThisPlayerInMatch())
		{
			//ImGui::Text("MatchID: %d", g_interfaces.pRoomManager->GetThisPlayerRoomMemberEntry()->matchId);

			for (const IMPlayer& player : g_interfaces.pRoomManager->GetIMPlayersInCurrentMatch())
			{
				const RoomMemberEntry* member = g_interfaces.pRoomManager->GetRoomMemberEntryBySteamID(player.steamID.ConvertToUint64());

				char buf[128];
				sprintf_s(buf, "%s", player.steamName.c_str());

				if (ImGui::TreeNode(buf))
				{
					//ImGui::Text("MemberIndex: %d", player.roomMemberIndex);
					ImGui::Text("MatchPlayerIndex: %d", member->matchPlayerIndex);
					ImGui::Text("SteamID: %llu", player.steamID.ConvertToUint64());

					ImGui::TreePop();
				}
			}
		}
		else
		{
			ImGui::TextUnformatted("Not in match!");
		}

		ImGui::TreePop();
	}

	if (ImGui::Button("Send announce"))
	{
		g_interfaces.pRoomManager->SendAnnounce();
	}
}

void DebugWindow::DrawNotificationSection()
{
	if (!ImGui::CollapsingHeader("Notification"))
		return;

	const int BUF_SIZE = 1024;

	char text[BUF_SIZE] = "Opponent BBCFIM detected - Ping: 130ms (Press F2 for more info)";

	ImGui::SliderFloat("Seconds to slide", &g_notificationBar->m_secondsToSlide, 0, 15, "%.1f");
	ImGui::InputText("##Notification", text, BUF_SIZE);

	if (ImGui::Button("Add notification"))
	{
		g_notificationBar->AddNotification(text);
	}

	if (ImGui::Button("Clear notifications"))
	{
		g_notificationBar->ClearNotifications();
	}
}
