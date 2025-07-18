#include "MainWindow.h"

//#include "CBR/CharacterStorage.h"
#include "HitboxOverlay.h"
#include "PaletteEditorWindow.h"

#include "Core/Settings.h"
#include "Core/info.h"
#include "Core/interfaces.h"
#include "Game/gamestates.h"
#include "Game/characters.h"
#include "Overlay/imgui_utils.h"
#include "Overlay/NotificationBar/NotificationBar.h"

#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>
#include "CBR/CharacterStorage.h"
#include "Trainer/TrainerInterface.h"

MainWindow::MainWindow(const std::string& windowTitle, bool windowClosable, WindowContainer& windowContainer, ImGuiWindowFlags windowFlags)
	: IWindow(windowTitle, windowClosable, windowFlags), m_pWindowContainer(&windowContainer)
{
	m_windowTitle = MOD_WINDOW_TITLE;
	m_windowTitle += " ";
	m_windowTitle += MOD_VERSION_NUM;

#ifdef _DEBUG
	m_windowTitle += " (DEBUG)";
#endif

	m_windowTitle += "###MainTitle"; // Set unique identifier
}

void MainWindow::BeforeDraw()
{
	ImGui::SetWindowPos(m_windowTitle.c_str(), ImVec2(12, 20), ImGuiCond_FirstUseEver);

	ImVec2 windowSizeConstraints;
	switch (Settings::settingsIni.menusize)
	{
		case 1:
			windowSizeConstraints = ImVec2(250, 190);
			break;
		case 3:
			windowSizeConstraints = ImVec2(400, 230);
			break;
		default:
			windowSizeConstraints = ImVec2(330, 230);
	}

	ImGui::SetNextWindowSizeConstraints(windowSizeConstraints, ImVec2(1000, 1000));
}

void MainWindow::Draw()
{
	ImGui::Text("Toggle me with %s", Settings::settingsIni.togglebutton.c_str());
	ImGui::Text("Toggle Online with %s", Settings::settingsIni.toggleOnlineButton.c_str());
	ImGui::Separator();
	
#ifdef _DEBUG
	RECT rect;
	if (GetWindowRect(g_gameProc.hWndGameWindow, &rect)) {
		ImGui::Text("Window position: x = %d, y = %d", rect.left, rect.top);
		ImGui::Text("Window handle: %#x", g_gameProc.hWndGameWindow);
	}
#endif

	ImGui::VerticalSpacing(5);

	if (ImGui::Button("Online", BTN_SIZE))
	{
		m_pWindowContainer->GetWindow(WindowType_Room)->ToggleOpen();
	}

	ImGui::VerticalSpacing(5);

	DrawTrainerSection();
	DrawCustomPalettesSection();
	DrawHitboxOverlaySection();
	DrawLoadedSettingsValuesSection();
	DrawCBRAiSection();
	DrawNetaSection();
		
	
	DrawUtilButtons();
	
	

	ImGui::VerticalSpacing(5);

	DrawCurrentPlayersCount();
	//DrawLinkButtons();
}

void MainWindow::DrawUtilButtons() const
{
#ifdef _DEBUG
	if (ImGui::Button("DEBUG", BTN_SIZE))
	{
		m_pWindowContainer->GetWindow(WindowType_Debug)->ToggleOpen();
	}
#endif

	if (ImGui::Button("Log", BTN_SIZE))
	{
		m_pWindowContainer->GetWindow(WindowType_Log)->ToggleOpen();
	}
}

void MainWindow::DrawCurrentPlayersCount() const
{
	ImGui::Text("Current online players:");
	ImGui::SameLine();

	std::string currentPlayersCount = g_interfaces.pSteamApiHelper ? g_interfaces.pSteamApiHelper->GetCurrentPlayersCountString() : "<No data>";
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", currentPlayersCount.c_str());
}


void MainWindow::DrawCustomPalettesSection() const
{
	if (!ImGui::CollapsingHeader("Custom palettes"))
		return;

	if (ImGui::Checkbox("Enable palette sharing##disPalShare", g_interfaces.pOnlinePaletteManager->GetSharePalettesOnlineRef()))
		g_interfaces.pOnlinePaletteManager->SendPalettePackets();
	ImGui::HoverTooltip("When disabled, will not send custom palettes to other players.");

	if (!isInMatch())
	{
		ImGui::HorizontalSpacing();
		ImGui::TextDisabled("YOU ARE NOT IN MATCH!");
	}
	else
	{
		ImGui::HorizontalSpacing();
		m_pWindowContainer->GetWindow<PaletteEditorWindow>(WindowType_PaletteEditor)->ShowAllPaletteSelections("Main");
	}

	ImGui::VerticalSpacing(15);
	ImGui::HorizontalSpacing();
	m_pWindowContainer->GetWindow<PaletteEditorWindow>(WindowType_PaletteEditor)->ShowReloadAllPalettesButton();

	if (isPaletteEditingEnabledInCurrentState())
	{
		ImGui::HorizontalSpacing();

		if (ImGui::Button("Palette editor"))
			m_pWindowContainer->GetWindow(WindowType_PaletteEditor)->ToggleOpen();
	}
}

