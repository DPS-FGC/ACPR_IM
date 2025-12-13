#include "HitboxOverlay.h"

#include "Core/interfaces.h"
#include "Game/gamestates.h"
#include "Game/Hitbox/HitBoxReader.h"
#include <math.h>

#define GGXXACPR_SCREEN_HEIGHT_PIXELS 480
#define GGXXACPR_SCREEN_GROUND_PIXEL_OFFSET 40

void HitboxOverlay::Update()
{
	if (HasNullptrInData() || !m_windowOpen)
	{
		return;
	}

	if (!isHitboxOverlayEnabledInCurrentState())
	{
		return;
	}

	BeforeDraw();

	ImGui::Begin("##HitboxOverlay", nullptr, m_overlayWindowFlags);

	Draw();
	if (g_interfaces.frameMeterInterface.settings.DisplayHSDComboMeters)
	{
		DrawComboTimeMeter(g_interfaces.Player1.GetData());
		DrawUntechTimeMeter(g_interfaces.Player1.GetData());
		DrawComboTimeMeter(g_interfaces.Player2.GetData());
		DrawUntechTimeMeter(g_interfaces.Player2.GetData());
	}
	if (g_interfaces.frameMeterInterface.settings.DisplayFrameMeter)
	{
		DrawFrameMeter();
		if (g_interfaces.frameMeterInterface.settings.DisplayFrameMeterLegend)
			DrawFrameMeterLegend();
	}

	ImGui::End();

	AfterDraw();
}

void HitboxOverlay::BeforeDraw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
}

void HitboxOverlay::Draw()
{
	ImVec2 entityWorldPos;

	if (drawCharacterHitbox[0])
	{
		entityWorldPos = CalculateObjWorldPosition(g_interfaces.Player1.GetData());
		DrawCollisionAreas(g_interfaces.Player1.GetData(), entityWorldPos);
		DrawPlayerPushboxes(g_interfaces.Player1.GetData());
		DrawPlayerGrabBox(g_interfaces.Player1.GetData(), g_interfaces.frameMeterInterface.settings.AlwaysDrawThrowRange);
		DrawCleanHitBox(g_interfaces.Player1.GetData());
		DrawProximityBoxes(g_interfaces.Player1.GetData(), true);
	}

	if (drawCharacterHitbox[1])
	{
		entityWorldPos = CalculateObjWorldPosition(g_interfaces.Player2.GetData());
		DrawCollisionAreas(g_interfaces.Player2.GetData(), entityWorldPos);
		DrawPlayerPushboxes(g_interfaces.Player2.GetData());
		DrawPlayerGrabBox(g_interfaces.Player2.GetData(), g_interfaces.frameMeterInterface.settings.AlwaysDrawThrowRange);
		DrawCleanHitBox(g_interfaces.Player2.GetData());
		DrawProximityBoxes(g_interfaces.Player2.GetData(), true);
	}

	if (*g_gameVals.entityCount > 0)
	{
		CharData* pEntity = reinterpret_cast<CharData*>(*g_gameVals.pEntityStartAddress);
		bool isEntityActive = pEntity->charIndex > 0;
		while (isEntityActive)
		{
			if (IsOwnerEnabled(pEntity->playerID))
			{
				entityWorldPos = CalculateObjWorldPosition(pEntity);
				DrawCollisionAreas(pEntity, entityWorldPos);
				DrawProximityBoxes(pEntity, false);
			}
			pEntity = pEntity->nextEntity;
			isEntityActive = pEntity->charIndex > 0;
		}
	}
}

void HitboxOverlay::AfterDraw()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

#define FRAME_METER_Y_ALT  90
#define FRAME_METER_BASE_LINE_X  5
#define FRAME_METER_BORDER_THICKNESS 3
#define FRAME_METER_LENGTH 80
#define FRAME_METER_Y 350
#define FRAME_METER_VERTICAL_SPACING 1

#define LEGEND_Y_POS_PX 120
#define LEGEND_ASSUMED_FRAME_METER_SIZE 80
#define LEGEND_MAX_COLUMN_SIZE 7
#define FONT_SIZE 24
#define LEGEND_FONT_SIZE 16
#define LABEL_BG 0x96000000

#define CLEAN_HIT_Y_OFFSET 170

