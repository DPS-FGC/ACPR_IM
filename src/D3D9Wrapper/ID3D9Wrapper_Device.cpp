#include "ID3D9Wrapper_Device.h"

#include "Core/interfaces.h"
#include "Core/logger.h"
#include "Game/MatchState.h"
#include "Hooks/hooks_acpr.h"
#include "Hooks/hooks_trainer.h"
#include "Hooks/hooks_cbr.h"
#include "Hooks/hooks_palette.h"
#include "Overlay/WindowManager.h"

#include <steam_api.h>

#pragma comment(lib, "steam_api.lib")

Direct3DDevice9Wrapper::Direct3DDevice9Wrapper(IDirect3DDevice9** ppReturnedDeviceInterface, D3DPRESENT_PARAMETERS* pPresentParam, IDirect3D9* pIDirect3D9)
{
	LOG(1, "cDirect3DDevice9Wrapper with modified PresentationParameters\n");

	m_Direct3DDevice9 = *ppReturnedDeviceInterface;
	*ppReturnedDeviceInterface = this;
	m_Direct3D9 = pIDirect3D9;

	g_interfaces.pD3D9Wrapper = *ppReturnedDeviceInterface;

	//place all other hooks that can only be placed after steamDRM unpacks the .exe in memory!!!
	placeHooks_acpr();
	placeHooks_trainer();
	placeHooks_palette();
	placeHooks_cbr();
	//placeHooks_CustomGameModes();

	g_interfaces.trainerInterface.initializeTrainer();
	MoveData::Initialize();
}

Direct3DDevice9Wrapper::~Direct3DDevice9Wrapper() {}

HRESULT APIENTRY Direct3DDevice9Wrapper::QueryInterface(const IID& riid, void** ppvObj)
{
	LOG(7, "QueryInterface\n");

	HRESULT hRes = m_Direct3DDevice9->QueryInterface(riid, ppvObj);

	if (hRes == S_OK)
		*ppvObj = this;
	else
		*ppvObj = NULL;

	return hRes;
}

ULONG APIENTRY Direct3DDevice9Wrapper::AddRef()
{
	LOG(7, "AddRef\n");
	return m_Direct3DDevice9->AddRef();
}

ULONG APIENTRY Direct3DDevice9Wrapper::Release()
{
	LOG(7, "Release\n");

	ULONG res = m_Direct3DDevice9->Release();

	if (res == 0)
		delete this;

	return res;
}

HRESULT APIENTRY Direct3DDevice9Wrapper::TestCooperativeLevel()
{
	LOG(7, "TestCooperativeLevel\n");
	return m_Direct3DDevice9->TestCooperativeLevel();
}

