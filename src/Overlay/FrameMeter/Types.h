#pragma once
#include <cstdint>
struct ActionStateFlags
{
private:
    uint32_t _flags;
public:
    ActionStateFlags(uint32_t flags) { _flags = flags; }
    ActionStateFlags(int flags) { _flags = (uint32_t)flags; }
    inline bool IsEntity() const { return (_flags & 0x0001) > 0; }
    inline bool IsPlayer1() const { return (_flags & 0x0002) > 0; }
    inline bool IsPlayer2() { return (_flags & 0x0004) > 0; }
    inline bool DrawSprite() const { return (_flags & 0x0008) > 0; }
    inline bool IsAirborne() const { return (_flags & 0x0010) > 0; }
    inline bool IsInHitstun() const { return (_flags & 0x0020) > 0; }
    inline bool DisableHitboxes() const { return (_flags & 0x0040) > 0; }
    inline bool DisableHurtboxes() const { return (_flags & 0x0080) > 0; }
    inline bool KnockedDown() const { return (_flags & 0x0100) > 0; }
    inline bool IsInBlockstun() const { return (_flags & 0x0200) > 0; }
    inline bool IsCrouching() const { return (_flags & 0x0400) > 0; }
    inline bool IsCornered() const { return (_flags & 0x0800) > 0; }
    inline bool LandingFlag() const { return (_flags & 0x1000) > 0; }
    inline bool IsAtScreenLimit() const { return (_flags & 0x2000) > 0; }
    inline bool ProjDisableHitboxes() const { return (_flags & 0x4000) > 0; }
    inline bool IsPushboxType1() const { return (_flags & 0x8000) > 0; }
    inline bool StayKnockedDown() const { return (_flags & 0x00010000) > 0; }
    inline bool StrikeInvuln() const { return (_flags & 0x00020000) > 0; }
    inline bool IsIdle() const { return (_flags & 0x00040000) > 0; }
    inline bool Freeze() const { return (_flags & 0x00080000) > 0; }
    inline bool NoCollision() const { return (_flags & 0x00100000) > 0; }
    inline bool Gravity() const { return (_flags & 0x00200000) > 0; }
    inline bool Unknown0x00400000() const { return (_flags & 0x00400000) > 0; }
    inline bool IsThrowInuvln() const { return (_flags & 0x00800000) > 0; }
    inline bool Unknown0x01000000() const { return (_flags & 0x01000000) > 0; }
    inline bool Unknown0x02000000() const { return (_flags & 0x02000000) > 0; }
    inline bool IgnoreHitEffectsRecieved() const { return (_flags & 0x04000000) > 0; }

    explicit operator uint32_t() const {
        return _flags;
    }

    explicit operator int() const {
        return (int)(_flags);
    }
};

struct GuardStateFlags
{
private:
    uint16_t _flags;
public:
    GuardStateFlags(uint16_t flags) { _flags = flags; }
    inline bool IsStandBlocking() const { return (_flags & 0x01) > 0; }
    inline bool IsCrouchBlocking() const { return (_flags & 0x02) > 0; }
    inline bool IsAirBlocking() const { return (_flags & 0x04) > 0; }
    inline bool IsFD() const { return (_flags & 0x08) > 0; }
    inline bool IsInBlockStun() const { return (_flags & 0x40) == 0; }
    inline bool GuardPoint() const { return (_flags & 0x0200) > 0; }
    inline bool Armor() const { return (_flags & 0x0400) > 0; }
    inline bool Parry1() const { return (_flags & 0x0800) > 0; }
    inline bool Parry2() const { return (_flags & 0x1000) > 0; }
    inline bool Unknownx4000() const { return (_flags & 0x4000) > 0; }
};

struct CommandFlags
{
private:
    uint32_t _flags;
public:
    CommandFlags(uint32_t flags) { _flags = flags; }

    inline bool IsIdle() const { return (_flags & 0xFFFF) == 0x0101; }
    //inline bool TurningAround () const { return (_flags & 0x0000FFFF) == 0x0109; }
    inline bool IsMove() const { return (_flags & 0xFFFF) == 0xC05F; }   // Has this value when any commital action is performed (?)
    inline bool FreeCancel() const { return (_flags & 0xFFFF) == 0xC01F; }    // Has this value when in free cancelable portion of taunt and airdash animation
    inline bool RunDash() const { return (_flags & 0xFFFF) == 0xE00F; }
    inline bool StepDash() const { return (_flags & 0xFFFF) == 0xE04F; }
    inline bool RunDashSkid() const { return (_flags & 0xFFFF) == 0xC00F; }
    inline bool FaustCrawlForward() const { return (_flags & 0xF000) == 0x4000; }
    inline bool FaustCrawlBackward() const { return (_flags & 0xF000) == 0x8000; }