char frameMeterLabel[128];
void HitboxOverlay::DrawFrameMeter()
{
	RECT rect;
	GetClientRect(g_gameProc.hWndGameWindow, &rect);

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	int screenHeight = windowHeight;
	int screenWidth = windowHeight * 4 / 3;

	int pipSpacing = (screenWidth - FRAME_METER_BASE_LINE_X * 2) / FRAME_METER_LENGTH;
	int pipWidth = pipSpacing - 1;
	int totalWidth = (pipSpacing * FRAME_METER_LENGTH) - 1;
	int pipHeight = pipWidth * 5 / 3;
	int entityPipHeight = pipWidth;
	int coreYPos = windowHeight * FRAME_METER_Y / GGXXACPR_SCREEN_HEIGHT_PIXELS;
	int yPos = coreYPos - pipHeight - 1;
	int xPos = (windowWidth - totalWidth) / 2;
	int propertyHighlightHeight = pipHeight * 2 / 7;
	propertyHighlightHeight += propertyHighlightHeight == 0 ? 1 : 0;
	int propertyHighlightTop = pipHeight - propertyHighlightHeight;
	int borderThickness = 2 * windowHeight / GGXXACPR_SCREEN_HEIGHT_PIXELS;

	ImVec2 pointA(xPos - borderThickness, yPos - borderThickness);
	ImVec2 pointB(xPos + borderThickness + totalWidth, yPos - borderThickness);
	ImVec2 pointC(xPos + borderThickness + totalWidth, yPos + borderThickness + 2 * pipHeight + FRAME_METER_VERTICAL_SPACING);
	ImVec2 pointD(xPos - borderThickness, yPos + borderThickness + 2 * pipHeight + FRAME_METER_VERTICAL_SPACING);

	const unsigned int rectBorderColor = 0xFF000000;
	RenderRect(pointA, pointB, pointC, pointD, rectBorderColor, borderThickness);

	const unsigned char transparency = 0xFF * m_rectFillTransparency;
	unsigned int clearedTransparencyBits = (rectBorderColor & ~0xFF000000);
	unsigned int transparencyPercentage = ((int)transparency << 24) & 0xFF000000;
	const unsigned int rectFillColor = clearedTransparencyBits | transparencyPercentage;
	RenderRectFilled(pointA, pointB, pointC, pointD, rectFillColor);

	std::array<FM_Frame, FRAME_METER_LENGTH>* frameArr;

	// Players
	for (int j = 0; j < 2; j++)
	{
		frameArr = &g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[j].FrameArr;
		for (int i = 0; i < FRAME_METER_LENGTH; i++)
		{
			pointA = ImVec2(xPos + (i * pipSpacing), yPos + (j * (FRAME_METER_VERTICAL_SPACING + pipHeight)));
			pointB = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos + (j * (FRAME_METER_VERTICAL_SPACING + pipHeight)));
			pointC = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos + pipHeight + (j * (FRAME_METER_VERTICAL_SPACING + pipHeight)));
			pointD = ImVec2(xPos + (i * pipSpacing), yPos + pipHeight + (j * (FRAME_METER_VERTICAL_SPACING + pipHeight)));
			
			ImVec2 pointA2 = pointA;
			ImVec2 pointB2 = pointB;
			ImVec2 pointC2 = pointC;
			ImVec2 pointD2 = pointD;

			RenderRectFilled(pointA, pointB, pointC, pointD,
				g_interfaces.frameMeterInterface.GetFrameColor((*frameArr)[i].Type));
			
			if ((*frameArr)[i].SecondaryProperty != SecondaryFrameProperty_Default)
			{
				pointC2.y = pointA.y + propertyHighlightHeight;
				pointD2.y = pointA.y + propertyHighlightHeight;
				
				RenderRectFilled(pointA2, pointB2, pointC2, pointD2,
					g_interfaces.frameMeterInterface.GetSecondaryPropertyColor((*frameArr)[i].SecondaryProperty));

				RenderLine(pointD2, pointC2, 0xFF000000, 1.0f);
			}
			if ((*frameArr)[i].PrimaryProperty1 != PrimaryFrameProperty_Default)
			{
				pointA2.y = pointA.y + propertyHighlightHeight;
				pointB2.y = pointB.y + propertyHighlightHeight;
				pointC2 = pointC;
				pointD2 = pointD;

				RenderRectFilled(pointA2, pointB2, pointC2, pointD2,
					g_interfaces.frameMeterInterface.GetPrimaryPropertyColor((*frameArr)[i].PrimaryProperty1));

				RenderLine(pointA2, pointB2, 0xFF000000, 1.0f);
				
				if ((*frameArr)[i].PrimaryProperty2 != PrimaryFrameProperty_Default)
				{
					pointA2 = pointA;
					pointB2 = pointB;

					pointA2.x = pointA.x + pipWidth / 2;
					pointD2.x = pointA.x + pipWidth / 2;
					pointA2.y = pointA.y + propertyHighlightTop;
					pointB2.y = pointA.y + propertyHighlightTop;

					RenderRectFilled(pointA2, pointB2, pointC2, pointD2,
						g_interfaces.frameMeterInterface.GetPrimaryPropertyColor((*frameArr)[i].PrimaryProperty2));
				}
			}
			if ((*frameArr)[i].RunSum > 0)
			{
				//dim.xPos + i * dim.pipSpacing, dim.yPos + j * (dim.pipSpacingVertical + dim.pipHeight)
				ImVec2 labelpos = ImVec2(xPos + i * pipSpacing, yPos + j * (FRAME_METER_VERTICAL_SPACING + pipHeight));
				if ((*frameArr)[i].RunSum > 9 && i > 0)
					labelpos = ImVec2(xPos + (i - 1) * pipSpacing, yPos + j * (FRAME_METER_VERTICAL_SPACING + pipHeight));

				sprintf(frameMeterLabel, "%d", (*frameArr)[i].RunSum);
				RenderText(labelpos, frameMeterLabel, 0xFFFFFFFF, FONT_SIZE);
			}
		}
	}

	// Entity
	if (!g_interfaces.frameMeterInterface.frameMeter.EntityMeters[0].Hide)
	{
		frameArr = &g_interfaces.frameMeterInterface.frameMeter.EntityMeters[0].FrameArr;
		pointA = ImVec2(xPos - borderThickness, yPos - borderThickness - FRAME_METER_VERTICAL_SPACING - entityPipHeight);
		pointB = ImVec2(xPos + totalWidth + borderThickness, yPos - borderThickness - FRAME_METER_VERTICAL_SPACING - entityPipHeight);
		pointC = ImVec2(xPos + totalWidth + borderThickness, yPos - FRAME_METER_VERTICAL_SPACING);
		pointD = ImVec2(xPos - borderThickness, yPos - FRAME_METER_VERTICAL_SPACING);

		RenderRectFilled(pointA, pointB, pointC, pointD, 0xFF000000);

		for (int i = 0; i < FRAME_METER_LENGTH; i++)
		{
			pointA = ImVec2(xPos + (i * pipSpacing), yPos - FRAME_METER_VERTICAL_SPACING - entityPipHeight);
			pointB = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos - FRAME_METER_VERTICAL_SPACING - entityPipHeight);
			pointC = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos - FRAME_METER_VERTICAL_SPACING);
			pointD = ImVec2(xPos + (i * pipSpacing), yPos - FRAME_METER_VERTICAL_SPACING);

			RenderRectFilled(pointA, pointB, pointC, pointD,
				g_interfaces.frameMeterInterface.GetFrameColor((*frameArr)[i].Type));
		}
	}
	if (!g_interfaces.frameMeterInterface.frameMeter.EntityMeters[1].Hide)
	{
		frameArr = &g_interfaces.frameMeterInterface.frameMeter.EntityMeters[1].FrameArr;

		pointA = ImVec2(xPos - borderThickness, yPos + 2 * (pipHeight + FRAME_METER_VERTICAL_SPACING));
		pointB = ImVec2(xPos + totalWidth + borderThickness, yPos + 2 * (pipHeight + FRAME_METER_VERTICAL_SPACING));
		pointC = ImVec2(xPos + totalWidth + borderThickness, yPos + borderThickness + entityPipHeight + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));
		pointD = ImVec2(xPos - borderThickness, yPos + borderThickness + entityPipHeight + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));
		
		RenderRectFilled(pointA, pointB, pointC, pointD, 0xFF000000);
		
		for (int i = 0; i < FRAME_METER_LENGTH; i++)
		{
			pointA = ImVec2(xPos + (i * pipSpacing), yPos + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));
			pointB = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));
			pointC = ImVec2(xPos + pipWidth + (i * pipSpacing), yPos + entityPipHeight + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));
			pointD = ImVec2(xPos + (i * pipSpacing), yPos + entityPipHeight + 2 * (FRAME_METER_VERTICAL_SPACING + pipHeight));

			RenderRectFilled(pointA, pointB, pointC, pointD,
				g_interfaces.frameMeterInterface.GetFrameColor((*frameArr)[i].Type));
		}
	}

	// Labels
	ImVec2 p1LabelPosition = ImVec2(xPos, yPos - entityPipHeight - borderThickness - FONT_SIZE);
	ImVec2 p2LabelPosition = ImVec2(xPos, coreYPos + pipHeight + entityPipHeight + borderThickness * 2);
	float advantageXOffset = ImGui::CalcTextSize("Startup: -99   ").x + 2 * FONT_SIZE;

	
	// Semi-transparent label backgrounds
	ImVec2 startupDimensions = ImGui::CalcTextSize("Startup: -99   Advantage: -99  ");
	pointA = ImVec2(p1LabelPosition.x - FONT_SIZE / 2, p1LabelPosition.y);
	pointB = ImVec2(p1LabelPosition.x + 2 * advantageXOffset + 2 * FONT_SIZE, p1LabelPosition.y);
	pointC = ImVec2(p1LabelPosition.x + 2 * advantageXOffset + 2 * FONT_SIZE, p1LabelPosition.y + startupDimensions.y + FONT_SIZE / 2);
	pointD = ImVec2(p1LabelPosition.x - FONT_SIZE / 2, p1LabelPosition.y + startupDimensions.y + FONT_SIZE / 2);

	RenderRectFilled(pointA, pointB, pointC, pointD, LABEL_BG);

	pointA = ImVec2(p2LabelPosition.x - FONT_SIZE / 2, p2LabelPosition.y);
	pointB = ImVec2(p2LabelPosition.x + 2 * advantageXOffset + 2 * FONT_SIZE, p2LabelPosition.y);
	pointC = ImVec2(p2LabelPosition.x + 2 * advantageXOffset + 2 * FONT_SIZE, p2LabelPosition.y + startupDimensions.y + FONT_SIZE / 2);
	pointD = ImVec2(p2LabelPosition.x - FONT_SIZE / 2, p2LabelPosition.y + startupDimensions.y + FONT_SIZE / 2);

	RenderRectFilled(pointA, pointB, pointC, pointD, LABEL_BG);

	// Startup
	if (g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].Startup >= 0)
		sprintf(frameMeterLabel, "Startup: %d", g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].Startup);
	else
		sprintf(frameMeterLabel, "Startup: - ");
	RenderText(p1LabelPosition, frameMeterLabel, 0xFFFFFFFF, FONT_SIZE);

	if (g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[1].Startup >= 0)
		sprintf(frameMeterLabel, "Startup: %d", g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[1].Startup);
	else
		sprintf(frameMeterLabel, "Startup: - ");
	RenderText(p2LabelPosition, frameMeterLabel, 0xFFFFFFFF, FONT_SIZE);


	// Advantage
	bool display = g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].DisplayAdvantage;
	unsigned int p1AdvantageFontColor = 0xFFFFFFFF;
	unsigned int p2AdvantageFontColor = 0xFFFFFFFF;
	if (display && g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].Advantage > 0)
	{
		p1AdvantageFontColor = 0xFF00FF00;
		p2AdvantageFontColor = 0xFFFF0000;
	}
	else if (display && g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].Advantage < 0)
	{
		p1AdvantageFontColor = 0xFFFF0000;
		p2AdvantageFontColor = 0xFF00FF00;
	}

	ImVec2 p1AdvLabelPosition = ImVec2(p1LabelPosition.x + advantageXOffset, p1LabelPosition.y);
	ImVec2 p2AdvLabelPosition = ImVec2(p2LabelPosition.x + advantageXOffset, p2LabelPosition.y);

	if (display)
		sprintf(frameMeterLabel, "Advantage: %d", g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[0].Advantage);
	else
		sprintf(frameMeterLabel, "Advantage: - ");

	RenderText(p1AdvLabelPosition, frameMeterLabel, p1AdvantageFontColor, FONT_SIZE);

	if (display)
		sprintf(frameMeterLabel, "Advantage: %d", g_interfaces.frameMeterInterface.frameMeter.PlayerMeters[1].Advantage);
	else
		sprintf(frameMeterLabel, "Advantage: - ");

	RenderText(p2AdvLabelPosition, frameMeterLabel, p2AdvantageFontColor, FONT_SIZE);

}

