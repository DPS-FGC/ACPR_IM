#include "WindowManager.h"

#include "fonts.h"
#include "NotificationBar/NotificationBar.h"
#include "WindowContainer/WindowContainer.h"
#include "Window/LogWindow.h"

#include "Core/info.h"
#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Core/Settings.h"
#include "Core/utils.h"
#include "Web/update_check.h"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <ctime>
#include <Game/gamestates.h>

#define DEFAULT_ALPHA 0.87f

int keyToggleMainWindow;
int keyToggleRoomWindow;
int keyCBRsave;
int keyCBRdiscard;

WindowManager* WindowManager::m_instance = nullptr;

WindowManager & WindowManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new WindowManager();
	}
	return *m_instance;
}

bool WindowManager::Initialize(void *hwnd, IDirect3DDevice9 *device)
{
	if (m_initialized)
	{
		return true;
	}

	LOG(2, "WindowManager::Initialize\n");

	if (!hwnd)
	{
		LOG(2, "HWND not found!\n");
		return false;
	}
	if (!device)
	{
		LOG(2, "Direct3DDevice9 not found!\n");
		return false;
	}

	m_initialized = ImGui_ImplDX9_Init(hwnd, device);
	if (!m_initialized)
	{
		LOG(2, "ImGui_ImplDX9_Init failed!\n");
		return false;
	}

	m_pLogger = g_imGuiLogger;

	m_pLogger->Log("[system] Initialization starting...\n");

	m_windowContainer = new WindowContainer();

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 1;
	style.FrameBorderSize = 1;
	style.ScrollbarSize = 14;
	style.Alpha = DEFAULT_ALPHA;

	// Add default font

	float unicodeFontSize = 18;

	if (Settings::settingsIni.menusize == 1)
	{
		ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(TinyFont_compressed_data_base85, 10);
		unicodeFontSize = 14;
	}
	else if (Settings::settingsIni.menusize == 3)
	{
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, 20);
		unicodeFontSize = 25;
	}
	else
	{
		ImGui::GetIO().Fonts->AddFontDefault();
	}

	// Add Unicode font

	ImFontConfig config;
	config.MergeMode = true;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.PixelSnapH = true;
	static const ImWchar ranges[] =
	{
		//0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0xFFFF, // All
		0,
	};
	//const ImWchar* ranges = ImGui::GetIO().Fonts->GetGlyphRangesJapanese();
	//const ImWchar* ranges2 = ImGui::GetIO().Fonts->GetGlyphRangesCyrillic();
	//ranges = ranges + ranges2;
	//ranges = ranges + 2; // Skip default ranges to prevent overwriting the default font

	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(mplusMedium_compressed_data, mplusMedium_compressed_size,
		unicodeFontSize, &config, ranges);

	// Set up toggle buttons

	keyToggleMainWindow = Settings::getButtonValue(Settings::settingsIni.togglebutton);
	m_pLogger->Log("[system] Toggling key set to '%s'\n", Settings::settingsIni.togglebutton.c_str());

	keyToggleRoomWindow = Settings::getButtonValue(Settings::settingsIni.toggleOnlineButton);
	m_pLogger->Log("[system] Online toggling key set to '%s'\n", Settings::settingsIni.toggleOnlineButton.c_str());

	keyCBRsave = Settings::getButtonValue(Settings::settingsIni.saveCBRbutton);
	keyCBRdiscard = Settings::getButtonValue(Settings::settingsIni.discardCBRbutton);

	// Load custom palettes

	g_interfaces.pPaletteManager->LoadAllPalettes();

	// Calling a frame to initialize beforehand to prevent a crash upon first call of Update() if the game window is not focused.
	// Simply calling ImGui_ImplDX9_CreateDeviceObjects() might be enough too
	ImGui_ImplDX9_NewFrame();
	ImGui::EndFrame();
	///////

	srand(time(NULL));

	StartAsyncUpdateCheck();

	std::string notificationText = MOD_WINDOW_TITLE;
	notificationText += " ";
	notificationText += MOD_VERSION_NUM;

#ifdef _DEBUG
	notificationText += " (DEBUG)";
#endif

	g_notificationBar->AddNotification("%s (Press %s to open the main window)",
		notificationText.c_str(), Settings::settingsIni.togglebutton.c_str());

	m_pLogger->Log("[system] Finished initialization\n");
	m_pLogger->LogSeparator();
	LOG(2, "Initialize end\n");

	return true;
}

void WindowManager::Shutdown()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::Shutdown\n");

	SAFE_DELETE(m_windowContainer);
	delete m_instance;

	ImGui_ImplDX9_Shutdown();
}

void WindowManager::InvalidateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::InvalidateDeviceObjects\n");
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void WindowManager::CreateDeviceObjects()
{
	if (!m_initialized)
	{
		return;
	}

	LOG(2, "WindowManager::CreateDeviceObjects\n");
	ImGui_ImplDX9_CreateDeviceObjects();
}

