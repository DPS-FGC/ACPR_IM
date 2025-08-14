#include "hooks_acpr.h"

#include "Core/interfaces.h"
#include "Core/Settings.h"
#include "Core/logger.h"
#include "Core/utils.h"
#include "Game/MatchState.h"
#include "Game/gamestates.h"
#include "Game/stages.h"
#include "Hooks/HookManager.h"
#include "Network/RoomManager.h"
#include "Overlay/WindowManager.h"
#include "SteamApiWrapper/steamApiWrappers.h"
#include "Windows.h"

DWORD GetGameStateJmpBackAddr = 0;
void __declspec(naked)GetGameState()
{
	_asm pushad

	if (g_gameVals.pGameScreen == NULL)
	{
		LOG_ASM(2, "GetGameState\n");

		_asm
		{
			lea ebx, g_gameVals.pGameScreen
			mov[ebx], eax
		}

		//InitSteamApiWrappers();
		//InitManagers();
		//WindowManager::GetInstance().Initialize(g_gameProc.hWndGameWindow, g_interfaces.pD3D9ExWrapper);
	}

	__asm
	{
		popad
		mov dl, [eax]
		mov bl, [ecx+eax]
		jmp[GetGameStateJmpBackAddr]
	}
}

DWORD MatchStartJmpBackAddr = 0;
DWORD P2PlayerExtraAddr = 0;
//This sets the Player extra address for P2.
//Activate OnMatchInit here as both players and their extra are initialized
void __declspec(naked) MatchStart()
{
	__asm
	{
		push eax
		mov eax, P2PlayerExtraAddr
		mov [esi + 2Ch], eax
		pop eax
		pushad
	}

	MatchState::OnMatchInit();

	__asm
	{
		popad
		jmp[MatchStartJmpBackAddr]
	}
}

DWORD MatchEndJmpBackAddr = 0;
//This activates when the inGameFlag is set to 0.
void __declspec(naked) MatchEnd()
{
	_asm pushad

	MatchState::OnMatchEnd();

	__asm
	{
		popad
		push eax
		mov eax, g_gameVals.pInGameFlag
		mov [eax], bl
		pop eax
		jmp[MatchEndJmpBackAddr]
	}
}

DWORD WindowMsgHandlerJmpBackAddr = 0;
DWORD GetWindowProcAddr = 0;
UINT lastMsg = 0;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __declspec(naked)PassMsgToImGui()
{
	static bool isWindowManagerInitialized = false;

	LOG_ASM(7, "PassMsgToImGui\n");

	__asm pushad
	isWindowManagerInitialized = WindowManager::GetInstance().IsInitialized();
	__asm popad

	__asm
	{
		pushad
		mov lastMsg, esi                        //Store windows instruction

		movzx ebx, isWindowManagerInitialized
		cmp ebx, 0                              //Skip this step if window manager is not initialized
		je SKIP

		popad
		call ImGui_ImplWin32_WndProcHandler
		jmp EXIT
SKIP:
		popad
		call dword ptr[GetWindowProcAddr]
		jmp[WindowMsgHandlerJmpBackAddr]
EXIT:
		call dword ptr [GetWindowProcAddr]
		jmp[WindowMsgHandlerJmpBackAddr]
	}
}

int PassKeyboardInputToGame()
{
	if (GetForegroundWindow() != g_gameProc.hWndGameWindow ||
		ImGui::GetIO().WantCaptureKeyboard)
	{
		return 0;
	}

	return 1;
}

DWORD DenyKeyboardInputFromGameJmpBackAddr = 0;
void __declspec(naked)DenyKeyboardInputFromGame()
{
	LOG_ASM(7, "DenyKeyboardInputFromGame\n");

	__asm
	{
		push eax
		call edx

		push eax
		call PassKeyboardInputToGame
		test eax, eax
		pop eax
		jz DENY

		cmp eax, esi
		jmp EXIT
DENY:
		push eax
		mov eax, 00h
		cmp eax, 01h //Force SF flag to 1 so that function exits
		pop eax
EXIT:
		jmp[DenyKeyboardInputFromGameJmpBackAddr]
	}
}

