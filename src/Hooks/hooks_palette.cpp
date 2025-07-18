#include "hooks_palette.h"

#include "HookManager.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Game/gamestates.h"

DWORD GetPalBaseAddressesJmpBackAddr = 0;
DWORD PaletteBaseAddr = 0;
void __declspec(naked) GetPalBaseAdresses()
{
	static int counter = 0;
	static char* palPointer = 0;

	__asm
	{
		pushad
		mov palPointer, ecx
	}

	if (counter == 0)
	{
		g_interfaces.Player1.GetPalHandle().SetPointerBasePal(palPointer);
	}
	else if (counter == 1)
	{
		g_interfaces.Player2.GetPalHandle().SetPointerBasePal(palPointer);
	}

	counter = (counter + 1) % 2;

	__asm
	{
		popad
		push ebx
		mov ebx, PaletteBaseAddr
		mov[eax * 4 + ebx], ecx
		pop ebx
		jmp[GetPalBaseAddressesJmpBackAddr]
	}
}

DWORD UpdatePaletteJmpBackAddr = 0;
void __declspec(naked) UpdatePalette()
{
	static int playerNr = -1;
	static bool requires_update = false;

	__asm
	{
		test eax, 00000400h
		je[CODE]
		jmp[UpdatePaletteJmpBackAddr]
	}

	__asm
	{
	CODE:
		pushad
		movzx eax, byte ptr[ebx + 27h]
		mov playerNr, eax
		popad
		pushad
	}
	
	if (playerNr == 0)
		requires_update = g_interfaces.Player1.GetPalHandle().RequiresUpdate();
	else if (playerNr == 1)
		requires_update = g_interfaces.Player2.GetPalHandle().RequiresUpdate();

	if (requires_update)
	{
		if (playerNr == 0)
			g_interfaces.Player1.GetPalHandle().PaletteUpdated();
		else if (playerNr == 1)
			g_interfaces.Player2.GetPalHandle().PaletteUpdated();
	}

	__asm
	{
		popad
		push eax
		movzx eax, requires_update
		test eax, 01h
		pop eax
		jmp[UpdatePaletteJmpBackAddr]
	}
}

bool placeHooks_palette()
{
	char* addr = nullptr;
	HookManager::RegisterHook("GetCharObjPointerP1", "\x03\x0D\x00\x00\x00\x00\x8D\x3C\xF5",
		"xx????xxx", 6);
	addr = (char*)HookManager::GetBytesFromAddr("GetCharObjPointerP1", 2, 4);
	g_interfaces.Player1.SetCharDataPtr(addr);

	HookManager::RegisterHook("GetCharObjPointerP2", "\x89\x0D\x00\x00\x00\x00\x83\xC4\x0C\x33\xC9",
		"xx????xxxxx", 6);
	addr = (char*)HookManager::GetBytesFromAddr("GetCharObjPointerP2", 2, 4);
	g_interfaces.Player2.SetCharDataPtr(addr);

	GetPalBaseAddressesJmpBackAddr = HookManager::SetHook("GetPalBaseAddresses", "\x89\x0C\x85\x00\x00\x00\x00\x33\xC0\x5D\xC3\x55",
		"xxx????xxxxx", 7, GetPalBaseAdresses, false);
	PaletteBaseAddr = HookManager::GetBytesFromAddr("GetPalBaseAddresses", 3, 4);
	HookManager::ActivateHook("GetPalBaseAddresses");
	
	HookManager::RegisterHook("GetPaletteIndexPointers", "\x0F\xB7\x0C\x7D\x00\x00\x00\x00\x8B\x04\x88",
		"xxxx????xxx", 8);
	uint16_t* pPalIndex = (uint16_t*)HookManager::GetBytesFromAddr("GetPaletteIndexPointers", 4, 4);
	g_interfaces.Player1.GetPalHandle().SetPointerPalIndex(pPalIndex);
	pPalIndex = (uint16_t*)((char*)pPalIndex + 2);
	g_interfaces.Player2.GetPalHandle().SetPointerPalIndex(pPalIndex);

	UpdatePaletteJmpBackAddr = HookManager::SetHook("UpdatePalette", "\xA9\x00\x04\x00\x00\x74\x0C\x8B\x8B\x94\x00\x00\x00",
		"xxxxxxxxxxxxx", 5, UpdatePalette, true);

	return true;
}