void HitboxOverlay::DrawFrameMeterLegend()
{
	RECT rect;
	GetClientRect(g_gameProc.hWndGameWindow, &rect);

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	int screenWidth = windowHeight * 4 / 3;

	int pipSpacing = (screenWidth - FRAME_METER_BASE_LINE_X * 2) / LEGEND_ASSUMED_FRAME_METER_SIZE;
	int pipWidth = pipSpacing - 1;
	int totalWidth = (pipSpacing * LEGEND_ASSUMED_FRAME_METER_SIZE) - 1;
	int pipHeight = pipWidth * 5 / 3;
	int yPos = LEGEND_Y_POS_PX * windowHeight / GGXXACPR_SCREEN_HEIGHT_PIXELS;
	int xPos = (windowWidth - totalWidth) / 2;
	int propertyHighlightHeight = pipHeight * 2 / 7;
	propertyHighlightHeight += propertyHighlightHeight == 0 ? 1 : 0;
	int propertyHighlightTop = pipHeight - propertyHighlightHeight;
	int borderThickness = 2 * windowHeight / GGXXACPR_SCREEN_HEIGHT_PIXELS;

	int legendEntryVerticalSpacing = (int)max((LEGEND_FONT_SIZE * 2), pipHeight + borderThickness * 2 + 2);
	int legendColumnSpacing = (int)(LEGEND_FONT_SIZE * 20);

	// Background
	ImVec2 pointA = ImVec2(xPos - LEGEND_FONT_SIZE, yPos - LEGEND_FONT_SIZE);
	ImVec2 pointB = ImVec2(xPos + pipWidth * 2 + legendColumnSpacing + LEGEND_FONT_SIZE * 15, yPos - LEGEND_FONT_SIZE);
	ImVec2 pointC = ImVec2(xPos + pipWidth * 2 + legendColumnSpacing + LEGEND_FONT_SIZE * 15, yPos + legendEntryVerticalSpacing * 6 + pipHeight + LEGEND_FONT_SIZE);
	ImVec2 pointD = ImVec2(xPos - LEGEND_FONT_SIZE, yPos + legendEntryVerticalSpacing * 6 + pipHeight + LEGEND_FONT_SIZE);

	RenderRectFilled(pointA, pointB, pointC, pointD, LABEL_BG);

	for (int i = 0; i < g_interfaces.frameMeterInterface.entries.size(); i++)
	{
		pointA = ImVec2(xPos + ((i / LEGEND_MAX_COLUMN_SIZE) * legendColumnSpacing), yPos + ((i % LEGEND_MAX_COLUMN_SIZE) * legendEntryVerticalSpacing));
		pointB = ImVec2(xPos + pipWidth + ((i / LEGEND_MAX_COLUMN_SIZE) * legendColumnSpacing), yPos + ((i % LEGEND_MAX_COLUMN_SIZE) * legendEntryVerticalSpacing));
		pointC = ImVec2(xPos + pipWidth + ((i / LEGEND_MAX_COLUMN_SIZE) * legendColumnSpacing), yPos + pipHeight + ((i % LEGEND_MAX_COLUMN_SIZE) * legendEntryVerticalSpacing));
		pointD = ImVec2(xPos + ((i / LEGEND_MAX_COLUMN_SIZE) * legendColumnSpacing), yPos + pipHeight + ((i % LEGEND_MAX_COLUMN_SIZE) * legendEntryVerticalSpacing));

		ImVec2 pointA2 = ImVec2(pointA.x - borderThickness, pointA.y - borderThickness);
		ImVec2 pointB2 = ImVec2(pointB.x + borderThickness, pointA.y - borderThickness);
		ImVec2 pointC2 = ImVec2(pointB.x + borderThickness, pointC.y + borderThickness);
		ImVec2 pointD2 = ImVec2(pointA.x - borderThickness, pointC.y + borderThickness);
		// Border
		RenderRect(pointA2, pointB2, pointC2, pointD2, 0xFF000000);
		RenderRectFilled(pointA, pointB, pointC, pointD,
			g_interfaces.frameMeterInterface.GetFrameColor(g_interfaces.frameMeterInterface.entries[i].ExampleFrame.Type));
		
		if (g_interfaces.frameMeterInterface.entries[i].ExampleFrame.SecondaryProperty != SecondaryFrameProperty_Default)
		{
			pointA2 = ImVec2(pointA.x, pointA.y);
			pointB2 = ImVec2(pointB.x, pointA.y);
			pointC2 = ImVec2(pointB.x, pointA.y + propertyHighlightHeight);
			pointD2 = ImVec2(pointA.x, pointA.y + propertyHighlightHeight);

			RenderRectFilled(pointA2, pointB2, pointC2, pointD2,
				g_interfaces.frameMeterInterface.GetSecondaryPropertyColor(g_interfaces.frameMeterInterface.entries[i].ExampleFrame.SecondaryProperty));

			RenderLine(pointD2, pointC2, 0xFF000000, 1.0f);
		}
		if (g_interfaces.frameMeterInterface.entries[i].ExampleFrame.PrimaryProperty1 != PrimaryFrameProperty_Default)
		{
			pointA2 = ImVec2(pointA.x, pointA.y + propertyHighlightHeight);
			pointB2 = ImVec2(pointB.x, pointA.y + propertyHighlightHeight);
			pointC2 = ImVec2(pointB.x, pointC.y);
			pointD2 = ImVec2(pointA.x, pointC.y);

			RenderRectFilled(pointA2, pointB2, pointC2, pointD2,
				g_interfaces.frameMeterInterface.GetPrimaryPropertyColor(g_interfaces.frameMeterInterface.entries[i].ExampleFrame.PrimaryProperty1));

			RenderLine(pointA2, pointB2, 0xFF000000, 1.0f);
		}

		// Draw label
		ImVec2 pos = ImVec2(pointB.x + LEGEND_FONT_SIZE, pointB.y);
		RenderText(pos, g_interfaces.frameMeterInterface.entries[i].Label.c_str(), 0xFFFFFFFF, LEGEND_FONT_SIZE);

	}
}

