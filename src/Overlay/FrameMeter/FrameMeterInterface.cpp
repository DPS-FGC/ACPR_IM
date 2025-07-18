#include "Overlay/FrameMeter/FrameMeterInterface.h"
#include"Core/interfaces.h"

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
        g_interfaces.Player1.GetData()->mark
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
        g_interfaces.Player2.GetData()->mark
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
                HitboxReader::getHitboxes(pEntity)
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
        return 0xFF1B1B1B;
        break;
    case FrameType_Movement:
        return 0xFF41F8FC;
        break;
    case FrameType_CounterHitState:
        return 0xFF01B597;
        break;
    case FrameType_Startup:
        return 0xFF01B597;
        break;
    case FrameType_Active:
        return 0xFFCB2B67;
        break;
    case FrameType_ActiveThrow:
        return 0xFFCB2B67;
        break;
    case FrameType_Recovery:
        return 0xFF006FBC;
        break;
    case FrameType_BlockStun:
        return 0xFFC8C800;
        break;
    case FrameType_HitStun:
        return 0xFFC8C800;
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
        return 0xFFFF0000;
        break;
    case PrimaryFrameProperty_InvulnFull:
        return 0xFFFFFFFF;
        break;
    case PrimaryFrameProperty_InvulnThrow:
        return 0xFFFF7D00;
        break;
    case PrimaryFrameProperty_InvulnStrike:
        return 0xFF007DFF;
        break;
    case PrimaryFrameProperty_Armor:
    case PrimaryFrameProperty_Parry:
    case PrimaryFrameProperty_GuardPointFull:
    case PrimaryFrameProperty_GuardPointHigh:
    case PrimaryFrameProperty_GuardPointLow:
        return 0xFF785000;
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
        return 0xFFFFFF00;
        break;
    }
    return 0x00000000;
}