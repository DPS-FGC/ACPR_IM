#pragma once
#include <vector>
#include<unordered_map>
#include <functional>

struct CharIdActIdKey
{
    uint16_t charId;
    uint16_t actId;

    bool operator==(const CharIdActIdKey& other) const {
        return charId == other.charId && actId == other.actId;
    }
};

template <>
struct std::hash<CharIdActIdKey> {
    size_t operator()(const CharIdActIdKey& key) const noexcept {
        // Simple hash combination
        return (static_cast<size_t>(key.charId) << 16) ^ key.actId;
    }
};

struct MoveEntry
{
    int sequenceIndex;
    int moveId;
};

struct CommandGrabData
{
    uint16_t CharId = 0;
    uint16_t ActId = 0;
    uint16_t CmdGrabId = 0;
};

struct MoveData
{
private:
    static std::vector<CommandGrabData> _activeByMarkCommandGrabs;
    static std::unordered_map<CharIdActIdKey, std::vector<MoveEntry>> actIdToMoveIds;
public:
    static void Initialize();
    static bool IsPrevAnimSameMove(uint16_t charId, uint16_t actId1, uint16_t actId2);
    static bool IsActiveByMark(uint16_t charId, uint16_t actId);
    static int GetCommandGrabRange(uint16_t charId, uint16_t actId);
};