#define HSD_METER_X -0.95f
#define HSD_METER_Y 0.1f
#define HSD_METER_W 0.05f
#define HSD_METER_H 0.4f
#define UNTECH_METER_COLOR 0xFF00FFFF
#define HSD_THRESHOLD_1 (18.0f / 86.0f)
#define HSD_THRESHOLD_2 (30.0f / 86.0f)
#define HSD_THRESHOLD_3 (42.0f / 86.0f)
#define HSD_THRESHOLD_4 (60.0f / 86.0f)
#define COMBO_TIME_MAX 860.0f
#define HSD_COLOR_100 0xFF0000FF
#define HSD_COLOR_95 0xFF00FF00
#define HSD_COLOR_90 0xFFFFFF00
#define HSD_COLOR_80 0xFFFF8000
#define HSD_COLOR_70 0xFFFF0000
void HitboxOverlay::DrawComboTimeMeter(const CharData* charObj)
{
	RECT rect;
	GetClientRect(g_gameProc.hWndGameWindow, &rect);

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	int screenHeight = windowHeight;
	int screenWidth = windowHeight * 4 / 3;

	int borderThickness = 2 * windowHeight / GGXXACPR_SCREEN_HEIGHT_PIXELS;

	float f1 = 1 - 2 * charObj->playerID;
	float f2 = -0.05f * charObj->playerID;

	float meterX = windowWidth / 2.0 + (HSD_METER_X * f1 + f2) * 0.5f * screenWidth;
	float meterY = windowHeight / 2.0 - HSD_METER_Y * 0.5f * screenHeight;
	float meterW = HSD_METER_W * 0.5 * screenWidth;
	float meterH = HSD_METER_H * 0.5 * screenHeight;

	ImVec2 pointA = ImVec2(meterX - borderThickness, meterY - meterH - borderThickness);
	ImVec2 pointB = ImVec2(meterX + meterW + borderThickness, meterY - meterH - borderThickness);
	ImVec2 pointC = ImVec2(meterX + meterW + borderThickness, meterY + borderThickness);
	ImVec2 pointD = ImVec2(meterX - borderThickness, meterY + borderThickness);

	const unsigned int rectBorderColor = 0xFF000000;
	RenderRect(pointA, pointB, pointC, pointD, rectBorderColor, borderThickness);

	const unsigned char transparency = 0xFF * m_rectFillTransparency;
	unsigned int clearedTransparencyBits = (rectBorderColor & ~0xFF000000);
	unsigned int transparencyPercentage = ((int)transparency << 24) & 0xFF000000;
	const unsigned int rectFillColor = clearedTransparencyBits | transparencyPercentage;
	RenderRectFilled(pointA, pointB, pointC, pointD, rectFillColor);

	float fillValue = charObj->extraData->damageTime / COMBO_TIME_MAX;

	pointA = ImVec2(meterX, 0.0f);
	pointB = ImVec2(meterX + meterW, 0.0f);
	pointC = ImVec2(meterX + meterW, 0.0f);
	pointD = ImVec2(meterX, 0.0f);

	if (fillValue < HSD_THRESHOLD_1)
	{
		pointB.y = pointA.y = meterY - meterH * (1 - fillValue);
		pointD.y = pointC.y = meterY - meterH * (1 - HSD_THRESHOLD_1);
		RenderRectFilled(pointA, pointB, pointC, pointD, HSD_COLOR_100);
	}
	if (fillValue < HSD_THRESHOLD_2)
	{
		pointB.y = pointA.y = meterY - meterH * (1 - max(fillValue, HSD_THRESHOLD_1));
		pointD.y = pointC.y = meterY - meterH * (1 - HSD_THRESHOLD_2);
		RenderRectFilled(pointA, pointB, pointC, pointD, HSD_COLOR_95);
	}
	if (fillValue < HSD_THRESHOLD_3)
	{
		pointB.y = pointA.y = meterY - meterH * (1 - max(fillValue, HSD_THRESHOLD_2));
		pointD.y = pointC.y = meterY - meterH * (1 - HSD_THRESHOLD_3);
		RenderRectFilled(pointA, pointB, pointC, pointD, HSD_COLOR_90);
	}
	if (fillValue < HSD_THRESHOLD_4)
	{
		pointB.y = pointA.y = meterY - meterH * (1 - max(fillValue, HSD_THRESHOLD_3));
		pointD.y = pointC.y = meterY - meterH * (1 - HSD_THRESHOLD_4);
		RenderRectFilled(pointA, pointB, pointC, pointD, HSD_COLOR_80);
	}
	
	pointB.y = pointA.y = meterY - meterH * (1 - max(fillValue, HSD_THRESHOLD_4));
	pointD.y = pointC.y = meterY;
	RenderRectFilled(pointA, pointB, pointC, pointD, HSD_COLOR_70);
	
}

