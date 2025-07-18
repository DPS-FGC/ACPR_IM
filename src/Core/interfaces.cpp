#include "interfaces.h"

#include "logger.h"
#include "utils.h"

interfaces_t g_interfaces = {};
gameProc_t g_gameProc = {};
temps_t g_tempVals = {};
gameVals_t g_gameVals = {};

void InitManagers()
{
	LOG(1, "InitManagers\n");

	if (g_interfaces.pSteamNetworkingWrapper &&
		!g_interfaces.pNetworkManager)
	{
		g_interfaces.pNetworkManager = new NetworkManager(
			g_interfaces.pSteamNetworkingWrapper,
			CSteamID(*g_gameVals.pSteamID)
		);
	}

	if (g_interfaces.pNetworkManager &&
		!g_interfaces.pRoomManager)
	{
		g_interfaces.pRoomManager = new RoomManager(
			g_interfaces.pNetworkManager,
			CSteamID(*g_gameVals.pSteamID)
		);
	}

	if (g_interfaces.pPaletteManager &&
		g_interfaces.pRoomManager &&
		!g_interfaces.pOnlinePaletteManager)
	{
		g_interfaces.pOnlinePaletteManager = new OnlinePaletteManager(
			g_interfaces.pPaletteManager,
			&g_interfaces.Player1.GetPalHandle(),
			&g_interfaces.Player2.GetPalHandle(),
			g_interfaces.pRoomManager
		);
	}
}

void CleanupInterfaces()
{
	LOG(1, "CleanupInterfaces\n");

	SAFE_DELETE(g_interfaces.pNetworkManager);
	SAFE_DELETE(g_interfaces.pPaletteManager);
	SAFE_DELETE(g_interfaces.pRoomManager);
	SAFE_DELETE(g_interfaces.pOnlinePaletteManager);

	SAFE_DELETE(g_interfaces.pD3D9ExWrapper);
	SAFE_DELETE(g_interfaces.pD3D9Wrapper);

	SAFE_DELETE(g_interfaces.pSteamNetworkingWrapper);
	SAFE_DELETE(g_interfaces.pSteamUserStatsWrapper);
	SAFE_DELETE(g_interfaces.pSteamApiHelper);
}
