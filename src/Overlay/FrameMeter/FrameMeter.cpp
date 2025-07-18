#include "Overlay/FrameMeter/FrameMeter.h"
FrameMeter::FrameMeter()
{
	_index = 0;
	PlayerMeters[0] = FM_Meter("Player 1");
	PlayerMeters[1] = FM_Meter("Player 2");
	EntityMeters[0] = FM_Meter("P1 Sub");
	EntityMeters[1] = FM_Meter("P2 Sub");
	ClearMeters();
}

int FrameMeter::Update(FM_GameState state, FM_Settings settings)
{
    FM_Frame p1PrevFrame = FrameAtOffset(&PlayerMeters[0], -1);
    FM_Frame p2PrevFrame = FrameAtOffset(&PlayerMeters[1], -1);

    // Skip update if either player is frozen in super flash while the opponent doesn't have an active hitbox.
    // The hitbox requirement is for moves that become active while in super flash.
    // Special exception for first freeze frame.
    if ((state.Player1.Status.Freeze() || state.Player2.Status.Freeze()) &&
        (p1PrevFrame.Status.Freeze() || p2PrevFrame.Status.Freeze()) &&
        !settings.RecordDuringSuperFlash)
    {
        // TODO: account for projectile supers that hit during flash

        // Special case for super's that connect during super flash (e.g. Jam 632146S)
        // Rewrite the previous frame to an active frame and recalculate startup
        if (state.Player2.Status.Freeze() && PlayerHasActiveFrame(state.Player1))
        {
            PlayerMeters[0].FrameArr[AddToLoopingIndex(-1)] = FM_Frame(
                FrameType_Active, p1PrevFrame.PrimaryProperty1, p1PrevFrame.PrimaryProperty2, p1PrevFrame.SecondaryProperty, p1PrevFrame.ActId,
                p1PrevFrame.ActTimer, p1PrevFrame.HitStop, (uint32_t)p1PrevFrame.Status);

            _index = AddToLoopingIndex(-1);
            UpdateStartupByCountBackWithMoveData(state.Player1, &PlayerMeters[0]);
            _index = AddToLoopingIndex(1);
        }
        else if (state.Player1.Status.Freeze() && PlayerHasActiveFrame(state.Player2))
        {
            PlayerMeters[1].FrameArr[AddToLoopingIndex(-1)] = FM_Frame(
                FrameType_Active, p2PrevFrame.PrimaryProperty1, p1PrevFrame.PrimaryProperty2, p2PrevFrame.SecondaryProperty, p2PrevFrame.ActId,
                p2PrevFrame.ActTimer, p2PrevFrame.HitStop, (uint32_t)p2PrevFrame.Status);

            _index = AddToLoopingIndex(-1);
            UpdateStartupByCountBackWithMoveData(state.Player2, &PlayerMeters[1]);
            _index = AddToLoopingIndex(1);
        }

        prevState = state;
        return 0;
    }
    // Skip update when both players are in hitstop (currently somewhat redundant when checking for unchanged animation timers above)
    // Special exception to also skip the first frame after hitstop counters have ended
    // Super freeze often uses the histop counter as well so need to except that situation
    if ((state.Player1.HitstopCounter > 0 && state.Player2.HitstopCounter > 0 ||
        prevState.Player1.HitstopCounter > 0 && prevState.Player2.HitstopCounter > 0) &&
        !(state.Player1.Status.Freeze() || state.Player2.Status.Freeze()) &&
        !settings.RecordDuringHitstop)
    {
        prevState = state;
        return 0;
    }


    // Pause logic
    if (_isPaused)
    {
        if ((DetermineFrameType(state, 0) == FrameType_Neutral) &&
            (DetermineFrameType(state, 1) == FrameType_Neutral) &&
            (DetermineEntityFrameType(state, 0) == FrameType_None) &&
            (DetermineEntityFrameType(state, 1) == FrameType_None))
        {
            prevState = state;
            return 0;
        }
        else
        {
            _isPaused = false;
            _index = 0;
            ClearMeters();
        }
    }

    // Update each meter
    UpdateIndividualMeter(state, 0);
    UpdateIndividualMeter(state, 1);
    UpdateIndividualEntityMeter(state, 0);
    UpdateIndividualEntityMeter(state, 1);

    // Check if frame meter should pause
    // TODO: account for frame properties?
    _isPaused = true;
    FrameType_ p1FrameType, p2FrameType;
    for (int i = 0; i < PAUSE_THRESHOLD; i++)
    {
        p1FrameType = FrameAtOffset(&PlayerMeters[0], -i).Type;
        p2FrameType = FrameAtOffset(&PlayerMeters[1], -i).Type;

        if ((p1FrameType != FrameType_Neutral && p1FrameType != FrameType_None) ||
            (p2FrameType != FrameType_Neutral && p2FrameType != FrameType_None) ||
            FrameAtOffset(&EntityMeters[0], -i).Type != FrameType_None ||
            FrameAtOffset(&EntityMeters[1], -i).Type != FrameType_None)
        {
            _isPaused = false;
            break;
        }
    }

    // Labels
    UpdateStartupByCountBackWithMoveData(state.Player1, &PlayerMeters[0]);
    UpdateStartupByCountBackWithMoveData(state.Player2, &PlayerMeters[1]);
    UpdateAdvantageByCountBack();

    _index = (_index + 1) % METER_LENGTH;
    prevState = state;
    return 0;
}