DWORD GetMatchVariablesJmpBackAddr = 0;
DWORD SetP2NameAddr = 0;
void __declspec(naked)GetMatchVariables()
{
	LOG_ASM(2, "GetMatchVariables\n");

	__asm
	{
		call dword ptr[SetP2NameAddr]
		pushad
	}

	MatchState::OnMatchInit();

	__asm
	{
END:
		popad
		jmp[GetMatchVariablesJmpBackAddr]
	}
}

DWORD JoinRoomJmpBackAddr = 0;
void __declspec(naked)JoinRoom()
{
	LOG_ASM(2, "JoinRoom\n");

	_asm pushad
	g_interfaces.pRoomManager->JoinRoom();
	__asm
	{
		popad
		mov [esi], 00000005h
		jmp[JoinRoomJmpBackAddr]
	}
}

DWORD SkipStateMismatchJmpBackAddr = 0;
DWORD CheckStateMismatchAddr = 0;
void __declspec(naked) SkipStateMismatch()
{
	__asm
	{
		call[CheckStateMismatchAddr]
		pushad
		cmp Settings::settingsIni.disableStateMismatch, 00h
		je EXIT
		test al, al
		jne EXIT
		popad
		mov al, 01h
		jmp RETURN
	EXIT:
		popad
	RETURN:
		jmp[SkipStateMismatchJmpBackAddr]
	}
}

int PassMouseMovementToGame()
{
	if (GetForegroundWindow() != g_gameProc.hWndGameWindow ||
		ImGui::GetIO().WantCaptureMouse)
	{
		return 0;
	}

	return 1;
}

DWORD User32SetWindowPosAddr = 0;
DWORD DenyWindowMovementJmpBackAddr = 0;
void __declspec(naked) DenyWindowMovement()
{
	__asm
	{
		push 00h
		push ebx

		pushad
		call PassMouseMovementToGame
		test eax, eax
		popad
		jz DENY

		call[User32SetWindowPosAddr]
		jmp EXIT
	DENY:
		add esp, 1Ch //Manually clean stack when not calling function		
	EXIT:
		jmp[DenyWindowMovementJmpBackAddr]
	}
}

DWORD GetSteamInterfacesJmpBackAddr = 0;
DWORD GetSteamFriendsPtr = 0;
void __declspec(naked)GetSteamInterfaces()
{
	LOG_ASM(2, "GetSteamInterfaces\n");

	__asm
	{
		push GetSteamFriendsPtr
		mov byte ptr[edi], 01h
		call esi
		pushad
		call placehooks_SteamAPI
		popad
		jmp[GetSteamInterfacesJmpBackAddr]
	}
}

DWORD GetReplayFileNameSaveJmpBackAddr = 0;
void __declspec(naked)GetReplayFileNameSave()
{
	static wchar_t* lpFileName = nullptr;

	__asm
	{
		lea edx, [esp + 224h]
		mov lpFileName, edx
		pushad
	}

	g_interfaces.pPaletteManager->SetReplayFileName(lpFileName);

	__asm
	{
		popad
		jmp[GetReplayFileNameSaveJmpBackAddr]
	}
}

DWORD LoadReplayJmpBackAddr = 0;
void __declspec(naked) LoadReplay()
{
	g_gameVals.loadingReplay = true;

	__asm
	{
		mov [esp+970h], 00000001h
		jmp[LoadReplayJmpBackAddr]
	}
}

DWORD GetReplayFileNameLoadJmpBackAddr = 0;
void __declspec(naked) GetReplayFileNameLoad()
{
	static wchar_t* lpFileName = nullptr;

	__asm
	{
		mov lpFileName, eax
	}

	//Indicator set by LoadReplay to distinguish between loading a replay and simply opening the file to read metadata.
	if (g_gameVals.loadingReplay)
		g_interfaces.pPaletteManager->SetReplayFileName(lpFileName);
	g_gameVals.loadingReplay = false;

	__asm
	{
		mov[ebp - 250h], edi
		jmp[GetReplayFileNameLoadJmpBackAddr]
	}
}

