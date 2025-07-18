#pragma once
#include <cstdint>

struct PlayerExtra
{
	uint16_t tension; //0x00. Max 10000.
	uint8_t staggerTime; //0x02. Time remaining before exiting stagger.
	uint8_t backdashInvalid; //0x03. Counts downward during backdash.
	uint16_t baseComboDamage; //0x04. Decreases when character is hit.
	uint16_t dangerTime; //0x06. Start increasing after some time. When reaches 10000, gets negative penalty. Performing active moves will make it decrease back to zero.
	uint16_t counterredTime; //0x08.
	char pad_09[14]; //0x0A.
	uint16_t throwProtectionTimer; //0x18.
	int16_t guardGauge; //0x1A. Guard gauge.
	uint16_t guardTimer; //0x1C. Counts from 5 down to 0 then resets. Doesn't change when being hit.
	int16_t downTimer; //0x1E. Seems to count down the time remaining until character hits the ground.
	char pad_20[3]; //0x20.
	uint8_t hitdispTimer; //0x23. Starts at 120 when hit and counts down. When reaches zero, the enemies combo counter disappears.
	float hitdispPos; //0x24. Value between 0-1 which gives the relative position of the combo counter along its trajectory.
	char pad_28[2]; //0x28.
	uint8_t invTime; //0x2A. Cannot be hit while this is > 0.
	char pad_2B[5]; //0x2B.
	uint16_t tensionPenaltyTimer;//0x30. Reduces tension gain while > 0. Is active after RC or negative penalty.
	uint8_t FRCTimer; //0x32. Can perform FRC when > 0.
	uint8_t ignoreFCTimer; //0x33. Starts counting down from 10 when RC button combination is pressed.
	char pad_34[8]; //0x34.
	uint8_t moveTensionPenalty; //0x3C. Non-zero for moves that move backward (move, jump, backdash).
	uint8_t ibTimer; //0x3D. Counts up after back direction is pressed. If 0 < ibTimer < 9, perform instant block.
	char pad_3D[10]; //0x3E.
	uint16_t pButton; //0x48. Gamepad button used for punch in controller button settings.
	uint16_t kButton; //0x4A. Gamepad button used for kick in controller button settings.
	uint16_t sButton; //0x4C. Gamepad button used for slash in controller button settings.
	uint16_t hButton; //0x4E. Gamepad button used for heavy slash in controller button settings.
	uint16_t dButton; //0x50. Gamepad button used for dust in controller button settings.
	uint16_t tButton; //0x52. Gamepad button used for taunt in controller button settings.
	uint16_t rButton; //0x54. Gamepad button used for reset in controller button settings.
	uint16_t stButton; //0x56. Gamepad button used for start in controller button settings.
	uint16_t rpButton; //0x58. Gamepad button used for record player in controller button settings.
	uint16_t reButton; //0x5A. Gamepad button used for record enemy in controller button settings.
	uint16_t pmButton; //0x5C. Gamepad button used for play memory in controller button settings.
	uint16_t swButton; //0x5E. Gamepad button used for switch in controller button settings.
	uint16_t ewButton; //0x60. Gamepad button used for enemy walk in controller button settings.
	uint16_t ejButton; //0x62. Gamepad button used for enemy jump in controller button settings.
	uint16_t pkButton; //0x64. Gamepad button used for p+k macro in controller button settings.
	uint16_t pdButton; //0x66. Gamepad button used for p+d macro in controller button settings.
	uint16_t pksButton; //0x68. Gamepad button used for p+k+s macro in controller button settings.
	uint16_t pkshButton; //0x6A. Gamepad button used for p+k+s+sh macro in controller button settings.
	char pad_6C[4]; //0x6C.
	uint8_t jumpCounter; //0x70. Counts the number of air jumps available.
	uint8_t airdashCounter; //0x71. Counts the number of air dashes available.
	char pad_72[2]; //0x72.
	uint32_t gatlingFlags; //0x74. Store possible gatlings from current move as different bits.
	char pad_78[16]; //0x78.
	uint16_t characterSLOT1; //0x88. Used for Sol dragon install, A.B.A mode (Moroga or Goku-Moroha), Johnny MF level, Zappa summon (1 - sword, 2 - dog, 3 - ghost, 4 - raou), Venom stinger charge (least significant byte only), Justice install counter, Eddie gauge recovering (most significant byte only), Testament crow attack (when lsb is 5).
	uint16_t characterSLOT2; //0x8A. Used for A.B.A blood packs, Jam Asanagi-K,
	uint16_t characterSLOT3; //0x8C. Used for Raou gauge, Johnny coins (least significant byte only, counts the number of uses, not the remaining ones), Jam Asanagi-HS (least significant byte), Jam Asanagi-S (most significant byte, 0x8D), Robo-Ky heat gauge, Eddie gauge, Testament puppets, Chipp invisibility timer.
	uint16_t characterSLOT4; //0x8E. Used for Chipp shuriken (0 = slow, 1 = fast), Robo-Ky self-destruct timer, Eddie type (regular or vice, most significant bit).
	uint32_t characterSLOT5; //0x90 Used for A.B.A gauge, first byte is non zero when Jam parry is active, Counts number of times Testament's crow attacked
	uint16_t characterSLOT6; //0x94. Used for Testament crow pattern, order-sol flame distortion.
	uint16_t characterSLOT7; //0x96. Used for Order-Sol gauge.
	char pad_98[48]; //0x98.
	int32_t specialFunction; //0xC8. Stores address of function to execute for (Jam parry, potemkin F.D.B projectile, Potemkin hammer fall, Anji parry, etc...)
	char pad_CC[13]; //0xCC.
	uint8_t projectileFlag; //0xD9. Used for Millia silent force (0 = not used, 2 = used) and to denote if projectiles of various characters are active (0 - not active, 1 - active).
	char pad_DA[24]; //0xDA.
	uint8_t jumpSealTimer; //0xF2. Character cannot jump when non zero.
	char pad_F3[1]; //0xF3.
	uint8_t poisonTimer; //0xF4. Character is poisoned when non zero.
	uint8_t mistTimer; //0xF5. Greater than zero when Bacchus sigh is active on player.
	uint8_t damageTime; //0xF6. Counts upwards when character is getting hit, resets to zero when back to neutral.
	uint16_t burstGauge; //0xF8. Burst gauge (max 15000).
	char pad_FA[4]; //0xFA.
	uint8_t hitCount; //0xFE. Counts number of hits in the opponents combo.
	char pad_FF[3]; //0xFF.
	uint8_t dashSealTimer; //0x102. Character cannot run, backdash or air dash when non zero.
	uint8_t allCounterTimer; //0x103. Every hit is a counter hit when non zero.
	uint8_t guardSealTimer; //0x104. Character cannot guard when non zero.
	uint8_t pSealTimer; //0x105. Character cannot punch when non zero.
	uint8_t kSealTimer; //0x106. Character cannot kick when non zero.
	uint8_t sSealTimer; //0x107. Character cannot slash when non zero.
	uint8_t hSealTimer; //0x108. Character cannot heavy slash when non zero.
	uint8_t dSealTimer; //0x109. Character cannot dust when non zero.
	char pad_10A[1]; //0x10A.
	uint8_t sbTimer; //0x10B. If blocking and >0 performs slashback.
	uint8_t sbCooldownTimer; //0x10C. Cannot block while >0. Increases after slashback.
	char pad_10C[5]; //0x10D.
	uint8_t lateThrowFlag; //0x112. When late throw break possible it's value is 0x01, after it's too late it's 0x02. If set to 0x0F during early throw, will result in throw tech.
	char pad_113[53];
};

