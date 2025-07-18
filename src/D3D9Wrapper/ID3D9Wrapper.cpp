#include "ID3D9Wrapper.h"

#include "ID3D9Wrapper_Device.h"

#include "Core/logger.h"

#include "Core/interfaces.h"

Direct3D9Wrapper::Direct3D9Wrapper(IDirect3D9** ppIDirect3D9)
{
	LOG(1, "cDirect3D9Wrapper\n");

	Direct3D9 = *ppIDirect3D9;
	*ppIDirect3D9 = this;
}

Direct3D9Wrapper::~Direct3D9Wrapper() {}

HRESULT APIENTRY Direct3D9Wrapper::RegisterSoftwareDevice(void* pInitializeFunction)
{
	return Direct3D9->RegisterSoftwareDevice(pInitializeFunction);
}

HRESULT APIENTRY Direct3D9Wrapper::QueryInterface(const IID& riid, void** ppvObj)
{
	HRESULT hRes = Direct3D9->QueryInterface(riid, ppvObj);

	if (hRes == S_OK)
		*ppvObj = this;
	else
		*ppvObj = NULL;

	return hRes;
}

ULONG APIENTRY Direct3D9Wrapper::AddRef()
{
	return Direct3D9->AddRef();
}

ULONG APIENTRY Direct3D9Wrapper::Release()
{
	ULONG res = Direct3D9->Release();

	if (res == 0)
		delete this;

	return res;
}

UINT APIENTRY Direct3D9Wrapper::GetAdapterCount()
{
	return Direct3D9->GetAdapterCount();
}

HRESULT APIENTRY Direct3D9Wrapper::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return Direct3D9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT APIENTRY Direct3D9Wrapper::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	return Direct3D9->GetAdapterModeCount(Adapter, Format);
}

HRESULT APIENTRY Direct3D9Wrapper::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	//LOG(1, "EnumAdapterModes\n");
	HRESULT ret = Direct3D9->EnumAdapterModes(Adapter, Format, Mode, pMode);
	//LOG(1, "\t- Adapter: %u\n", Adapter);
	//LOG(1, "\t- Format: %d\n", Format);
	//LOG(1, "\t- Mode: %u\n", Mode);
	//LOG(1, "\t- ReturnFormat: %d\n", pMode->Format);
	//LOG(1, "\t- ReturnWidth: %u\n", pMode->Width);
	//LOG(1, "\t- ReturnHeight: %d\n", pMode->Height);
	//LOG(1, "\t- ReturnRefreshRate: %u\n", pMode->RefreshRate);
	//Settings::savedSettings.adapterRefreshRate = pMode->RefreshRate;
	return ret;
}

HRESULT APIENTRY Direct3D9Wrapper::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	return Direct3D9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT APIENTRY Direct3D9Wrapper::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return Direct3D9->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT APIENTRY Direct3D9Wrapper::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return Direct3D9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT APIENTRY Direct3D9Wrapper::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return Direct3D9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT APIENTRY Direct3D9Wrapper::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return Direct3D9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT APIENTRY Direct3D9Wrapper::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return Direct3D9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT APIENTRY Direct3D9Wrapper::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
	return Direct3D9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR APIENTRY Direct3D9Wrapper::GetAdapterMonitor(UINT Adapter)
{
	return Direct3D9->GetAdapterMonitor(Adapter);
}

HRESULT APIENTRY Direct3D9Wrapper::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	LOG(1, "CreateDevice\n")
	LOG(1, "\tAdapter: %d\n\tDeviceType: %d\n\thFocusWindow: 0x%p\n\tBehaviorFlags: 0x%p\n",
		Adapter, DeviceType, hFocusWindow, BehaviorFlags);
	g_gameProc.hWndGameWindow = hFocusWindow;
	logD3DPParams(pPresentationParameters, true);

	HRESULT hRet = Direct3D9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (SUCCEEDED(hRet))
	{
		LOG(1, "CreateDevice created with original PresentationParameters\n");
		Settings::applySettingsIni(pPresentationParameters);
		logD3DPParams(pPresentationParameters, false);
		Direct3DDevice9Wrapper* ret = new Direct3DDevice9Wrapper(ppReturnedDeviceInterface, pPresentationParameters, this);
	}
	else
	{
		LOG(1, "CreateDevice failed to be created with original PresentationParameters\n");
	}

	return hRet;
}