bool placeHooks_acpr()
{
	LOG(2, "placeHooks_acpr\n");

	GetGameStateJmpBackAddr = HookManager::SetHook("GetGameState", "\x8A\x10\x8A\x1C\x01", "xxxxx", 5, GetGameState);

	HookManager::RegisterHook("GetInGameFlag", "\x80\x3D\x00\x00\x00\x00\x00\x0F\x84\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00\x89\x9D",
		"xx????xxx????xx????xx", 6);
	g_gameVals.pInGameFlag = (byte*)HookManager::GetBytesFromAddr("GetInGameFlag", 2, 4);

	HookManager::RegisterHook("GetGameMode", "\x66\xA3\x00\x00\x00\x00\x89\x1D\x00\x00\x00\x00\x89\x3D\x00\x00\x00\x00\xE8",
		"xx????xx????xx????x", 6);
	g_gameVals.pGameMode = (int*)HookManager::GetBytesFromAddr("GetGameMode", 2, 4);

	WindowMsgHandlerJmpBackAddr = HookManager::SetHook("WindowMsgHandler", "\xFF\x15\x00\x00\x00\x00\x5F\x5E\x8B\xE5\x5D\xC2\x10\x00\x83\x3D\x00\x00\x00\x00\x01",
		"xx????xxxxxxxxxx????x", 6, PassMsgToImGui, false);
	GetWindowProcAddr = *(DWORD*)HookManager::GetBytesFromAddr("WindowMsgHandler", 2, 4);
	HookManager::ActivateHook("WindowMsgHandler");

	DenyKeyboardInputFromGameJmpBackAddr = HookManager::SetHook("DenyKeyboardInputFromGame", "\x50\xFF\xD2\x3B\xC6",
		"xxxxx", 5, DenyKeyboardInputFromGame);

	HookManager::RegisterHook("GetMatchTimer", "\x29\x1D\x00\x00\x00\x00\x0F\x85",
		"xx????xx", 6);
	g_gameVals.pMatchTimer = (int*)HookManager::GetBytesFromAddr("GetMatchTimer", 2, 4);

	HookManager::RegisterHook("GetFrameCounterP1", "\x01\x0C\x85\x00\x00\x00\x00\xF7\x05\x00\x00\x00\x00\x00\x02\x00\x00",
		"xxx????xx????xxxx", 7);
	g_gameVals.pframe_count_minus_1_P1 = (int*)HookManager::GetBytesFromAddr("GetFrameCounterP1", 3, 4);

	HookManager::RegisterHook("GetInPauseMenu", "\x89\x1D\x00\x00\x00\x00\x89\x1D\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\xF0\x00\x00\x00",
		"xx????xx????xx????xxxx", 6);
	g_gameVals.pInPauseMenu = (int*)HookManager::GetBytesFromAddr("GetInPauseMenu", 2, 4);

	HookManager::RegisterHook("GetInReplayPauseMenu", "\xC6\x05\x00\x00\x00\x00\x01\xFF\xD2\xA3",
		"xx????xxxx", 7);
	g_gameVals.pInReplayPauseMenu = (byte*)HookManager::GetBytesFromAddr("GetInReplayPauseMenu", 2, 4);

	HookManager::RegisterHook("GetIsPauseMenuDisplayed", "\x89\x1D\x00\x00\x00\x00\x39\x3D\x00\x00\x00\x00\x74\x07",
		"xx????xx????xx", 6);
	g_gameVals.pIsPauseMenuDisplayed = (int*)HookManager::GetBytesFromAddr("GetIsPauseMenuDisplayed", 2, 4);
	
	GetMatchVariablesJmpBackAddr = HookManager::SetHook("GetMatchVariables", "\xE8\x00\x00\x00\x00\x83\xC4\x04\xEB\x05\xE8\x00\x00\x00\x00\x0F\xB7\x0E\xC7\x46\x10\xFF\xFF\xFF\xFF\x8B\x15\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x68\x48\x01\x00\x00\x53",
		"x????xxxxxx????xxxxxxxxxxxx????x????xxxxxx", 5, GetMatchVariables, false);
	SetP2NameAddr = (DWORD)HookManager::GetBytesFromAddr("GetMatchVariables", 1, 4) + GetMatchVariablesJmpBackAddr;
	HookManager::ActivateHook("GetMatchVariables");

	HookManager::RegisterHook("GetEntityStartAddress", "\xA3\x00\x00\x00\x00\x33\xC0\x89\x15\x00\x00\x00\x00\x66\xA3\x00\x00\x00\x00\x83\xC3\x04\xC7\x45\xF0",
		"x????xxxx????xx????xxxxxx", 5);
	g_gameVals.pEntityStartAddress = (void**)HookManager::GetBytesFromAddr("GetEntityStartAddress", 1, 4);

	HookManager::RegisterHook("GetEntityCount", "\x66\xA3\x00\x00\x00\x00\x83\xC3\x04\xC7\x45\xF0",
		"xx????xxxxxx", 6);
	g_gameVals.entityCount = (uint16_t*)HookManager::GetBytesFromAddr("GetEntityCount", 2, 4);

	HookManager::RegisterHook("GetRoundCounter", "\x89\x15\x00\x00\x00\x00\x8B\x15\x00\x00\x00\x00\x69\xD2\xE2\x04\x00\x00",
		"xx????xx????xxxxxx", 6);
	g_gameVals.pRoundCounter = (int*)HookManager::GetBytesFromAddr("GetRoundCounter", 2, 4);

	HookManager::RegisterHook("GetCameraPtr", "\xD9\x1D\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00\xD8\xD1\xDF\xE0\xF6\xC4\x05\x7A\x0A",
		"xx????xx????xxxxxxxxx", 6);
	g_gameVals.pCamera = (Camera*)HookManager::GetBytesFromAddr("GetCameraPtr", 2, 4);

	HookManager::RegisterHook("GetThrowRangeLookupAddr", "\x0f\xB7\x0C\x45\x00\x00\x00\x00\x83\xF8\x19",
		"xxxx????xxx", 8);
	g_gameVals.pThrowRangeLookupTable = (uint16_t*)HookManager::GetBytesFromAddr("GetThrowRangeLookupAddr", 4, 4);

	HookManager::RegisterHook("GetCommandGrabIdAddr", "\x8B\x14\x9D\00\x00\x00\x00\x0F\xB7\x04\x55\x00\x00\x00\x00",
		"xxx????xxx????", 7);
	g_gameVals.pCommandThrowId[0] = (int*)HookManager::GetBytesFromAddr("GetCommandGrabIdAddr", 3, 4);
	g_gameVals.pCommandThrowId[1] = (int*)((char*)g_gameVals.pCommandThrowId[0] + 4);

	HookManager::RegisterHook("GetStandingPushboxDimensions", "\x0F\xBF\x94\x00\x00\x00\x00\x00\x03\xC0\x83\x3D\x00\x00\x00\x00\x00\x89\x55\xF8\x74\x0C\x0F\xBF\x80\x00\x00\x00\x00",
		"xxxx????xxxx????xxxxxxxxx????", 8);
	g_gameVals.pPushboxDimensions[0][0] = (uint16_t*)HookManager::GetBytesFromAddr("GetStandingPushboxDimensions",  4, 4); //Standing width
	g_gameVals.pPushboxDimensions[0][1] = (uint16_t*)HookManager::GetBytesFromAddr("GetStandingPushboxDimensions", 25, 4); //Standing height

	HookManager::RegisterHook("GetCrouchingPushboxDimensions", "\x0F\xBF\x94\x00\x00\x00\x00\x00\x03\xC0\x0F\xBF\x80\x00\x00\x00\x00\x89\x55\xF8",
		"xxxx????xxxxx????xxx", 8);
	g_gameVals.pPushboxDimensions[1][0] = (uint16_t*)HookManager::GetBytesFromAddr("GetCrouchingPushboxDimensions",  4, 4); //Crouching width
	g_gameVals.pPushboxDimensions[1][1] = (uint16_t*)HookManager::GetBytesFromAddr("GetCrouchingPushboxDimensions", 13, 4); //Crouching height

	HookManager::RegisterHook("GetAirPushboxDimensions", "\x0F\xBF\x9C\x00\x00\x00\x00\x00\x03\xC0\x83\x3D\x00\x00\x00\x00\x00\x89\x5D\xF8\x0F\xBF\x98\x00\x00\x00\x00\x89\x5D\xF0",
		"xxxx????xxxx????xxxxxxx????xxx", 8);
	g_gameVals.pPushboxDimensions[2][0] = (uint16_t*)HookManager::GetBytesFromAddr("GetAirPushboxDimensions",  4, 4); //Air width
	g_gameVals.pPushboxDimensions[2][1] = (uint16_t*)HookManager::GetBytesFromAddr("GetAirPushboxDimensions", 23, 4); //Air height

	HookManager::RegisterHook("GetPlayerPushboxYoffset", "\x89\x3C\x95\x00\x00\x00\x00\x0F\xB6\x56\x27",
		"xxx????xxxx", 7);
	g_gameVals.pPushboxJumpOffset[0] = (uint16_t*)HookManager::GetBytesFromAddr("GetPlayerPushboxYoffset", 3, 4);
	g_gameVals.pPushboxJumpOffset[1] = (uint16_t*)((char*)g_gameVals.pPushboxJumpOffset[0] + 4);

	HookManager::RegisterHook("GetGameVer", "\x83\x3D\x00\x00\x00\x00\x00\x74\x16\x0F\xB7\x09",
		"xx????xxxxxx", 6);
	g_gameVals.pGameVerFlag = (byte*)HookManager::GetBytesFromAddr("GetGameVer", 2, 4);

	HookManager::RegisterHook("GetGroundThrowRange", "\x0F\xBF\x14\x4D\x00\x00\x00\x00\x39\x15\x00\x00\x00\x00\x7E\x16\x33\xC0\xC3\x0F\xB7\x01\x0F\xBF\x0C\x45\x00\x00\x00\x00",
		"xxxx????xx????xxxxxxxxxxxx????", 8);
	g_gameVals.pGroundThrowRange[0] = (int16_t*)HookManager::GetBytesFromAddr("GetGroundThrowRange", 26, 4);
	g_gameVals.pGroundThrowRange[1] = (int16_t*)HookManager::GetBytesFromAddr("GetGroundThrowRange",  4, 4);

	HookManager::RegisterHook("GetAirHorizontalThrowRange", "\x0F\xBF\x90\x00\x00\x00\x00\x39\x15\x00\x00\x00\x00\x7E\x12\x33\xC0\xC3\x0F\xBF\x90\x00\x00\x00\x00",
		"xxx????xx????xxxxxxxx????", 7);
	g_gameVals.pAirHorizontalThrowRange[0] = (int16_t*)HookManager::GetBytesFromAddr("GetAirHorizontalThrowRange", 21, 4);
	g_gameVals.pAirHorizontalThrowRange[1] = (int16_t*)HookManager::GetBytesFromAddr("GetAirHorizontalThrowRange",  3, 4);

	HookManager::RegisterHook("GetAirVerticalThrowRange", "\x0F\xBF\x90\x00\x00\x00\x00\x3B\xCA\x7F\xC8\x0F\xBF\x80\x00\x00\x00\x00",
		"xxx????xxxxxxx????", 7);
	g_gameVals.pAirVerticalThrowRange[0] = (int16_t*)HookManager::GetBytesFromAddr("GetAirVerticalThrowRange",  3, 4);
	g_gameVals.pAirVerticalThrowRange[1] = (int16_t*)HookManager::GetBytesFromAddr("GetAirVerticalThrowRange", 14, 4);

	HookManager::RegisterHook("GetCommandGrabThrowRange", "\x0F\xB7\x0C\x45\x00\x00\x00\x00\x83\xF8\x19",
		"xxxx????xxx", 8);
	g_gameVals.pCommandGrabThrowRanges = (int16_t*)HookManager::GetBytesFromAddr("GetCommandGrabThrowRange", 4, 4);

	MatchStartJmpBackAddr = HookManager::SetHook("MatchStart", "\xC7\x46\x2C\x00\x00\x00\x00\x83\xC4\x0C\x33\xC9",
		"xxx????xxxxx", 7, MatchStart, false);
	P2PlayerExtraAddr = HookManager::GetBytesFromAddr("MatchStart", 3, 4);
	HookManager::ActivateHook("MatchStart");

	MatchEndJmpBackAddr = HookManager::SetHook("MatchEnd", "\x88\x1D\x00\x00\x00\x00\xD9\x05\x00\x00\x00\x00",
		"xx????xx????", 6, MatchEnd, true);

	HookManager::RegisterHook("GetFPSCounter", "\xF7\x25\x00\x00\x00\x00\x8B\xCA\xC1\xE9\x05",
		"xx????xxxxx", 6);
	g_gameVals.pFPSCounter = (int*)HookManager::GetBytesFromAddr("GetFPSCounter", 2, 4);

	SkipStateMismatchJmpBackAddr = HookManager::SetHook("SkipStateMismatch", "\xE8\x00\x00\x00\x00\x81\xC4\x08\x01\x00\x00\x84\xC0",
		"x????xxxxxxxx", 5, SkipStateMismatch, false);
	CheckStateMismatchAddr = SkipStateMismatchJmpBackAddr + (DWORD)HookManager::GetBytesFromAddr("SkipStateMismatch", 1, 4);
	HookManager::ActivateHook("SkipStateMismatch");

	DenyWindowMovementJmpBackAddr = HookManager::SetHook("DenyWindowMovement", "\x6A\x00\x53\xFF\x15\x00\x00\x00\x00\x8B\x4D\xFC",
		"xxxxx????xxx", 9, DenyWindowMovement, false);
	User32SetWindowPosAddr = *(DWORD*)(HookManager::GetBytesFromAddr("DenyWindowMovement", 5, 4));
	HookManager::ActivateHook("DenyWindowMovement");

	GetSteamInterfacesJmpBackAddr = HookManager::SetHook("GetSteamFriends", "\x68\x00\x00\x00\x00\xC6\x07\x01\xFF\xD6",
		"x????xxxxx", 10, GetSteamInterfaces, false);
	GetSteamFriendsPtr = HookManager::GetBytesFromAddr("GetSteamFriends", 1, 4);
	HookManager::ActivateHook("GetSteamFriends");

	HookManager::RegisterHook("GetBaseNetworkAddress", "\xA1\x00\x00\x00\x00\x8B\x50\x1C\xB9\x00\x00\x00\x00\xFF\xD2\x83\x78\x08\x01\x0F\x85\xDA\x00\x00\x00",
		"x????xxxx????xxxxxxxxxxxx", 5);
	g_gameVals.pRoom = (Room*)(HookManager::GetBytesFromAddr("GetBaseNetworkAddress", 1, 4) + 0x2A60);

	HookManager::RegisterHook("GetBaseRoomAddress", "\xC6\x45\xFC\x05\xE8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\x8B\x4D\xF4\x64\x89\x0D\x00\x00\x00\x00\x59\x5E\x5B\x8B\xE5\x5D\xC3",
		"xxxxx????x????xxxxxxxxxxxxxxxxx", 14);
	g_gameVals.pSelectedRoomMember = (int*)(HookManager::GetBytesFromAddr("GetBaseRoomAddress", 10, 4) + 0x2750);
	g_gameVals.ppFirstRoomMemberDynamic = (RoomMemberEntry**)(HookManager::GetBytesFromAddr("GetBaseRoomAddress", 10, 4) + 0x2788);

	HookManager::RegisterHook("GetRoomMemberList", "\x89\x04\xB5\x00\x00\x00\x00\x46\xEB\x0A", "xxx????xxx", 7);
	g_gameVals.ppFirstRoomMemberStatic = (RoomMemberEntry**)(HookManager::GetBytesFromAddr("GetRoomMemberList", 3, 4));

	HookManager::RegisterHook("GetBaseRoomAddress2", "\xA1\x00\x00\x00\x00\x8B\x50\x1C\xB9\x00\x00\x00\x00\xFF\xD2\x83\x78\x08\x01\x0F\x85\xDA\x00\x00\x00",
		"x????xxxx????xxxxxxxxxxxx", 5);
	g_gameVals.pNumberOfPlayersInRoom = (int*)(HookManager::GetBytesFromAddr("GetBaseRoomAddress2", 1, 4) + 0x1658);

	JoinRoomJmpBackAddr = HookManager::SetHook("JoinRoom", "\xC7\x06\x05\x00\x00\x00\xD9\x56\x04",
		"xxxxxxxxx", 6, JoinRoom, true);

	HookManager::RegisterHook("GetSpectatingTestVariables", "\xA1\x00\x00\x00\x00\x0B\x05\x00\x00\x00\x00\x75\x0F\x8B\x4D\xF4",
		"x????xx????xxxxx", 11);
	g_gameVals.pSpectatingTest1 = (uint32_t*)HookManager::GetBytesFromAddr("GetSpectatingTestVariables", 1, 4);
	g_gameVals.pSpectatingTest2 = (uint32_t*)HookManager::GetBytesFromAddr("GetSpectatingTestVariables", 7, 4);

	GetReplayFileNameSaveJmpBackAddr = HookManager::SetHook("GetReplayFileNameSave", "\x8D\x94\x24\x24\x02\x00\x00\x52",
		"xxxxxxxx", 7, GetReplayFileNameSave, true);

	LoadReplayJmpBackAddr = HookManager::SetHook("LoadReplay", "\xC7\x84\x24\x70\x09\x00\x00\x01\x00\x00\x00",
		"xxxxxxxxxxx", 11, LoadReplay, true);

	GetReplayFileNameLoadJmpBackAddr = HookManager::SetHook("GetReplayFileNameLoad", "\x89\xBD\xB0\xFD\xFF\xFF\x89\x75\xFC",
		"xxxxxxxxx", 6, GetReplayFileNameLoad, true);

	HookManager::RegisterHook("GetSteamID", "\x89\x0D\x00\x00\x00\x00\x8B\x50\x04\x6A\x40",
		"xx????xxxxx", 6);
	g_gameVals.pSteamID = (uint64_t*)HookManager::GetBytesFromAddr("GetSteamID", 2, 4);

	HookManager::RegisterHook("GetPlayerNameReplay", "\xB9\x00\x00\x00\x00\xFF\xD0\x88\x44\x24\x15",
		"x????xxxxxx", 5);
	g_gameVals.pPlayerNameReplay[0] = (char*)HookManager::GetBytesFromAddr("GetPlayerNameReplay", 1, 4) + 0x50;
	g_gameVals.pPlayerNameReplay[1] = (char*)HookManager::GetBytesFromAddr("GetPlayerNameReplay", 1, 4) + 0x70;

	HookManager::RegisterHook("GetPlayerNameOnline", "\x8B\x50\x30\xB9\x00\x00\x00\x00\xFF\xD2\x8B\x4D\xF4",
		"xxxx????xxxxx", 8);
	g_gameVals.pPlayerNameOnline[0] = (char*)HookManager::GetBytesFromAddr("GetPlayerNameOnline", 4, 4) + 0x4C;
	g_gameVals.pPlayerNameOnline[1] = (char*)HookManager::GetBytesFromAddr("GetPlayerNameOnline", 4, 4) + 0x6C;

	return true;
}

