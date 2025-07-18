#include "steamApiWrappers.h"

#include "Core/interfaces.h"
#include "Core/logger.h"

bool InitSteamApiWrappers()
{
	LOG(1, "InitSteamApiWrappers\n");

	if (g_tempVals.ppSteamNetworking &&
		!g_interfaces.pSteamNetworkingWrapper)
	{
		g_interfaces.pSteamNetworkingWrapper = new SteamNetworkingWrapper(g_tempVals.ppSteamNetworking);
	}

	if (g_tempVals.ppSteamUserStats &&
		!g_interfaces.pSteamUserStatsWrapper)
	{
		g_interfaces.pSteamUserStatsWrapper = new SteamUserStatsWrapper(g_tempVals.ppSteamUserStats);
	}

	if (g_interfaces.pSteamUserStatsWrapper &&
		!g_interfaces.pSteamApiHelper)
	{
		g_interfaces.pSteamApiHelper = new SteamApiHelper(g_interfaces.pSteamUserStatsWrapper);
	}

	return true;
}