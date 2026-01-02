#include "Overlay/FrameMeter/FrameMeterInterface.h"
#include"Core/interfaces.h"
#include"Core/Settings.h"
#include <atlstr.h>

bool FrameMeterInterface::LoadSettings()
{
    CString strINIPath;

    _wfullpath((wchar_t*)strINIPath.GetBuffer(MAX_PATH), L"settings.ini", MAX_PATH);
    strINIPath.ReleaseBuffer();

    if (GetFileAttributes(strINIPath) == 0xFFFFFFFF)
    {
        MessageBoxA(NULL, "Settings INI File Was Not Found!", "Error", MB_OK);
        return false;
    }
    //X-Macro
#define SETTING_BOOL(_var, _inistring, _defaultval) \
    g_interfaces.frameMeterInterface.settings._var = Settings::readSettingsFilePropertyInt(_T("Overlay"), L##_inistring, L##_defaultval, strINIPath) != 0;
#include "overlay.def"
#undef SETTING_BOOL

#define SETTING_ARGB(_var, _inistring, _defaultval) \
    g_interfaces.frameMeterInterface.palettes._var = Settings::readSettingsFilePropertyUInt(_T("Overlay.Palette"), L##_inistring, L##_defaultval, strINIPath);
#include "overlay_palette.def"
#undef SETTING_ARGB

#define SETTING_FLOAT(_var, _inistring, _defaultval) \
    g_interfaces.frameMeterInterface.misc._var = Settings::readSettingsFilePropertyFloat(_T("Overlay.Misc"), L##_inistring, L##_defaultval, strINIPath);
#include "overlay_misc.def"
#undef SETTING_FLOAT

    g_interfaces.frameMeterInterface.misc.rectThickness = max(0.0f, g_interfaces.frameMeterInterface.misc.rectThickness);
    g_interfaces.frameMeterInterface.misc.rectThickness = min(5.0f, g_interfaces.frameMeterInterface.misc.rectThickness);

    g_interfaces.frameMeterInterface.misc.rectFillTransparency = max(0.0f, g_interfaces.frameMeterInterface.misc.rectFillTransparency);
    g_interfaces.frameMeterInterface.misc.rectFillTransparency = min(1.0f, g_interfaces.frameMeterInterface.misc.rectFillTransparency);

    g_interfaces.frameMeterInterface.misc.pivotSize = max(0.0, g_interfaces.frameMeterInterface.misc.pivotSize);
    g_interfaces.frameMeterInterface.misc.pivotThickness = max(0.0, g_interfaces.frameMeterInterface.misc.pivotThickness);

    g_interfaces.frameMeterInterface.misc.hsdMeterXPosition = max(-1.0f, g_interfaces.frameMeterInterface.misc.hsdMeterXPosition);
    g_interfaces.frameMeterInterface.misc.hsdMeterXPosition = min( 1.0f, g_interfaces.frameMeterInterface.misc.hsdMeterXPosition);

    g_interfaces.frameMeterInterface.misc.hsdMeterYPosition = max(-1.0f, g_interfaces.frameMeterInterface.misc.hsdMeterYPosition);
    g_interfaces.frameMeterInterface.misc.hsdMeterYPosition = min( 1.0f, g_interfaces.frameMeterInterface.misc.hsdMeterYPosition);
}

void FrameMeterInterface::Update()
{
    if (g_interfaces.Player1.IsCharDataNullPtr() || g_interfaces.Player2.IsCharDataNullPtr() ||
        g_interfaces.Player1.GetData()->extraData == NULL || g_interfaces.Player2.GetData()->extraData == NULL)
        return;

    if (*g_gameVals.pFPSCounter > 60)
        return;

    if (*g_gameVals.pInReplayPauseMenu)
        return;

    if (g_gameVals.GetGameMode() != GameMode_ReplayTheater || *g_gameVals.pframe_count_minus_1_P1 > last_frame_count)
    {
        if (rewinding)
            frameMeter.PushMeterState();
        frameMeter.Update(GetGameState(), settings);
        if (g_gameVals.GetGameMode() == GameMode_ReplayTheater && settings.AllowRewindingInReplay)
            frameMeter.PushMeterState();
        rewinding = false;
    }
    else if (g_gameVals.GetGameMode() == GameMode_ReplayTheater && *g_gameVals.pframe_count_minus_1_P1 < last_frame_count)
    {
        if (settings.AllowRewindingInReplay)
        {
            if (!rewinding)
            {
                frameMeter.PopMeterState();
                rewinding = true;
            }
            frameMeter.PopMeterState();
        }
        else
            frameMeter.Reset();
    }

    last_frame_count = *g_gameVals.pframe_count_minus_1_P1;
}

void FrameMeterInterface::UpdateFastForward()
{
    if (g_interfaces.Player1.IsCharDataNullPtr() || g_interfaces.Player2.IsCharDataNullPtr() ||
        g_interfaces.Player1.GetData()->extraData == NULL || g_interfaces.Player2.GetData()->extraData == NULL)
        return;

    if (*g_gameVals.pFPSCounter <= 60)
        return;

    if (g_gameVals.GetGameMode() != GameMode_ReplayTheater || *g_gameVals.pframe_count_minus_1_P1 > last_frame_count)
    {
        if (rewinding)
            frameMeter.PushMeterState();
        frameMeter.Update(GetGameState(), settings);
        if (g_gameVals.GetGameMode() == GameMode_ReplayTheater)
            frameMeter.PushMeterState();
        rewinding = false;
    }

    last_frame_count = *g_gameVals.pframe_count_minus_1_P1;
}

void FrameMeterInterface::Reset()
{
    frameMeter.Reset();
    frameMeter.ResetRecordedFrames();
}

FM_GameState FrameMeterInterface::GetGameState()
{
    FM_Player player1 = {
        g_interfaces.Player1.GetData()->charIndex,
        ActionStateFlags(g_interfaces.Player1.GetData()->status),
        GuardStateFlags(g_interfaces.Player1.GetData()->blockType),
        CommandFlags(g_interfaces.Player1.GetData()->commandFlags),
        AttackStateFlags(g_interfaces.Player1.GetData()->attackFlags),
        g_interfaces.Player1.GetData()->actId,
        g_interfaces.Player1.GetData()->frameCounter,
        {
            g_interfaces.Player1.GetData()->extraData->throwProtectionTimer,
            g_interfaces.Player1.GetData()->extraData->invTime,
            g_interfaces.Player1.GetData()->extraData->FRCTimer,
            g_interfaces.Player1.GetData()->extraData->sbTimer,
            (uint8_t)g_interfaces.Player1.GetData()->extraData->characterSLOT5
        },
        g_interfaces.Player1.GetData()->hitstopTimer,
        HitboxReader::getHitboxes(g_interfaces.Player1.GetData()),
        HitboxReader::getExtraHitboxes(g_interfaces.Player1.GetData()),
        g_interfaces.Player1.GetData()->mark,
        g_interfaces.Player1.GetData()->actionHeaderFlags,
    };

    FM_Player player2 = {
        g_interfaces.Player2.GetData()->charIndex,
        ActionStateFlags(g_interfaces.Player2.GetData()->status),
        GuardStateFlags(g_interfaces.Player2.GetData()->blockType),
        CommandFlags(g_interfaces.Player2.GetData()->commandFlags),
        AttackStateFlags(g_interfaces.Player2.GetData()->attackFlags),
        g_interfaces.Player2.GetData()->actId,
        g_interfaces.Player2.GetData()->frameCounter,
        {
            g_interfaces.Player2.GetData()->extraData->throwProtectionTimer,
            g_interfaces.Player2.GetData()->extraData->invTime,
            g_interfaces.Player2.GetData()->extraData->FRCTimer,
            g_interfaces.Player2.GetData()->extraData->sbTimer,
            (uint8_t)g_interfaces.Player2.GetData()->extraData->characterSLOT5
        },
        g_interfaces.Player2.GetData()->hitstopTimer,
        HitboxReader::getHitboxes(g_interfaces.Player2.GetData()),
        HitboxReader::getExtraHitboxes(g_interfaces.Player2.GetData()),
        g_interfaces.Player2.GetData()->mark,
        g_interfaces.Player1.GetData()->actionHeaderFlags,
    };

    std::vector<FM_Entity> entities;

    if (*g_gameVals.entityCount > 0)
    {
        CharData* pEntity = (CharData*)(*g_gameVals.pEntityStartAddress);
        bool isEntityActive = pEntity->charIndex > 0;
        while (isEntityActive)
        {
            entities.push_back({
                ActionStateFlags(pEntity->status),
                pEntity->playerID,
                HitboxReader::getHitboxes(pEntity),
                HitboxReader::getExtraHitboxes(pEntity),
                });
            pEntity = pEntity->nextEntity;
            isEntityActive = pEntity->charIndex > 0;
        }
    }

    FM_GlobalFlags globalFlags = {
        ThrowFlags(*g_gameVals.pGlobalThrowFlags),
        *(byte*)((char*)g_gameVals.pThrowRangeLookupTable + *g_gameVals.pCommandThrowId[0]),
        *(byte*)((char*)g_gameVals.pThrowRangeLookupTable + *g_gameVals.pCommandThrowId[1])
    };

    return FM_GameState(player1, player2, entities, globalFlags);
}

unsigned int FrameMeterInterface::GetFrameColor(FrameType_ type)
{
	switch (type)
	{
    case FrameType_None:
        return 0xFF0F0F0F;
        break;
    case FrameType_Neutral:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameNeutral;
        break;
    case FrameType_Movement:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameMovement;
        break;
    case FrameType_CounterHitState:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameCounterHitState;
        break;
    case FrameType_Startup:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameStartup;
        break;
    case FrameType_Active:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameActive;
        break;
    case FrameType_ActiveThrow:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameActiveThrow;
        break;
    case FrameType_Recovery:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameRecovery;
        break;
    case FrameType_BlockStun:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameBlockStun;
        break;
    case FrameType_HitStun:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameHitstun;
        break;
    case FrameType_TechableHitStun:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameTechableHitstun;
        break;
    case FrameType_KnockDownHitStun:
        return g_interfaces.frameMeterInterface.palettes.colorFM_FrameKnockDownHitstun;
        break;
	}
    return 0x00000000;
}

unsigned int FrameMeterInterface::GetPrimaryPropertyColor(PrimaryFrameProperty_ prop)
{
    switch (prop)
    {
    case PrimaryFrameProperty_Default:
        return 0xFF000000;
        break;
    case PrimaryFrameProperty_SlashBack:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimarySlashback;
        break;
    case PrimaryFrameProperty_InvulnFull:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryInvulnFull;
        break;
    case PrimaryFrameProperty_InvulnThrow:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryInvulnThrow;
        break;
    case PrimaryFrameProperty_InvulnStrike:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryInvulnStrike;
        break;
    case PrimaryFrameProperty_Armor:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryArmor;
        break;
    case PrimaryFrameProperty_Parry:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryParry;
        break;
    case PrimaryFrameProperty_GuardPointFull:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryGuardPointFull;
        break;
    case PrimaryFrameProperty_GuardPointHigh:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryGuardPointHigh;
        break;
    case PrimaryFrameProperty_GuardPointLow:
        return g_interfaces.frameMeterInterface.palettes.colorFM_PrimaryGuardPointLow;
        break;
    case PrimaryFrameProperty_TEST:
        return 0xFFFFFF00;
        break;
    }
    return 0x00000000;
}

unsigned int FrameMeterInterface::GetSecondaryPropertyColor(SecondaryFrameProperty_ prop2)
{
    switch (prop2)
    {
    case SecondaryFrameProperty_Default:
        return 0xFF000000;
        break;
    case SecondaryFrameProperty_FRC:
        return g_interfaces.frameMeterInterface.palettes.colorFM_SecondaryFRC;
        break;
    }
    return 0x00000000;
}