void HitboxOverlay::DrawUntechTimeMeter(const CharData* charObj)
{
	RECT rect;
	GetClientRect(g_gameProc.hWndGameWindow, &rect);

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	int screenHeight = windowHeight;
	int screenWidth = windowHeight * 4 / 3;

	int borderThickness = 2 * windowHeight / GGXXACPR_SCREEN_HEIGHT_PIXELS;

	float f1 = 1 - 2 * charObj->playerID;
	float f2 = -0.05f * charObj->playerID;

	float meterX = windowWidth / 2.0 + ((HSD_METER_X + 0.1f) * f1 + f2) * 0.5f * screenWidth;
	float meterY = windowHeight / 2.0 - HSD_METER_Y * 0.5f * screenHeight;
	float meterW = HSD_METER_W * 0.5 * screenWidth;
	float meterH = HSD_METER_H * 0.5 * screenHeight;

	ImVec2 pointA = ImVec2(meterX - borderThickness, meterY - meterH - borderThickness);
	ImVec2 pointB = ImVec2(meterX + meterW + borderThickness, meterY - meterH - borderThickness);
	ImVec2 pointC = ImVec2(meterX + meterW + borderThickness, meterY + borderThickness);
	ImVec2 pointD = ImVec2(meterX - borderThickness, meterY + borderThickness);

	const unsigned int rectBorderColor = 0xFF000000;
	RenderRect(pointA, pointB, pointC, pointD, rectBorderColor, borderThickness);

	const unsigned char transparency = 0xFF * m_rectFillTransparency;
	unsigned int clearedTransparencyBits = (rectBorderColor & ~0xFF000000);
	unsigned int transparencyPercentage = ((int)transparency << 24) & 0xFF000000;
	const unsigned int rectFillColor = clearedTransparencyBits | transparencyPercentage;
	RenderRectFilled(pointA, pointB, pointC, pointD, rectFillColor);

	float fillValue = charObj->extraData->downTimer / 59.0f;

	if (fillValue > 0)
	{
		pointA = ImVec2(meterX, meterY - meterH * fillValue);
		pointB = ImVec2(meterX + meterW, meterY - meterH * fillValue);
		pointC = ImVec2(meterX + meterW, meterY);
		pointD = ImVec2(meterX, meterY);

		RenderRectFilled(pointA, pointB, pointC, pointD, UNTECH_METER_COLOR);
	}

}

bool HitboxOverlay::IsOwnerEnabled(byte playerID)
{
	for (int i = 0; i < 2; i++)
	{
		if (playerID == i)
		{
			return drawCharacterHitbox[i];
		}
	}

	return false;
}

bool HitboxOverlay::HasNullptrInData()
{
	return g_interfaces.Player1.IsCharDataNullPtr() || g_interfaces.Player2.IsCharDataNullPtr();
}

ImVec2 HitboxOverlay::CalculateObjWorldPosition(const CharData* charObj)
{
	return ImVec2(
		charObj->posX,
		charObj->posY
	);
}

ImVec2 HitboxOverlay::CalculateScreenPosition(ImVec2 worldPos)
{
	//D3DXVECTOR3 result;
	//D3DXVECTOR3 vec3WorldPos(worldPos.x, worldPos.y, 0.0f);
	//WorldToScreen(g_interfaces.pD3D9ExWrapper, g_gameVals.viewMatrix, g_gameVals.projMatrix, &vec3WorldPos, &result);
	ImVec2 result = PixelToScreen(WorldToPixel(worldPos));

	return ImVec2(floor(result.x), floor(result.y));
}

ImVec2 HitboxOverlay::RotatePoint(ImVec2 center, float angleInRad, ImVec2 point)
{
	if (!angleInRad)
	{
		return point;
	}

	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	float s = sin(angleInRad);
	float c = cos(angleInRad);

	// rotate point
	float xNew = point.x * c - point.y * s;
	float yNew = point.x * s + point.y * c;

	// translate point back:
	point.x = xNew + center.x;
	point.y = yNew + center.y;
	return point;
}

void HitboxOverlay::DrawOriginLine(ImVec2 worldPos, float rotationRad)
{
	const unsigned int colorOrange = 0xFFFF9900;
	const int horizontalLength = 20;
	const int verticalLength = 50;

	ImVec2 horizontalFrom = RotatePoint(worldPos, rotationRad, ImVec2(worldPos.x - horizontalLength / 2, worldPos.y));
	ImVec2 horizontalTo = RotatePoint(worldPos, rotationRad, ImVec2(worldPos.x + horizontalLength / 2, worldPos.y));
	horizontalFrom = CalculateScreenPosition(horizontalFrom);
	horizontalTo = CalculateScreenPosition(horizontalTo);
	RenderLine(horizontalFrom, horizontalTo, colorOrange, 3);

	ImVec2 verticalFrom = worldPos;
	ImVec2 verticalTo = RotatePoint(verticalFrom, rotationRad, ImVec2(verticalFrom.x, verticalFrom.y + verticalLength));
	verticalFrom = CalculateScreenPosition(verticalFrom);
	verticalTo = CalculateScreenPosition(verticalTo);
	RenderLine(verticalFrom, verticalTo, colorOrange, 3);
}

Hitbox HitboxOverlay::ScaleHitbox(const Hitbox* hitbox, const CharData* p)
{
	if (p->scaleX < 0 && p->scaleY < 0) { return *hitbox; }
	// If scale var is -1, apply the other var to both dimensions
	auto scaleY = p->scaleY < 0 ? p->scaleX : p->scaleY;
	auto scaleX = p->scaleX < 0 ? p->scaleY : p->scaleX;

	return {(int16_t)floor(hitbox->offsetX * scaleX / 1000.0f),
		    (int16_t)floor(hitbox->offsetY * scaleY / 1000.0f),
			(int16_t)floor(hitbox->width * scaleX / 1000.0f),
			(int16_t)floor(hitbox->height * scaleY / 1000.0f),
			hitbox->type,
			hitbox->flags };
}

ImRect HitboxOverlay::MapHitboxToOrigin(const Hitbox* hitbox, bool facingRight, int xPos, int yPos)
{
	auto offset = facingRight ? ((hitbox->offsetX + hitbox->width) * -100) : (hitbox->offsetX * 100);
	return ImRect(
		xPos + offset,
		yPos + hitbox->offsetY * 100,
		xPos + offset + hitbox->width * 100,
		yPos + (hitbox->offsetY + hitbox->height) * 100);
}

void HitboxOverlay::DrawHitbox(Hitbox* drawbox, const CharData* charObj, const unsigned int rectBorderColor)
{
	ImRect mappedRect = MapHitboxToOrigin(drawbox, charObj->facingRight, charObj->posX, charObj->posY);

	ImVec2 pointA = mappedRect.Min;
	ImVec2 pointB(mappedRect.Max.x, mappedRect.Min.y);
	ImVec2 pointC = mappedRect.Max;
	ImVec2 pointD(mappedRect.Min.x, mappedRect.Max.y);

	pointA = CalculateScreenPosition(pointA);
	pointB = CalculateScreenPosition(pointB);
	pointC = CalculateScreenPosition(pointC);
	pointD = CalculateScreenPosition(pointD);

	pointA = ImVec2(pointA.x + m_rectThickness / 2.0, pointA.y + m_rectThickness / 2.0);
	pointB = ImVec2(pointB.x - m_rectThickness / 2.0, pointB.y + m_rectThickness / 2.0);
	pointC = ImVec2(pointC.x - m_rectThickness / 2.0, pointC.y - m_rectThickness / 2.0);
	pointD = ImVec2(pointD.x + m_rectThickness / 2.0, pointD.y - m_rectThickness / 2.0);

	RenderRect(pointA, pointB, pointC, pointD, rectBorderColor, m_rectThickness);

	const unsigned char transparency = 0xFF * m_rectFillTransparency;
	unsigned int clearedTransparencyBits = (rectBorderColor & ~0xFF000000);
	unsigned int transparencyPercentage = ((int)transparency << 24) & 0xFF000000;
	const unsigned int rectFillColor = clearedTransparencyBits | transparencyPercentage;
	RenderRectFilled(pointA, pointB, pointC, pointD, rectFillColor);
}