void WindowManager::Render()
{
	if (!m_initialized)
	{
		return;
	}

	if (g_interfaces.pSteamApiHelper->IsSteamOverlayActive())
	{
		return;
	}

	LOG(7, "WindowManager::Render\n");

	HandleButtons();
	CbrUIManagement();

	ImGui_ImplDX9_NewFrame();

	bool isMainWindowOpen = 
		m_windowContainer->GetWindow(WindowType_Main)->IsOpen();
	bool isUpdateNotifierWindowOpen =
		m_windowContainer->GetWindow(WindowType_UpdateNotifier)->IsOpen();
	bool isPaletteEditorWindowOpen =
		m_windowContainer->GetWindow(WindowType_PaletteEditor)->IsOpen();
	bool isLogWindowOpen =
		m_windowContainer->GetWindow(WindowType_Log)->IsOpen();
	bool isRoomWindowOpen =
		m_windowContainer->GetWindow(WindowType_Room)->IsOpen();
	bool isDebugWindowOpen =
		m_windowContainer->GetWindow(WindowType_Debug)->IsOpen();
	bool isCbrServerWindowOpen =
		m_windowContainer->GetWindow(WindowType_CbrServer)->IsOpen();

	ImGui::GetIO().MouseDrawCursor = isMainWindowOpen || isLogWindowOpen || isPaletteEditorWindowOpen
		|| isUpdateNotifierWindowOpen || isRoomWindowOpen || isDebugWindowOpen || isCbrServerWindowOpen;

	DrawAllWindows();

	g_notificationBar->DrawNotifications();

	ImGui::Render();

	if (g_gameVals.GetGameState() != 15) {
		g_interfaces.cbrInterface.EndCbrActivities();
	}
}

void WindowManager::HandleButtons()
{
	if (!m_initialized)
	{
		return;
	}

	if (ImGui::IsKeyPressed(keyToggleMainWindow))
	{
		m_windowContainer->GetWindow(WindowType_Main)->ToggleOpen();
	}
	
	if (ImGui::IsKeyPressed(keyToggleRoomWindow))
	{
		m_windowContainer->GetWindow(WindowType_Room)->ToggleOpen();
	}
}

void WindowManager::DrawAllWindows() const
{
	for (const auto& window : m_windowContainer->GetWindows())
	{
		window.second->Update();
	}
}

void WindowManager::CbrUIManagement() {
	int stateVal = stateChange(g_gameVals.GetGameState());
	if (g_interfaces.cbrInterface.autoRecordFinished == true ) {
		if (g_interfaces.cbrInterface.autoRecordConfirmation == false && stateVal == 16) {
			g_notificationBar->AddNotification("Currently %s replays unsaved. Press %s to save or %s to discard them.", std::to_string(g_interfaces.cbrInterface.getAutoRecordReplayAmount()).c_str(), Settings::settingsIni.saveCBRbutton.c_str(), Settings::settingsIni.discardCBRbutton.c_str());
		}
		if (g_interfaces.cbrInterface.autoRecordConfirmation == true && stateVal == 16) {
			g_notificationBar->AddNotification("Currently %s replays unsaved. Press %s or return to lobby/character select to save or press %s to discard them. ", std::to_string(g_interfaces.cbrInterface.getAutoRecordReplayAmount()).c_str(), Settings::settingsIni.saveCBRbutton.c_str(), Settings::settingsIni.discardCBRbutton.c_str());

			g_interfaces.cbrInterface.autoRecordSaveCompleted = false;
		}
		g_interfaces.cbrInterface.autoRecordFinished = false;
	}

	if (g_interfaces.cbrInterface.autoRecordConfirmation == true && stateVal != 14 && stateVal != 15 && stateVal != 16 && stateVal != -1 && (g_interfaces.cbrInterface.recordBufferP1.size() > 0 || g_interfaces.cbrInterface.recordBufferP2.size() > 0)) {
		g_interfaces.cbrInterface.autoRecordSaveCompleted = false;
		g_notificationBar->AddNotification("Saving Replays... please dont close the game");
		g_interfaces.cbrInterface.threadSaveReplay(true);
	}

	if (ImGui::IsKeyPressed(keyCBRsave, false)) {
		if (g_interfaces.cbrInterface.recordBufferP1.size() > 0 || g_interfaces.cbrInterface.recordBufferP2.size() > 0) {
			if (!isInMatch()) {
				g_notificationBar->AddNotification("Saving Replays... please dont close the game");
				g_interfaces.cbrInterface.threadSaveReplay(true);
			}
			else {
				g_notificationBar->AddNotification("Cant save during a match.");
			}
		}
		else {
			g_notificationBar->AddNotification("No Replays to save");
		}

	}
	if (ImGui::IsKeyPressed(keyCBRdiscard, false)) {
		if (g_interfaces.cbrInterface.recordBufferP1.size() > 0 || g_interfaces.cbrInterface.recordBufferP2.size() > 0) {
			g_notificationBar->AddNotification("Replays Deleted");
			g_interfaces.cbrInterface.clearAutomaticRecordReplays();
		}
		else {
			g_notificationBar->AddNotification("No Replays to save");
		}
	}
	if (g_interfaces.cbrInterface.threadCheckSaving()) {
		g_notificationBar->AddNotification("Saving Completed");
	}
	if (g_interfaces.cbrInterface.threadCheckSaving()) {
		g_notificationBar->AddNotification("Saving Completed");
	}
}