char state_info[64];
void MainWindow::DrawHitboxOverlaySection() const
{
	if (!ImGui::CollapsingHeader("Game overlay"))
		return;

	static bool isOpen = false;

	if (!isHitboxOverlayEnabledInCurrentState())
	{
		ImGui::HorizontalSpacing();
		ImGui::TextDisabled("YOU ARE NOT IN TRAINING, VERSUS, OR REPLAY!");
//		return;
	}
	else {
		ImGui::HorizontalSpacing();
		if (ImGui::Checkbox("Enable", &isOpen))
		{
			if (isOpen)
			{
				m_pWindowContainer->GetWindow(WindowType_HitboxOverlay)->Open();
			}
			else
			{
				if (!g_interfaces.cbrInterface.netaOpen)
					g_gameVals.isFrameFrozen = false;
				m_pWindowContainer->GetWindow(WindowType_HitboxOverlay)->Close();
			}
		}

		if (isOpen)
		{
			ImGui::VerticalSpacing(10);

			if (!g_interfaces.Player1.IsCharDataNullPtr() && !g_interfaces.Player2.IsCharDataNullPtr())
			{
				ImGui::HorizontalSpacing();
				ImGui::Checkbox("Player1##hitboxP1", &m_pWindowContainer->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->drawCharacterHitbox[0]);
				ImGui::HoverTooltip(getCharacterNameByIndexA(g_interfaces.Player1.GetData()->charIndex).c_str());
				ImGui::SameLine(); ImGui::HorizontalSpacing();
				ImGui::TextUnformatted(std::to_string(g_interfaces.Player1.GetData()->actId).c_str());

				ImGui::HorizontalSpacing();
				ImGui::Checkbox("Player2##hitboxP2", &m_pWindowContainer->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->drawCharacterHitbox[1]);
				ImGui::HoverTooltip(getCharacterNameByIndexA(g_interfaces.Player2.GetData()->charIndex).c_str());
				ImGui::SameLine(); ImGui::HorizontalSpacing();
				ImGui::TextUnformatted(std::to_string(g_interfaces.Player2.GetData()->actId).c_str());

				sprintf(state_info, "Horizontal distance: %d, Vertical distance: %d",
					(int)abs(g_interfaces.Player1.GetData()->posX - g_interfaces.Player2.GetData()->posX),
					(int)abs(g_interfaces.Player1.GetData()->posY - g_interfaces.Player2.GetData()->posY));
				ImGui::Text(state_info);
			}

			ImGui::VerticalSpacing(10);

			ImGui::HorizontalSpacing();
			m_pWindowContainer->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->DrawRectThicknessSlider();

			ImGui::HorizontalSpacing();
			m_pWindowContainer->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->DrawRectFillTransparencySlider();

			//ImGui::HorizontalSpacing();
			//ImGui::Checkbox("Draw origin",
			//	&m_pWindowContainer->GetWindow<HitboxOverlay>(WindowType_HitboxOverlay)->drawOriginLine);


		}
		ImGui::VerticalSpacing();

		if (g_gameVals.GetGameMode() != GameMode_ReplayTheater)
		{
			ImGui::HorizontalSpacing();
			ImGui::Checkbox("Freeze frame:##freeze_hitbox", &g_gameVals.isFrameFrozen);
			ImGui::HoverTooltip("Pause current frame to allow stepping forward at desired frame intervals.");
			if (g_gameVals.pframe_count_minus_1_P1)
			{
				ImGui::SameLine();
				ImGui::Text("%d", *g_gameVals.pframe_count_minus_1_P1);
				ImGui::SameLine();
				if (ImGui::Button("Reset##reset_hitbox"))
				{
					*g_gameVals.pframe_count_minus_1_P1 = 0;
					g_gameVals.framesToReach = 0;
				}
			}
		}

		if (g_gameVals.isFrameFrozen)
		{
			static int framesToStep = 1;
			ImGui::HorizontalSpacing();
			if (ImGui::Button("Step frames##stepframe_hitbox"))
			{
				g_gameVals.framesToReach = *g_gameVals.pframe_count_minus_1_P1 + framesToStep;
			}

			ImGui::SameLine();
			ImGui::SliderInt("", &framesToStep, 1, 60);
			ImGui::Text("FRC point P1: %s", g_interfaces.Player1.GetData()->extraData->FRCTimer > 0 ? "yes" : "no");
			ImGui::Text("FRC point P2: %s", g_interfaces.Player2.GetData()->extraData->FRCTimer > 0 ? "yes" : "no");
		}
	}

	ImGui::Text("Overlay settings:");

	ImGui::Checkbox("Always draw throw box##throwbox", &g_interfaces.frameMeterInterface.settings.AlwaysDrawThrowRange);
	ImGui::HoverTooltip("If on, always draws throw range in purple.");

	ImGui::Checkbox("Show Frame Meter:##frameMeterOn", &g_interfaces.frameMeterInterface.settings.DisplayFrameMeter);
	if (g_interfaces.frameMeterInterface.settings.DisplayFrameMeter)
		ImGui::Checkbox("Show Legend##legendOn", &g_interfaces.frameMeterInterface.settings.DisplayFrameMeterLegend);

	ImGui::Checkbox("Allow rewinding##rewindOn", &g_interfaces.frameMeterInterface.settings.AllowRewindingInReplay);
	ImGui::HoverTooltip("Will rewind frame meter when rewinding replays in replay mode.\nIs costly on memory so only activate if really needed.");
	ImGui::Checkbox("Record during hitstop##recordHitstop", &g_interfaces.frameMeterInterface.settings.RecordDuringHitstop);
	ImGui::HoverTooltip("Will not pause frame meter during hitstop frames.");
	ImGui::Checkbox("Record during super flash##recordSuperflash", &g_interfaces.frameMeterInterface.settings.RecordDuringSuperFlash);
	ImGui::HoverTooltip("Will not pause frame meter during super flash.");
}

void MainWindow::DrawLinkButtons() const
{
	if (ImGui::Button("AI Filehost"))
	{
		m_pWindowContainer->GetWindow(WindowType_CbrServer)->ToggleOpen();
	}ImGui::SameLine();
	ImGui::ButtonUrl("Discord", MOD_LINK_DISCORD, BTN_SIZE);

	ImGui::SameLine();
	ImGui::ButtonUrl("Forum", MOD_LINK_FORUM, BTN_SIZE);

	ImGui::SameLine();
	ImGui::ButtonUrl("GitHub", MOD_LINK_GITHUB, BTN_SIZE);
}