void HitboxOverlay::DrawCollisionAreas(const CharData* charObj, const ImVec2 playerWorldPos)
{
	std::vector<Hitbox> entries = HitboxReader::getHitboxes(charObj);

	const unsigned int colorGreen = 0xFF00FF00;
	const unsigned int colorRed = 0xFFFF0000;

	int index = -1;
	for (const Hitbox &entry : entries)
	{
		index++;
		if (entry.type != HitboxType_Hurtbox && entry.type != HitboxType_Hitbox && entry.type != HitboxType_Extra)
			continue; //This is needed to not draw special effect entities
		if (charObj->extraData != NULL) //No extra data available for entities, skip this check
		{
			if (entry.type == HitboxType_Hurtbox && (
				charObj->extraData->invTime > 0 ||
				(charObj->status & 0x0080) > 0 || //Check if hurboxes are disabled
				(charObj->status & 0x00020000) > 0)) //Check if strike invulnerable
				continue;
			if ((entry.type == HitboxType_Hitbox) && (
				(charObj->status & 0x0040) > 0 &&  //Check if hitboxes are disabled
				!(charObj->hitstopTimer > 0 &&
					(charObj->attackFlags & 0x1000) > 0)    // Hitstop counter is also used in super flash, so need to check attack flags as well
				))
				continue;
		}
		else
		{
			if ((entry.type == HitboxType_Hurtbox && (charObj->status & 0x0080) > 0) ||
				(entry.type == HitboxType_Hitbox && (charObj->status & 0x0040) > 0))
				continue;
		}

		Hitbox drawbox = ScaleHitbox(&entry, charObj);
		if (entry.type == HitboxType_Extra)
		{
			if (charObj->hitboxExtraArray == NULL)
				continue;
			
			Hitbox* extra = (Hitbox*)((char*)charObj->hitboxExtraArray + index * sizeof(Hitbox));
			if (extra->type != HitboxType_Hitbox && extra->type != HitboxType_Hurtbox)
				continue;
			drawbox = ScaleHitbox(extra, charObj);
		}
	
		const unsigned int rectBorderColor = entry.type == HitboxType_Hurtbox ? colorGreen : colorRed;

		DrawHitbox(&drawbox, charObj, rectBorderColor);

	}

	//Special check for Slide head
	if (charObj->charIndex == CharIndex_Potemkin &&
		charObj->actId == 181 && //Slide head animation id
		charObj->actionHeaderFalgs == 0x10102015 && //Falgs for unblockable section of slide head
		charObj->hitboxExtraArray != NULL)
	{
		Hitbox drawbox = ScaleHitbox((Hitbox*)charObj->hitboxExtraArray, charObj);
		DrawHitbox(&drawbox, charObj, colorRed);
	}

	if (drawOriginLine)
	{
		DrawOriginLine(playerWorldPos, 0.0f);
	}
}

#define CharID_Bridget 20
#define BRIDGET_SHOOT_ACT_ID 134
#define BRIDGET_SHOOT_PUSHBOX_ADJUST 7000
Hitbox HitboxOverlay::GetPlayerPushBox(const CharData* charObj)
{
	int yOffset = 0;
	uint16_t* widthArr;
	uint16_t* heightArr;
	if ((charObj->status & 0x0010) > 0) //Is airborne?
	{
		widthArr = g_gameVals.pPushboxDimensions[2][0]; //Air pushbox width
		heightArr = g_gameVals.pPushboxDimensions[2][1]; //Air pushbox heights
		// Special offsets for pushbox collision checks
		yOffset = *g_gameVals.pPushboxJumpOffset[charObj->playerID] + charObj->posY;
	}
	else if ((charObj->status & 0x0400) > 0) //Is crouching?
	{
		widthArr = g_gameVals.pPushboxDimensions[1][0]; //Crouching pushbox width
		heightArr = g_gameVals.pPushboxDimensions[1][1]; //Crouching pushbox heights
	}
	else if ((charObj->status & 0x8000) > 0) //Is pushbox type1?
	{
		// Not really sure what state this is. Adapting the draw logic from another project.
		widthArr = g_gameVals.pPushboxDimensions[2][0]; //Air pushbox width
		heightArr = g_gameVals.pPushboxDimensions[0][1]; //Standing pushbox height
	}
	else    // IsStanding
	{
		widthArr = g_gameVals.pPushboxDimensions[0][0]; //Standing pushbox width
		heightArr = g_gameVals.pPushboxDimensions[0][1]; //Standing pushbox heights
	}

	int16_t w = widthArr[charObj->charIndex];
	int16_t h = heightArr[charObj->charIndex];

	bool hasPushBoxOverride = false;

	std::vector<Hitbox> boxSet = HitboxReader::getHitboxes(charObj);
	for (const Hitbox& entry : boxSet)
	{
		if (entry.type == HitboxType_Pushbox)
		{
			hasPushBoxOverride = true;
			return {
				entry.offsetX,
				(int16_t)((h + yOffset) / -100),
				entry.width,
				(int16_t)(h / 100) };
		}
	}

	if (!hasPushBoxOverride)
	{
		if (charObj->charIndex == CharID_Bridget && charObj->actId == BRIDGET_SHOOT_ACT_ID)
		{
			return {
				(int16_t)(w / -100),
				(int16_t)((h + yOffset + BRIDGET_SHOOT_PUSHBOX_ADJUST) / -100),
				(int16_t)(w / 100 * 2),
				(int16_t)((h + BRIDGET_SHOOT_PUSHBOX_ADJUST) / 100) };
		}
		else
		{
			return { (int16_t)(w / -100),
				(int16_t)((h + yOffset) / -100),
				(int16_t)(w / 100 * 2),
				(int16_t)(h / 100) };
		}
	}

	return Hitbox();
}

#define POTEMKIN_HAMMER_FALL_ACT_ID 0x78
#define POTEMKIN_HAMMER_FALL_RANGE 170
#define FAUST_HACK_N_SLASH_FAIL_ACT_ID 190
#define FAUST_HACK_N_SLASH_UNBLOCKABLE_ACT_ID 122
#define FAUST_HACK_N_SLASH_RANGE 100
Hitbox HitboxOverlay::GetProximityBoxPlayer(const CharData* charObj)
{
	Hitbox box = Hitbox();
	if (charObj == NULL)
		return box;

	if (charObj->charIndex == CharIndex_Potemkin &&
		charObj->actId == POTEMKIN_HAMMER_FALL_ACT_ID &&
		charObj->mark != 0)
	{
		Hitbox push = GetPlayerPushBox(charObj);
		box.offsetX = push.offsetX - POTEMKIN_HAMMER_FALL_RANGE;
		box.offsetY = push.offsetY;
		box.width = push.width + POTEMKIN_HAMMER_FALL_RANGE * 2;
		box.height = push.height;
	}
	else if (charObj->charIndex == CharIndex_Faust &&
		(charObj->actId == FAUST_HACK_N_SLASH_FAIL_ACT_ID || charObj->actId == FAUST_HACK_N_SLASH_UNBLOCKABLE_ACT_ID) &&
		charObj->frameCounter == 1)
	{
		Hitbox push = GetPlayerPushBox(charObj);
		box.offsetX = push.offsetX - FAUST_HACK_N_SLASH_RANGE;
		box.offsetY = push.offsetY;
		box.width = push.width + FAUST_HACK_N_SLASH_RANGE * 2;
		box.height = push.height;
	}

	return box;
}

