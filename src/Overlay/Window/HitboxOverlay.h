#pragma once
#include "IWindow.h"

#include "Game/CharData.h"
#include "Game/Hitbox/Hitbox.h"

#include "imgui_internal.h"

#include <imgui.h>
#include <d3dx9.h>

typedef unsigned int uint32_t;

class HitboxOverlay : public IWindow
{
public:
	bool drawOriginLine = false;
	bool drawCharacterHitbox[2] = {true, true};

	HitboxOverlay(const std::string& windowTitle, bool windowClosable,
		ImGuiWindowFlags windowFlags)
		: IWindow(windowTitle, windowClosable, windowFlags) {}
	void Update() override;
	float& GetScale();
	void DrawRectThicknessSlider();
	void DrawRectFillTransparencySlider();
	bool HasNullptrInData();

protected:
	void BeforeDraw() override;
	void Draw() override;
	void AfterDraw() override;

private:
	void DrawFrameMeter();
	void DrawFrameMeterLegend();
	void DrawOriginLine(ImVec2 worldPos, float rotationRad);
	void DrawCollisionAreas(const CharData* charObj, const ImVec2 playerWorldPos);
	void DrawPlayerPushboxes(const CharData* charObj);
	void DrawPlayerGrabBox(const CharData* charObj, bool drawOverride);
	bool DrawPlayerCommandGrabBox(const CharData* charObj);

	Hitbox GetPlayerPushBox(const CharData* charObj);
	Hitbox GetPlayerThrowBox(const CharData* charObj);

	bool IsOwnerEnabled(byte playerID);
	bool WorldToScreen(LPDIRECT3DDEVICE9 pDevice, D3DXMATRIX* view, D3DXMATRIX* proj, D3DXVECTOR3* pos, D3DXVECTOR3* out);
	ImVec2 WorldToPixel(ImVec2 worldPos);
	ImVec2 PixelToScreen(ImVec2 pixelPos);
	ImVec2 CalculateObjWorldPosition(const CharData* charObj);
	ImVec2 CalculateScreenPosition(ImVec2 worldPos);
	ImVec2 RotatePoint(ImVec2 center, float angleInRad, ImVec2 point);

	Hitbox ScaleHitbox(const Hitbox* hitbox, const CharData* p);
	ImRect MapHitboxToOrigin(const Hitbox* hitbox, bool facingRight, int xPos, int yPos);

	void RenderLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness = 1.0f);
	void RenderCircle(const ImVec2& position, float radius, uint32_t color, float thickness = 1.0f, uint32_t segments = 16);
	void RenderCircleFilled(const ImVec2& position, float radius, uint32_t color, uint32_t segments = 16);
	void RenderRect(const ImVec2& pointA, const ImVec2& pointB, const ImVec2& pointC, const ImVec2& pointD, uint32_t color, float thickness = 1.0f);
	void RenderRect(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding = 0.0f, uint32_t roundingCornersFlags = ImDrawCornerFlags_All, float thickness = 1.0f);
	void RenderRectFilled(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding = 0.0f, uint32_t roundingCornersFlags = ImDrawCornerFlags_All);
	void RenderRectFilled(const ImVec2& pointA, const ImVec2& pointB, const ImVec2& pointC, const ImVec2& pointD, uint32_t color);
	void RenderText(const ImVec2& pos, const char* text, uint32_t color, float fontsize);

	// Literally a magic number as of now
	float m_scale = 0.346f;
	float m_rectThickness = 2.5f;
	float m_rectFillTransparency = 0.5f;

	ImGuiWindowFlags m_overlayWindowFlags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoFocusOnAppearing;
};