    inline bool NoNeutral() const { return (_flags & 0x0001) > 0; }
    inline bool NoForward() const { return (_flags & 0x0002) > 0; }
    inline bool NoBackward() const { return (_flags & 0x0004) > 0; }
    inline bool NoCrouching() const { return (_flags & 0x0008) > 0; }
    inline bool NoAttack() const { return (_flags & 0x0040) > 0; }
    inline bool Unknown0x0100() const { return (_flags & 0x0100) > 0; }
    inline bool Unknown0x0200() const { return (_flags & 0x0200) > 0; }
    inline bool Airdash() const { return (_flags & 0x0400) > 0; }
    inline bool Ukemi() const { return (_flags & 0x0800) > 0; }
    inline bool Prejump() const { return (_flags & 0x1000) > 0; }
    inline bool DisableThrow() const { return (_flags & 0x2000) > 0; }
};

struct AttackStateFlags
{
private:
    uint32_t _flags;
public:
    AttackStateFlags(uint32_t flags) { _flags = flags; }
    inline bool IsAttack() const { return (_flags & 0x0001) > 0; }
    inline bool IsInGatlingWindow() const { return (_flags & 0x0010) > 0; }
    inline bool SpecialCancelOkay() const { return (_flags & 0x0020) > 0; }
    inline bool UnknownDustFlag1() const { return (_flags & 0x0080) > 0; }
    inline bool HomingJumpOkay() const { return (_flags & 0x0100) > 0; }
    inline bool KaraFDOkay() const { return (_flags & 0x0200) > 0; }
    inline bool NoSpecialCancel() const { return (_flags & 0x0400) > 0; }
    inline bool IsInRecovery() const { return (_flags & 0x0800) > 0; }
    inline bool HasConnected() const { return (_flags & 0x1000) > 0; }
    inline bool IsJumpCancelable() const { return (_flags & 0x00040000) > 0; }
    inline bool HasHitOpponent() const { return (_flags & 0x00080000) > 0; }
};

struct ThrowFlags
{
private:
    uint8_t _flags;

public:
    ThrowFlags(uint8_t flags) { _flags = flags; }
    ThrowFlags(int flags) { _flags = (uint8_t)flags; }

    inline bool Player1ThrowSuccess() const { return (_flags & 0x1) > 0; }
    inline bool Player2ThrowSuccess() const { return (_flags & 0x2) > 0; }
    inline bool Player2Throwable() const { return (_flags & 0x4) > 0; }
    inline bool Player1Throwable() const { return (_flags & 0x8) > 0; }
    inline bool Player1CommandThrowSuccess() const { return (_flags & 0x10) > 0; }
    inline bool Player2CommandThrowSuccess() const { return (_flags & 0x20) > 0; }
};

struct FM_GlobalFlags
{
    ThrowFlags ThrowFlags = 0;

    int P1CommandGrabRange = 0;
    int P2CommandGrabRange = 0;
};

struct FM_PlayerExtra
{
    uint16_t ThrowProtectionTimer = 0;
    uint8_t InvulnCounter = 0;
    uint8_t RCTime = 0;
    uint8_t SBTime = 0;
    uint8_t JamParryTime = 0;
};

struct FM_Player
{
    uint16_t CharId = 0;
    ActionStateFlags Status = 0;
    GuardStateFlags GuardFlags = 0;
    CommandFlags CommandFlags = 0;
    AttackStateFlags AttackFlags = 0;
    uint16_t ActionId = 0;
    uint16_t AnimationCounter = 0;
    FM_PlayerExtra Extra;
    uint8_t HitstopCounter = 0;
    std::vector<Hitbox> HitboxSet;
    uint8_t Mark = 0;
};

struct FM_Entity
{
    ActionStateFlags Status = 0;
    uint8_t PlayerIndex = 0;
    std::vector<Hitbox> HitboxSet;
};

struct FM_GameState
{
    FM_GameState() {};
    FM_GameState(FM_Player player1, FM_Player player2, std::vector<FM_Entity> entities, FM_GlobalFlags globalFlags) {
        Player1 = player1;
        Player2 = player2;
        Entities = entities;
        GlobalFlags = globalFlags;
    }
    FM_Player Player1;
    FM_Player Player2;
    std::vector<FM_Entity> Entities;
    FM_GlobalFlags GlobalFlags;
};