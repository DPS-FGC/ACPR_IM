#pragma once
#include <cstdint>
#include<string>
#include<vector>
#include<array>
#include"Game/Camera.h"
#include"Game/Player.h"
#include"Game/Hitbox/HitboxReader.h"
#include<unordered_map>
#include<Overlay/FrameMeter/Types.h>
#include<Overlay/FrameMeter/MoveData.h>
#include<stack>

#define METER_LENGTH 80
#define PAUSE_THRESHOLD 10

enum FrameType_
{
    FrameType_None,
    FrameType_Neutral,
    FrameType_Movement,
    FrameType_CounterHitState,
    FrameType_Startup,
    FrameType_Active,
    FrameType_ActiveThrow,
    FrameType_Recovery,
    FrameType_BlockStun,
    FrameType_HitStun
};

enum PrimaryFrameProperty_
{
    PrimaryFrameProperty_Default,
    PrimaryFrameProperty_InvulnFull,
    PrimaryFrameProperty_InvulnStrike,
    PrimaryFrameProperty_InvulnThrow,
    PrimaryFrameProperty_Parry,
    PrimaryFrameProperty_GuardPointFull,
    PrimaryFrameProperty_GuardPointHigh,
    PrimaryFrameProperty_GuardPointLow,
    PrimaryFrameProperty_Armor,
    PrimaryFrameProperty_FRC,
    PrimaryFrameProperty_SlashBack,
    PrimaryFrameProperty_TEST
};

enum SecondaryFrameProperty_
{
    SecondaryFrameProperty_Default,
    SecondaryFrameProperty_FRC
};

struct FM_Settings
{
    bool DisplayHitboxes = true;
    bool DisplayFrameMeter = true;
    bool AlwaysDrawThrowRange = false;
    bool DisplayFrameMeterLegend = false;
    bool RecordDuringHitstop = false;
    bool RecordDuringSuperFlash = false;
    bool AdvancedMode = false;
    bool AllowRewindingInReplay = false;
};

struct FM_Frame
{
    FM_Frame() {}
    FM_Frame(FrameType_ type) { Type = type; }
    FM_Frame(FrameType_ type, PrimaryFrameProperty_ pprop1) { Type = type; PrimaryProperty1 = pprop1; }
    FM_Frame(FrameType_ type, PrimaryFrameProperty_ pprop1, PrimaryFrameProperty_ pprop2, SecondaryFrameProperty_ prop2)
    {
        Type = type;
        PrimaryProperty1 = pprop1;
        PrimaryProperty2 = pprop2;
        SecondaryProperty = prop2;
    }
    FM_Frame(FrameType_ type, PrimaryFrameProperty_ pprop1, PrimaryFrameProperty_ pprop2, SecondaryFrameProperty_ prop2,
        uint16_t actId, uint16_t actTimer, uint8_t hitstop, uint32_t status) {
        Type = type;
        PrimaryProperty1 = pprop1;
        PrimaryProperty2 = pprop2;
        SecondaryProperty = prop2;
        ActId = actId;
        ActTimer = actTimer;
        HitStop = hitstop;
        Status = ActionStateFlags(status);
    }
    FrameType_ Type = FrameType_None;
    PrimaryFrameProperty_ PrimaryProperty1 = PrimaryFrameProperty_Default;
    PrimaryFrameProperty_ PrimaryProperty2 = PrimaryFrameProperty_Default;
    SecondaryFrameProperty_ SecondaryProperty = SecondaryFrameProperty_Default;
    uint16_t ActId = -1;
    uint16_t ActTimer = -1;
    uint8_t HitStop = 0;
    ActionStateFlags Status = 0x00000000;
};

struct FM_Meter
{
    std::string Label = "";
    int Startup = -1;
    int LastAttackActId = -1;
    int Total = -1;
    int Advantage = 0;
    bool DisplayAdvantage = false;
    bool Hide = false;
    std::array<FM_Frame, METER_LENGTH> FrameArr;

    FM_Meter() {}
    FM_Meter(std::string name) { Label = name; }
};

struct FM_FrameState
{
    FM_Frame playerStates[2];
    FM_Frame entityStates[2];

    FM_FrameState::FM_FrameState() {};

    FM_FrameState::FM_FrameState(FM_Frame player1Frame, FM_Frame player2Frame, FM_Frame entity1Frame, FM_Frame entity2Frame)
    {
        playerStates[0] = player1Frame;
        playerStates[1] = player2Frame;
        entityStates[0] = entity1Frame;
        entityStates[1] = entity2Frame;
    }
};

struct FM_MeterState
{
    std::array<FM_FrameState, METER_LENGTH> meterArray;
    int _index;

    FM_MeterState::FM_MeterState() {}

    FM_MeterState::FM_MeterState(std::array<FM_FrameState, METER_LENGTH> arr, int index) 
    {
        meterArray = arr;
        _index = index;
    }

};

class FrameMeter
{
private:
    int _index;
    bool _isPaused = true;

    FM_GameState prevState;

    void ClearMeters();
    void ClearMeter(FM_Meter* m, bool hide);
    static FrameType_ DetermineFrameType(FM_GameState state, int index);
    static std::array<PrimaryFrameProperty_, 2> DeterminePrimaryFrameProperties(FM_GameState state, int index);
    static FrameType_ DetermineEntityFrameType(FM_GameState state, int index);
    void UpdateIndividualMeter(FM_GameState state, int index);
    void UpdateIndividualEntityMeter(FM_GameState state, int index);
    void UpdateAdvantageByCountBack();
    void AdvCountBackFromPlayer(FM_Meter* pMeterA, FM_Meter* pMeterB);
    void UpdateStartupByCountBackWithMoveData(FM_Player p, FM_Meter* pMeter);
    FM_Frame FrameAtOffset(FM_Meter* meter, int offset);
    int AddToLoopingIndex(int offset);
    bool PlayerHasActiveFrame(FM_Player player);

    std::stack<FM_MeterState> recordedFrames;

public:
    FM_Meter PlayerMeters[2];
    FM_Meter EntityMeters[2];

    FrameMeter();
    int Update(FM_GameState state, FM_Settings settings);
    void Reset();
    void PushMeterState();
    void PopMeterState();
    int GetNumberOfRecordedFrames();
    void ResetRecordedFrames();
};