void FrameMeter::Reset()
{
    _index = 0;
    ClearMeters();
}

void FrameMeter::ClearMeters()
{
    ClearMeter(&PlayerMeters[0], false);
    ClearMeter(&PlayerMeters[1], false);
    ClearMeter(&EntityMeters[0], true);
    ClearMeter(&EntityMeters[1], true);
}

void FrameMeter::ClearMeter(FM_Meter* m, bool hide)
{
    for (int i = 0; i < METER_LENGTH; i++)
    {
        m->FrameArr[i] = FM_Frame();
    }
    m->Startup = -1;
    m->LastAttackActId = -1;
    m->Advantage = -1;
    m->Total = -1;
    m->DisplayAdvantage = false;
    m->Hide = hide;
}

FrameType_ FrameMeter::DetermineFrameType(FM_GameState state, int index)
{
    FM_Player player = index == 0 ? state.Player1 : state.Player2;
    int cmdGrabId = index == 0 ? state.GlobalFlags.P1CommandGrabRange : state.GlobalFlags.P2CommandGrabRange;

    bool hasHitbox = false;
    for (int i = 0; i < player.HitboxSet.size(); i++)
    {
        if (player.HitboxSet[i].type == HitboxType_Hitbox)
            hasHitbox = true;
    }

    if (player.Mark == 1 && MoveData::IsActiveByMark(player.CharId, player.ActionId))
    {
        return FrameType_ActiveThrow;
    }
    else if ((state.GlobalFlags.ThrowFlags.Player1ThrowSuccess() && index == 0 &&
        state.Player2.Status.IsInHitstun() ||
        state.GlobalFlags.ThrowFlags.Player2ThrowSuccess() && index == 1 &&
        state.Player1.Status.IsInHitstun()) &&
        !player.CommandFlags.DisableThrow())
    {
        return FrameType_ActiveThrow;
    }
    else if (hasHitbox && !player.Status.DisableHitboxes())
    {
        return FrameType_Active;
    }
    else if (player.Status.IsInBlockstun() || player.Extra.SBTime > 0)
    {
        return FrameType_BlockStun;
    }
    else if (player.Status.IsInHitstun())
    {
        return FrameType_HitStun;
    }
    else if (player.AttackFlags.IsInRecovery())
    {
        return FrameType_Recovery;
    }
    else if (player.CommandFlags.IsMove() && !player.AttackFlags.IsInRecovery())
    {
        return FrameType_CounterHitState;
    }
    else if (player.CommandFlags.IsMove())
    {
        return FrameType_Startup;
    }
    else if (player.CommandFlags.Prejump() ||
        player.CommandFlags.FreeCancel() || player.CommandFlags.RunDash() ||
        player.CommandFlags.StepDash() || player.CommandFlags.RunDashSkid())
    {
        return FrameType_Movement;
    }
    else
    {
        return FrameType_Neutral;
    }
}