#define ROBO_KY_MAT_ID 0x62
#define ROBO_KY_MAT_COLLISION_RANGE 132
#define TESTAMENT_ENTITY_ID 0x50
#define TESTAMENT_HITOMI_ACTIVATION_RANGE_PR 55
#define TESTAMENT_HITOMI_ACTIVATION_RANGE_AC 70
#define MAX_DISPLAY_HEIGHT 1000
#define FAUST_ENTITY_ID 0x2B
#define FAUST_DONUT_PICKUP_ACT_ID 18
#define FAUST_CHOCOLATE_PICKUP_ACT_ID 19
#define FAUST_CHIKUWA_PICKUP_ACT_ID 53
#define FAUST_FOOD_PICKUP_RANGE 48
#define EDDIE_ENTITY_ID 0x23
#define EDDIE_PUDDLE_VERTICAL_RANGE 40
bool HitboxOverlay::GetProximityBoxEntity(const CharData* charObj, Hitbox& box)
{
	box = Hitbox();
	if (charObj == NULL)
		return false;

	auto halfHeight = m_rectThickness * 150.0f;

	if (charObj->charIndex == ROBO_KY_MAT_ID)
	{
		box.offsetX = -ROBO_KY_MAT_COLLISION_RANGE;
		box.offsetY = -halfHeight;
		box.width = ROBO_KY_MAT_COLLISION_RANGE * 2;
		box.height = halfHeight * 2;
		return true;
	}
	else if (charObj->charIndex == TESTAMENT_ENTITY_ID && (charObj->actId == 10 || charObj->actId == 17))
	{
		auto range = *g_gameVals.pGameVerFlag ? TESTAMENT_HITOMI_ACTIVATION_RANGE_PR : TESTAMENT_HITOMI_ACTIVATION_RANGE_AC;

		box.offsetX = -range;
		box.offsetY = g_interfaces.frameMeterInterface.settings.DrawInfiniteHeight ? -MAX_DISPLAY_HEIGHT : -halfHeight;
		box.width = range * 2;
		box.height = g_interfaces.frameMeterInterface.settings.DrawInfiniteHeight ? MAX_DISPLAY_HEIGHT * 2 : halfHeight * 2;

		return true;
	}
	else if (charObj->charIndex == FAUST_ENTITY_ID &&
		(charObj->actId == FAUST_DONUT_PICKUP_ACT_ID || charObj->actId == FAUST_CHOCOLATE_PICKUP_ACT_ID || charObj->actId ==  FAUST_CHIKUWA_PICKUP_ACT_ID))
	{
		box.offsetX = -FAUST_FOOD_PICKUP_RANGE;
		box.offsetY = -halfHeight;
		box.width = FAUST_FOOD_PICKUP_RANGE * 2;
		box.height = halfHeight * 2;
		return true;
	}
	else if (charObj->charIndex == EDDIE_ENTITY_ID && charObj->entityIndex == 1 && charObj->transition == 0)
	{
		box.offsetX = -charObj->mark;
		box.offsetY = -EDDIE_PUDDLE_VERTICAL_RANGE;
		box.width = charObj->mark * 2;
		box.height = EDDIE_PUDDLE_VERTICAL_RANGE * 2;
		return true;
	}

	return false;
}

void HitboxOverlay::DrawPlayerPushboxes(const CharData* charObj)
{
	
	Hitbox pushbox = GetPlayerPushBox(charObj);
	Hitbox drawbox = ScaleHitbox(&pushbox, charObj);
	

	const unsigned int colorCollision = 0xFF00FFFF;

	DrawHitbox(&drawbox, charObj, colorCollision);

}

void HitboxOverlay::DrawProximityBoxes(const CharData* charObj, bool isPlayer)
{

	Hitbox rangebox;
	Hitbox drawbox;

	const unsigned int colorProximityPlayer = 0x80FF00FF;
	const unsigned int colorProximityEntity = 0x80FF8000;

	if (isPlayer)
	{
		rangebox = GetProximityBoxPlayer(charObj);
		drawbox = ScaleHitbox(&rangebox, charObj);
		DrawHitbox(&drawbox, charObj, colorProximityPlayer);
	}
	else if (GetProximityBoxEntity(charObj, rangebox))
	{
		drawbox = ScaleHitbox(&rangebox, charObj);
		DrawHitbox(&drawbox, charObj, colorProximityEntity);
	}
}

void HitboxOverlay::DrawCleanHitBox(const CharData* charObj)
{

	Hitbox clbox = GetCLRect(charObj);
	Hitbox drawbox = ScaleHitbox(&clbox, charObj);

	const unsigned int colorCL = 0x80FF8000;

	DrawHitbox(&drawbox, charObj, colorCL);

}

Hitbox HitboxOverlay::GetPlayerThrowBox(const CharData* charObj)
{
	Hitbox pushbox = GetPlayerPushBox(charObj);

	if ((charObj->status & 0x0010) > 0) //Check if airborne
	{
		int16_t horizontalRange;

		horizontalRange = g_gameVals.pAirHorizontalThrowRange[*g_gameVals.pGameVerFlag][charObj->charIndex];
		int16_t upperBound = g_gameVals.pAirVerticalThrowRange[1][charObj->charIndex];
		int16_t lowerBound = g_gameVals.pAirVerticalThrowRange[0][charObj->charIndex];

		return
		{
			(int16_t)(pushbox.offsetX - horizontalRange / 100),
			(int16_t)(pushbox.offsetY + pushbox.height + upperBound / 100),
			(int16_t)(pushbox.width + horizontalRange / 50),
			(int16_t)((lowerBound - upperBound) / 100),
		};
	}
	else
	{
		int16_t range;

		range = g_gameVals.pGroundThrowRange[*g_gameVals.pGameVerFlag][charObj->charIndex];

		return
		{
			(int16_t)(pushbox.offsetX - range / 100),
			pushbox.offsetY,
			(int16_t)(pushbox.width + range / 50),
			pushbox.height,
		};
	}
	
}

