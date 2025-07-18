#include "hooks_detours.h"

#include "HookManager.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "D3D9Wrapper/ID3D9Wrapper.h"

#include <detours.h>

#pragma comment(lib, "detours.lib")

typedef IDirect3D9* (__stdcall* Direct3DCreate9_t)(UINT SDKVersion);
typedef HWND(__stdcall* CreateWindowExW_t)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
	DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

Direct3DCreate9_t orig_Direct3DCreate9;
CreateWindowExW_t orig_CreateWindowExW;

IDirect3D9* __stdcall hook_Direct3DCreate9(UINT sdkVers)
{
	LOG(1, "Direct3DCreate9\n");
	IDirect3D9* retval = orig_Direct3DCreate9(sdkVers); // real one

	Direct3D9Wrapper* ret = new Direct3D9Wrapper(&retval);
	return retval;
}


HWND WINAPI hook_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
	DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	LOG(7, "CreateWindowExW\n");
	static int counter = 1;
	HWND hWnd = orig_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	if (SUCCEEDED(hWnd))
	{
		LOG(7, "\tSuccess: 0x%p\n", hWnd);
		if (counter == 4) // determined it's 4th window by looking at ImGui_ImplWin32_WndProcHandler and comaring with hwnd there. Doesn't work in borderless mode.
		{
			LOG(2, "Correct window: 0x%p\n", hWnd);
			g_gameProc.hWndGameWindow = hWnd;
		}
	}
	counter++;
	return hWnd;
}

bool placeHooks_detours()
{
	LOG(1, "placeHooks_detours\n");

	HMODULE hM_d3d9 = GetModuleHandleA("d3d9.dll");
	HMODULE hM_user32 = GetModuleHandleA("user32.dll");

	PBYTE pDirect3DCreate9 = (PBYTE)GetProcAddress(hM_d3d9, "Direct3DCreate9");
	PBYTE pCreateWindowExW = (PBYTE)GetProcAddress(hM_user32, "CreateWindowExW");

	if (!hookSucceeded((PBYTE)pDirect3DCreate9, "Direct3DCreate9"))
		return false;
	if (!hookSucceeded((PBYTE)pCreateWindowExW, "CreateWindowExW"))
		return false;

	orig_Direct3DCreate9 = (Direct3DCreate9_t)DetourFunction(pDirect3DCreate9, (LPBYTE)hook_Direct3DCreate9);
	orig_CreateWindowExW = (CreateWindowExW_t)DetourFunction(pCreateWindowExW, (LPBYTE)hook_CreateWindowExW);
	
	return true;
}