void MainWindow::DrawLoadedSettingsValuesSection() const
{
	if (!ImGui::CollapsingHeader("Loaded settings.ini values"))
		return;

	// Not using ImGui columns here because they are bugged if the window has always_autoresize flag. The window 
	// starts extending to infinity, if the left edge of the window touches any edges of the screen

	std::ostringstream oss;

	ImGui::BeginChild("loaded_settings", ImVec2(0, 300.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	//X-Macro
#define SETTING_STRING(_var, _inistring, _defaultval) \
	oss << " " << _inistring; \
	ImGui::TextUnformatted(oss.str().c_str()); ImGui::SameLine(ImGui::GetWindowWidth() * 0.6f); \
	oss.str(""); \
	oss << "= " << Settings::settingsIni.##_var; \
	ImGui::TextUnformatted(oss.str().c_str()); ImGui::Separator(); \
	oss.str("");
#define SETTING_INT SETTING_STRING
#define SETTING_BOOL SETTING_STRING
#define SETTING_FLOAT SETTING_STRING
#include "Core/settings.def"
#undef SETTING_STRING
#undef SETTING_INT
#undef SETTING_BOOL
#undef SETTING_FLOAT

	ImGui::EndChild();
}

char elementName[32];
char* trainerHeaders[] = { "General", "Defence", "Character Specific", "Extra Menu", "Game Modes" };
char* trainerHeaderTags[] = { "ge:", "de:", "cs:", "em:", "gm:"};
void MainWindow::DrawTrainerSection() const {
	g_interfaces.trainerInterface.change_flags = 0x00;
	std::string current_item;
	bool is_selected = false;
	
	if (!ImGui::CollapsingHeader("Trainer##trainer"))
		return;

	if (g_interfaces.trainerInterface.disableInReplayAndOnline &&
		g_gameVals.GetGameMode() == GameMode_Online)
	{
		ImGui::Text("Trainer disabled.");
		return;
	}

	//ImGui::Checkbox("Disable in replay and online##disabletrainer", &g_interfaces.trainerInterface.disableInReplayAndOnline);
	//ImGui::HoverTooltip("When checked, will automatically disable trainer effects in replay and online to avoid desyncs.\nTurn this off only if planning to play online with another person who has\nthe trainer active or for replays recorded with the trainer on.");

	if (g_interfaces.trainerInterface.disableInReplayAndOnline &&
		g_gameVals.GetGameMode() == GameMode_ReplayTheater)
	{
		ImGui::Text("Trainer disabled.");
		return;
	}

	for (int nheader = 0; nheader < 5; nheader++)
	{
		sprintf(elementName, "%s##%s", trainerHeaders[nheader], trainerHeaderTags[nheader]);
		if (ImGui::CollapsingHeader(elementName))
		{
			if (!strcmp(trainerHeaderTags[nheader], "gm:"))
			{
				if (g_gameVals.GetGameMode() != GameMode_Arcade && g_gameVals.GetGameMode() != GameMode_Survival)
				{
					ImGui::TextDisabled("This section is available only in arcade and survival modes.");
					continue;
				}
			}
			else
			{
				if (!*g_gameVals.pInGameFlag)
				{
					ImGui::TextDisabled("This section is available only in match.");
					continue;
				}
			}

			ImGui::Columns(3);
			ImGui::NextColumn();
			ImGui::Text("Player 1");
			ImGui::NextColumn();
			ImGui::Text("Player 2");
			ImGui::Separator();
			ImGui::NextColumn();
			ImGui::Columns(1);

			for (int i = 0; i < g_interfaces.trainerInterface.NumberOfItems(); i++)
			{
				if (g_interfaces.trainerInterface.GetItem(i)->GetTag().rfind(trainerHeaderTags[nheader], 0) != 0)
					continue;
				if (!g_interfaces.Player1.IsCharDataNullPtr() && !g_interfaces.Player2.IsCharDataNullPtr() &&
					!g_interfaces.trainerInterface.GetItem(i)->ShouldDisplay(g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr()))
					continue;
				if (!g_interfaces.trainerInterface.GetItem(i)->ShouldDisplay(g_gameVals.GetGameMode()))
					continue;
				sprintf(elementName, "##%scol%d", trainerHeaderTags[nheader], i);
				ImGui::Columns(g_interfaces.trainerInterface.GetItem(i)->GetNplayers() + 1, elementName);
				ImGui::Text(g_interfaces.trainerInterface.GetItem(i)->GetName().c_str());
				for (int playerNr = 0; playerNr < g_interfaces.trainerInterface.GetItem(i)->GetNplayers(); playerNr++)
				{
					ImGui::NextColumn();
					if (!g_interfaces.GetPlayer(playerNr)->IsCharDataNullPtr() &&
						!g_interfaces.trainerInterface.GetItem(i)->ShouldDisplay(g_interfaces.GetPlayer(playerNr)->GetCharAbbr()))
						continue;
					sprintf(elementName, "##%s:P%d", g_interfaces.trainerInterface.GetItem(i)->GetTag().c_str(), playerNr);
					switch (g_interfaces.trainerInterface.GetItem(i)->GetType())
					{
					case TrainerItemType_Bool:
						if (ImGui::Checkbox(elementName, g_interfaces.trainerInterface.GetItem(i)->GetBoolPtr(playerNr)))
						{
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "ge:DisHurtbox")
								g_interfaces.trainerInterface.change_flags |= 0x02;
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "ge:ThInvul")
								g_interfaces.trainerInterface.change_flags |= 0x04;
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "de:AutoThrowTech")
							{
								if (g_interfaces.trainerInterface.GetItem(i)->GetBool(playerNr))
									g_interfaces.trainerInterface.GetItem("de:AutoLateThrowTech")->SetBool(false, playerNr);
							}
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "de:AutoLateThrowTech")
							{
								if (g_interfaces.trainerInterface.GetItem(i)->GetBool(playerNr))
									g_interfaces.trainerInterface.GetItem("de:AutoThrowTech")->SetBool(false, playerNr);
							}
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "de:SArmor")
							{
								if (g_interfaces.trainerInterface.GetItem(i)->GetBool(playerNr))
									g_interfaces.trainerInterface.GetItem("de:HArmor")->SetBool(false, playerNr);
								else
									g_interfaces.GetPlayer(playerNr)->GetData()->blockType &= 0xFBFF; //Turn off armor flag as it doesn't change in neutral
							}
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "de:HArmor")
							{
								if (g_interfaces.trainerInterface.GetItem(i)->GetBool(playerNr))
									g_interfaces.trainerInterface.GetItem("de:SArmor")->SetBool(false, playerNr);
								else
									g_interfaces.GetPlayer(playerNr)->GetData()->blockType &= 0xFBFF; //Turn off armor flag as it doesn't change in neutral
							}
							/*if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "sm:StylishMode")
							{
								if (g_interfaces.trainerInterface.GetItem("sm:StylishMode")->GetBool(playerNr))
									g_interfaces.stylishModeManager.InitializeComboItems((CharIndex)g_interfaces.GetPlayer(playerNr)->GetData()->charIndex, playerNr);
								else
									g_interfaces.stylishModeManager.ClearComboItems(playerNr);
							}*/
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "cs:za:Nightmare")
							{
								if (g_interfaces.trainerInterface.GetItem(i)->GetBool(playerNr))
								{
									g_interfaces.trainerInterface.settingUpNightmare[playerNr] = true;
									g_interfaces.trainerInterface.nightmareFrameCount[playerNr] = 0;
								}
								else
								{
									g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0;
									g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 0;
									g_interfaces.trainerInterface.settingUpNightmare[playerNr] = false;
									g_interfaces.trainerInterface.nightmareFrameCount[playerNr] = 0;
								}
							}
							if (!strcmp(trainerHeaderTags[nheader], "em:"))
								g_interfaces.trainerInterface.change_flags |= 0x10;
						}
						break;
					case TrainerItemType_List:
						current_item = g_interfaces.trainerInterface.GetItem(i)->GetCurrentLabel(playerNr);
						if (ImGui::BeginCombo(elementName, current_item.c_str()))
						{
							for (int n = 0; n < g_interfaces.trainerInterface.GetItem(i)->NumberOfItems(); n++)
							{
								is_selected = current_item == g_interfaces.trainerInterface.GetItem(i)->GetLabel(n);
								if (ImGui::Selectable(g_interfaces.trainerInterface.GetItem(i)->GetLabel(n).c_str(), is_selected))
								{
									g_interfaces.trainerInterface.GetItem(i)->SetInt(n, playerNr);
									if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "ge:SpMode")
										g_interfaces.trainerInterface.change_flags |= 0x01;
									if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "gm:SurvivalLevel") {
										if (n > 0)
										{
											*g_gameVals.pDaredevilCounter = n - 1;
											if (n <= 25)
												*g_gameVals.pSurvivalLevel = n * 20;
											else if (n <= 40)
												*g_gameVals.pSurvivalLevel = (n - 25) * 20 + 600;
											else if (n < 50)
												*g_gameVals.pSurvivalLevel = (n - 40) * 10 + 900;
											else
												*g_gameVals.pSurvivalLevel = 999;
										}
									}
									if (!strcmp(trainerHeaderTags[nheader], "em:"))
										g_interfaces.trainerInterface.change_flags |= 0x10;
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
						break;
					case TrainerItemType_Int:
						if (ImGui::DragInt(elementName, g_interfaces.trainerInterface.GetItem(i)->GetIntPtr(playerNr),
							(g_interfaces.trainerInterface.GetItem(i)->GetMaxInt() - g_interfaces.trainerInterface.GetItem(i)->GetMinInt()) / 30.0f,
							g_interfaces.trainerInterface.GetItem(i)->GetMinInt(),
							g_interfaces.trainerInterface.GetItem(i)->GetMaxInt()))
						{
							g_interfaces.trainerInterface.GetItem(i)->CheckInt(playerNr);
							if (g_interfaces.trainerInterface.GetItem(i)->GetTag() == "gm:ArcadeLevel")
								*g_gameVals.pArcadeLevel_minus_1[playerNr] = (uint16_t)g_interfaces.trainerInterface.GetItem(i)->GetInt(playerNr) - 1;
							if (!strcmp(trainerHeaderTags[nheader], "em:"))
								g_interfaces.trainerInterface.change_flags |= 0x10;
						}
						break;
					case TrainerItemType_Float:
						if (ImGui::DragFloat(elementName, g_interfaces.trainerInterface.GetItem(i)->GetFloatPtr(playerNr),
							(g_interfaces.trainerInterface.GetItem(i)->GetMaxFloat() - g_interfaces.trainerInterface.GetItem(i)->GetMinFloat()) / 30.0f,
							g_interfaces.trainerInterface.GetItem(i)->GetMinFloat(),
							g_interfaces.trainerInterface.GetItem(i)->GetMaxFloat()), "%.2f")
						{
							g_interfaces.trainerInterface.GetItem(i)->CheckFloat(playerNr);
						}
						break;
					}
					ImGui::HoverTooltip(g_interfaces.trainerInterface.GetItem(i)->GetToolTip().c_str());
				}
				ImGui::NextColumn();
				ImGui::Columns(1);
			}
		}
	}

	if (ImGui::Button("Reset", { 120, 25 }))
		g_interfaces.trainerInterface.resetTrainer();
	if (ImGui::Button("Save Settings", { 180, 25 }))
		g_interfaces.trainerInterface.saveSettings(g_interfaces.trainerInterface.settingsFile);
	ImGui::SameLine();
	ImGui::Text("File name:");
	ImGui::SameLine();
	ImGui::InputText("##trainerfile", g_interfaces.trainerInterface.settingsFile, IM_ARRAYSIZE(g_interfaces.trainerInterface.settingsFile));

	if (ImGui::Button("Load Settings", { 180, 25 }))
		g_interfaces.trainerInterface.loadSettings(g_interfaces.trainerInterface.currentSettingsFile);
	ImGui::SameLine();
	current_item = g_interfaces.trainerInterface.currentSettingsFile;
	if (ImGui::BeginCombo("##trainerfiles", g_interfaces.trainerInterface.currentSettingsFile.c_str())) {
		for (int i = 0; i < g_interfaces.trainerInterface.settingFiles.size(); i++) {
			is_selected = (g_interfaces.trainerInterface.settingFiles[i] == current_item);
			if (ImGui::Selectable(g_interfaces.trainerInterface.settingFiles[i].c_str(), is_selected)) {
				g_interfaces.trainerInterface.currentSettingsFile = g_interfaces.trainerInterface.settingFiles[i];
			}
		}
		if (is_selected)
			ImGui::SetItemDefaultFocus();
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh##refreshtrainerfiles", { 120, 25 }))
		g_interfaces.trainerInterface.refreshSettingsFiles();
}