struct CharData
{
	uint16_t charIndex; //0x00. ID of the character.
	uint8_t facingRight; //0x02. Is player facing right.
	uint8_t onRightSide; //0x03. Is player on the right side of the second player.
	CharData* previousEntity; //0x04.
	CharData* nextEntity; //0x08.
	uint32_t status; //0x0C. Various status flags.
	int16_t pad_10; //0x10. Id of next animation (when different from -1, the animation will change to this value on the next frame).
	int16_t bufferFlags; //0x12.
	int16_t pad_14; //0x14. Id of next animation temp. //Used when transitioning animations via character input.
	int16_t actFlags; //0x16. Flags related to next animation. Some examples are 0x0400 is active for supers, 0x0200 for specials, 0x0008 for all attacks, 0x0003 when airborne, etc...
	uint16_t actId; //0x18. Animation ID.
	uint16_t entityIndex; //0x1A. Stores index of Zappa's orbs (when the entity is an orb).
	uint16_t frameCounter; //0x1C. Number of frames spent in current animation.
	uint16_t currentHP; //0x1E.
	CharData* parent; //0x20.
	int16_t pad_24; //0x24. Something to do with jumps.
	int8_t pad_26; //0x26.
	uint8_t playerID; //0x27. 0 - for player 1, 1 - for player 2.
	uint16_t parentFlag; //0x28.
	uint16_t blockType; //0x2A. Blocking flags.
	PlayerExtra* extraData; //0x2C. Pointer to struct hosting extra character data.
	int32_t pad_30; //0x30.
	uint32_t attackFlags; //0x34.
	uint16_t commandFlags; //0x38.
	int16_t pad_3A; //0x3A.
	uint32_t pad_3C; //0x3C. Pointer to some addresses (possibly a double pointer).
	uint16_t frameIdx; //0x40. Index of currently drawn frame (sprite).
	int16_t pad_42; //0x42.
	uint16_t frameIdx2; //0x44. Same as frameIdx.
	uint16_t frameIdx3; //0x46. Same as frameIdx.
	uint32_t pad_48; //0x48. Pointer to something.
	int16_t coreX; //0x4C.
	int16_t coreY; //0x4E.
	int16_t scaleX; //0x50.
	int16_t scaleY; //0x52.
	void* hitboxArray; //0x54.
	char pad_58[4]; //0x58.
	int8_t colliflag; //0x5C. Active during hitbox-hurtbox collisions.
	int8_t hitcolliflag; //0x5D. Active during hitbox-hurtbox collisions.
	int8_t damageflag; //0x5E. Active during hitbox-hurtbox collisions.
	char pad_5F[25]; //0x5F.
	uint32_t enemyAttackFlags; //0x78. Stores properties of enemies attack when attack connects. For example if it is a low or overhead.
	char pad_7C[8]; //0x7C.
	uint8_t numHitbox; //0x84. Number of hitboxes in array.
	uint8_t idxHitbox; //0x85.
	char pad_86[42]; //0x86.
	int32_t posX; //0xB0.
	int32_t posY; //0xB4.
	char pad_B8[68]; //0xB8.
	int8_t pad_FC; //0xFC.
	uint8_t hitstopTimer; //0xFD.
	int8_t totalBlockstun; //0xFE. During block stun or stagger, when frameCounter reaches this value, return back to neutral.
	uint8_t mark; //0xFF.
	char pad_100[48]; //0x100.
};