UINT APIENTRY Direct3DDevice9Wrapper::GetAvailableTextureMem()
{
	LOG(7, "GetAvailableTextureMem\n");
	return m_Direct3DDevice9->GetAvailableTextureMem();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::EvictManagedResources()
{
	LOG(7, "EvictManagedResources\n");
	return m_Direct3DDevice9->EvictManagedResources();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetDirect3D(IDirect3D9** ppD3D9)
{
	LOG(7, "GetDirect3D\n");

	HRESULT hRet = m_Direct3DDevice9->GetDirect3D(ppD3D9);

	if (SUCCEEDED(hRet))
		*ppD3D9 = m_Direct3D9;

	return hRet;
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetDeviceCaps(D3DCAPS9* pCaps)
{
	LOG(7, "GetDeviceCaps\n");
	return m_Direct3DDevice9->GetDeviceCaps(pCaps);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	LOG(7, "GetDisplayMode\n");
	return m_Direct3DDevice9->GetDisplayMode(iSwapChain, pMode);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	LOG(7, "GetCreationParameters\n");
	return m_Direct3DDevice9->GetCreationParameters(pParameters);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	LOG(7, "SetCursorProperties\n");
	return m_Direct3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void APIENTRY Direct3DDevice9Wrapper::SetCursorPosition(int X, int Y, DWORD Flags)
{
	LOG(7, "SetCursorPosition\n");
	return m_Direct3DDevice9->SetCursorPosition(X, Y, Flags);
}

BOOL APIENTRY Direct3DDevice9Wrapper::ShowCursor(BOOL bShow)
{
	LOG(7, "ShowCursor\n");
	return m_Direct3DDevice9->ShowCursor(bShow);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	LOG(7, "CreateAdditionalSwapChain\n");
	return m_Direct3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	LOG(7, "GetSwapChain\n");
	return m_Direct3DDevice9->GetSwapChain(iSwapChain, pSwapChain);
}

UINT APIENTRY Direct3DDevice9Wrapper::GetNumberOfSwapChains()
{
	LOG(7, "GetNumberOfSwapChains\n");
	return m_Direct3DDevice9->GetNumberOfSwapChains();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	LOG(3, "Reset\n");
	logD3DPParams(pPresentationParameters, true);
	Settings::applySettingsIni(pPresentationParameters);
	logD3DPParams(pPresentationParameters, false);

	WindowManager::GetInstance().InvalidateDeviceObjects();

	HRESULT ret = m_Direct3DDevice9->Reset(pPresentationParameters);

	WindowManager::GetInstance().CreateDeviceObjects();

	return ret;
}

HRESULT APIENTRY Direct3DDevice9Wrapper::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	LOG(7, "Present\n");
	return m_Direct3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	LOG(7, "GetBackBuffer\n");
	return m_Direct3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	LOG(7, "GetRasterStatus\n");
	return m_Direct3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetDialogBoxMode(BOOL bEnableDialogs)
{
	LOG(7, "SetDialogBoxMode\n");
	return m_Direct3DDevice9->SetDialogBoxMode(bEnableDialogs);
}

void APIENTRY Direct3DDevice9Wrapper::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	LOG(7, "SetGammaRamp\n");
	return m_Direct3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void APIENTRY Direct3DDevice9Wrapper::GetGammaRamp(UINT iSwapChaiTn, D3DGAMMARAMP* pRamp)
{
	LOG(7, "GetGammaRamp\n");
	return m_Direct3DDevice9->GetGammaRamp(iSwapChaiTn, pRamp);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	LOG(7, "CreateTexture %u %u %u\n", Width, Height, Levels);
	return m_Direct3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	LOG(7, "CreateVolumeTexture\n");
	return m_Direct3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	LOG(7, "CreateCubeTexture\n");
	return m_Direct3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	LOG(7, "CreateVertexBuffer\n");
	return m_Direct3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	LOG(7, "CreateIndexBuffer\n");
	return m_Direct3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG(3, "CreateRenderTarget\n");
	LOG(3, "\t- Width: %u\n", Width);
	LOG(3, "\t- Height: %u\n", Height);
	LOG(3, "\t- Multisample: %d\n", MultiSample);
	LOG(3, "\t- ppSurface: 0x%p\n", ppSurface);

	return m_Direct3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG(3, "CreateDepthStencilSurface\n");
	LOG(3, "\t- Width: %u\n", Width);
	LOG(3, "\t- Height: %u\n", Height);
	LOG(3, "\t- Multisample: %d\n", MultiSample);
	LOG(3, "\t- ppSurface: 0x%p\n", ppSurface);

	HRESULT ret = m_Direct3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

	LOG(2, "\t- HRESULT: %ld\n", ret);

	return ret;
}

HRESULT APIENTRY Direct3DDevice9Wrapper::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	LOG(7, "UpdateSurface\n");
	return m_Direct3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	LOG(7, "UpdateTexture\n");
	return m_Direct3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	LOG(7, "GetRenderTargetData\n");
	return m_Direct3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	LOG(7, "GetFrontBufferData\n");
	return m_Direct3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	LOG(7, "StretchRect\n");

	if (pSourceRect)
	{
		LOG(7, "StretchRect 0x%p : %ld %ld -- 0x%p\n", pSourceRect, pSourceRect->right, pSourceRect->bottom, pDestRect);
	}

	return m_Direct3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	LOG(7, "ColorFill\n");
	return m_Direct3DDevice9->ColorFill(pSurface, pRect, color);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	LOG(7, "CreateOffscreenPlainSurface\n");
	return m_Direct3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	LOG(7, "SetRenderTarget %d 0x%p\n", RenderTargetIndex, pRenderTarget);

	return m_Direct3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	LOG(7, "GetRenderTarget\n");
	return m_Direct3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	LOG(7, "SetDepthStencilSurface\n");
	return m_Direct3DDevice9->SetDepthStencilSurface(pNewZStencil);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	LOG(7, "GetDepthStencilSurface\n");
	return m_Direct3DDevice9->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::BeginScene()
{
	LOG(7, "BeginScene\n");
	return m_Direct3DDevice9->BeginScene();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::EndScene()
{
	LOG(7, "EndScene\n");

	MatchState::OnUpdate();
	WindowManager::GetInstance().Render();

	return m_Direct3DDevice9->EndScene();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	LOG(7, "Clear\n");
	return m_Direct3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	LOG(7, "SetTransform %ld &pMatrix: 0x%p &*pMatrix: 0x%p pMatrix: 0x%p\n", State, &pMatrix, &*pMatrix, pMatrix);
	return m_Direct3DDevice9->SetTransform(State, pMatrix);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	LOG(7, "GetTransform\n");
	return m_Direct3DDevice9->GetTransform(State, pMatrix);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	LOG(7, "MultiplyTransform\n");
	return m_Direct3DDevice9->MultiplyTransform(State, pMatrix);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	LOG(7, "SetViewport 0x%p : %d %d\n", pViewport, pViewport->Width, pViewport->Height);

	return m_Direct3DDevice9->SetViewport(pViewport);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetViewport(D3DVIEWPORT9* pViewport)
{
	HRESULT ret = m_Direct3DDevice9->GetViewport(pViewport);
	LOG(7, "GetViewport %ld %ld %ld %ld 0x%p\n", pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height, pViewport);

	return ret;
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	LOG(7, "SetMaterial\n");
	return m_Direct3DDevice9->SetMaterial(pMaterial);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetMaterial(D3DMATERIAL9* pMaterial)
{
	LOG(7, "GetMaterial\n");
	return m_Direct3DDevice9->GetMaterial(pMaterial);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	LOG(7, "SetLight\n");
	return m_Direct3DDevice9->SetLight(Index, pLight);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	LOG(7, "GetLight\n");
	return m_Direct3DDevice9->GetLight(Index, pLight);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::LightEnable(DWORD Index, BOOL Enable)
{
	LOG(7, "LightEnable\n");
	return m_Direct3DDevice9->LightEnable(Index, Enable);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	LOG(7, "GetLightEnable\n");
	return m_Direct3DDevice9->GetLightEnable(Index, pEnable);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	LOG(7, "SetClipPlane\n");
	return m_Direct3DDevice9->SetClipPlane(Index, pPlane);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetClipPlane(DWORD Index, float* pPlane)
{
	LOG(7, "GetClipPlane\n");
	return m_Direct3DDevice9->GetClipPlane(Index, pPlane);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	LOG(7, "SetRenderState\n");
	return m_Direct3DDevice9->SetRenderState(State, Value);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	LOG(7, "GetRenderState\n");
	return m_Direct3DDevice9->GetRenderState(State, pValue);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	LOG(7, "CreateStateBlock\n");
	return m_Direct3DDevice9->CreateStateBlock(Type, ppSB);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::BeginStateBlock()
{
	LOG(7, "BeginStateBlock\n");
	return m_Direct3DDevice9->BeginStateBlock();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	LOG(7, "EndStateBlock\n");
	return m_Direct3DDevice9->EndStateBlock(ppSB);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	LOG(7, "SetClipStatus\n");
	return m_Direct3DDevice9->SetClipStatus(pClipStatus);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	LOG(7, "GetClipStatus\n");
	return m_Direct3DDevice9->GetClipStatus(pClipStatus);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	LOG(7, "GetTexture\n");
	return m_Direct3DDevice9->GetTexture(Stage, ppTexture);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	LOG(7, "SetTexture 0x%p\n", pTexture);
	return m_Direct3DDevice9->SetTexture(Stage, pTexture);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	LOG(7, "GetTextureStageState\n");
	return m_Direct3DDevice9->GetTextureStageState(Stage, Type, pValue);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	LOG(7, "SetTextureStageState\n");
	return m_Direct3DDevice9->SetTextureStageState(Stage, Type, Value);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	LOG(7, "GetSamplerState\n");
	return m_Direct3DDevice9->GetSamplerState(Sampler, Type, pValue);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	LOG(7, "SetSamplerState\n");

	return m_Direct3DDevice9->SetSamplerState(Sampler, Type, Value);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::ValidateDevice(DWORD* pNumPasses)
{
	LOG(7, "ValidateDevice\n");
	return m_Direct3DDevice9->ValidateDevice(pNumPasses);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	LOG(7, "SetPaletteEntries\n");
	return m_Direct3DDevice9->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	LOG(7, "GetPaletteEntries\n");
	return m_Direct3DDevice9->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetCurrentTexturePalette(UINT PaletteNumber)
{
	LOG(7, "SetCurrentTexturePalette\n");
	return m_Direct3DDevice9->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetCurrentTexturePalette(UINT* PaletteNumber)
{
	LOG(7, "GetCurrentTexturePalette\n");
	return m_Direct3DDevice9->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetScissorRect(CONST RECT* pRect)
{
	LOG(7, "SetScissorRect\n");
	return m_Direct3DDevice9->SetScissorRect(pRect);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetScissorRect(RECT* pRect)
{
	LOG(7, "GetScissorRect\n");
	return m_Direct3DDevice9->GetScissorRect(pRect);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	LOG(7, "SetSoftwareVertexProcessing\n");
	return m_Direct3DDevice9->SetSoftwareVertexProcessing(bSoftware);
}

BOOL APIENTRY Direct3DDevice9Wrapper::GetSoftwareVertexProcessing()
{
	LOG(7, "GetSoftwareVertexProcessing\n");
	return m_Direct3DDevice9->GetSoftwareVertexProcessing();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetNPatchMode(float nSegments)
{
	LOG(7, "SetNPatchMode\n");
	return m_Direct3DDevice9->SetNPatchMode(nSegments);
}

float APIENTRY Direct3DDevice9Wrapper::GetNPatchMode()
{
	LOG(7, "GetNPatchMode\n");
	return m_Direct3DDevice9->GetNPatchMode();
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	LOG(7, "DrawPrimitive\n");
	return m_Direct3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	LOG(7, "DrawIndexedPrimitive\n");
	Settings::savedSettings.isFiltering = true;
	return m_Direct3DDevice9->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	LOG(7, "DrawPrimitiveUP\n");
	return m_Direct3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	LOG(7, "DrawIndexedPrimitiveUP\n");
	return m_Direct3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	LOG(7, "ProcessVertices\n");
	return m_Direct3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	LOG(7, "CreateVertexDeclaration\n");
	return m_Direct3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	LOG(7, "SetVertexDeclaration\n");
	return m_Direct3DDevice9->SetVertexDeclaration(pDecl);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	LOG(7, "GetVertexDeclaration\n");
	return m_Direct3DDevice9->GetVertexDeclaration(ppDecl);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetFVF(DWORD FVF)
{
	LOG(7, "SetFVF\n");
	return m_Direct3DDevice9->SetFVF(FVF);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetFVF(DWORD* pFVF)
{
	LOG(7, "GetFVF\n");
	return m_Direct3DDevice9->GetFVF(pFVF);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	LOG(7, "CreateVertexShader\n");
	return m_Direct3DDevice9->CreateVertexShader(pFunction, ppShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	LOG(7, "SetVertexShader: 0x%p\n", pShader);
	return m_Direct3DDevice9->SetVertexShader(pShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	LOG(7, "GetVertexShader\n");
	return m_Direct3DDevice9->GetVertexShader(ppShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	LOG(7, "SetVertexShaderConstantF 0x%p %u\n", pConstantData, Vector4fCount);
	//for (int i = 0; i < Vector4fCount; i++)
	//{
	//	LOG(7, "%.2f ", pConstantData[i]);
	//}
	//LOG(7, "\n");
	return m_Direct3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	LOG(7, "GetVertexShaderConstantF\n");
	return m_Direct3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	LOG(7, "SetVertexShaderConstantI\n");
	return m_Direct3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	LOG(7, "GetVertexShaderConstantI\n");
	return m_Direct3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	LOG(7, "SetVertexShaderConstantB\n");
	return m_Direct3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	LOG(7, "GetVertexShaderConstantB\n");
	return m_Direct3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	LOG(7, "SetStreamSource\n");
	if (StreamNumber == 0)
		m_Stride = Stride;

	return m_Direct3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride)
{
	LOG(7, "GetStreamSource\n");
	return m_Direct3DDevice9->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetStreamSourceFreq(UINT StreamNumber, UINT Setting)
{
	LOG(7, "SetStreamSourceFreq\n");
	return m_Direct3DDevice9->SetStreamSourceFreq(StreamNumber, Setting);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting)
{
	LOG(7, "GetStreamSourceFreq\n");
	return m_Direct3DDevice9->GetStreamSourceFreq(StreamNumber, pSetting);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	LOG(7, "SetIndices\n");
	return m_Direct3DDevice9->SetIndices(pIndexData);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	LOG(7, "GetIndices\n");
	return m_Direct3DDevice9->GetIndices(ppIndexData);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	LOG(7, "CreatePixelShader\n");
	return m_Direct3DDevice9->CreatePixelShader(pFunction, ppShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	LOG(7, "SetPixelShader: 0x%p\n", pShader);
	return m_Direct3DDevice9->SetPixelShader(pShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	LOG(7, "GetPixelShader\n");
	return m_Direct3DDevice9->GetPixelShader(ppShader);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	LOG(7, "SetPixelShaderConstantF\n");
	return m_Direct3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	LOG(7, "GetPixelShaderConstantF\n");
	return m_Direct3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	LOG(7, "SetPixelShaderConstantI\n");
	return m_Direct3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	LOG(7, "GetPixelShaderConstantI\n");
	return m_Direct3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	LOG(7, "SetPixelShaderConstantB\n");
	return m_Direct3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	LOG(7, "GetPixelShaderConstantB\n");
	return m_Direct3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	LOG(7, "DrawRectPatch\n");
	return m_Direct3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	LOG(7, "DrawTriPatch\n");
	return m_Direct3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::DeletePatch(UINT Handle)
{
	LOG(7, "DeletePatch\n");
	return m_Direct3DDevice9->DeletePatch(Handle);
}

HRESULT APIENTRY Direct3DDevice9Wrapper::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	LOG(7, "CreateQuery\n");
	return m_Direct3DDevice9->CreateQuery(Type, ppQuery);
}