//CBR MOD related -----------------------------------------------------------------------------------------------------------------

int selected_radio;
int weightsPlayer;
void MainWindow::DrawCBRAiSection() const
{
	if (!ImGui::CollapsingHeader("CBR AI"))
		return;

	g_interfaces.cbrInterface.loadSettings(&g_interfaces.cbrInterface);

	if (g_interfaces.cbrInterface.debugErrorCounter[0] > 0) {
		ImGui::Text("ErrorCountP1: %d", g_interfaces.cbrInterface.debugErrorCounter[0]);
	}
	if (g_interfaces.cbrInterface.debugErrorCounter[1] > 0) {
		ImGui::Text("ErrorCountP2: %d", g_interfaces.cbrInterface.debugErrorCounter[1]);
	}
	if (g_interfaces.cbrInterface.threadActiveCheck()) {
		ImGui::TextDisabled("SAVING OR LOADING CBR DATA. \nPLEASE WAIT.");
		return;
	}
	ImGui::Text("CBR AI Explanation:");
	ImGui::SameLine(); // Move to the same line as the button
	ImGui::SmallButton("?");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("CBR AI learns by recording player behaviour and imitating them, essentially letting you generate player clones.\nThis means the AI will only know how to do things that a player demonstrated beforehand.\nThis lets you create smarter AI opponents to play with, or even training dummys with behaviour finetuned for training.\nAn example of this would be a carl AI that only runs blockstrings, which lets you practice blocking carl.\n\nTo create an AI you must record yourself and/or your opponent by playing online, watching replays, or use the CBR menu in training mode.\nIn training mode set the opponents behaviour to \"Controller\" to let the AI control them.\n\nYou can use instant learning in mirror matches to play against an AI while it learns.\nI recommend setting life to not regenerate to play normal rounds against the AI.\nThis gives the AI more chances to learn neutral, since it will be bad at everything while it has little data.");
	}
	if (!isInMatch() || !(g_gameVals.GetGameMode() == GameMode_Training || g_gameVals.GetGameMode() == GameMode_Versus ||
		g_gameVals.GetGameMode() == GameMode_Online || g_gameVals.GetGameMode() == GameMode_ReplayTheater))
	{
		ImGui::HorizontalSpacing();
		ImGui::TextDisabled("CBR Menu only accesible in training, versus, replay and online versus");


		if (ImGui::Checkbox("Auto Record Myself", &g_interfaces.cbrInterface.autoRecordGameOwner)) {
			g_interfaces.cbrInterface.saveSettings();
		}
		ImGui::SameLine();
		if (ImGui::InputText("Game Owner Name##gameOwner", g_interfaces.cbrInterface.myName, IM_ARRAYSIZE(g_interfaces.cbrInterface.myName))) {
			g_interfaces.cbrInterface.saveSettings();
		}
		ImGui::HoverTooltip("Matches containing a player with this name will be recorded.");

		if (ImGui::Checkbox("Auto Record Opponents", &g_interfaces.cbrInterface.autoRecordAllOtherPlayers)) {
			g_interfaces.cbrInterface.saveSettings();
		}
		//if (ImGui::Checkbox("Auto Upload Own Data", &g_interfaces.cbrInterface.autoUploadOwnData)) {
		//	g_interfaces.cbrInterface.saveSettings();
		//}
		if (ImGui::Checkbox("Auto Saving In Lobby", &g_interfaces.cbrInterface.autoRecordConfirmation)) {
			g_interfaces.cbrInterface.saveSettings();
		}

		if (g_gameVals.GetGameState() == GameState_ReplayMenu)
		{
			if (ImGui::Button("Auto Scan Replay Folder", { 170, 20 }))
			{
				if (!g_interfaces.cbrInterface.autoScanReplayFolder)
				{
					g_interfaces.cbrInterface.autoScanReplayFolder = true;
					g_interfaces.cbrInterface.currentReplayItem = -1;
					g_interfaces.cbrInterface.currentReplayWatched = false;
					g_interfaces.cbrInterface.replayInputCooldown = 0;
					g_interfaces.cbrInterface.stopAutoScan = false;
				}
				else
					g_interfaces.cbrInterface.autoScanReplayFolder = false;
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Automatically go over all replays in a folder and record them.\nThe folder must contain replays only, without other folders.\nWhile in the replay, the game will be fast forwarded.\nCheck atomatic recording checkboxes for learning to occur.");
			}

			ImGui::Text("Replay scan range:");
			ImGui::PushItemWidth(80);
			if (!g_interfaces.cbrInterface.autoScanReplayFolder &&
				g_interfaces.cbrInterface.replayRangeEnd > g_interfaces.cbrInterface.GetNumberOfReplayMenuItems() + 1)
				g_interfaces.cbrInterface.replayRangeEnd = g_interfaces.cbrInterface.GetNumberOfReplayMenuItems() + 1;
			ImGui::DragIntRange2("##ReplayRange", &g_interfaces.cbrInterface.replayRangeStart, &g_interfaces.cbrInterface.replayRangeEnd, 1.0F, 
				0, g_interfaces.cbrInterface.GetNumberOfReplayMenuItems() + 1);
			ImGui::PushItemWidth(0);
			if (g_interfaces.cbrInterface.autoScanReplayFolder)
			{
				if (!g_interfaces.cbrInterface.stopAutoScan)
					ImGui::Text("Automatic replay scan in progress");
				else
					ImGui::Text("Automatic replay scan will stop after this replay");
			}
		}
		ImGui::Text("Names for players in versus mode:");
		ImGui::InputText("Player1Name", g_interfaces.cbrInterface.nameVersusP1, IM_ARRAYSIZE(g_interfaces.cbrInterface.nameVersusP1));
		ImGui::InputText("Player2Name", g_interfaces.cbrInterface.nameVersusP2, IM_ARRAYSIZE(g_interfaces.cbrInterface.nameVersusP2));
		
		return;
	}

	if (isInMatch())
	{
		//ImGui::VerticalSpacing(10);
		//ImGui::HorizontalSpacing();
		if (!m_pWindowContainer->GetWindow(WindowType_CbrServer)->IsOpen()) {
			g_interfaces.cbrInterface.windowLoadNr = -1;
		}
		ImGui::Columns(2, "cbr");
		ImGui::Text("Player 1");
		ImGui::NextColumn();
		ImGui::Text("Player 2");
		ImGui::Separator();
		ImGui::NextColumn();
		ImGui::PushID(1);
		ImGui::Text("Replays: %d", g_interfaces.cbrInterface.getCbrData(0)->getReplayCount());
		ImGui::Text("FramesRecorded: %d",g_interfaces.cbrInterface.getAnnotatedReplay(1)->getInputSize());
		ImGui::Text("Input: %d", g_interfaces.cbrInterface.input);
		ImVec2 buttonSize = { 120,20 };
		ImVec2 buttonSizeHalf = { 60,20 };

		if (!g_interfaces.cbrInterface.autoScanReplayFolder)
		{
			if (ImGui::Button("Record", buttonSize))
			{
				if (g_interfaces.cbrInterface.Recording == false) {
					g_interfaces.cbrInterface.EndCbrActivities(0);
					g_interfaces.cbrInterface.StartCbrRecording(g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetData()->charIndex, g_interfaces.Player2.GetData()->charIndex, 0);
				}
				else {
					g_interfaces.cbrInterface.EndCbrActivities(0, true);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Records all the actions of player 1,\nwhich the AI will then learn from. \nLearned data is stored in slot 1. \nClick to turn on and again to turn off.");
			}
			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online))) {
				if (ImGui::Button("Replaying", buttonSize))
				{

					if (g_interfaces.cbrInterface.getCbrData(0)->getReplayCount() > 0 && g_interfaces.cbrInterface.Replaying == false) {
						g_interfaces.cbrInterface.EndCbrActivities(0);
						g_interfaces.cbrInterface.Replaying = true;
					}
					else {
						g_interfaces.cbrInterface.EndCbrActivities(0);
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Let the AI take control of player 1s character.\nYou need AI data stored in Slot 1 for this to work.\nClick to turn on and again to turn off.");
				}

			}
			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online)) &&
				!g_interfaces.Player1.IsCharDataNullPtr() && !g_interfaces.Player2.IsCharDataNullPtr() && g_interfaces.Player1.GetData()->charIndex == g_interfaces.Player2.GetData()->charIndex) {
				if (ImGui::Button("InstantLearning", buttonSize))
				{

					if (g_interfaces.cbrInterface.instantLearning != true) {
						g_interfaces.cbrInterface.EndCbrActivities();
						g_interfaces.cbrInterface.StartCbrInstantLearning(g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetData()->charIndex, g_interfaces.Player2.GetData()->charIndex, 0);
					}
					else {
						g_interfaces.cbrInterface.EndCbrActivities();
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("The AI takes control over player 2 while learning in real time from player 1.\nOnly use if you are player 1.\nStores data in slot 2.\nClick to turn on and again to turn off.");
				}

			}
			if (ImGui::Button("Delete", buttonSizeHalf))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(0)->deleteReplays(g_interfaces.cbrInterface.deletionStart, g_interfaces.cbrInterface.deletionEnd);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Deletes AI data in a certain range from the AI file in slot 1.\nThe range to be deleted can be set in the \"Replay Deletion Range\" entry below.");
			}ImGui::SameLine(0, 0);
			if (ImGui::Button("Last", buttonSizeHalf))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(0)->deleteLastReplay();
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Deletes the last AI replay that was added.");
			}

			if (ImGui::Button("Save", buttonSize))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(0)->setPlayerName(g_interfaces.cbrInterface.playerName);
				g_interfaces.cbrInterface.getCbrData(0)->setCharName(g_interfaces.Player1.GetCharAbbr());
				g_interfaces.cbrInterface.SaveCbrDataThreaded(*g_interfaces.cbrInterface.getCbrData(0), true);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Save the AI data stored in slot 1.\nYou can determine under what player name the data is stored by writing a name in the \"Player name: \" textbox");
			}

			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online)))
			{
				if (ImGui::Button("Load", buttonSize)) {
					g_interfaces.cbrInterface.EndCbrActivities();
					m_pWindowContainer->GetWindow(WindowType_CbrServer)->ToggleOpen();
					g_interfaces.cbrInterface.windowLoadNr = 0;
					g_interfaces.cbrInterface.windowReload = true;
					//g_interfaces.cbrInterface.setCbrData(g_interfaces.cbrInterface.LoadCbrData(g_interfaces.cbrInterface.playerName, g_interfaces.player1.GetData()->char_abbr), 0);
				}
				if (ImGui::Button("Load By Name", buttonSize)) {
					g_interfaces.cbrInterface.EndCbrActivities();
					g_interfaces.cbrInterface.LoadCbrData(g_interfaces.cbrInterface.playerName, g_interfaces.Player1.GetCharAbbr(), true, 0);
					//g_interfaces.cbrInterface.setCbrData(g_interfaces.cbrInterface.LoadCbrDataNoThread(g_interfaces.cbrInterface.playerName, g_interfaces.player1.GetData()->char_abbr), 0);
				}
			}
		}

		ImGui::NextColumn();
		ImGui::Text("Replays: %d", g_interfaces.cbrInterface.getCbrData(1)->getReplayCount());
		ImGui::Text("FramesRecorded: %d", g_interfaces.cbrInterface.getAnnotatedReplay(0)->getInputSize());
		ImGui::Text("Input: %d", g_interfaces.cbrInterface.inputP2);
		ImGui::PopID();
		ImGui::PushID(2);

		if (!g_interfaces.cbrInterface.autoScanReplayFolder)
		{
			if (ImGui::Button("Record", buttonSize))
			{

				if (g_interfaces.cbrInterface.RecordingP2 == false) {
					g_interfaces.cbrInterface.EndCbrActivities(1);
					g_interfaces.cbrInterface.StartCbrRecording(g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetData()->charIndex, g_interfaces.Player1.GetData()->charIndex, 1);
				}
				else {
					g_interfaces.cbrInterface.EndCbrActivities(1, true);
				}
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Records all the actions of player 2,\nwhich the AI will then learn from. \nLearned data is stored in slot 2. \nClick to turn on and again to turn off.");
			}
			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online))) {
				if (ImGui::Button("Replaying", buttonSize))
				{

					if (g_interfaces.cbrInterface.getCbrData(1)->getReplayCount() > 0 && g_interfaces.cbrInterface.ReplayingP2 == false) {
						g_interfaces.cbrInterface.EndCbrActivities(1);
						g_interfaces.cbrInterface.ReplayingP2 = true;
					}
					else {
						g_interfaces.cbrInterface.EndCbrActivities(1);
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Let the AI take control of player 1s character.\nYou need AI data stored in Slot 1 for this to work.\nClick to turn on and again to turn off.");
				}

			}
			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online)) &&
				!g_interfaces.Player1.IsCharDataNullPtr() && !g_interfaces.Player2.IsCharDataNullPtr() && g_interfaces.Player1.GetData()->charIndex == g_interfaces.Player2.GetData()->charIndex) {
				if (ImGui::Button("InstantLearning", buttonSize))
				{

					if (g_interfaces.cbrInterface.instantLearningP2 != true) {
						g_interfaces.cbrInterface.EndCbrActivities();
						g_interfaces.cbrInterface.StartCbrInstantLearning(g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetData()->charIndex, g_interfaces.Player1.GetData()->charIndex, 1);
					}
					else {
						g_interfaces.cbrInterface.EndCbrActivities();
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("The AI takes control over player 1 while learning in real time from player 2.\nOnly use if you are player 2.\nStores data in slot 1.\nClick to turn on and again to turn off.");
				}
			}
			if (ImGui::Button("Delete", buttonSizeHalf))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(1)->deleteReplays(g_interfaces.cbrInterface.deletionStart, g_interfaces.cbrInterface.deletionEnd);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Deletes AI data in a certain range from the AI file in slot 2.\n The range to be deleted can be set in the \"Replay Deletion Range\" entry below.");
			}
			ImGui::SameLine(0, 0);
			if (ImGui::Button("Last", buttonSizeHalf))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(1)->deleteLastReplay();
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Deletes the last AI replay that was added.");
			}
			if (ImGui::Button("Save", buttonSize))
			{
				g_interfaces.cbrInterface.EndCbrActivities();
				g_interfaces.cbrInterface.getCbrData(1)->setPlayerName(g_interfaces.cbrInterface.playerName);
				g_interfaces.cbrInterface.getCbrData(1)->setCharName(g_interfaces.Player2.GetCharAbbr());
				g_interfaces.cbrInterface.SaveCbrDataThreaded(*g_interfaces.cbrInterface.getCbrData(1), true);
			}

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Save the AI data stored in slot 2.\nYou can determine under what player name the data is stored by writing a name in the \"Player name: \" textbox");
			}

			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online)))
			{
				if (ImGui::Button("Load", buttonSize)) {
					g_interfaces.cbrInterface.EndCbrActivities();
					m_pWindowContainer->GetWindow(WindowType_CbrServer)->ToggleOpen();
					g_interfaces.cbrInterface.windowLoadNr = 1;
					g_interfaces.cbrInterface.windowReload = true;
					//g_interfaces.cbrInterface.setCbrData(g_interfaces.cbrInterface.LoadCbrData(g_interfaces.cbrInterface.playerName, g_interfaces.player2.GetData()->char_abbr), 1);
				}
				if (ImGui::Button("Load By Name", buttonSize)) {
					g_interfaces.cbrInterface.EndCbrActivities();
					g_interfaces.cbrInterface.LoadCbrData(g_interfaces.cbrInterface.playerName, g_interfaces.Player2.GetCharAbbr(), true, 1);
					//g_interfaces.cbrInterface.setCbrData(g_interfaces.cbrInterface.LoadCbrDataNoThread(g_interfaces.cbrInterface.playerName, g_interfaces.player2.GetData()->char_abbr), 1);
				}
			}
		}

		ImGui::PopID();
		ImGui::Columns(1);
		ImGui::Separator();

		if (!g_interfaces.cbrInterface.autoScanReplayFolder)
		{
			if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
				(g_gameVals.GetGameMode() == GameMode_Online))) {
				if (ImGui::Button("Replaying Both"))
				{

					if (g_interfaces.cbrInterface.getCbrData(0)->getReplayCount() > 0 && g_interfaces.cbrInterface.Replaying == false
						&& g_interfaces.cbrInterface.getCbrData(1)->getReplayCount() > 0 && g_interfaces.cbrInterface.ReplayingP2 == false) {
						g_interfaces.cbrInterface.EndCbrActivities();
						g_interfaces.cbrInterface.Replaying = true;
						g_interfaces.cbrInterface.ReplayingP2 = true;
					}
					else {
						g_interfaces.cbrInterface.EndCbrActivities();
					}
				}
				ImGui::Separator();
			}

			ImGui::Text("Player name:");
			ImGui::PushID(177);
			ImGui::InputText("", g_interfaces.cbrInterface.playerName, IM_ARRAYSIZE(g_interfaces.cbrInterface.playerName));
			ImGui::PopID();
			ImGui::Text("Replay deletion range:");
			ImGui::PushItemWidth(80);
			ImGui::DragIntRange2("", &g_interfaces.cbrInterface.deletionStart, &g_interfaces.cbrInterface.deletionEnd, 1.0F, 0);
		}

		ImGui::PushItemWidth(0);
		ImGui::Text(g_interfaces.cbrInterface.WriteAiInterfaceState().c_str());

		if (g_interfaces.cbrInterface.autoScanReplayFolder)
		{
			if (ImGui::Button("Stop replay auto scan", { 170, 20 }))
				g_interfaces.cbrInterface.stopAutoScan = true;
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Stops automatic scanning after this replay.");

			if (!g_interfaces.cbrInterface.stopAutoScan)
				ImGui::Text("Automatic replay scan in progress");
			else
				ImGui::Text("Automatic replay scan will stop after this replay");
			return;
		}

		if (!(g_gameVals.GetGameMode() == GameMode_Versus || g_gameVals.GetGameMode() == GameMode_ReplayTheater ||
			(g_gameVals.GetGameMode() == GameMode_Online)))
		{
			ImGui::RadioButton("Basic: ", &selected_radio, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Pressure: ", &selected_radio, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Blocking: ", &selected_radio, 2);
			ImGui::SameLine();
			ImGui::RadioButton("Combo: ", &selected_radio, 3);
			if (selected_radio == 0) {
				if (ImGui::CollapsingHeader("Weights")) {
					ImGui::RadioButton("Player1: ", &weightsPlayer, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Player2: ", &weightsPlayer, 1);
					ImGui::Text("Save File Name:");
					ImGui::PushID(179);
					ImGui::InputText("", g_interfaces.cbrInterface.weightName, IM_ARRAYSIZE(g_interfaces.cbrInterface.weightName));
					ImGui::PopID();
					if (ImGui::Button("Save Cst"))
					{
						g_interfaces.cbrInterface.SaveWeights(g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs);
					}ImGui::SameLine();
					if (ImGui::Button("Load Cst"))
					{
						auto Cst = g_interfaces.cbrInterface.LoadWeights(g_interfaces.cbrInterface.weightName);
						if (Cst.name[0] != "-1") {
							g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs = Cst;
						}
					}



					auto costs = &g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs;
					for (int i = 0; i < costs->basic.size(); i++) {
						if (costs->name[i] != "-1" && costs->name[i] != "") {
							ImGui::InputFloat(costs->name[i].c_str(), &costs->basic[i]);
						}

					}
				}
			}
			if (selected_radio == 1) {
				if (ImGui::CollapsingHeader("Weights")) {
					ImGui::RadioButton("Player1: ", &weightsPlayer, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Player2: ", &weightsPlayer, 1);

					ImGui::Text("Save File Name:");
					ImGui::PushID(179);
					ImGui::InputText("", g_interfaces.cbrInterface.weightName, IM_ARRAYSIZE(g_interfaces.cbrInterface.weightName));
					ImGui::PopID();
					if (ImGui::Button("Save Cst"))
					{
						g_interfaces.cbrInterface.SaveWeights(g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs);
					}ImGui::SameLine();
					if (ImGui::Button("Load Cst"))
					{
						auto Cst = g_interfaces.cbrInterface.LoadWeights(g_interfaces.cbrInterface.weightName);
						if (Cst.name[0] != "-1") {
							g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs = Cst;
						}
					}

					auto costs = &g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs;
					for (int i = 0; i < costs->pressure.size(); i++) {
						if (costs->name[i] != "-1" && costs->name[i] != "") {
							ImGui::InputFloat(costs->name[i].c_str(), &costs->pressure[i]);
						}

					}
				}
			}
			if (selected_radio == 2) {
				if (ImGui::CollapsingHeader("Weights")) {
					ImGui::RadioButton("Player1: ", &weightsPlayer, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Player2: ", &weightsPlayer, 1);

					ImGui::Text("Save File Name:");
					ImGui::PushID(179);
					ImGui::InputText("", g_interfaces.cbrInterface.weightName, IM_ARRAYSIZE(g_interfaces.cbrInterface.weightName));
					ImGui::PopID();
					if (ImGui::Button("Save Cst"))
					{
						g_interfaces.cbrInterface.SaveWeights(g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs);
					}ImGui::SameLine();
					if (ImGui::Button("Load Cst"))
					{
						auto Cst = g_interfaces.cbrInterface.LoadWeights(g_interfaces.cbrInterface.weightName);
						if (Cst.name[0] != "-1") {
							g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs = Cst;
						}
					}

					auto costs = &g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs;
					for (int i = 0; i < costs->blocking.size(); i++) {
						if (costs->name[i] != "-1" && costs->name[i] != "") {
							ImGui::InputFloat(costs->name[i].c_str(), &costs->blocking[i]);
						}

					}
				}
			}
			if (selected_radio == 3) {
				if (ImGui::CollapsingHeader("Weights")) {
					ImGui::RadioButton("Player1: ", &weightsPlayer, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Player2: ", &weightsPlayer, 1);

					ImGui::Text("Save File Name:");
					ImGui::PushID(179);
					ImGui::InputText("", g_interfaces.cbrInterface.weightName, IM_ARRAYSIZE(g_interfaces.cbrInterface.weightName));
					ImGui::PopID();
					if (ImGui::Button("Save Cst"))
					{
						g_interfaces.cbrInterface.SaveWeights(g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs);
					}ImGui::SameLine();
					if (ImGui::Button("Load Cst"))
					{
						auto Cst = g_interfaces.cbrInterface.LoadWeights(g_interfaces.cbrInterface.weightName);
						if (Cst.name[0] != "-1") {
							g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs = Cst;
						}
					}

					auto costs = &g_interfaces.cbrInterface.getCbrData(weightsPlayer)->costs;
					for (int i = 0; i < costs->combo.size(); i++) {
						if (costs->name[i] != "-1" && costs->name[i] != "") {
							ImGui::InputFloat(costs->name[i].c_str(), &costs->combo[i]);
						}

					}
				}
			}
		}
	}
}

void MainWindow::DrawNetaSection() const
{
	if (!ImGui::CollapsingHeader("Neta Recorder"))
	{
		g_interfaces.cbrInterface.netaOpen = false;
		return;
	}

	if (!isInMatch())
	{
		ImGui::HorizontalSpacing();
		ImGui::TextDisabled("YOU ARE NOT IN MATCH!");
		g_interfaces.cbrInterface.netaOpen = false;

		return;
	}

	if (g_gameVals.GetGameMode() != GameMode_Training)
	{
		ImGui::HorizontalSpacing();
		ImGui::TextDisabled("Neta recorder only accesible in training mode.");
		g_interfaces.cbrInterface.netaOpen = false;
		return;
	}

	ImGui::HorizontalSpacing();

	if (isInMatch())
	{
		//ImGui::VerticalSpacing(10);
		//ImGui::HorizontalSpacing();

		g_interfaces.cbrInterface.netaOpen = true;

		if (ImGui::Button("Record Both Players"))
		{
			if (!g_interfaces.cbrInterface.netaPlaying) {
				if (g_interfaces.cbrInterface.netaRecording == false) {
					g_interfaces.cbrInterface.netaRecording = true;
					g_interfaces.cbrInterface.setAnnotatedReplay(AnnotatedReplay(g_interfaces.cbrInterface.playerName, g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetData()->charIndex, g_interfaces.Player2.GetData()->charIndex), 0);
					g_interfaces.cbrInterface.setAnnotatedReplay(AnnotatedReplay(g_interfaces.cbrInterface.playerName, g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetData()->charIndex, g_interfaces.Player1.GetData()->charIndex), 1);
				}
				else {
					g_interfaces.cbrInterface.netaRecording = false;
				}
			}
		}
		if (ImGui::Button("Replay Both Players"))
		{
			if (g_interfaces.cbrInterface.netaPlaying == false && g_interfaces.cbrInterface.getAnnotatedReplay(0)->getInputSize()> 0 &&
				g_interfaces.cbrInterface.getAnnotatedReplay(1)->getInputSize() > 0) {
				g_interfaces.cbrInterface.netaPlaying = !g_interfaces.cbrInterface.netaPlaying;
				g_interfaces.cbrInterface.netaReplayCounter = 0;
			}
			else {
				g_interfaces.cbrInterface.netaReplayCounter = 0;
				g_interfaces.cbrInterface.netaPlaying = false;
			}
		}

		ImGui::Checkbox("Freeze frame:##freeze_neta", &g_gameVals.isFrameFrozen);
		if (g_gameVals.pframe_count_minus_1_P1)
		{
			ImGui::SameLine();
			ImGui::Text("%d", *g_gameVals.pframe_count_minus_1_P1);
			ImGui::SameLine();
			if (ImGui::Button("Reset##reset_neta"))
			{
				*g_gameVals.pframe_count_minus_1_P1 = 0;
				g_gameVals.framesToReach = 0;
			}
		}

		if (g_gameVals.isFrameFrozen)
		{
			static int framesToStep = 1;
			ImGui::HorizontalSpacing();
			if (ImGui::Button("Step frames##stepframe_neta"))
			{
				g_gameVals.framesToReach = *g_gameVals.pframe_count_minus_1_P1 + framesToStep;
			}

			ImGui::SameLine();
			ImGui::SliderInt("", &framesToStep, 1, 60);
			ImGui::Text("FRC point P1: %s", g_interfaces.Player1.GetData()->extraData->FRCTimer > 0 ? "yes" : "no");
			ImGui::Text("FRC point P2: %s", g_interfaces.Player2.GetData()->extraData->FRCTimer > 0 ? "yes" : "no");
		}
		ImGui::Text("Recording: %d", g_interfaces.cbrInterface.netaRecording);
		ImGui::Text("Replaying %d", g_interfaces.cbrInterface.netaPlaying);
	
	}
}
void MainWindow::DrawCBRDebuggingSection() const {
	if (ImGui::CollapsingHeader("Debugging")) {
		static std::string debugS = "";
		static int replay = 0;
		static int caseIndex = 0;
		static int playerIndex = 0;
		ImGui::DragInt("Replay", &replay, 1.0F, 0, 99999);
		ImGui::SameLine();
		ImGui::DragInt("Case", &caseIndex);
		ImGui::DragInt("Player", &playerIndex, 1.0F, 0, 1);
		ImGui::SameLine();
		if (ImGui::Button("Display Case")) {
			auto caseFile = g_interfaces.cbrInterface.getCbrData(playerIndex)->getReplayFiles()->at(replay).getCase(caseIndex);
			debugS = "";
			debugS = caseFile->getMetadata()->PrintState();

			debugS += "\nInput: ";
			for (int i = caseFile->getStartingIndex(); i <= caseFile->getEndIndex(); i++) {
				debugS += std::to_string(g_interfaces.cbrInterface.getCbrData(playerIndex)->getReplayFiles()->at(replay).getInput(i)) + ", ";
			}

		}
		ImGui::Text(debugS.c_str());
	}
}