Hitbox HitboxOverlay::GetCLRect(const CharData* charObj)
{
	if (charObj->hitParam->clScale == -1)
		return Hitbox();

	CharData* opponent = charObj->playerID == 0 ? g_interfaces.Player2.GetData() : g_interfaces.Player1.GetData();
	if (opponent == NULL || opponent->extraData == NULL)
		return Hitbox();

	uint8_t CLCounter = opponent->extraData->cleanHitCounter;

	// If player has just hit a clean hit, draw the CLRect as it was before it shrunk.
	//if (IsInCLHitstop(p)) CLCounter--;
	if ((opponent->extraData->hitstunFlags & 0x00020000) > 0)
		CLCounter--;

	HitParam* hp = charObj->hitParam;

	int16_t halfWidth = hp->clBaseWidth - (hp->clScale * CLCounter);
	int16_t halfHeight = hp->clBaseHeight - (hp->clScale * CLCounter);
	halfWidth = halfWidth >= 1 ? halfWidth : 1;
	halfHeight = halfHeight >= 1 ? halfHeight : 1;

	return
	{
		(int16_t)(hp->clCenterX - halfWidth),
		(int16_t)(hp->clCenterY - halfHeight + CLEAN_HIT_Y_OFFSET),
		(int16_t)(halfWidth * 2),
		(int16_t)(halfHeight * 2)
	};
}

void HitboxOverlay::DrawPlayerGrabBox(const CharData* charObj, bool drawOverride)
{
	if (DrawPlayerCommandGrabBox(charObj)) { return; }
	if ((charObj->commandFlags & 0x2000) > 0) { return; } //If disable throws

	if ((*g_gameVals.pGlobalThrowFlags & 0x1) > 0 && charObj->playerID == 0 ||
		(*g_gameVals.pGlobalThrowFlags & 0x2) > 0 && charObj->playerID == 1 ||
		drawOverride)
	{
		Hitbox throwBox = GetPlayerThrowBox(charObj);
		Hitbox drawbox = ScaleHitbox(&throwBox, charObj);

		const unsigned int colorGrab = 0xFFFF00FF;

		DrawHitbox(&drawbox, charObj, colorGrab);
	}
}

bool HitboxOverlay::DrawPlayerCommandGrabBox(const CharData* charObj)
{
	if ((charObj->mark == 1) && MoveData::IsActiveByMark(charObj->charIndex, charObj->actId))
	{
		Hitbox pushbox = GetPlayerPushBox(charObj);

		auto cmdThrowRange = MoveData::GetCommandGrabRange(charObj->charIndex, charObj->actId);
		Hitbox cmdThrowHitboxRep = 
		{
			(int16_t)(pushbox.offsetX - cmdThrowRange / 100),
			pushbox.offsetY,
			(int16_t)(pushbox.width + cmdThrowRange * 2 / 100),
			pushbox.height
		};

		Hitbox drawbox = ScaleHitbox(&cmdThrowHitboxRep, charObj);
		
		const unsigned int colorGrab = 0xFFFF00FF;

		DrawHitbox(&drawbox, charObj, colorGrab);
		return true;
	}
	return false;
}

float& HitboxOverlay::GetScale()
{
	return m_scale;
}

void HitboxOverlay::DrawRectThicknessSlider()
{
	ImGui::SliderFloat("Border thickness", &m_rectThickness, 0.0f, 5.0f, "%.1f");
}

void HitboxOverlay::DrawRectFillTransparencySlider()
{
	ImGui::SliderFloat("Fill transparency", &m_rectFillTransparency, 0.0f, 1.0f, "%.2f");
}

void HitboxOverlay::RenderLine(const ImVec2& from, const ImVec2& to, uint32_t color, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddLine(from, to, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), thickness);
}

void HitboxOverlay::RenderCircle(const ImVec2& position, float radius, uint32_t color, float thickness, uint32_t segments)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircle(position, radius, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), segments, thickness);
}

void HitboxOverlay::RenderCircleFilled(const ImVec2& position, float radius, uint32_t color, uint32_t segments)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddCircleFilled(position, radius, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), segments);
}

void HitboxOverlay::RenderRect(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding, uint32_t roundingCornersFlags, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xFF;
	float r = (color >> 16) & 0xFF;
	float g = (color >> 8) & 0xFF;
	float b = (color) & 0xFF;

	window->DrawList->AddRect(from, to, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), rounding, roundingCornersFlags, thickness);
}

void HitboxOverlay::RenderRect(const ImVec2& pointA, const ImVec2& pointB, const ImVec2& pointC, const ImVec2& pointD, uint32_t color, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xFF;
	float r = (color >> 16) & 0xFF;
	float g = (color >> 8) & 0xFF;
	float b = (color) & 0xFF;

	window->DrawList->AddQuad(pointA, pointB, pointC, pointD, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), thickness);
}

void HitboxOverlay::RenderRectFilled(const ImVec2& from, const ImVec2& to, uint32_t color, float rounding, uint32_t roundingCornersFlags)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xFF;
	float r = (color >> 16) & 0xFF;
	float g = (color >> 8) & 0xFF;
	float b = (color) & 0xFF;

	window->DrawList->AddRectFilled(from, to, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), rounding, roundingCornersFlags);
}

void HitboxOverlay::RenderRectFilled(const ImVec2& pointA, const ImVec2& pointB, const ImVec2& pointC, const ImVec2& pointD, uint32_t color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xFF;
	float r = (color >> 16) & 0xFF;
	float g = (color >> 8) & 0xFF;
	float b = (color) & 0xFF;

	window->DrawList->AddQuadFilled(pointA, pointB, pointC, pointD, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }));
}

void HitboxOverlay::RenderText(const ImVec2& pos, const char* text, uint32_t color, float fontsize)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = (color >> 24) & 0xff;
	float r = (color >> 16) & 0xff;
	float g = (color >> 8) & 0xff;
	float b = (color) & 0xff;

	window->DrawList->AddText(ImGui::GetFont(), fontsize, pos, ImGui::GetColorU32({ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }), text);
}

bool HitboxOverlay::WorldToScreen(LPDIRECT3DDEVICE9 pDevice, D3DXMATRIX* view, D3DXMATRIX* proj, D3DXVECTOR3* pos, D3DXVECTOR3* out)
{
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX world;

	pDevice->GetViewport(&viewPort);
	D3DXMatrixIdentity(&world);

	D3DXVec3Project(out, pos, &viewPort, proj, view, &world);
	if (out->z < 1) {
		return true;
	}
	return false;
}

ImVec2 HitboxOverlay::WorldToPixel(ImVec2 worldPos)
{
	float z = g_gameVals.pCamera->Zoom;
	int x = (int)floor((worldPos.x - g_gameVals.pCamera->LeftEdge) * z / 100);
	int y = (int)floor((worldPos.y - g_gameVals.pCamera->BottomEdge) * z / 100);
	y = y + GGXXACPR_SCREEN_HEIGHT_PIXELS - GGXXACPR_SCREEN_GROUND_PIXEL_OFFSET;
	return ImVec2(x, y);
}

ImVec2 HitboxOverlay::PixelToScreen(ImVec2 pixelPos)
{
	RECT rect;
	GetClientRect(g_gameProc.hWndGameWindow, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	// Should math out to zero if not in widescreen
	int wideScreenOffset = (width - (height * 4 / 3)) / 2;

	return ImVec2(
		(int)floor(1.0f * pixelPos.x * height / GGXXACPR_SCREEN_HEIGHT_PIXELS) + wideScreenOffset,
		(int)floor(1.0f * pixelPos.y * height / GGXXACPR_SCREEN_HEIGHT_PIXELS)
	);
}