#define CharId_Jam 12
#define CharId_Axl 5
#define CharId_Dizzy 16
#define AXL_TENHOU_SEKI_UPPER_ACT_ID 188
#define AXL_TENHOU_SEKI_LOWER_ACT_ID 189
#define DIZZY_EX_NECRO_UNLEASHED_ACT_ID 247
std::array<PrimaryFrameProperty_, 2> FrameMeter::DeterminePrimaryFrameProperties(FM_GameState state, int index)
{
    std::array<PrimaryFrameProperty_, 2> output = { PrimaryFrameProperty_Default, PrimaryFrameProperty_Default };

    FM_Player p = state.Player1;
    if (index > 0) p = state.Player2;

    index = 0;

    if (p.Extra.SBTime > 0)
    {
        output[index] = PrimaryFrameProperty_SlashBack;
        index = (index + 1) % 2;
    }

    bool hasHurbox = false;
    for (int i = 0; i < p.HitboxSet.size(); i++)
    {
        if (p.HitboxSet[i].type == HitboxType_Hurtbox)
            hasHurbox = true;
    }

    if ((p.Status.DisableHurtboxes() ||
        p.Status.StrikeInvuln() ||
        p.Extra.InvulnCounter > 0 ||
        !(hasHurbox || p.Status.ProjDisableHitboxes())) &&
        (p.Status.IsThrowInuvln() ||
            p.Extra.ThrowProtectionTimer > 0))
    {
        output[index] = PrimaryFrameProperty_InvulnFull;
        index = (index + 1) % 2;
    }
    else if (p.Status.IsThrowInuvln() ||
        (p.Extra.ThrowProtectionTimer > 0 &&
            !(p.Status.IsInHitstun() || p.Status.IsInBlockstun())))
    {
        output[index] = PrimaryFrameProperty_InvulnThrow;
        index = (index + 1) % 2;
    }
    else if (p.Status.DisableHurtboxes() ||
        p.Status.StrikeInvuln() ||
        p.Extra.InvulnCounter > 0 ||
        !hasHurbox ||
        p.Status.ProjDisableHitboxes())
    {
        output[index] = PrimaryFrameProperty_InvulnStrike;
    }

    if (p.GuardFlags.Armor())
    {
        output[index] = PrimaryFrameProperty_Armor;
        index = (index + 1) % 2;
    }
    else if (p.GuardFlags.Parry1() || p.GuardFlags.Parry2())
    {
        if (p.CharId == CharId_Jam)
        {
            if (p.Extra.JamParryTime == 0xFF && (p.GuardFlags.IsStandBlocking() || p.GuardFlags.IsCrouchBlocking()))
            {
                output[index] = PrimaryFrameProperty_Parry;
                index = (index + 1) % 2;
            }
        }
        else if ((p.CharId == CharId_Axl && p.ActionId == AXL_TENHOU_SEKI_UPPER_ACT_ID) ||
            (p.CharId == CharId_Axl && p.ActionId == AXL_TENHOU_SEKI_LOWER_ACT_ID) ||
            (p.CharId == CharId_Dizzy && p.ActionId == DIZZY_EX_NECRO_UNLEASHED_ACT_ID))
        {
            // These moves are marked as in parry state for their full animation and use a special move specific
            //  variable (Player.Mark) to actually determine if the move should parry.
            if (p.Mark == 1)
            {
                output[index] = PrimaryFrameProperty_Parry;
                index = (index + 1) % 2;
            }
        }
        else
        {
            output[index] = PrimaryFrameProperty_Parry;
            index = (index + 1) % 2;
        }
    }
    else if (p.GuardFlags.GuardPoint())
    {
        if (p.GuardFlags.IsStandBlocking() && p.GuardFlags.IsCrouchBlocking())
        {
            output[index] = PrimaryFrameProperty_GuardPointFull;
            index = (index + 1) % 2;
        }
        else if (p.GuardFlags.IsStandBlocking())
        {
            output[index] = PrimaryFrameProperty_GuardPointHigh;
            index = (index + 1) % 2;
        }
        else if (p.GuardFlags.IsCrouchBlocking())
        {
            output[index] = PrimaryFrameProperty_GuardPointLow;
            index = (index + 1) % 2;
        }
    }

    return output;
}

