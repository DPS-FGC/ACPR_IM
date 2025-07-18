#pragma once
#include"Overlay/FrameMeter/FrameMeter.h"

struct LegendEntry
{
    LegendEntry(FM_Frame frame, std::string label) {
        ExampleFrame = frame;
        Label = label;
    }
    FM_Frame ExampleFrame;
    std::string Label = "";
};

class FrameMeterInterface
{
public:
    FrameMeter frameMeter;
    FM_Settings settings;
    std::vector<LegendEntry> entries =
    {
        LegendEntry(FM_Frame(FrameType_Neutral), "Neutral"),
        LegendEntry(FM_Frame(FrameType_Movement), "Movement"),
        LegendEntry(FM_Frame(FrameType_Startup), "Startup/CH"),
        LegendEntry(FM_Frame(FrameType_Active), "Active"),
        LegendEntry(FM_Frame(FrameType_Recovery), "Recovery"),
        LegendEntry(FM_Frame(FrameType_BlockStun), "Block/Hit Stun"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_InvulnFull), "Full Invuln"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_InvulnStrike), "Strike Invuln"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_InvulnThrow), "Throw Invuln"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_Armor), "Armor/Parry/Guard point"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_SlashBack), "Slashback"),
        LegendEntry(FM_Frame(FrameType_None, PrimaryFrameProperty_Default, PrimaryFrameProperty_Default, SecondaryFrameProperty_FRC), "FRC")
    };

    unsigned int GetFrameColor(FrameType_ type);
    unsigned int GetPrimaryPropertyColor(PrimaryFrameProperty_ prop);
    unsigned int GetSecondaryPropertyColor(SecondaryFrameProperty_ prop2);

    FM_GameState GetGameState();
    void UpdateFastForward();
    void Update();
    void Reset();

private:
    int last_frame_count = -1;
    bool rewinding = false;
};