void __stdcall placehooks_SteamAPI()
{
	LOG(1, "Placing SteamAPI Hooks\n");

	HookManager::RegisterHook("GetSteamUserStats", "\x68\x00\x00\x00\x00\x89\x56\x0C\xFF\xD7", "x????xxxxx", 5);
	g_tempVals.ppSteamUserStats = (ISteamUserStats**)(HookManager::GetBytesFromAddr("GetSteamUserStats", 1, 4) + 8);

	HookManager::RegisterHook("GetSteamMatchmakingAndNetworking", "\x68\x00\x00\x00\x00\xFF\xD6\x8B\x08\x8B\x01\x8B\x40\x04", "x????xxxxxxxxx", 5);

	//Real steap_api.dll uses a wrapper for ISteamNetworking, therefore getting the pointer is slightly different
	//from Goldberg's steam emulator.
	if (Settings::settingsIni.steamdlltype == 0)
		g_tempVals.ppSteamNetworking = (ISteamNetworking**)((char*)*(uint32_t*)(HookManager::GetBytesFromAddr("GetSteamMatchmakingAndNetworking", 1, 4) + 8) + 4);
	else
		g_tempVals.ppSteamNetworking = (ISteamNetworking**)(HookManager::GetBytesFromAddr("GetSteamMatchmakingAndNetworking", 1, 4) + 8);

	InitSteamApiWrappers();
	InitManagers();
	WindowManager::GetInstance().Initialize(g_gameProc.hWndGameWindow, g_interfaces.pD3D9Wrapper);
}