FrameType_ FrameMeter::DetermineEntityFrameType(FM_GameState state, int index)
{
    int numHitboxes = 0;
    int numHurtboxes = 0;

    for (int i = 0; i < state.Entities.size(); i++)
    {
        if (state.Entities[i].PlayerIndex != index)
            continue;
        for (int j = 0; j < state.Entities[i].HitboxSet.size(); j++)
        {
            if (!state.Entities[i].Status.DisableHitboxes() && state.Entities[i].HitboxSet[j].type == HitboxType_Hitbox)
                numHitboxes++;
            if (!state.Entities[i].Status.DisableHitboxes() && state.Entities[i].HitboxSet[j].type == HitboxType_Hitbox)
                numHurtboxes++;
        }
    }

    if (numHitboxes > 0)
        return FrameType_Active;
    if (numHurtboxes > 0)
        return FrameType_Startup;
    return FrameType_None;
}

void FrameMeter::UpdateIndividualMeter(FM_GameState state, int index)
{
    FM_Player players[] = { state.Player1, state.Player2 };

    FrameType_ type = DetermineFrameType(state, index);
    std::array<PrimaryFrameProperty_, 2> pprops = DeterminePrimaryFrameProperties(state, index);
    SecondaryFrameProperty_ prop2 = SecondaryFrameProperty_Default;

    if (players[index].Extra.RCTime > 0)
    {
        prop2 = SecondaryFrameProperty_FRC;
    }

    PlayerMeters[index].FrameArr[_index] = FM_Frame(type, pprops[0], pprops[1], prop2,
        players[index].ActionId, players[index].AnimationCounter, players[index].HitstopCounter, (uint32_t)players[index].Status);
    PlayerMeters[index].FrameArr[(_index + 2 + METER_LENGTH) % METER_LENGTH] = FM_Frame(); // Forward erasure
}

void FrameMeter::UpdateIndividualEntityMeter(FM_GameState state, int index)
{
    FrameType_ type = DetermineEntityFrameType(state, index);

    EntityMeters[index].FrameArr[_index] = FM_Frame(type);
    EntityMeters[index].FrameArr[(_index + 2 + METER_LENGTH) % METER_LENGTH] = FM_Frame();

    // Update hide flag
    EntityMeters[index].Hide = true;
    for (int i = 0; i < EntityMeters[index].FrameArr.size(); i++)
    {
        if (EntityMeters[index].FrameArr[i].Type != FrameType_None)
            EntityMeters[index].Hide = false;
    }
}

void FrameMeter::UpdateAdvantageByCountBack()
{
    AdvCountBackFromPlayer(&PlayerMeters[0], &PlayerMeters[1]);
    AdvCountBackFromPlayer(&PlayerMeters[1], &PlayerMeters[0]);
}

void FrameMeter::AdvCountBackFromPlayer(FM_Meter* pMeterA, FM_Meter* pMeterB)
{
    if (FrameAtOffset(pMeterA, 0).Type == FrameType_Neutral &&
        FrameAtOffset(pMeterA, -1).Type != FrameType_Neutral &&
        FrameAtOffset(pMeterB, 0).Type == FrameType_Neutral)
    {
        for (int i = 1; i < METER_LENGTH; i++)
        {
            if (FrameAtOffset(pMeterB, -i).Type != FrameType_Neutral)
            {
                pMeterA->Advantage = 1 - i;
                pMeterB->Advantage = i - 1;
                pMeterA->DisplayAdvantage = true;
                pMeterB->DisplayAdvantage = true;
                break;
            }
        }
    }
}

