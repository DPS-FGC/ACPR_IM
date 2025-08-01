#include "crashdump.h"
#include "interfaces.h"
#include "logger.h"
#include "Settings.h"
#include "utils.h"

#include "Hooks/hooks_detours.h"
#include "Overlay/WindowManager.h"

#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>

HMODULE hOriginalDinput;

typedef HRESULT(WINAPI *DirectInput8Create_t)(HINSTANCE inst_handle, DWORD version, const IID& r_iid, LPVOID* out_wrapper, LPUNKNOWN p_unk);
DirectInput8Create_t orig_DirectInput8Create;

//For input bridge
STARTUPINFOA ib_si = { sizeof(STARTUPINFOA) };
PROCESS_INFORMATION ib_pi;

// Exported function
HRESULT WINAPI DirectInput8Create(HINSTANCE hinstHandle, DWORD version, const IID& r_iid, LPVOID* outWrapper, LPUNKNOWN pUnk)
{
	LOG(1, "DirectInput8Create\n");

	HRESULT ret = orig_DirectInput8Create(hinstHandle, version, r_iid, outWrapper, pUnk);

	LOG(1, "DirectInput8Create result: %d\n", ret);

	return ret;
}

void CreateCustomDirectories()
{
	LOG(1, "CreateCustomDirectories\n");

	CreateDirectory(L"ACPR_IM", NULL);
}



DWORD GetIBProcessID()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	std::wstring processName = L"ib.exe";

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return NULL;

	DWORD id = NULL;

	if (Process32First(snapshot, &entry)) {
		do {
			if (!_wcsicmp(entry.szExeFile, processName.c_str())) {
				id = entry.th32ProcessID;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return id;
}

void CloseInputBridge()
{
	if (!Settings::settingsIni.ibClose)
		return;

	DWORD id = GetIBProcessID();

	if (id != NULL)
	{
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, id);
		if (hProcess) {
			if (TerminateProcess(hProcess, 0)) {
				LOG(1, "Closed input bridge.\n");
			}
			else {
				LOG(1, "Failed to close input bridge.\n");
			}
			CloseHandle(hProcess);
		}
		else {
			LOG(1, "Couldn't open process ib.exe before terminating. Error: %d\n", GetLastError());
		}
	}
	
}

void ACPR_IM_Shutdown()
{
	LOG(1, "ACPR_IM_Shutdown\n");

	
	CloseInputBridge();
	WindowManager::GetInstance().Shutdown();
	CleanupInterfaces();
	closeLogger();
}

bool LoadOriginalDinputDll()
{
	if (Settings::settingsIni.dinputDllWrapper == "none" || Settings::settingsIni.dinputDllWrapper == "")
	{
		char dllPath[MAX_PATH];
		GetSystemDirectoryA(dllPath, MAX_PATH);
		strcat_s(dllPath, "\\dinput8.dll");
		hOriginalDinput = LoadLibraryA(dllPath);
	}
	else
	{
		LOG(2, "Loading dinput wrapper: %s\n", Settings::settingsIni.dinputDllWrapper.c_str());
		hOriginalDinput = LoadLibraryA(Settings::settingsIni.dinputDllWrapper.c_str());
	}

	if (hOriginalDinput == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	orig_DirectInput8Create = (DirectInput8Create_t)GetProcAddress(hOriginalDinput, "DirectInput8Create");

	if (!orig_DirectInput8Create)
	{
		return false;
	}

	LOG(1, "orig_DirectInput8Create: 0x%p\n", orig_DirectInput8Create);

	return true;
}

bool RunInputBridge()
{
	if (Settings::settingsIni.ibExec == "none" || Settings::settingsIni.ibExec == "")
	{
		return false;
	}
	else
	{
		DWORD id = GetIBProcessID();
		if (id != NULL) return true;

		ib_si.dwFlags = STARTF_USESHOWWINDOW;
		ib_si.wShowWindow = SW_MINIMIZE;
		// Start the child process.
		if (!CreateProcessA(
			NULL,             
			(LPSTR)Settings::settingsIni.ibExec.c_str(),
			NULL,             
			NULL,             
			FALSE,            
			CREATE_NO_WINDOW,                
			NULL,             
			NULL,             
			&ib_si,
			&ib_pi)
			)
		{
			LOG(1, "Could not initialize input bridge: Error %d.\n", GetLastError());
			return false;
		}
	}

	LOG(1, "Running input bridge\n");
	return true;
}

DWORD WINAPI ACPR_IM_Start(HMODULE hModule)
{
	openLogger();

	LOG(1, "Starting ACPR_IM_Start thread\n");

	//Check for game executable in working directory. This will fail when running game from a .ggr file
	//for the first time, but will succeed when launched from steam after confirming launch arguments
	if (!CheckGameExecutable())
	{
		return 1;
	}

	CreateCustomDirectories();
	SetUnhandledExceptionFilter(UnhandledExFilter);

	if (!Settings::loadSettingsFile())
	{
		ExitProcess(0);
	}
	logSettingsIni();
	Settings::initSavedSettings();

	if (!LoadOriginalDinputDll())
	{
		MessageBoxA(nullptr, "Could not load original dinput8.dll!", "ACPRIM", MB_OK);
		ExitProcess(0);
	}

	if (!placeHooks_detours())
	{
		MessageBoxA(nullptr, "Failed IAT hook", "ACPRIM", MB_OK);
		ExitProcess(0);
	}

	g_interfaces.pPaletteManager = new PaletteManager();

	RunInputBridge();

	return 0;
}

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ACPR_IM_Start, hinstDLL, 0, nullptr));
		break;

	case DLL_PROCESS_DETACH:
		ACPR_IM_Shutdown();
		FreeLibrary(hOriginalDinput);
		break;
	}
	return TRUE;
}