void FrameMeter::UpdateStartupByCountBackWithMoveData(FM_Player p, FM_Meter* pMeter)
{
    std::array<FrameType_, 2> activeTypes = { FrameType_Active, FrameType_ActiveThrow };
    std::array<FrameType_, 3> prevFrameTypesAllowed = { FrameType_CounterHitState, FrameType_Startup, FrameType_None };
    FM_Frame currFrame = FrameAtOffset(pMeter, 0);
    FrameType_ prevFrameType = FrameAtOffset(pMeter, -1).Type;

    //if (activeTypes.Contains(currFrame.Type) && prevFrameTypesAllowed.Contains(prevFrameType))
    bool containsCurrentFrameType = false;
    bool containsPreviousFrameType = false;
    for (int i = 0; i < activeTypes.size(); i++)
    {
        if (activeTypes[i] == currFrame.Type)
            containsCurrentFrameType = true;
        if (activeTypes[i] == prevFrameType)
            containsPreviousFrameType = true;
    }

    if (containsCurrentFrameType && !containsPreviousFrameType)
    {
        pMeter->LastAttackActId = currFrame.ActId;
        FM_Frame frame;
        for (int i = 1; i < METER_LENGTH; i++)
        {
            frame = FrameAtOffset(pMeter, -i);
            if (frame.ActId != pMeter->LastAttackActId && !MoveData::IsPrevAnimSameMove(p.CharId, frame.ActId, pMeter->LastAttackActId))
            {
                pMeter->Startup = i;
                break;
            }
        }
    }
}

FM_Frame FrameMeter::FrameAtOffset(FM_Meter* meter, int offset)
{
	return meter->FrameArr[AddToLoopingIndex(offset)];
}

int FrameMeter::AddToLoopingIndex(int offset)
{
	return (_index + offset + METER_LENGTH) % METER_LENGTH;
}

bool FrameMeter::PlayerHasActiveFrame(FM_Player player)
{
    bool hasHitbox = false;
    for (int i = 0; i < player.HitboxSet.size(); i++)
    {
        if (player.HitboxSet[i].type == HitboxType_Hitbox)
        {
            hasHitbox = true;
            break;
        }
    }

    return hasHitbox && !player.Status.DisableHitboxes() ||
        player.Mark == 1 && MoveData::IsActiveByMark(player.CharId, player.ActionId);
}

void FrameMeter::PushMeterState()
{
    std::array<FM_FrameState, METER_LENGTH> state;
    for (int i = 0; i < METER_LENGTH; i++)
    {
        state[i] = FM_FrameState(PlayerMeters[0].FrameArr[i], PlayerMeters[1].FrameArr[i],
            EntityMeters[0].FrameArr[i], EntityMeters[1].FrameArr[i]);
    }
    recordedFrames.push(FM_MeterState(state, _index));
}

void FrameMeter::PopMeterState()
{
    if (recordedFrames.empty())
        return;

    FM_MeterState state = recordedFrames.top();
    for (int i = 0; i < METER_LENGTH; i++)
    {
        PlayerMeters[0].FrameArr[i] = state.meterArray[i].playerStates[0];
        PlayerMeters[1].FrameArr[i] = state.meterArray[i].playerStates[1];
        EntityMeters[0].FrameArr[i] = state.meterArray[i].entityStates[0];
        EntityMeters[1].FrameArr[i] = state.meterArray[i].entityStates[1];
    }
    _index = state._index;
    recordedFrames.pop();
}

void FrameMeter::ResetRecordedFrames()
{
    while (!recordedFrames.empty())
        recordedFrames.pop();
}

int FrameMeter::GetNumberOfRecordedFrames()
{
    return recordedFrames.size();
}