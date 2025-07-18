#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <unordered_map>
#include "Metadata.h"
#include "CbrCase.h"
#include <utility>  // std::forward
#include <sstream>
#include <random>


#define rWallDist 74000
#define lWallDist -74000
#define maxWallDist 148000
//(1850000/2)
#define maxXDist 18500
#define maxXDistScreen 37000
//(800000/2)
#define maxYDist 150000 

#define maxYVelocity 800000
#define maxXVelocity 800000

#define maxProration 100
#define maxComboTime 200

#define minCaseCooldown 30
#define maxCaseCooldown 600
const static double preComputedCooldownMulti = double(1) / double(maxCaseCooldown - minCaseCooldown);

#define minBlockstun 0
#define maxBlockstun 15
const static double preComputedBlockstunMulti = double(1) / double(maxBlockstun - minBlockstun);
#define maxHpDiff 13000




#define rachelWindMax 40000
#define rachelWindMin 10000

#define nextBestMulti 1.3
#define nextBestAdd 0.05
#define nextBestAddInputSequence 0.2

#define maxRandomDiff 0.1

struct twoFloat {
    float high = 0;
    float low = 0;
};

inline twoFloat getCombinedAndLowestWeights(std::array<float, 200>& curCosts) {
    twoFloat returner;
    returner.low = 100;
    for (int i = 0; i < curCosts.size(); i++) {
        if (curCosts[i] <= 100) {
            returner.high += curCosts[i];

            if (curCosts[i] > 0 && curCosts[i] < returner.low) {
                returner.low = curCosts[i];
            }
        }
    }
    return returner;
}

inline float RandomFloatC(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}


inline int RandomInt(float a, float b) {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(a, b); // define the range
    return distr(gen);
}


inline float CalcNormalizedDifference(int cur, int cas, int min, float precomputedMulti) {
    return (abs(cur - cas) - min) * precomputedMulti;
}

inline float compAirborneState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}

inline float  compRelativePosX(int curP1, int curP2, int caseP1, int caseP2) {
    auto dif1 = abs(curP1 - curP2);
    auto dif2 = abs(caseP1 - caseP2);
    auto dif3 = abs(dif1 - dif2);
    auto dif4 = (float)dif3 / maxXDist;
    //auto dif4 = fmin((float)dif3 / maxXDist, 1);
    if (dif4 > 1) {
        return 1;
    }
    return dif4;
}

inline float  compMaxDistanceAttack(int curP1, int curP2, int caseP1, int caseP2, int maxX) {
    if (maxX == -1) { return 0; }
    auto dif1 = abs(curP1 - curP2);
    auto dif2 = abs(caseP1 - caseP2);
    auto dif3 = dif1 - dif2 - (maxX);
    if (dif3 > 0) {
        return 1;
    }
    return 0;
}
inline float compDirectionHeld(bool curFwd, bool curBack, bool curUp, bool curDown, bool caseFwd, bool caseBack, bool caseUp, bool caseDown) {
    float compVal = 0;
    if (curFwd != caseFwd) {
        compVal += 0.05;
    }
    if (curBack != caseBack) {
        compVal += 0.05;
    }
    if (curUp != caseUp) {
        compVal += 0.05;
    }
    if (curDown != caseDown) {
        compVal += 0.05;
    }
    return compVal;
}
inline float compButtonsHeld(bool curP, bool curK, bool curS, bool curH, bool curD, bool curT, bool caseP, bool caseK, bool caseS, bool caseH, bool caseD, bool caseT) {
    float compVal = 0;
    if (curP != caseP) {
        compVal += 0.2;
    }
    if (curK != caseK) {
        compVal += 0.2;
    }
    if (curS != caseS) {
        compVal += 0.2;
    }
    if (curH != caseH) {
        compVal += 0.2;
    }
    if (curD != caseD) {
        compVal += 0.2;
    }
    if (curT != caseT) {
        compVal += 0.2;
    }
    return compVal;
}

inline float compNegativeEdge(bool curP, bool curK, bool curS, bool curH, bool curD, bool caseP, bool caseK, bool caseS, bool caseH, bool caseD, int charId) {
    float compVal = 0;
    switch (charId)
    {
    case 3: //May
        if (curP != curP) {
            compVal += 0.2;
        }
        if (curK != caseK) {
            compVal += 0.2;
        }
        if (curS != caseS) {
            compVal += 0.2;
        }
        if (curH != caseH) {
            compVal += 0.2;
        }
        if (curD != caseD) {
            compVal += 0.2;
        }
        break;
    case 8: //Eddie
        if (curP != curP) {
            compVal += 0.2;
        }
        if (curK != caseK) {
            compVal += 0.2;
        }
        if (curS != caseS) {
            compVal += 0.2;
        }
        if (curH != caseH) {
            compVal += 0.2;
        }
        if (curD != caseD) {
            compVal += 0.2;
        }
        break;
    case 11: //Testament
        if (curS != caseS) {
            compVal += 0.5;
        }
        if (curH != caseH) {
            compVal += 0.5;
        }
        break;
    case 15: //Venom
        if (curS != caseS) {
            compVal += 0.5;
        }
        if (curH != caseH) {
            compVal += 0.5;
        }
        break;
    case 16: //Dizzy
        if (curS != caseS) {
            compVal += 1;
        }
        break;
    case 18: //I-no
        if (curP != curP) {
            compVal += 0.2;
        }
        if (curK != caseK) {
            compVal += 0.2;
        }
        if (curS != caseS) {
            compVal += 0.2;
        }
        if (curH != caseH) {
            compVal += 0.2;
        }
        if (curD != caseD) {
            compVal += 0.2;
        }
        break;
    case 20: //Bridget
        if (curH != caseH) {
            compVal += 1.0;
        }
        break;
    case 24: //Kliff
        if (curP != caseP) {
            compVal += 0.5;
        }
        if (curH != caseH) {
            compVal += 0.5;
        }
        break;
    case 25: //Justice
        if (curP != curP) {
            compVal += 0.2;
        }
        if (curK != caseK) {
            compVal += 0.2;
        }
        if (curS != caseS) {
            compVal += 0.2;
        }
        if (curH != caseH) {
            compVal += 0.2;
        }
        if (curD != caseD) {
            compVal += 0.2;
        }
        break;
    default:
        break;
    }
    return compVal;
}

inline float  compRelativePosY(int curP1, int curP2, int caseP1, int caseP2) {
    auto dif1 = abs(curP1 - curP2);
    auto dif2 = abs(caseP1 - caseP2);
    auto dif3 = abs(dif1 - dif2);
    //auto dif4 = fmin((float)dif3 / maxYDist, 1);
    auto dif4 = (float)dif3 / maxYDist;
    if (dif4 > 1) {
        return 1;
    }
    return dif4;
}
inline float compHelperOrder(int curPosOpponent, int curPosChar, int curPosHelper, int casePosOpponent, int casePosChar, int casePosHelper) {
    bool curInFront;
    bool caseInFront;
    auto curFacing = curPosChar >= curPosOpponent;
    if (curFacing) {
        curInFront = curPosChar > curPosHelper;
    }
    else {
        curInFront = curPosChar <= curPosHelper;
    }
    auto caseFacing = casePosChar >= casePosOpponent;
    if (caseFacing) {
        caseInFront = casePosChar > casePosHelper;
    }
    else {
        caseInFront = casePosChar <= casePosHelper;
    }

    return curInFront != caseInFront;

}

inline float  compDistanceToWall(int curPosXP1, int casePosXP1, bool curFacing, bool caseFacing) {
    int dif1;
    int dif2;
    if (curFacing) {
        dif1 = abs(curPosXP1 - lWallDist);
    }
    else {
        dif1 = abs(curPosXP1 - rWallDist);
    }
    if (caseFacing) {
        dif2 = abs(casePosXP1 - lWallDist);
    }
    else {
        dif2 = abs(casePosXP1 - rWallDist);
    }

    auto dif3 = abs(dif1 - dif2);
    //auto dif4 = fmin((float)dif3 / maxWallDist, 1);
    auto dif4 = (float)dif3 / maxWallDist;
    if (dif4 > 1) {
        return 1;
    }
    return dif4;
}

inline float compAct(uint16_t curA, uint16_t caseA) {
    if (curA != caseA) {
        return 1;
    }
    return 0;
}

inline float  compState(std::string& curS, std::string& caseS) {
    if (curS != caseS) {
        return 1;
    }
    return 0;
}

inline float  compStateHash(size_t curS, size_t caseS) {
    if (curS != caseS) {
        return 1;
    }
    return 0;
}

inline float compInt(int cur, int cas, int max) {
    auto dif = abs(cur - cas);
    //float dif2 = fmin(((float)dif / max), 1);
    auto dif2 = (float)dif / max;
    if (dif2 > 1) {
        return 1;
    }
    return dif2;
}

inline float compIntState(int cur, int cas) {
    return cur != cas;
}

inline float compBool(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}

inline float compNeutralState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
inline float compWakeupState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
inline float compBlockState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
inline float compBlockStun(bool cur, bool cas, int curStun, int casStun) {
    float ret = 0;
    if (cur != cas) { ret += 0.1; }
    ret += CalcNormalizedDifference(curStun, casStun, minBlockstun, preComputedBlockstunMulti);
    return ret;
}
inline float compHitState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
inline float compGetHitThisFrameState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
inline float compBlockingThisFrameState(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}

inline float compCrouching(bool cur, bool cas) {
    if (cur != cas) { return 1; }
    return 0;
}
//Add a cost for reusing a case within a certain timeframe.
inline float compCaseCooldown(CbrCase* caseData, int framesActive) {
    if (framesActive == -1) { return 0; }
    auto diff = framesActive - caseData->caseCooldownFrameStart;
    if (caseData->caseCooldownFrameStart == -1 || caseData->caseCooldownFrameStart > framesActive) {
        caseData->caseCooldownFrameStart = -1;
        return 0;
    }

    if (diff >= minCaseCooldown) {
        if (diff >= maxCaseCooldown) {
            caseData->caseCooldownFrameStart = -1;
            return 0;
        }
        //Return a cost that gets lesser the longer ago the case was replayed.
        return 1 - (diff - minCaseCooldown) * preComputedCooldownMulti;

    }
    //Return 2 if a  case tries to repeat itself in less than 30 frames to prevent buggy looking movement.
    return 2;
}

struct costWeights {

    std::array<float, 200> basic = { };
    std::array<float, 200> combo = { };
    std::array<float, 200> pressure = { };
    std::array<float, 200> blocking = { };
    std::array<std::string, 200> name;
    float combinedCost = 0;
    float lowestCost = 0;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& basic;
        ar& combo;
        ar& pressure;
        ar& blocking;
        ar& name;
    }
};


#define costXDist 0//1
#define costYDist 1//1
#define costWallDist 2//0.3
#define costWallDistCombo 2//0.5
#define costAiState 3//0.5
#define costEnemyState 4//0.15
#define costlastAiState 5//0.05
#define costlastEnemyState 6//0.05
#define costAiNeutral 7//1
#define costEnemyNeutral 8//0.2
#define costAiAir 9//1
#define costEnemyAir 10//0.2
#define costAiWakeup 11//1
#define costEnemyWakeup 12//0.2
#define costAiBlocking 13//1
#define costEnemyBlocking 14//1
#define costAiHit 15//1
#define costEnemyHit 16//1
#define costAiAttack 17//1
#define costEnemyAttack 18//0.2
#define costAiHitThisFrame 19//0.01
#define costEnemyHitThisFrame 20//0.01
#define costAiBlockThisFrame 21//0.1
#define costEnemyBlockhisFrame 22//0.1
#define costAiCrouching 23//0.1
#define costEnemyCrouching 24//0.1
#define costAiProration 25//0.05
#define costEnemyProration 26//0.05
#define costAiStarterRating 27//0.05
#define costEnemyStarterRating 28//0.05
#define costAiComboTime 29//0.05
#define costEnemyComboTime 30//0.05
#define costAiOverdriveState 31//1
#define costEnemyOverdriveState 32//0.01
#define costMatchState 33//100
#define costCaseCooldown 34//2
#define costAiHp 35//0.01
#define costEnemyHp 36//0.03

//projectilecosts
#define costHelperType 37//0.2
#define costHelperPosX 38//0.1
#define costHelperPosY 39//0.1
#define costHelperState 40//0.05
#define costHelperHit 41//0.05
#define costHelperAttack 42//0.1
#define costHelperOrder 43//0.05
#define costHelperSum 44//(costHelperType + costHelperPosX + costHelperPosY + costHelperState + costHelperHit + costHelperAttack + costHelperOrder + costHelperOrder)

//Sol specific costs
#define costSoDI 45
#define costSoDIEnemy 46

//Millia specific costs
#define costMiSFActive 47
#define costMiSFActiveEnemy 48

//Chipp specific costs
#define costChShuriken 49
#define costChInvis 50
#define costChShurikenEnemy 51
#define costChInvisEnemy 52

//Eddie specific costs
#define costEdGauge 53
#define costEdRecovering 54
#define costEdType 55
#define costEdActive 56
#define costEdGaugeEnemy 57
#define costEdRecoveringEnemy 58
#define costEdTypeEnemy 59
#define costEdActiveEnemy 60

//Baiken specific costs
#define costBaMovementSeal 61
#define costBaCounterSeal 62
#define costBaGuardSeal 63
#define costBaPSeal 64
#define costBaKSeal 65
#define costBaSSeal 66
#define costBaHSeal 67
#define costBaDSeal 68
#define costBaMovementSealEnemy 69
#define costBaCounterSealEnemy 70
#define costBaGuardSealEnemy 71
#define costBaPSealEnemy 72
#define costBaKSealEnemy 73
#define costBaSSealEnemy 74
#define costBaHSealEnemy 75
#define costBaDSealEnemy 76

//Faust specific costs
#define costFaItemActive 77
#define costFaItemActiveEnemy 78

//Testament specific costs
#define costTePuppets 79
#define costTeCrowAttackNo 80
#define costTeCrowActive 81
#define costTeCrowPattern 82
#define costTePuppetsEnemy 83
#define costTeCrowAttackNoEnemy 84
#define costTeCrowActiveEnemy 85
#define costTeCrowPatternEnemy 86

//Jam specific costs
#define costJaAsanagiK 87
#define costJaAsanagiS 88
#define costJaAsanagiH 89
#define costJaAsanagiKEnemy 90
#define costJaAsanagiSEnemy 91
#define costJaAsanagiHEnemy 92

//Johnny specific costs
#define costJoCoins 93
#define costJoMFlvl 94
#define costJoStance 95
#define costJoCoinsEnemy 96
#define costJoMFlvlEnemy 97
#define costJoStanceEnemy 98

//Venom specific costs
#define costVeCharge 99
#define costVePball 100
#define costVeKball 101
#define costVeSball 102
#define costVeHball 103
#define costVeDball 104
#define costVeChargeEnemy 105
#define costVePballEnemy 106
#define costVeKballEnemy 107
#define costVeSballEnemy 108
#define costVeHballEnemy 109
#define costVeDballEnemy 110

//Zappa specific costs
#define costZaSummon 111
#define costZaGauge 112
#define costZaSummonEnemy 113
#define costZaGaugeEnemy 114

//Bridget specific costs
#define costBrYoyo 115
#define costBrYoyoEnemy 116

//Robo-Ky specific weights
#define costRoHeat 117
#define costRoTimer 118
#define costRoHeatEnemy 119
#define costRoTimerEnemy 120

//A.B.A specific weights
#define costAbMode 121
#define costAbPacks 122
#define costAbGauge 123
#define costAbModeEnemy 124
#define costAbPacksEnemy 125
#define costAbGaugeEnemy 126

//Order-Sol specific weights
#define costOsGauge 127
#define costOsGaugeEnemy 128

//Justice specific weights
#define costJuInstall 129
#define costJuInstallEnemy 130

//Generic
#define costMinDistanceAttack 184//0.5

#define costNonNeutralState 185//0.3

#define costButtonsHeld 186
#define costNegativeEdge 187
#define costVelocity 188
#define costVelocityEnemy 189

#define costSameOpponent 189

inline void initalizeCosts(costWeights& costs) {

    costs.name[costXDist] = "costXDist";
    costs.basic[costXDist] = 1;
    costs.combo[costXDist] = 1;
    costs.pressure[costXDist] = 1;
    costs.blocking[costXDist] = 1;

    costs.name[costYDist] = "costYDist";
    costs.basic[costYDist] = 1;
    costs.combo[costYDist] = 1;
    costs.pressure[costYDist] = 1;
    costs.blocking[costXDist] = 1;

    costs.name[costWallDist] = "costWallDist";
    costs.basic[costWallDist] = 0.3;
    costs.combo[costWallDist] = 0.6;
    costs.pressure[costWallDist] = 0.4;
    costs.blocking[costWallDist] = 0.4;

    costs.name[costAiState] = "costAiState";
    costs.basic[costAiState] = 0.1;
    costs.combo[costAiState] = 1;
    costs.pressure[costAiState] = 1;
    costs.blocking[costAiState] = 0.4;

    costs.name[costNonNeutralState] = "costNonNeutralState";
    costs.basic[costNonNeutralState] = 0.3;
    costs.combo[costNonNeutralState] = 0.4;
    costs.pressure[costNonNeutralState] = 0.4;
    costs.blocking[costNonNeutralState] = 0.0;


    costs.name[costEnemyState] = "costEnemyState";
    costs.basic[costEnemyState] = 0.05;
    costs.combo[costEnemyState] = 0.15;
    costs.pressure[costEnemyState] = 0.15;
    costs.blocking[costEnemyState] = 1;

    costs.name[costlastAiState] = "costlastAiState";
    costs.basic[costlastAiState] = 0.01;
    costs.combo[costlastAiState] = 0.1;
    costs.pressure[costlastAiState] = 0.15;
    costs.blocking[costlastAiState] = 0.05;

    costs.name[costlastEnemyState] = "costlastEnemyState";
    costs.basic[costlastEnemyState] = 0.01;
    costs.combo[costlastEnemyState] = 0.01;
    costs.pressure[costlastEnemyState] = 0.05;
    costs.blocking[costlastEnemyState] = 0.15;

    costs.name[costAiNeutral] = "costAiNeutral";
    costs.basic[costAiNeutral] = 1;
    costs.combo[costAiNeutral] = 1;
    costs.pressure[costAiNeutral] = 1;
    costs.blocking[costAiNeutral] = 1;

    costs.name[costEnemyNeutral] = "costEnemyNeutral";
    costs.basic[costEnemyNeutral] = 0.1;
    costs.combo[costEnemyNeutral] = 0.1;
    costs.pressure[costEnemyNeutral] = 0.1;
    costs.blocking[costEnemyNeutral] = 0.1;

    costs.name[costAiAir] = "costAiAir";
    costs.basic[costAiAir] = 0.5;
    costs.combo[costAiAir] = 0.5;
    costs.pressure[costAiAir] = 0.5;
    costs.blocking[costAiAir] = 0.5;

    costs.name[costEnemyAir] = "costEnemyAir";
    costs.basic[costEnemyAir] = 0.8;
    costs.combo[costEnemyAir] = 0.8;
    costs.pressure[costEnemyAir] = 0.8;
    costs.blocking[costEnemyAir] = 0.8;

    costs.name[costAiWakeup] = "costAiWakeup";
    costs.basic[costAiWakeup] = 1;
    costs.combo[costAiWakeup] = 1;
    costs.pressure[costAiWakeup] = 1;
    costs.blocking[costAiWakeup] = 1;

    costs.name[costEnemyWakeup] = "costEnemyWakeup";
    costs.basic[costEnemyWakeup] = 1;
    costs.combo[costEnemyWakeup] = 1;
    costs.pressure[costEnemyWakeup] = 1;
    costs.blocking[costEnemyWakeup] = 1;

    costs.name[costAiBlocking] = "costAiBlocking";
    costs.basic[costAiBlocking] = 1;
    costs.combo[costAiBlocking] = 1;
    costs.pressure[costAiBlocking] = 1;
    costs.blocking[costAiBlocking] = 1;

    costs.name[costEnemyBlocking] = "costEnemyBlocking";
    costs.basic[costEnemyBlocking] = 0.2;
    costs.combo[costEnemyBlocking] = 0.2;
    costs.pressure[costEnemyBlocking] = 0.2;
    costs.blocking[costEnemyBlocking] = 0.2;

    costs.name[costAiHit] = "costAiHit";
    costs.basic[costAiHit] = 1;
    costs.combo[costAiHit] = 1;
    costs.pressure[costAiHit] = 1;
    costs.blocking[costAiHit] = 1;

    costs.name[costEnemyHit] = "costEnemyHit";
    costs.basic[costEnemyHit] = 1;
    costs.combo[costEnemyHit] = 1;
    costs.pressure[costEnemyHit] = 1;
    costs.blocking[costEnemyHit] = 1;

    costs.name[costAiAttack] = "costAiAttack";
    costs.basic[costAiAttack] = 1;
    costs.combo[costAiAttack] = 1;
    costs.pressure[costAiAttack] = 1;
    costs.blocking[costAiAttack] = 1;

    costs.name[costEnemyAttack] = "costEnemyAttack";
    costs.basic[costEnemyAttack] = 0.2;
    costs.combo[costEnemyAttack] = 0.2;
    costs.pressure[costEnemyAttack] = 0.2;
    costs.blocking[costEnemyAttack] = 0.4;

    costs.name[costAiHitThisFrame] = "costAiHitThisFrame";
    costs.basic[costAiHitThisFrame] = 0.1;
    costs.combo[costAiHitThisFrame] = 0.1;
    costs.pressure[costAiHitThisFrame] = 0.1;
    costs.blocking[costAiHitThisFrame] = 0.1;

    costs.name[costEnemyHitThisFrame] = "costEnemyHitThisFrame";
    costs.basic[costEnemyHitThisFrame] = 0.1;
    costs.combo[costEnemyHitThisFrame] = 0.1;
    costs.pressure[costEnemyHitThisFrame] = 0.1;
    costs.blocking[costEnemyHitThisFrame] = 0.1;

    costs.name[costAiBlockThisFrame] = "costAiBlockThisFrame";
    costs.basic[costAiBlockThisFrame] = 0.1;
    costs.combo[costAiBlockThisFrame] = 0.1;
    costs.pressure[costAiBlockThisFrame] = 0.1;
    costs.blocking[costAiBlockThisFrame] = 0.1;

    costs.name[costEnemyBlockhisFrame] = "costEnemyBlockhisFrame";
    costs.basic[costEnemyBlockhisFrame] = 0.1;
    costs.combo[costEnemyBlockhisFrame] = 0.1;
    costs.pressure[costEnemyBlockhisFrame] = 0.1;
    costs.blocking[costEnemyBlockhisFrame] = 0.1;

    costs.name[costAiCrouching] = "costAiCrouching";
    costs.basic[costAiCrouching] = 0.1;
    costs.combo[costAiCrouching] = 0.1;
    costs.pressure[costAiCrouching] = 0.1;
    costs.blocking[costAiCrouching] = 0.1;

    costs.name[costEnemyCrouching] = "costEnemyCrouching";
    costs.basic[costEnemyCrouching] = 0.1;
    costs.combo[costEnemyCrouching] = 0.5;
    costs.pressure[costEnemyCrouching] = 0.5;
    costs.blocking[costEnemyCrouching] = 0.1;

    costs.name[costAiProration] = "costAiProration";
    costs.basic[costAiProration] = 0.1;
    costs.combo[costAiProration] = 0.1;
    costs.pressure[costAiProration] = 0.1;
    costs.blocking[costAiProration] = 0.1;

    costs.name[costEnemyProration] = "costEnemyProration";
    costs.basic[costEnemyProration] = 0.1;
    costs.combo[costEnemyProration] = 0.1;
    costs.pressure[costEnemyProration] = 0.1;
    costs.blocking[costEnemyProration] = 0.1;

    costs.name[costAiStarterRating] = "costAiStarterRating";
    costs.basic[costAiStarterRating] = 0.1;
    costs.combo[costAiStarterRating] = 0.1;
    costs.pressure[costAiStarterRating] = 0.1;
    costs.blocking[costAiStarterRating] = 0.1;

    costs.name[costEnemyStarterRating] = "costEnemyStarterRating";
    costs.basic[costEnemyStarterRating] = 0.1;
    costs.combo[costEnemyStarterRating] = 0.1;
    costs.pressure[costEnemyStarterRating] = 0.1;
    costs.blocking[costEnemyStarterRating] = 0.1;

    costs.name[costAiComboTime] = "costAiComboTime";
    costs.basic[costAiComboTime] = 0.1;
    costs.combo[costAiComboTime] = 0.1;
    costs.pressure[costAiComboTime] = 0.1;
    costs.blocking[costAiComboTime] = 0.1;

    costs.name[costEnemyComboTime] = "costEnemyComboTime";
    costs.basic[costEnemyComboTime] = 0.1;
    costs.combo[costEnemyComboTime] = 0.1;
    costs.pressure[costEnemyComboTime] = 0.1;
    costs.blocking[costEnemyComboTime] = 0.1;

    costs.name[costAiOverdriveState] = "costAiOverdriveState";
    costs.basic[costAiOverdriveState] = 1;
    costs.combo[costAiOverdriveState] = 1;
    costs.pressure[costAiOverdriveState] = 0.3;
    costs.blocking[costAiOverdriveState] = 0.5;

    costs.name[costEnemyOverdriveState] = "costEnemyOverdriveState";
    costs.basic[costEnemyOverdriveState] = 0.5;
    costs.combo[costEnemyOverdriveState] = 0.1;
    costs.pressure[costEnemyOverdriveState] = 0.5;
    costs.blocking[costEnemyOverdriveState] = 0.5;

    costs.name[costMatchState] = "costMatchState";
    costs.basic[costMatchState] = 10;
    costs.combo[costMatchState] = 10;
    costs.pressure[costMatchState] = 10;
    costs.blocking[costMatchState] = 10;

    costs.name[costCaseCooldown] = "costCaseCooldown";
    costs.basic[costCaseCooldown] = 1;
    costs.combo[costCaseCooldown] = 0.1;
    costs.pressure[costCaseCooldown] = 0.1;
    costs.blocking[costCaseCooldown] = 0.1;

    costs.name[costAiHp] = "costAiHp";
    costs.basic[costAiHp] = 0.1;
    costs.combo[costAiHp] = 0.1;
    costs.pressure[costAiHp] = 0.1;
    costs.blocking[costAiHp] = 0.1;

    costs.name[costEnemyHp] = "costEnemyHp";
    costs.basic[costEnemyHp] = 0.1;
    costs.combo[costEnemyHp] = 0.2;
    costs.pressure[costEnemyHp] = 0.2;
    costs.blocking[costEnemyHp] = 0.1;

    costs.name[costHelperType] = "costHelperType";
    costs.basic[costHelperType] = 0.2;
    costs.combo[costHelperType] = 0.2;
    costs.pressure[costHelperType] = 0.2;
    costs.blocking[costHelperType] = 0.2;

    costs.name[costHelperPosX] = "costHelperPosX";
    costs.basic[costHelperPosX] = 0.1;
    costs.combo[costHelperPosX] = 0.1;
    costs.pressure[costHelperPosX] = 0.1;
    costs.blocking[costHelperPosX] = 0.1;

    costs.name[costHelperPosY] = "costHelperPosY";
    costs.basic[costHelperPosY] = 0.1;
    costs.combo[costHelperPosY] = 0.1;
    costs.pressure[costHelperPosY] = 0.1;
    costs.blocking[costHelperPosY] = 0.1;

    costs.name[costHelperState] = "costHelperState";
    costs.basic[costHelperState] = 0.05;
    costs.combo[costHelperState] = 0.05;
    costs.pressure[costHelperState] = 0.05;
    costs.blocking[costHelperState] = 0.05;

    costs.name[costHelperHit] = "costHelperHit";
    costs.basic[costHelperHit] = 0.05;
    costs.combo[costHelperHit] = 0.05;
    costs.pressure[costHelperHit] = 0.05;
    costs.blocking[costHelperHit] = 0.05;

    costs.name[costHelperAttack] = "costHelperAttack";
    costs.basic[costHelperAttack] = 0.1;
    costs.combo[costHelperAttack] = 0.1;
    costs.pressure[costHelperAttack] = 0.1;
    costs.blocking[costHelperAttack] = 0.1;

    costs.name[costHelperOrder] = "costHelperOrder";
    costs.basic[costHelperOrder] = 0.05;
    costs.combo[costHelperOrder] = 0.05;
    costs.pressure[costHelperOrder] = 0.05;
    costs.blocking[costHelperOrder] = 0.05;

    costs.name[costHelperSum] = "-1";
    costs.basic[costHelperSum] = costs.basic[costHelperType] + costs.basic[costHelperPosX] + costs.basic[costHelperPosY] + costs.basic[costHelperState] + costs.basic[costHelperHit] + costs.basic[costHelperAttack] + costs.basic[costHelperOrder];
    costs.combo[costHelperSum] = costs.combo[costHelperType] + costs.combo[costHelperPosX] + costs.combo[costHelperPosY] + costs.combo[costHelperState] + costs.combo[costHelperHit] + costs.combo[costHelperAttack] + costs.combo[costHelperOrder];
    costs.pressure[costHelperSum] = costs.pressure[costHelperType] + costs.pressure[costHelperPosX] + costs.pressure[costHelperPosY] + costs.pressure[costHelperState] + costs.pressure[costHelperHit] + costs.pressure[costHelperAttack] + costs.pressure[costHelperOrder];
    costs.blocking[costHelperSum] = costs.blocking[costHelperType] + costs.blocking[costHelperPosX] + costs.blocking[costHelperPosY] + costs.blocking[costHelperState] + costs.blocking[costHelperHit] + costs.blocking[costHelperAttack] + costs.blocking[costHelperOrder];

    //Sol specific costs
    costs.name[costSoDI] = "costSoDI";
    costs.basic[costSoDI] = 0.4;
    costs.combo[costSoDI] = 0.4;
    costs.pressure[costSoDI] = 0.4;
    costs.blocking[costSoDI] = 0.1;

    costs.name[costSoDIEnemy] = "costSoDIEnemy";
    costs.basic[costSoDIEnemy] = 0.4;
    costs.combo[costSoDIEnemy] = 0;
    costs.pressure[costSoDIEnemy] = 0.4;
    costs.blocking[costSoDIEnemy] = 0.4;

    //Millia specific costs
    costs.name[costMiSFActive] = "costMiSFActive";
    costs.basic[costMiSFActive] = 0.4;
    costs.combo[costMiSFActive] = 0.4;
    costs.pressure[costMiSFActive] = 0.1;
    costs.blocking[costMiSFActive] = 0.1;

    costs.name[costMiSFActiveEnemy] = "costMiSFActiveEnemy";
    costs.basic[costMiSFActiveEnemy] = 0.4;
    costs.combo[costMiSFActiveEnemy] = 0;
    costs.pressure[costMiSFActiveEnemy] = 0;
    costs.blocking[costMiSFActiveEnemy] = 0.4;

    //Chipp specific costs
    costs.name[costChShuriken] = "costChShuriken";
    costs.basic[costChShuriken] = 0.1;
    costs.combo[costChShuriken] = 0;
    costs.pressure[costChShuriken] = 0;
    costs.blocking[costChShuriken] = 0;

    costs.name[costChInvis] = "costChInvis";
    costs.basic[costChInvis] = 0.1;
    costs.combo[costChInvis] = 0;
    costs.pressure[costChInvis] = 0.1;
    costs.blocking[costChInvis] = 0;

    costs.name[costChShurikenEnemy] = "costChShurikenEnemy";
    costs.basic[costChShurikenEnemy] = 0.1;
    costs.combo[costChShurikenEnemy] = 0;
    costs.pressure[costChShurikenEnemy] = 0;
    costs.blocking[costChShurikenEnemy] = 0;

    costs.name[costChInvisEnemy] = "costChInvisEnemy";
    costs.basic[costChInvisEnemy] = 0.1;
    costs.combo[costChInvisEnemy] = 0;
    costs.pressure[costChInvisEnemy] = 0;
    costs.blocking[costChInvisEnemy] = 0;

    //Eddie specific costs
    costs.name[costEdGauge] = "costEdGauge";
    costs.basic[costEdGauge] = 0.1;
    costs.combo[costEdGauge] = 0.1;
    costs.pressure[costEdGauge] = 0.1;
    costs.blocking[costEdGauge] = 0.1;

    costs.name[costEdRecovering] = "costEdRecovering";
    costs.basic[costEdRecovering] = 1;
    costs.combo[costEdRecovering] = 1;
    costs.pressure[costEdRecovering] = 1;
    costs.blocking[costEdRecovering] = 0;

    costs.name[costEdType] = "costEdType";
    costs.basic[costEdType] = 0.5;
    costs.combo[costEdType] = 0.5;
    costs.pressure[costEdType] = 0.5;
    costs.blocking[costEdType] = 0;

    costs.name[costEdActive] = "costEdActive";
    costs.basic[costEdActive] = 1;
    costs.combo[costEdActive] = 1;
    costs.pressure[costEdActive] = 1;
    costs.blocking[costEdActive] = 0;

    costs.name[costEdGaugeEnemy] = "costEdGaugeEnemy";
    costs.basic[costEdGaugeEnemy] = 0.1;
    costs.combo[costEdGaugeEnemy] = 0.1;
    costs.pressure[costEdGaugeEnemy] = 0.1;
    costs.blocking[costEdGaugeEnemy] = 0.1;

    costs.name[costEdRecoveringEnemy] = "costEdRecoveringEnemy";
    costs.basic[costEdRecoveringEnemy] = 1;
    costs.combo[costEdRecoveringEnemy] = 0;
    costs.pressure[costEdRecoveringEnemy] = 0;
    costs.blocking[costEdRecoveringEnemy] = 1;

    costs.name[costEdTypeEnemy] = "costEdTypeEnemy";
    costs.basic[costEdTypeEnemy] = 0.5;
    costs.combo[costEdTypeEnemy] = 0;
    costs.pressure[costEdTypeEnemy] = 0;
    costs.blocking[costEdTypeEnemy] = 0.5;

    costs.name[costEdActiveEnemy] = "costEdActiveEnemy";
    costs.basic[costEdActiveEnemy] = 1;
    costs.combo[costEdActiveEnemy] = 0;
    costs.pressure[costEdActiveEnemy] = 0;
    costs.blocking[costEdActiveEnemy] = 1;

    //Baiken specific costs
    costs.name[costBaMovementSeal] = "costBaMovementSeal";
    costs.basic[costBaMovementSeal] = 0.1;
    costs.combo[costBaMovementSeal] = 0.1;
    costs.pressure[costBaMovementSeal] = 0.1;
    costs.blocking[costBaMovementSeal] = 0.1;

    costs.name[costBaCounterSeal] = "costBaCounterSeal";
    costs.basic[costBaCounterSeal] = 0.1;
    costs.combo[costBaCounterSeal] = 0.1;
    costs.pressure[costBaCounterSeal] = 0.1;
    costs.blocking[costBaCounterSeal] = 0.1;

    costs.name[costBaGuardSeal] = "costBaGuardSeal";
    costs.basic[costBaGuardSeal] = 0.1;
    costs.combo[costBaGuardSeal] = 0.1;
    costs.pressure[costBaGuardSeal] = 0.1;
    costs.blocking[costBaGuardSeal] = 0.1;

    costs.name[costBaPSeal] = "costBaPSeal";
    costs.basic[costBaPSeal] = 0.1;
    costs.combo[costBaPSeal] = 0.1;
    costs.pressure[costBaPSeal] = 0.1;
    costs.blocking[costBaPSeal] = 0.1;

    costs.name[costBaKSeal] = "costBaKSeal";
    costs.basic[costBaKSeal] = 0.1;
    costs.combo[costBaKSeal] = 0.1;
    costs.pressure[costBaKSeal] = 0.1;
    costs.blocking[costBaKSeal] = 0.1;

    costs.name[costBaSSeal] = "costBaSSeal";
    costs.basic[costBaSSeal] = 0.1;
    costs.combo[costBaSSeal] = 0.1;
    costs.pressure[costBaSSeal] = 0.1;
    costs.blocking[costBaSSeal] = 0.1;

    costs.name[costBaHSeal] = "costBaHSeal";
    costs.basic[costBaHSeal] = 0.1;
    costs.combo[costBaHSeal] = 0.1;
    costs.pressure[costBaHSeal] = 0.1;
    costs.blocking[costBaHSeal] = 0.1;

    costs.name[costBaDSeal] = "costBaDSeal";
    costs.basic[costBaDSeal] = 0.1;
    costs.combo[costBaDSeal] = 0.1;
    costs.pressure[costBaDSeal] = 0.1;
    costs.blocking[costBaDSeal] = 0.1;

    costs.name[costBaMovementSealEnemy] = "costBaMovementSealEnemy";
    costs.basic[costBaMovementSealEnemy] = 0.1;
    costs.combo[costBaMovementSealEnemy] = 0.1;
    costs.pressure[costBaMovementSealEnemy] = 0.1;
    costs.blocking[costBaMovementSealEnemy] = 0.1;

    costs.name[costBaCounterSealEnemy] = "costBaCounterSealEnemy";
    costs.basic[costBaCounterSealEnemy] = 0.1;
    costs.combo[costBaCounterSealEnemy] = 0.1;
    costs.pressure[costBaCounterSealEnemy] = 0.1;
    costs.blocking[costBaCounterSealEnemy] = 0.1;

    costs.name[costBaGuardSealEnemy] = "costBaGuardSealEnemy";
    costs.basic[costBaGuardSealEnemy] = 0.1;
    costs.combo[costBaGuardSealEnemy] = 0.1;
    costs.pressure[costBaGuardSealEnemy] = 0.1;
    costs.blocking[costBaGuardSealEnemy] = 0.1;

    costs.name[costBaPSealEnemy] = "costBaPSealEnemy";
    costs.basic[costBaPSealEnemy] = 0.1;
    costs.combo[costBaPSealEnemy] = 0.1;
    costs.pressure[costBaPSealEnemy] = 0.1;
    costs.blocking[costBaPSealEnemy] = 0.1;

    costs.name[costBaKSealEnemy] = "costBaKSealEnemy";
    costs.basic[costBaKSealEnemy] = 0.1;
    costs.combo[costBaKSealEnemy] = 0.1;
    costs.pressure[costBaKSealEnemy] = 0.1;
    costs.blocking[costBaKSealEnemy] = 0.1;

    costs.name[costBaSSealEnemy] = "costBaSSealEnemy";
    costs.basic[costBaSSealEnemy] = 0.1;
    costs.combo[costBaSSealEnemy] = 0.1;
    costs.pressure[costBaSSealEnemy] = 0.1;
    costs.blocking[costBaSSealEnemy] = 0.1;

    costs.name[costBaHSealEnemy] = "costBaHSealEnemy";
    costs.basic[costBaHSealEnemy] = 0.1;
    costs.combo[costBaHSealEnemy] = 0.1;
    costs.pressure[costBaHSealEnemy] = 0.1;
    costs.blocking[costBaHSealEnemy] = 0.1;

    costs.name[costBaDSealEnemy] = "costBaDSealEnemy";
    costs.basic[costBaDSealEnemy] = 0.1;
    costs.combo[costBaDSealEnemy] = 0.1;
    costs.pressure[costBaDSealEnemy] = 0.1;
    costs.blocking[costBaDSealEnemy] = 0.1;

    //Faust specific costs
    costs.name[costFaItemActive] = "costFaItemActive";
    costs.basic[costFaItemActive] = 0.5;
    costs.combo[costFaItemActive] = 0.5;
    costs.pressure[costFaItemActive] = 0.5;
    costs.blocking[costFaItemActive] = 0.5;

    costs.name[costFaItemActiveEnemy] = "costFaItemActiveEnemy";
    costs.basic[costFaItemActiveEnemy] = 0.5;
    costs.combo[costFaItemActiveEnemy] = 0.5;
    costs.pressure[costFaItemActiveEnemy] = 0.5;
    costs.blocking[costFaItemActiveEnemy] = 0.5;

    //Testament specific costs
    costs.name[costTePuppets] = "costTePuppets";
    costs.basic[costTePuppets] = 0.3;
    costs.combo[costTePuppets] = 0.3;
    costs.pressure[costTePuppets] = 0.3;
    costs.blocking[costTePuppets] = 0;

    costs.name[costTeCrowAttackNo] = "costTeCrowAttackNo";
    costs.basic[costTeCrowAttackNo] = 0.1;
    costs.combo[costTeCrowAttackNo] = 0.1;
    costs.pressure[costTeCrowAttackNo] = 0.1;
    costs.blocking[costTeCrowAttackNo] = 0.1;

    costs.name[costTeCrowActive] = "costTeCrowActive";
    costs.basic[costTeCrowActive] = 0.5;
    costs.combo[costTeCrowActive] = 0.5;
    costs.pressure[costTeCrowActive] = 0.5;
    costs.blocking[costTeCrowActive] = 0.5;

    costs.name[costTeCrowPattern] = "costTeCrowPattern";
    costs.basic[costTeCrowPattern] = 0.05;
    costs.combo[costTeCrowPattern] = 0.05;
    costs.pressure[costTeCrowPattern] = 0.05;
    costs.blocking[costTeCrowPattern] = 0.05;

    costs.name[costTePuppetsEnemy] = "costTePuppetsEnemy";
    costs.basic[costTePuppetsEnemy] = 0.3;
    costs.combo[costTePuppetsEnemy] = 0.3;
    costs.pressure[costTePuppetsEnemy] = 0.3;
    costs.blocking[costTePuppetsEnemy] = 0.3;

    costs.name[costTeCrowAttackNoEnemy] = "costTeCrowAttackNoEnemy";
    costs.basic[costTeCrowAttackNoEnemy] = 0.1;
    costs.combo[costTeCrowAttackNoEnemy] = 0.1;
    costs.pressure[costTeCrowAttackNoEnemy] = 0.1;
    costs.blocking[costTeCrowAttackNoEnemy] = 0.1;

    costs.name[costTeCrowActiveEnemy] = "costTeCrowActiveEnemy";
    costs.basic[costTeCrowActiveEnemy] = 0.5;
    costs.combo[costTeCrowActiveEnemy] = 0.5;
    costs.pressure[costTeCrowActiveEnemy] = 0.5;
    costs.blocking[costTeCrowActiveEnemy] = 0.5;

    costs.name[costTeCrowPatternEnemy] = "costTeCrowPattern";
    costs.basic[costTeCrowPatternEnemy] = 0.05;
    costs.combo[costTeCrowPatternEnemy] = 0.05;
    costs.pressure[costTeCrowPatternEnemy] = 0.05;
    costs.blocking[costTeCrowPatternEnemy] = 0.05;

    //Jam specific costs
    costs.name[costJaAsanagiK] = "costJaAsanagiK";
    costs.basic[costJaAsanagiK] = 0.3;
    costs.combo[costJaAsanagiK] = 0.3;
    costs.pressure[costJaAsanagiK] = 0.3;
    costs.blocking[costJaAsanagiK] = 0.3;

    costs.name[costJaAsanagiS] = "costJaAsanagiS";
    costs.basic[costJaAsanagiS] = 0.3;
    costs.combo[costJaAsanagiS] = 0.3;
    costs.pressure[costJaAsanagiS] = 0.3;
    costs.blocking[costJaAsanagiS] = 0.3;

    costs.name[costJaAsanagiH] = "costJaAsanagiH";
    costs.basic[costJaAsanagiH] = 0.3;
    costs.combo[costJaAsanagiH] = 0.3;
    costs.pressure[costJaAsanagiH] = 0.3;
    costs.blocking[costJaAsanagiH] = 0.3;

    costs.name[costJaAsanagiKEnemy] = "costJaAsanagiKEnemy";
    costs.basic[costJaAsanagiKEnemy] = 0.3;
    costs.combo[costJaAsanagiKEnemy] = 0.3;
    costs.pressure[costJaAsanagiKEnemy] = 0.3;
    costs.blocking[costJaAsanagiKEnemy] = 0.3;

    costs.name[costJaAsanagiSEnemy] = "costJaAsanagiSEnemy";
    costs.basic[costJaAsanagiSEnemy] = 0.3;
    costs.combo[costJaAsanagiSEnemy] = 0.3;
    costs.pressure[costJaAsanagiSEnemy] = 0.3;
    costs.blocking[costJaAsanagiSEnemy] = 0.3;

    costs.name[costJaAsanagiHEnemy] = "costJaAsanagiHEnemy";
    costs.basic[costJaAsanagiHEnemy] = 0.3;
    costs.combo[costJaAsanagiHEnemy] = 0.3;
    costs.pressure[costJaAsanagiHEnemy] = 0.3;
    costs.blocking[costJaAsanagiHEnemy] = 0.3;

    //Johnny specific costs
    costs.name[costJoCoins] = "costJoCoins";
    costs.basic[costJoCoins] = 0.3;
    costs.combo[costJoCoins] = 0.3;
    costs.pressure[costJoCoins] = 0.3;
    costs.blocking[costJoCoins] = 0;

    costs.name[costJoMFlvl] = "costJoMFlvl";
    costs.basic[costJoMFlvl] = 0.5;
    costs.combo[costJoMFlvl] = 0.5;
    costs.pressure[costJoMFlvl] = 0.5;
    costs.blocking[costJoMFlvl] = 0;

    costs.name[costJoStance] = "costJoStance";
    costs.basic[costJoStance] = 1;
    costs.combo[costJoStance] = 0.2;
    costs.pressure[costJoStance] = 0.2;
    costs.blocking[costJoStance] = 0;

    costs.name[costJoCoinsEnemy] = "costJoCoinsEnemy";
    costs.basic[costJoCoinsEnemy] = 0.3;
    costs.combo[costJoCoinsEnemy] = 0.3;
    costs.pressure[costJoCoinsEnemy] = 0.3;
    costs.blocking[costJoCoinsEnemy] = 0;

    costs.name[costJoMFlvlEnemy] = "costJoMFlvlEnemy";
    costs.basic[costJoMFlvlEnemy] = 0.5;
    costs.combo[costJoMFlvlEnemy] = 0.5;
    costs.pressure[costJoMFlvlEnemy] = 0.5;
    costs.blocking[costJoMFlvlEnemy] = 0;

    costs.name[costJoStanceEnemy] = "costJoStanceEnemy";
    costs.basic[costJoStanceEnemy] = 1;
    costs.combo[costJoStanceEnemy] = 0.2;
    costs.pressure[costJoStanceEnemy] = 0.2;
    costs.blocking[costJoStanceEnemy] = 0;

    //Venom specific costs
    costs.name[costVeCharge] = "costVeCharge";
    costs.basic[costVeCharge] = 0.1;
    costs.combo[costVeCharge] = 0.1;
    costs.pressure[costVeCharge] = 0.1;
    costs.blocking[costVeCharge] = 0;

    costs.name[costVeChargeEnemy] = "costVeChargeEnemy";
    costs.basic[costVeChargeEnemy] = 0.1;
    costs.combo[costVeChargeEnemy] = 0.1;
    costs.pressure[costVeChargeEnemy] = 0.1;
    costs.blocking[costVeChargeEnemy] = 0.1;

    costs.name[costVePball] = "costVePball";
    costs.basic[costVePball] = 1;
    costs.combo[costVePball] = 0.8;
    costs.pressure[costVePball] = 0.8;
    costs.blocking[costVePball] = 0;

    costs.name[costVeKball] = "costVeKball";
    costs.basic[costVeKball] = 1;
    costs.combo[costVeKball] = 0.8;
    costs.pressure[costVeKball] = 0.8;
    costs.blocking[costVeKball] = 0;

    costs.name[costVeSball] = "costVeSball";
    costs.basic[costVeSball] = 1;
    costs.combo[costVeSball] = 0.8;
    costs.pressure[costVeSball] = 0.8;
    costs.blocking[costVeSball] = 0;

    costs.name[costVeHball] = "costVeHball";
    costs.basic[costVeHball] = 1;
    costs.combo[costVeHball] = 0.8;
    costs.pressure[costVeHball] = 0.8;
    costs.blocking[costVeHball] = 0;

    costs.name[costVeDball] = "costVeDball";
    costs.basic[costVeDball] = 1;
    costs.combo[costVeDball] = 0.8;
    costs.pressure[costVeDball] = 0.8;
    costs.blocking[costVeDball] = 0;

    costs.name[costVePballEnemy] = "costVePballEnemy";
    costs.basic[costVePballEnemy] = 1;
    costs.combo[costVePballEnemy] = 0.8;
    costs.pressure[costVePballEnemy] = 0.8;
    costs.blocking[costVePballEnemy] = 0;

    costs.name[costVeKballEnemy] = "costVeKballEnemy";
    costs.basic[costVeKballEnemy] = 1;
    costs.combo[costVeKballEnemy] = 0.8;
    costs.pressure[costVeKballEnemy] = 0.8;
    costs.blocking[costVeKballEnemy] = 0;

    costs.name[costVeSballEnemy] = "costVeSballEnemy";
    costs.basic[costVeSballEnemy] = 1;
    costs.combo[costVeSballEnemy] = 0.8;
    costs.pressure[costVeSballEnemy] = 0.8;
    costs.blocking[costVeSballEnemy] = 0;

    costs.name[costVeHballEnemy] = "costVeHballEnemy";
    costs.basic[costVeHballEnemy] = 1;
    costs.combo[costVeHballEnemy] = 0.8;
    costs.pressure[costVeHballEnemy] = 0.8;
    costs.blocking[costVeHballEnemy] = 0;

    costs.name[costVeDballEnemy] = "costVeDballEnemy";
    costs.basic[costVeDballEnemy] = 1;
    costs.combo[costVeDballEnemy] = 0.8;
    costs.pressure[costVeDballEnemy] = 0.8;
    costs.blocking[costVeDballEnemy] = 0;

    //Zappa specific costs
    costs.name[costZaSummon] = "costZaSummon";
    costs.basic[costZaSummon] = 1;
    costs.combo[costZaSummon] = 1;
    costs.pressure[costZaSummon] = 1;
    costs.blocking[costZaSummon] = 1;

    costs.name[costZaGauge] = "costZaGauge";
    costs.basic[costZaGauge] = 0.1;
    costs.combo[costZaGauge] = 0.1;
    costs.pressure[costZaGauge] = 0.1;
    costs.blocking[costZaGauge] = 0.1;

    costs.name[costZaSummonEnemy] = "costZaSummonEnemy";
    costs.basic[costZaSummonEnemy] = 1;
    costs.combo[costZaSummonEnemy] = 1;
    costs.pressure[costZaSummonEnemy] = 1;
    costs.blocking[costZaSummonEnemy] = 1;

    costs.name[costZaGaugeEnemy] = "costZaGaugeEnemy";
    costs.basic[costZaGaugeEnemy] = 0.1;
    costs.combo[costZaGaugeEnemy] = 0.1;
    costs.pressure[costZaGaugeEnemy] = 0.1;
    costs.blocking[costZaGaugeEnemy] = 0.1;

    //Bridget specific costs
    costs.name[costBrYoyo] = "costBrYoyo";
    costs.basic[costBrYoyo] = 1;
    costs.combo[costBrYoyo] = 1;
    costs.pressure[costBrYoyo] = 1;
    costs.blocking[costBrYoyo] = 1;

    costs.name[costBrYoyoEnemy] = "costBrYoyoEnemy";
    costs.basic[costBrYoyoEnemy] = 1;
    costs.combo[costBrYoyoEnemy] = 1;
    costs.pressure[costBrYoyoEnemy] = 1;
    costs.blocking[costBrYoyoEnemy] = 1;

    //Robo-Ky specific weights
    costs.name[costRoHeat] = "costRoHeat";
    costs.basic[costRoHeat] = 0.5;
    costs.combo[costRoHeat] = 0.5;
    costs.pressure[costRoHeat] = 0.5;
    costs.blocking[costRoHeat] = 0.1;

    costs.name[costRoTimer] = "costRoTimer";
    costs.basic[costRoTimer] = 0.1;
    costs.combo[costRoTimer] = 0.1;
    costs.pressure[costRoTimer] = 0.1;
    costs.blocking[costRoTimer] = 0.1;

    costs.name[costRoHeatEnemy] = "costRoHeatEnemy";
    costs.basic[costRoHeatEnemy] = 0.5;
    costs.combo[costRoHeatEnemy] = 0.5;
    costs.pressure[costRoHeatEnemy] = 0.5;
    costs.blocking[costRoHeatEnemy] = 0.5;

    costs.name[costRoTimerEnemy] = "costRoTimerEnemy";
    costs.basic[costRoTimerEnemy] = 0.1;
    costs.combo[costRoTimerEnemy] = 0.1;
    costs.pressure[costRoTimerEnemy] = 0.1;
    costs.blocking[costRoTimerEnemy] = 0.1;

    //A.B.A specific weights
    costs.name[costAbMode] = "costAbMode";
    costs.basic[costAbMode] = 1;
    costs.combo[costAbMode] = 1;
    costs.pressure[costAbMode] = 1;
    costs.blocking[costAbMode] = 1;

    costs.name[costAbPacks] = "costAbPacks";
    costs.basic[costAbPacks] = 0.3;
    costs.combo[costAbPacks] = 0;
    costs.pressure[costAbPacks] = 0;
    costs.blocking[costAbPacks] = 0;

    costs.name[costAbGauge] = "costAbGauge";
    costs.basic[costAbGauge] = 0.5;
    costs.combo[costAbGauge] = 0.3;
    costs.pressure[costAbGauge] = 0.3;
    costs.blocking[costAbGauge] = 0.3;

    costs.name[costAbModeEnemy] = "costAbModeEnemy";
    costs.basic[costAbModeEnemy] = 1;
    costs.combo[costAbModeEnemy] = 1;
    costs.pressure[costAbModeEnemy] = 1;
    costs.blocking[costAbModeEnemy] = 1;

    costs.name[costAbPacksEnemy] = "costAbPacksEnemy";
    costs.basic[costAbPacksEnemy] = 0.3;
    costs.combo[costAbPacksEnemy] = 0;
    costs.pressure[costAbPacksEnemy] = 0;
    costs.blocking[costAbPacksEnemy] = 0;

    costs.name[costAbGaugeEnemy] = "costAbGaugeEnemy";
    costs.basic[costAbGaugeEnemy] = 0.5;
    costs.combo[costAbGaugeEnemy] = 0.3;
    costs.pressure[costAbGaugeEnemy] = 0.3;
    costs.blocking[costAbGaugeEnemy] = 0.3;

    //Order-Sol specific weights
    costs.name[costOsGauge] = "costOsGauge";
    costs.basic[costOsGauge] = 0.5;
    costs.combo[costOsGauge] = 0.5;
    costs.pressure[costOsGauge] = 0.5;
    costs.blocking[costOsGauge] = 0.5;

    costs.name[costOsGaugeEnemy] = "costOsGaugeEnemy";
    costs.basic[costOsGaugeEnemy] = 0.5;
    costs.combo[costOsGaugeEnemy] = 0.5;
    costs.pressure[costOsGaugeEnemy] = 0.5;
    costs.blocking[costOsGaugeEnemy] = 0.5;

//Justice specific weights
    costs.name[costJuInstall] = "costJuInstall";
    costs.basic[costJuInstall] = 0.4;
    costs.combo[costJuInstall] = 0.4;
    costs.pressure[costJuInstall] = 0.4;
    costs.blocking[costJuInstall] = 0.4;

    costs.name[costJuInstallEnemy] = "costJuInstallEnemy";
    costs.basic[costJuInstallEnemy] = 0.4;
    costs.combo[costJuInstallEnemy] = 0.4;
    costs.pressure[costJuInstallEnemy] = 0.4;
    costs.blocking[costJuInstallEnemy] = 0.4;

    //Generic
    costs.name[costMinDistanceAttack] = "costMinDistanceAttack";
    costs.basic[costMinDistanceAttack] = 0.5;
    costs.combo[costMinDistanceAttack] = 0.5;
    costs.pressure[costMinDistanceAttack] = 0.5;
    costs.blocking[costMinDistanceAttack] = 0.5;

    costs.name[costButtonsHeld] = "costButtonsHeld";
    costs.basic[costButtonsHeld] = 1;
    costs.combo[costButtonsHeld] = 1;
    costs.pressure[costButtonsHeld] = 1;
    costs.blocking[costButtonsHeld] = 1;

    costs.name[costNegativeEdge] = "costNegativeEdge";
    costs.basic[costNegativeEdge] = 0.4;
    costs.combo[costNegativeEdge] = 0.4;
    costs.pressure[costNegativeEdge] = 0.4;
    costs.blocking[costNegativeEdge] = 0.4;

    costs.name[costVelocity] = "costVelocity";
    costs.basic[costVelocity] = 0.5;
    costs.combo[costVelocity] = 0.5;
    costs.pressure[costVelocity] = 0.5;
    costs.blocking[costVelocity] = 0.5;

    costs.name[costVelocityEnemy] = "costVelocityEnemy";
    costs.basic[costVelocityEnemy] = 0.5;
    costs.combo[costVelocityEnemy] = 0.5;
    costs.pressure[costVelocityEnemy] = 0.5;
    costs.blocking[costVelocityEnemy] = 0.5;

    costs.name[costSameOpponent] = "costSameOpponent";
    costs.basic[costSameOpponent] = 0.2;
    costs.combo[costSameOpponent] = 0.0;
    costs.pressure[costSameOpponent] = 0.2;
    costs.blocking[costSameOpponent] = 0.1;


    auto result = getCombinedAndLowestWeights(costs.basic);
    costs.lowestCost = result.low;
    costs.combinedCost = result.high;
    result = getCombinedAndLowestWeights(costs.combo);
    if (result.low < costs.lowestCost) { costs.lowestCost = result.low; }
    if (result.high > costs.combinedCost) { costs.combinedCost = result.high + (result.high/2); }
    result = getCombinedAndLowestWeights(costs.pressure);
    if (result.low < costs.lowestCost) { costs.lowestCost = result.low; }
    if (result.high > costs.combinedCost) { costs.combinedCost = result.high + (result.high / 2); }
    result = getCombinedAndLowestWeights(costs.blocking);
    if (result.low < costs.lowestCost) { costs.lowestCost = result.low; }
    if (result.high > costs.combinedCost) { costs.combinedCost = result.high + (result.high / 2); }
}
struct HelperMapping
{
    int curHelperIndex;
    int caseHelperIndex;
    float comparisonValue;
};

struct recurseHelperMapping {
    float compValue;
    std::vector<HelperMapping*> hMaps;
};



struct debugCaseIndex {

    int replayIndex;
    int caseIndex;
    float compValue;
};
struct bestCaseContainer {
    int replayIndex;
    int caseIndex;
    float compValue;
    bool instantLearn;
};
struct bestCaseSelector {

    std::vector <bestCaseContainer> bcc;
    float bestCompValue = 9999999;
    float combinedValue = 0;
};
inline void updateDebugCaseIndex(std::vector<debugCaseIndex>& dci, debugCaseIndex& newDci) {
    for (int i = 0; i < dci.size(); i++) {

        if (i == 0 && newDci.compValue <= dci[i].compValue) {
            dci.insert(dci.begin(), newDci);
            return;
        }
        if (newDci.compValue <= dci[i].compValue) {
            dci.insert(dci.begin() + i, newDci);
            return;
        }
    }
    dci.insert(dci.end(), newDci);
    return;
}

inline bool checkBestCaseBetterThanNext(float bufComparison, float bestComparison, CbrCase* nextCase) {

    if (nextCase->getInputBufferSequence()) {
        return ((bestComparison * nextBestMulti) + nextBestAdd + nextBestAddInputSequence) < bufComparison;
    }
    return ((bestComparison * nextBestMulti + nextBestAdd) < bufComparison);

}
inline bool randomDifferenceSelection(float bufComparison, float bestComparison) {
    auto diff = bufComparison - bestComparison;

    if (diff < maxRandomDiff * -1) {
        return true;
    }
    if (diff > maxRandomDiff) {
        return false;
    }

    //if the new case is better than the old, give the old case a chance to stay depending on how much better the new case is
    if (diff < 0) {
        diff = abs(diff) * -5 + 0.5; // map the difference to a scale of 0 to 0.5 for easier probability calc.
        auto fl = RandomFloatC(0, 1);
        if (fl <= diff) { return false; }
        else { return true; }
    }
    //If the old case is better than the new case, give the new case a chance to be selected anyway.
    if (diff >= 0) {
        diff = diff * -5 + 0.5; // map the difference to a scale of 0 to 0.5 for easier probability calc.
        auto fl = RandomFloatC(0, 1);
        if (fl > diff) { return false; }
        else { return true; }
    }
}

inline void bestCaseCultivator(float bufComparison, int replayIndex, int caseIndex, bestCaseSelector& caseSelector, bool instantLearn) {
    auto diff = bufComparison - caseSelector.bestCompValue;
    if (bufComparison == 0) { bufComparison = 0.001; }
    if (diff < maxRandomDiff * -1) {
        caseSelector.bestCompValue = bufComparison;
        caseSelector.combinedValue = 1 / bufComparison;
        caseSelector.bcc.clear();
        bestCaseContainer b{ replayIndex, caseIndex, bufComparison, instantLearn};
        caseSelector.bcc.push_back(b);
    }
    else {
        if (diff <= maxRandomDiff) {
            if (diff < 0) {
                caseSelector.bestCompValue = bufComparison;
            }
            caseSelector.combinedValue += 1 / bufComparison;
            bestCaseContainer b{ replayIndex, caseIndex, bufComparison, instantLearn };
            caseSelector.bcc.push_back(b);
        }
    }
}

inline bestCaseContainer bestCaseSelectorFunc(bestCaseSelector& caseSelector) {
    int combinedPropability = caseSelector.combinedValue;
    float multi = static_cast<float>(100) / combinedPropability;
    float probSave = 0;
    float rand = RandomFloatC(0, 100);
    for (int i = 0; i < caseSelector.bcc.size(); i++) {
        float max = (1 / caseSelector.bcc[i].compValue) * multi;
        auto test = max + probSave;
        probSave += max;
        if (test > rand) {
            return caseSelector.bcc[i];
            break;
        }
    }
    bestCaseContainer b = { -1,-1,-1 };
    return b;

}

inline bool sameOrNextCaseCheck(int activeReplay, int activeCase, Metadata* curGamestate, int replayIndex, int caseIndex) {
    return replayIndex == activeReplay && (caseIndex == activeCase || caseIndex == activeCase + 1);
}
inline bool inputSequenceCheck(CbrReplayFile& replay, Metadata* curGamestate, int replayIndex, int caseIndex) {
    return caseIndex != 0 && replay.getCase(caseIndex)->getInputBufferSequence() == true;
}
inline bool meterCheck(CbrReplayFile& replay, Metadata* curGamestate, int replayIndex, int caseIndex) {
    return replay.getCase(caseIndex)->heatConsumed > curGamestate->heatMeter[0];
}
inline bool OdCheck(CbrReplayFile& replay, Metadata* curGamestate, int replayIndex, int caseIndex) {
    return replay.getCase(caseIndex)->overDriveConsumed != 0 && curGamestate->overdriveMeter[0] != 15000; //Max Burst value
}
//Checks if the next case can be used.
inline bool nextCaseValidityCheck(CbrReplayFile& replay, Metadata* curGamestate, int replayIndex, int caseIndex) {
    auto check1 = meterCheck(replay, curGamestate, replayIndex, caseIndex);//Has enough meter to use a super
    auto check2 = OdCheck(replay, curGamestate, replayIndex, caseIndex);//Can use overdrive
    return check1 && check2;
}
inline bool caseValidityCheck(int activeReplay, int activeCase, CbrReplayFile& replay, Metadata* curGamestate, int replayIndex, int caseIndex) {
    //The case beeing checked isent the same as the one that was just played.
    auto check1 = sameOrNextCaseCheck(activeReplay, activeCase, curGamestate, replayIndex, caseIndex); //The case beeing checked isent the same as the one that was just played.
    auto check2 = inputSequenceCheck(replay, curGamestate, replayIndex, caseIndex);//The case isent starting in the middle of a input sequence
    auto check3 = nextCaseValidityCheck(replay, curGamestate, replayIndex, caseIndex);//Meter and OD checks

    return check1 && check2 && check3;
}
inline void print_all(std::stringstream& ss) {
    // base case
}

template <class T, class... Ts>
inline void print_all(std::stringstream& ss, T const& first, Ts const&... rest) {
    ss << first << ", ";

    print_all(ss, rest...);
}

template<class F, class ...Args>
inline auto debug_call(F func, const char* str, std::string* debugText, float multi, Args... args) ->
decltype(std::forward<F>(func)(std::forward<Args>(args)...))
{

    if constexpr (std::is_same<decltype(std::forward<F>(func)(std::forward<Args>(args)...)), void>::value) {

        std::stringstream ss;
        ((ss << args << ", ") << ...);
        ss << str << "\n";

        func(args...);
    }
    else {
        auto res = func(args...) * multi;
        if (res != -1) {
            std::stringstream ss;
            //((ss << args), ...);
            ss << res << " = ";
            print_all(ss, args...);
            ss << str << "\n";
            *debugText += ss.str();
        }

        return  res;
    }
}
#define REFLECT_INVOKE(func, ...) (debug_call)(func, #func "(" #__VA_ARGS__ ") ", &debugText, __VA_ARGS__)

template<class F, class ...Args>
inline auto pure_call(F func, const char* str, float multi, Args... args) ->
decltype(std::forward<F>(func)(std::forward<Args>(args)...))
{
    if constexpr (std::is_same<decltype(std::forward<F>(func)(std::forward<Args>(args)...)), void>::value) {
        func(args...);
    }
    else {
        auto res = func(args...) * multi;
        return  res;
    }
}
#define PURE_INVOKE(func, ...) (pure_call)(func, #func "(" #__VA_ARGS__ ") ", __VA_ARGS__)

inline void setCurGamestateCosts(Metadata* curGamestate, costWeights& costs, std::array<float, 200>& curCosts) {
    curCosts = costs.basic;
    if (curGamestate->getHit()[1] == true) { curCosts = costs.combo; }
    if (curGamestate->getBlocking()[0] == true) { curCosts = costs.blocking; }
    if (curGamestate->getBlocking()[1] == true) { curCosts = costs.pressure; }
}

inline float comparisonFunctionQuick(Metadata* curGamestate, Metadata* caseGamestate, CbrReplayFile& replayFile, CbrCase* caseData, int replayIndex, int caseIndex, bool nextCaseCheck, std::array<float, 200>& curCosts, int framesActive, int activeReplay, int activeCase, bool instantLearnSameReplay) {
    float compValue = 0;

    bool buffVal = false;
    if (instantLearnSameReplay && !nextCaseCheck) {
        buffVal = sameOrNextCaseCheck(activeReplay, activeCase, curGamestate, replayIndex, caseIndex);
        if (buffVal) {
            //debugText += "sameOrNextCaseCheck Fail\n";
            buffVal = false;
            compValue += 999;
        }
        buffVal = inputSequenceCheck(replayFile, curGamestate, replayIndex, caseIndex);
        if (buffVal) {
            //debugText += "inputSequenceCheck Fail\n";
            buffVal = false;
            compValue += 999;
        }
    }
    buffVal = meterCheck(replayFile, curGamestate, replayIndex, caseIndex);
    if (buffVal) {
        //debugText += "meterCheck Fail\n";
        buffVal = false;
        compValue += 999;
    }
    buffVal = OdCheck(replayFile, curGamestate, replayIndex, caseIndex);
    if (buffVal) {
        //debugText += "OdCheck Fail\n";
        buffVal = false;
        compValue += 999;
    }
    
    return compValue;
    
}

inline float comparisonFunction(Metadata* curGamestate, Metadata* caseGamestate, CbrReplayFile& caseReplay, CbrCase* caseData, int replayIndex, int caseIndex, bool nextCaseCheck, std::array<float, 200>& curCosts, int framesActive, int activeReplay, int activeCase) {
    float compValue = 0;

    
    compValue += PURE_INVOKE(compIntState, curCosts[costSameOpponent], curGamestate->opponentId, caseReplay.getCharIds()[1]);
    

    //Velocity
    if (curGamestate->getFacing() == caseGamestate->getFacing()) {
        compValue += PURE_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][0], caseGamestate->velocity[0][0], maxXVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][1], caseGamestate->velocity[0][1], maxYVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][0], caseGamestate->velocity[1][0], maxXVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][1], caseGamestate->velocity[1][1], maxYVelocity);
    }
    else {
        compValue += PURE_INVOKE(compInt, curCosts[costVelocity], -curGamestate->velocity[0][0], caseGamestate->velocity[0][0], maxXVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][1], caseGamestate->velocity[0][1], maxYVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocityEnemy], -curGamestate->velocity[1][0], caseGamestate->velocity[1][0], maxXVelocity);
        compValue += PURE_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][1], caseGamestate->velocity[1][1], maxYVelocity);
    }


    compValue += PURE_INVOKE(compInt, curCosts[costAiHp], curGamestate->healthMeter[0], caseGamestate->healthMeter[0], maxHpDiff);
    compValue += PURE_INVOKE(compInt, curCosts[costEnemyHp], curGamestate->healthMeter[1], caseGamestate->healthMeter[1], maxHpDiff);
    //compValue += compCaseCooldown(caseData, framesActive) * costCaseCooldown;
    compValue += PURE_INVOKE(compCaseCooldown, curCosts[costCaseCooldown], caseData, framesActive);

    compValue += PURE_INVOKE(compRelativePosX, curCosts[costXDist], curGamestate->getPosX()[0], curGamestate->getPosX()[1], caseGamestate->getPosX()[0], caseGamestate->getPosX()[1]);
    //compRelativePosX(curGamestate->getPosX(), caseGamestate->getPosX()) * costXDist;
    compValue += PURE_INVOKE(compRelativePosY, curCosts[costYDist], curGamestate->getPosY()[0], curGamestate->getPosY()[1], caseGamestate->getPosY()[0], caseGamestate->getPosY()[1]);
    //compValue += compRelativePosY(curGamestate->getPosY(), caseGamestate->getPosY()) * costYDist;

    compValue += PURE_INVOKE(compMaxDistanceAttack, curCosts[costMinDistanceAttack], curGamestate->getPosX()[0], curGamestate->getPosX()[1], caseGamestate->getPosX()[0], caseGamestate->getPosX()[1], caseGamestate->hitMinX);
    //testvalue += PURE_INVOKE(compMaxDistanceAttack, curCosts[costMinDistanceAttack], curGamestate->getPosY()[0], curGamestate->getPosY()[1], caseGamestate->getPosY()[0], caseGamestate->getPosY()[1], caseGamestate->hitMinY);

    float testvalue = 0;
    testvalue += PURE_INVOKE(compAct, curCosts[costAiState], curGamestate->getCurrentActId()[0], caseGamestate->getCurrentActId()[0]);
    //testvalue += PURE_INVOKE(compStateHash, curCosts[costAiState], curGamestate->getCurrentActionHash()[0], caseGamestate->getCurrentActionHash()[0]);
    //compValue += compStateHash(curGamestate->getCurrentActionHash()[0], caseGamestate->getCurrentActionHash()[0]) * costAiState;
    if (testvalue != 0 && !curGamestate->getNeutral()[0]) {
        testvalue += curCosts[costNonNeutralState];
    }
    compValue += testvalue;
    compValue += PURE_INVOKE(compAct, curCosts[costEnemyState], curGamestate->getCurrentActId()[1], caseGamestate->getCurrentActId()[1]);
    //compValue += PURE_INVOKE(compStateHash, curCosts[costEnemyState], curGamestate->getCurrentActionHash()[1], caseGamestate->getCurrentActionHash()[1]);
    //compValue += compStateHash(curGamestate->getCurrentActionHash()[1], caseGamestate->getCurrentActionHash()[1]) * costEnemyState;
    compValue += PURE_INVOKE(compAct, curCosts[costlastAiState], curGamestate->getLastActId()[0], caseGamestate->getLastActId()[0]);
    //compValue += PURE_INVOKE(compStateHash, curCosts[costlastAiState], curGamestate->getLastActionHash()[0], caseGamestate->getLastActionHash()[0]);
    //compValue += compStateHash(curGamestate->getLastActionHash()[0], caseGamestate->getLastActionHash()[0]) * costlastAiState;
    compValue += PURE_INVOKE(compAct, curCosts[costlastEnemyState], curGamestate->getLastActId()[1], caseGamestate->getLastActId()[1]);
    //compValue += PURE_INVOKE(compStateHash, curCosts[costlastEnemyState], curGamestate->getLastActionHash()[1], caseGamestate->getLastActionHash()[1]);
    //compValue += compStateHash(curGamestate->getLastActionHash()[1], caseGamestate->getLastActionHash()[1]) * costlastEnemyState;
    compValue += PURE_INVOKE(compNeutralState, curCosts[costAiNeutral], curGamestate->getNeutral()[0], caseGamestate->getNeutral()[0]);
    //compValue += compNeutralState(curGamestate->getNeutral()[0], caseGamestate->getNeutral()[0]) * costAiNeutral;
    compValue += PURE_INVOKE(compNeutralState, curCosts[costEnemyNeutral], curGamestate->getNeutral()[1], caseGamestate->getNeutral()[1]);
    //compValue += compNeutralState(curGamestate->getNeutral()[1], caseGamestate->getNeutral()[1]) * costEnemyNeutral;
    compValue += PURE_INVOKE(compBool, curCosts[costAiAttack], curGamestate->getAttack()[0], caseGamestate->getAttack()[0]);
    //compValue += compBool(curGamestate->getAttack()[0], caseGamestate->getAttack()[0]) * costAiAttack;
    compValue += PURE_INVOKE(compBool, curCosts[costEnemyAttack], curGamestate->getAttack()[1], caseGamestate->getAttack()[1]);
    //compValue += compBool(curGamestate->getAttack()[1], caseGamestate->getAttack()[1]) * costEnemyAttack;
    compValue += PURE_INVOKE(compAirborneState, curCosts[costAiAir], curGamestate->getAir()[0], caseGamestate->getAir()[0]);
    //compValue += compAirborneState(curGamestate->getAir()[0], caseGamestate->getAir()[0]) * costAiAir;
    compValue += PURE_INVOKE(compAirborneState, curCosts[costEnemyAir], curGamestate->getAir()[1], caseGamestate->getAir()[1]);
    //compValue += compAirborneState(curGamestate->getAir()[1], caseGamestate->getAir()[1]) * costEnemyAir;
    compValue += PURE_INVOKE(compWakeupState, curCosts[costAiWakeup], curGamestate->getWakeup()[0], caseGamestate->getWakeup()[0]);
    //compValue += compWakeupState(curGamestate->getWakeup()[0], caseGamestate->getWakeup()[0]) * costAiWakeup;
    compValue += PURE_INVOKE(compWakeupState, curCosts[costEnemyWakeup], curGamestate->getWakeup()[1], caseGamestate->getWakeup()[1]);
    //compValue += compWakeupState(curGamestate->getWakeup()[1], caseGamestate->getWakeup()[1]) * costEnemyWakeup;
    compValue += PURE_INVOKE(compBlockState, curCosts[costAiBlocking], curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0]);
    //compValue += compBlockState(curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0]) * costAiBlocking;
    compValue += PURE_INVOKE(compBlockState, curCosts[costEnemyBlocking], curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1]);
    compValue += PURE_INVOKE(compBlockStun, curCosts[costAiBlocking], curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0], curGamestate->getBlockstun()[0], caseGamestate->getBlockstun()[0]);
    compValue += PURE_INVOKE(compBlockStun, curCosts[costEnemyBlocking], curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1], curGamestate->getBlockstun()[1], caseGamestate->getBlockstun()[1]);
    //compValue += compBlockState(curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1]) * costEnemyBlocking;
    compValue += PURE_INVOKE(compHitState, curCosts[costAiHit], curGamestate->getHit()[0], caseGamestate->getHit()[0]);
    //compValue += compHitState(curGamestate->getHit()[0], caseGamestate->getHit()[0]) * costAiHit;
    compValue += PURE_INVOKE(compHitState, curCosts[costEnemyHit], curGamestate->getHit()[1], caseGamestate->getHit()[1]);
    //compValue += compHitState(curGamestate->getHit()[1], caseGamestate->getHit()[1]) * costEnemyHit;
    compValue += PURE_INVOKE(compGetHitThisFrameState, curCosts[costAiHitThisFrame], curGamestate->getHitThisFrame()[0], caseGamestate->getHitThisFrame()[0]);
    //compValue += compGetHitThisFrameState(curGamestate->getHitThisFrame()[0], caseGamestate->getHitThisFrame()[0]) * costAiHitThisFrame;
    compValue += PURE_INVOKE(compGetHitThisFrameState, curCosts[costEnemyHitThisFrame], curGamestate->getHitThisFrame()[1], caseGamestate->getHitThisFrame()[1]);
    //compValue += compGetHitThisFrameState(curGamestate->getHitThisFrame()[1], caseGamestate->getHitThisFrame()[1]) * costEnemyHitThisFrame;
    compValue += PURE_INVOKE(compBlockingThisFrameState, curCosts[costAiBlockThisFrame], curGamestate->getBlockThisFrame()[0], caseGamestate->getBlockThisFrame()[0]);
    //compValue += compBlockingThisFrameState(curGamestate->getBlockThisFrame()[0], caseGamestate->getBlockThisFrame()[0]) * costAiBlockThisFrame;
    compValue += PURE_INVOKE(compBlockingThisFrameState, curCosts[costEnemyBlockhisFrame], curGamestate->getBlockThisFrame()[1], caseGamestate->getBlockThisFrame()[1]);
    //compValue += compBlockingThisFrameState(curGamestate->getBlockThisFrame()[1], caseGamestate->getBlockThisFrame()[1]) * costEnemyBlockhisFrame;
    compValue += PURE_INVOKE(compCrouching, curCosts[costAiCrouching], curGamestate->getCrouching()[0], caseGamestate->getCrouching()[0]);
    //compValue += compCrouching(curGamestate->getCrouching()[0], caseGamestate->getCrouching()[0]) * costAiCrouching;
    compValue += PURE_INVOKE(compCrouching, curCosts[costEnemyCrouching], curGamestate->getCrouching()[1], caseGamestate->getCrouching()[1]);
    //compValue += compCrouching(curGamestate->getCrouching()[1], caseGamestate->getCrouching()[1]) * costEnemyCrouching;
    compValue += PURE_INVOKE(compIntState, curCosts[costMatchState], curGamestate->matchState, caseGamestate->matchState);
    //compValue += compIntState(curGamestate->matchState, caseGamestate->matchState) * costMatchState;
    compValue += PURE_INVOKE(compBool, curCosts[costAiOverdriveState], curGamestate->overdriveTimeleft[0] > 0, caseGamestate->overdriveTimeleft[0] > 0);
    //compValue += compBool(curGamestate->overdriveTimeleft[0] > 0, caseGamestate->overdriveTimeleft[0] > 0) * costAiOverdriveState;
    compValue += PURE_INVOKE(compBool, curCosts[costEnemyOverdriveState], curGamestate->overdriveTimeleft[1] > 0, caseGamestate->overdriveTimeleft[1] > 0);
    //compValue += compBool(curGamestate->overdriveTimeleft[1] > 0, caseGamestate->overdriveTimeleft[1] > 0) * costEnemyOverdriveState;



    if (curGamestate->getHit()[0] == true) {
        compValue += PURE_INVOKE(compInt, curCosts[costAiProration], curGamestate->getComboProration()[0], caseGamestate->getComboProration()[0], maxProration);
        //compValue += compInt(curGamestate->getComboProration()[0], caseGamestate->getComboProration()[0], maxProration) * costAiProration;
        compValue += PURE_INVOKE(compIntState, curCosts[costAiStarterRating], curGamestate->getStarterRating()[0], caseGamestate->getStarterRating()[0]);
        //compValue += compIntState(curGamestate->getStarterRating()[0], caseGamestate->getStarterRating()[0]) * costAiStarterRating;
        compValue += PURE_INVOKE(compInt, curCosts[costAiComboTime], curGamestate->getComboTime()[0], caseGamestate->getComboTime()[0], maxComboTime);
        //compValue += compInt(curGamestate->getComboTime()[0], caseGamestate->getComboTime()[0], maxComboTime) * costAiComboTime;

    }
    if (curGamestate->getHit()[1] == true) {
        compValue += PURE_INVOKE(compInt, curCosts[costEnemyProration], curGamestate->getComboProration()[1], caseGamestate->getComboProration()[1], maxProration);
        //compValue += compInt(curGamestate->getComboProration()[1], caseGamestate->getComboProration()[1], maxProration) * costEnemyProration;
        compValue += PURE_INVOKE(compIntState, curCosts[costEnemyStarterRating], curGamestate->getStarterRating()[1], caseGamestate->getStarterRating()[1]);
        //compValue += compIntState(curGamestate->getStarterRating()[1], caseGamestate->getStarterRating()[1]) * costEnemyStarterRating;
        compValue += PURE_INVOKE(compInt, curCosts[costEnemyComboTime], curGamestate->getComboTime()[1], caseGamestate->getComboTime()[1], maxComboTime);
        //compValue += compInt(curGamestate->getComboTime()[1], caseGamestate->getComboTime()[1], maxComboTime) * costEnemyComboTime;
        compValue += PURE_INVOKE(compDistanceToWall, curCosts[costWallDistCombo], curGamestate->getPosX()[0], caseGamestate->getPosX()[0], curGamestate->getFacing(), caseGamestate->getFacing());
        //compValue += compDistanceToWall(curGamestate->getPosX(), caseGamestate->getPosX(), curGamestate->getFacing(), caseGamestate->getFacing()) * costWallDistCombo;
    }
    else {
        compValue += PURE_INVOKE(compDistanceToWall, curCosts[costWallDist], curGamestate->getPosX()[0], caseGamestate->getPosX()[0], curGamestate->getFacing(), caseGamestate->getFacing());
        //compValue += compDistanceToWall(curGamestate->getPosX(), caseGamestate->getPosX(), curGamestate->getFacing(), caseGamestate->getFacing()) * costWallDist;
    }

    switch (caseReplay.getCharIds()[0])
    {
    case 1: //Sol
        compValue += PURE_INVOKE(compBool, curCosts[costSoDI], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        break;
    case 4: //Millia
        compValue += PURE_INVOKE(compIntState, curCosts[costMiSFActive], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 7: //Chipp
        compValue += PURE_INVOKE(compIntState, curCosts[costChShuriken], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costChInvis], curGamestate->CharSpecific2[0] > 0, caseGamestate->CharSpecific2[0] > 0);
        break;
    case 8: //Eddie
        compValue += PURE_INVOKE(compInt, curCosts[costEdGauge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 1200);
        compValue += PURE_INVOKE(compIntState, curCosts[costEdRecovering], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costEdType], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costEdActive], curGamestate->CharSpecific4[0], caseGamestate->CharSpecific4[0]);
        break;
    case 9: //Baiken
        compValue += PURE_INVOKE(compBool, curCosts[costBaMovementSeal], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaCounterSeal], curGamestate->CharSpecific2[0] > 0, caseGamestate->CharSpecific2[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaGuardSeal], curGamestate->CharSpecific3[0] > 0, caseGamestate->CharSpecific3[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaPSeal], curGamestate->CharSpecific5[0] > 0, caseGamestate->CharSpecific5[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaKSeal], curGamestate->CharSpecific6[0] > 0, caseGamestate->CharSpecific6[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaSSeal], curGamestate->CharSpecific7[0] > 0, caseGamestate->CharSpecific7[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaHSeal], curGamestate->CharSpecific8[0] > 0, caseGamestate->CharSpecific8[0] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaDSeal], curGamestate->CharSpecific9[0] > 0, caseGamestate->CharSpecific9[0] > 0);
        break;
    case 10: //Faust
        compValue += PURE_INVOKE(compIntState, curCosts[costFaItemActive], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 11: //Testament
        compValue += PURE_INVOKE(compIntState, curCosts[costTePuppets], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costTeCrowAttackNo], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costTeCrowActive], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costTeCrowPattern], curGamestate->CharSpecific4[0], caseGamestate->CharSpecific4[0]);
        break;
    case 12: //Jam
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiK], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiS], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiH], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        break;
    case 14: //Johnny
        compValue += PURE_INVOKE(compIntState, curCosts[costJoMFlvl], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costJoCoins], curGamestate->CharSpecific2[0] < 8, caseGamestate->CharSpecific2[0] < 8);
        compValue += PURE_INVOKE(compBool, curCosts[costJoStance], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        break;
    case 15: //Venom
        compValue += PURE_INVOKE(compInt, curCosts[costVeCharge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 20);
        compValue += PURE_INVOKE(compBool, curCosts[costVePball], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeKball], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeSball], curGamestate->CharSpecific4[0], caseGamestate->CharSpecific4[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeHball], curGamestate->CharSpecific5[0], caseGamestate->CharSpecific5[0]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeDball], curGamestate->CharSpecific6[0], caseGamestate->CharSpecific6[0]);
        break;
    case 19: //Zappa
        compValue += PURE_INVOKE(compIntState, curCosts[costZaSummon], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compInt, curCosts[costZaGauge], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0], 500);
        break;
    case 20: //Bridget
        compValue += PURE_INVOKE(compIntState, curCosts[costBrYoyo], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 21: //Robo-Ky
        compValue += PURE_INVOKE(compInt, curCosts[costRoHeat], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 3000);
        compValue += PURE_INVOKE(compInt, curCosts[costRoTimer], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0], 200);
        break;
    case 22: //A.B.A
        compValue += PURE_INVOKE(compIntState, curCosts[costAbMode], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += PURE_INVOKE(compIntState, curCosts[costAbPacks], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += PURE_INVOKE(compInt, curCosts[costAbGauge], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0], 3000);
        break;
    case 23: //Order-Sol
        compValue += PURE_INVOKE(compInt, curCosts[costOsGauge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 2000);
        break;
    case 25: //Justice
        compValue += PURE_INVOKE(compBool, curCosts[costJuInstall], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        break;
    default:
        break;
    }
    switch (caseReplay.getCharIds()[1])
    {
    case 1: //Sol
        compValue += PURE_INVOKE(compBool, curCosts[costSoDIEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        break;
    case 4: //Millia
        compValue += PURE_INVOKE(compIntState, curCosts[costMiSFActiveEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 7: //Chipp
        compValue += PURE_INVOKE(compIntState, curCosts[costChShurikenEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costChInvisEnemy], curGamestate->CharSpecific2[1] > 0, caseGamestate->CharSpecific2[1] > 0);
        break;
    case 8: //Eddie
        compValue += PURE_INVOKE(compInt, curCosts[costEdGaugeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 1200);
        compValue += PURE_INVOKE(compIntState, curCosts[costEdRecoveringEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costEdTypeEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costEdActiveEnemy], curGamestate->CharSpecific4[1], caseGamestate->CharSpecific4[1]);
        break;
    case 9: //Baiken
        compValue += PURE_INVOKE(compBool, curCosts[costBaMovementSealEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaCounterSealEnemy], curGamestate->CharSpecific2[1] > 0, caseGamestate->CharSpecific2[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaGuardSealEnemy], curGamestate->CharSpecific3[1] > 0, caseGamestate->CharSpecific3[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaPSealEnemy], curGamestate->CharSpecific5[1] > 0, caseGamestate->CharSpecific5[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaKSealEnemy], curGamestate->CharSpecific6[1] > 0, caseGamestate->CharSpecific6[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaSSealEnemy], curGamestate->CharSpecific7[1] > 0, caseGamestate->CharSpecific7[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaHSealEnemy], curGamestate->CharSpecific8[1] > 0, caseGamestate->CharSpecific8[1] > 0);
        compValue += PURE_INVOKE(compBool, curCosts[costBaDSealEnemy], curGamestate->CharSpecific9[1] > 0, caseGamestate->CharSpecific9[1] > 0);
        break;
    case 10: //Faust
        compValue += PURE_INVOKE(compIntState, curCosts[costFaItemActiveEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 11: //Testament
        compValue += PURE_INVOKE(compIntState, curCosts[costTePuppetsEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costTeCrowAttackNoEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costTeCrowActiveEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costTeCrowPatternEnemy], curGamestate->CharSpecific4[1], caseGamestate->CharSpecific4[1]);
        break;
    case 12: //Jam
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiKEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiSEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costJaAsanagiHEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        break;
    case 14: //Johnny
        compValue += PURE_INVOKE(compIntState, curCosts[costJoMFlvlEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costJoCoinsEnemy], curGamestate->CharSpecific2[1] < 8, caseGamestate->CharSpecific2[1] < 8);
        compValue += PURE_INVOKE(compBool, curCosts[costJoStanceEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        break;
    case 15: //Venom
        compValue += PURE_INVOKE(compInt, curCosts[costVeChargeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 20);
        compValue += PURE_INVOKE(compBool, curCosts[costVePballEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeKballEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeSballEnemy], curGamestate->CharSpecific4[1], caseGamestate->CharSpecific4[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeHballEnemy], curGamestate->CharSpecific5[1], caseGamestate->CharSpecific5[1]);
        compValue += PURE_INVOKE(compBool, curCosts[costVeDballEnemy], curGamestate->CharSpecific6[1], caseGamestate->CharSpecific6[1]);
        break;
    case 19: //Zappa
        compValue += PURE_INVOKE(compIntState, curCosts[costZaSummonEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compInt, curCosts[costZaGaugeEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1], 500);
        break;
    case 20: //Bridget
        compValue += PURE_INVOKE(compIntState, curCosts[costBrYoyoEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 21: //Robo-Ky
        compValue += PURE_INVOKE(compInt, curCosts[costRoHeatEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 3000);
        compValue += PURE_INVOKE(compInt, curCosts[costRoTimerEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1], 200);
        break;
    case 22: //A.B.A
        compValue += PURE_INVOKE(compIntState, curCosts[costAbModeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += PURE_INVOKE(compIntState, curCosts[costAbPacksEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += PURE_INVOKE(compInt, curCosts[costAbGaugeEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1], 3000);
        break;
    case 23: //Order-Sol
        compValue += PURE_INVOKE(compInt, curCosts[costOsGaugeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 2000);
        break;
    case 25: //Justice
        compValue += PURE_INVOKE(compBool, curCosts[costJuInstallEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        break;
    default:
        break;
    }
    return compValue;

}


inline float comparisonFunctionDebug(Metadata * curGamestate, Metadata * caseGamestate, CbrReplayFile& caseReplay, CbrCase* caseData, int replayIndex, int caseIndex, bool nextCaseCheck, std::array<float, 200>&curCosts, int framesActive, int activeReplay, int activeCase, std::string & debugText, bool instantLearnSameReplay) {
    float compValue = 0;

    bool buffVal = false;
    if (instantLearnSameReplay && !nextCaseCheck) {
        buffVal = sameOrNextCaseCheck(activeReplay, activeCase, curGamestate, replayIndex, caseIndex);
        if (buffVal) {
            debugText += "sameOrNextCaseCheck Fail\n";
            buffVal = false;
            compValue += 999;
        }
        buffVal = inputSequenceCheck(caseReplay, curGamestate, replayIndex, caseIndex);
        if (buffVal) {
            debugText += "inputSequenceCheck Fail\n";
            buffVal = false;
            compValue += 999;
        }
    }
    buffVal = meterCheck(caseReplay, curGamestate, replayIndex, caseIndex);
    if (buffVal) {
        debugText += "meterCheck Fail\n";
        buffVal = false;
        compValue += 999;
    }
    buffVal = OdCheck(caseReplay, curGamestate, replayIndex, caseIndex);
    if (buffVal) {
        debugText += "OdCheck Fail\n";
        buffVal = false;
        compValue += 999;
    }
    if (compValue > 0) {
        return compValue;
    }

    compValue += REFLECT_INVOKE(compIntState, curCosts[costSameOpponent], curGamestate->opponentId, caseReplay.getCharIds()[1]);
    //Velocity
    if (curGamestate->getFacing() == caseGamestate->getFacing()) {
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][0], caseGamestate->velocity[0][0], maxXVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][1], caseGamestate->velocity[0][1], maxYVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][0], caseGamestate->velocity[1][0], maxXVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][1], caseGamestate->velocity[1][1], maxYVelocity);
    }
    else {
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocity], -curGamestate->velocity[0][0], caseGamestate->velocity[0][0], maxXVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocity], curGamestate->velocity[0][1], caseGamestate->velocity[0][1], maxYVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocityEnemy], -curGamestate->velocity[1][0], caseGamestate->velocity[1][0], maxXVelocity);
        compValue += REFLECT_INVOKE(compInt, curCosts[costVelocityEnemy], curGamestate->velocity[1][1], caseGamestate->velocity[1][1], maxYVelocity);
    }


    compValue += REFLECT_INVOKE(compInt, curCosts[costAiHp], curGamestate->healthMeter[0], caseGamestate->healthMeter[0], maxHpDiff);
    compValue += REFLECT_INVOKE(compInt, curCosts[costEnemyHp], curGamestate->healthMeter[1], caseGamestate->healthMeter[1], maxHpDiff);
    //compValue += compCaseCooldown(caseData, framesActive) * costCaseCooldown;
    compValue += REFLECT_INVOKE(compCaseCooldown, curCosts[costCaseCooldown], caseData, framesActive);

    compValue += REFLECT_INVOKE(compRelativePosX, curCosts[costXDist], curGamestate->getPosX()[0], curGamestate->getPosX()[1], caseGamestate->getPosX()[0], caseGamestate->getPosX()[1]);
    //compRelativePosX(curGamestate->getPosX(), caseGamestate->getPosX()) * costXDist;
    compValue += REFLECT_INVOKE(compRelativePosY, curCosts[costYDist], curGamestate->getPosY()[0], curGamestate->getPosY()[1], caseGamestate->getPosY()[0], caseGamestate->getPosY()[1]);
    //compValue += compRelativePosY(curGamestate->getPosY(), caseGamestate->getPosY()) * costYDist;

    compValue += REFLECT_INVOKE(compMaxDistanceAttack, curCosts[costMinDistanceAttack], curGamestate->getPosX()[0], curGamestate->getPosX()[1], caseGamestate->getPosX()[0], caseGamestate->getPosX()[1], caseGamestate->hitMinX);
    //testvalue += REFLECT_INVOKE(compMaxDistanceAttack, curCosts[costMinDistanceAttack], curGamestate->getPosY()[0], curGamestate->getPosY()[1], caseGamestate->getPosY()[0], caseGamestate->getPosY()[1], caseGamestate->hitMinY);

    float testvalue = 0;
    //testvalue += REFLECT_INVOKE(compState, curCosts[costAiState], curGamestate->getCurrentAction()[0], caseGamestate->getCurrentAction()[0]);
    testvalue += REFLECT_INVOKE(compAct, curCosts[costAiState], curGamestate->getCurrentActId()[0], caseGamestate->getCurrentActId()[0]);
    //compValue += compStateHash(curGamestate->getCurrentActionHash()[0], caseGamestate->getCurrentActionHash()[0]) * costAiState;
    if (testvalue != 0 && !curGamestate->getNeutral()[0]) {
        testvalue += curCosts[costNonNeutralState];
    }
    compValue += testvalue;
    //compValue += REFLECT_INVOKE(compState, curCosts[costEnemyState], curGamestate->getCurrentAction()[1], caseGamestate->getCurrentAction()[1]);
    compValue += REFLECT_INVOKE(compAct, curCosts[costEnemyState], curGamestate->getCurrentActId()[1], caseGamestate->getCurrentActId()[1]);
    //compValue += compStateHash(curGamestate->getCurrentActionHash()[1], caseGamestate->getCurrentActionHash()[1]) * costEnemyState;
    compValue += REFLECT_INVOKE(compAct, curCosts[costlastAiState], curGamestate->getLastActId()[0], caseGamestate->getLastActId()[0]);
    //compValue += REFLECT_INVOKE(compState, curCosts[costlastAiState], curGamestate->getLastAction()[0], caseGamestate->getLastAction()[0]);
    //compValue += compStateHash(curGamestate->getLastActionHash()[0], caseGamestate->getLastActionHash()[0]) * costlastAiState;
    compValue += REFLECT_INVOKE(compAct, curCosts[costlastEnemyState], curGamestate->getLastActId()[1], caseGamestate->getLastActId()[1]);
    //compValue += REFLECT_INVOKE(compState, curCosts[costlastEnemyState], curGamestate->getLastAction()[1], caseGamestate->getLastAction()[1]);
    //compValue += compStateHash(curGamestate->getLastActionHash()[1], caseGamestate->getLastActionHash()[1]) * costlastEnemyState;
    compValue += REFLECT_INVOKE(compNeutralState, curCosts[costAiNeutral], curGamestate->getNeutral()[0], caseGamestate->getNeutral()[0]);
    //compValue += compNeutralState(curGamestate->getNeutral()[0], caseGamestate->getNeutral()[0]) * costAiNeutral;
    compValue += REFLECT_INVOKE(compNeutralState, curCosts[costEnemyNeutral], curGamestate->getNeutral()[1], caseGamestate->getNeutral()[1]);
    //compValue += compNeutralState(curGamestate->getNeutral()[1], caseGamestate->getNeutral()[1]) * costEnemyNeutral;
    compValue += REFLECT_INVOKE(compBool, curCosts[costAiAttack], curGamestate->getAttack()[0], caseGamestate->getAttack()[0]);
    //compValue += compBool(curGamestate->getAttack()[0], caseGamestate->getAttack()[0]) * costAiAttack;
    compValue += REFLECT_INVOKE(compBool, curCosts[costEnemyAttack], curGamestate->getAttack()[1], caseGamestate->getAttack()[1]);
    //compValue += compBool(curGamestate->getAttack()[1], caseGamestate->getAttack()[1]) * costEnemyAttack;
    compValue += REFLECT_INVOKE(compAirborneState, curCosts[costAiAir], curGamestate->getAir()[0], caseGamestate->getAir()[0]);
    //compValue += compAirborneState(curGamestate->getAir()[0], caseGamestate->getAir()[0]) * costAiAir;
    compValue += REFLECT_INVOKE(compAirborneState, curCosts[costEnemyAir], curGamestate->getAir()[1], caseGamestate->getAir()[1]);
    //compValue += compAirborneState(curGamestate->getAir()[1], caseGamestate->getAir()[1]) * costEnemyAir;
    compValue += REFLECT_INVOKE(compWakeupState, curCosts[costAiWakeup], curGamestate->getWakeup()[0], caseGamestate->getWakeup()[0]);
    //compValue += compWakeupState(curGamestate->getWakeup()[0], caseGamestate->getWakeup()[0]) * costAiWakeup;
    compValue += REFLECT_INVOKE(compWakeupState, curCosts[costEnemyWakeup], curGamestate->getWakeup()[1], caseGamestate->getWakeup()[1]);
    //compValue += compWakeupState(curGamestate->getWakeup()[1], caseGamestate->getWakeup()[1]) * costEnemyWakeup;
    compValue += REFLECT_INVOKE(compBlockState, curCosts[costAiBlocking], curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0]);
    //compValue += compBlockState(curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0]) * costAiBlocking;
    compValue += REFLECT_INVOKE(compBlockState, curCosts[costEnemyBlocking], curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1]);
    compValue += REFLECT_INVOKE(compBlockStun, curCosts[costAiBlocking], curGamestate->getBlocking()[0], caseGamestate->getBlocking()[0], curGamestate->getBlockstun()[0], caseGamestate->getBlockstun()[0]);
    compValue += REFLECT_INVOKE(compBlockStun, curCosts[costEnemyBlocking], curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1], curGamestate->getBlockstun()[1], caseGamestate->getBlockstun()[1]);
    //compValue += compBlockState(curGamestate->getBlocking()[1], caseGamestate->getBlocking()[1]) * costEnemyBlocking;
    compValue += REFLECT_INVOKE(compHitState, curCosts[costAiHit], curGamestate->getHit()[0], caseGamestate->getHit()[0]);
    //compValue += compHitState(curGamestate->getHit()[0], caseGamestate->getHit()[0]) * costAiHit;
    compValue += REFLECT_INVOKE(compHitState, curCosts[costEnemyHit], curGamestate->getHit()[1], caseGamestate->getHit()[1]);
    //compValue += compHitState(curGamestate->getHit()[1], caseGamestate->getHit()[1]) * costEnemyHit;
    compValue += REFLECT_INVOKE(compGetHitThisFrameState, curCosts[costAiHitThisFrame], curGamestate->getHitThisFrame()[0], caseGamestate->getHitThisFrame()[0]);
    //compValue += compGetHitThisFrameState(curGamestate->getHitThisFrame()[0], caseGamestate->getHitThisFrame()[0]) * costAiHitThisFrame;
    compValue += REFLECT_INVOKE(compGetHitThisFrameState, curCosts[costEnemyHitThisFrame], curGamestate->getHitThisFrame()[1], caseGamestate->getHitThisFrame()[1]);
    //compValue += compGetHitThisFrameState(curGamestate->getHitThisFrame()[1], caseGamestate->getHitThisFrame()[1]) * costEnemyHitThisFrame;
    compValue += REFLECT_INVOKE(compBlockingThisFrameState, curCosts[costAiBlockThisFrame], curGamestate->getBlockThisFrame()[0], caseGamestate->getBlockThisFrame()[0]);
    //compValue += compBlockingThisFrameState(curGamestate->getBlockThisFrame()[0], caseGamestate->getBlockThisFrame()[0]) * costAiBlockThisFrame;
    compValue += REFLECT_INVOKE(compBlockingThisFrameState, curCosts[costEnemyBlockhisFrame], curGamestate->getBlockThisFrame()[1], caseGamestate->getBlockThisFrame()[1]);
    //compValue += compBlockingThisFrameState(curGamestate->getBlockThisFrame()[1], caseGamestate->getBlockThisFrame()[1]) * costEnemyBlockhisFrame;
    compValue += REFLECT_INVOKE(compCrouching, curCosts[costAiCrouching], curGamestate->getCrouching()[0], caseGamestate->getCrouching()[0]);
    //compValue += compCrouching(curGamestate->getCrouching()[0], caseGamestate->getCrouching()[0]) * costAiCrouching;
    compValue += REFLECT_INVOKE(compCrouching, curCosts[costEnemyCrouching], curGamestate->getCrouching()[1], caseGamestate->getCrouching()[1]);
    //compValue += compCrouching(curGamestate->getCrouching()[1], caseGamestate->getCrouching()[1]) * costEnemyCrouching;
    compValue += REFLECT_INVOKE(compIntState, curCosts[costMatchState], curGamestate->matchState, caseGamestate->matchState);
    //compValue += compIntState(curGamestate->matchState, caseGamestate->matchState) * costMatchState;
    compValue += REFLECT_INVOKE(compBool, curCosts[costAiOverdriveState], curGamestate->overdriveTimeleft[0] > 0, caseGamestate->overdriveTimeleft[0] > 0);
    //compValue += compBool(curGamestate->overdriveTimeleft[0] > 0, caseGamestate->overdriveTimeleft[0] > 0) * costAiOverdriveState;
    compValue += REFLECT_INVOKE(compBool, curCosts[costEnemyOverdriveState], curGamestate->overdriveTimeleft[1] > 0, caseGamestate->overdriveTimeleft[1] > 0);
    //compValue += compBool(curGamestate->overdriveTimeleft[1] > 0, caseGamestate->overdriveTimeleft[1] > 0) * costEnemyOverdriveState;



    if (curGamestate->getHit()[0] == true) {
        compValue += REFLECT_INVOKE(compInt, curCosts[costAiProration], curGamestate->getComboProration()[0], caseGamestate->getComboProration()[0], maxProration);
        //compValue += compInt(curGamestate->getComboProration()[0], caseGamestate->getComboProration()[0], maxProration) * costAiProration;
        compValue += REFLECT_INVOKE(compIntState, curCosts[costAiStarterRating], curGamestate->getStarterRating()[0], caseGamestate->getStarterRating()[0]);
        //compValue += compIntState(curGamestate->getStarterRating()[0], caseGamestate->getStarterRating()[0]) * costAiStarterRating;
        compValue += REFLECT_INVOKE(compInt, curCosts[costAiComboTime], curGamestate->getComboTime()[0], caseGamestate->getComboTime()[0], maxComboTime);
        //compValue += compInt(curGamestate->getComboTime()[0], caseGamestate->getComboTime()[0], maxComboTime) * costAiComboTime;

    }
    if (curGamestate->getHit()[1] == true) {
        compValue += REFLECT_INVOKE(compInt, curCosts[costEnemyProration], curGamestate->getComboProration()[1], caseGamestate->getComboProration()[1], maxProration);
        //compValue += compInt(curGamestate->getComboProration()[1], caseGamestate->getComboProration()[1], maxProration) * costEnemyProration;
        compValue += REFLECT_INVOKE(compIntState, curCosts[costEnemyStarterRating], curGamestate->getStarterRating()[1], caseGamestate->getStarterRating()[1]);
        //compValue += compIntState(curGamestate->getStarterRating()[1], caseGamestate->getStarterRating()[1]) * costEnemyStarterRating;
        compValue += REFLECT_INVOKE(compInt, curCosts[costEnemyComboTime], curGamestate->getComboTime()[1], caseGamestate->getComboTime()[1], maxComboTime);
        //compValue += compInt(curGamestate->getComboTime()[1], caseGamestate->getComboTime()[1], maxComboTime) * costEnemyComboTime;
        compValue += REFLECT_INVOKE(compDistanceToWall, curCosts[costWallDistCombo], curGamestate->getPosX()[0], caseGamestate->getPosX()[0], curGamestate->getFacing(), caseGamestate->getFacing());
        //compValue += compDistanceToWall(curGamestate->getPosX(), caseGamestate->getPosX(), curGamestate->getFacing(), caseGamestate->getFacing()) * costWallDistCombo;
    }
    else {
        compValue += REFLECT_INVOKE(compDistanceToWall, curCosts[costWallDist], curGamestate->getPosX()[0], caseGamestate->getPosX()[0], curGamestate->getFacing(), caseGamestate->getFacing());
        //compValue += compDistanceToWall(curGamestate->getPosX(), caseGamestate->getPosX(), curGamestate->getFacing(), caseGamestate->getFacing()) * costWallDist;
    }

    switch (caseReplay.getCharIds()[0])
    {
    case 1: //Sol
        compValue += REFLECT_INVOKE(compBool, curCosts[costSoDI], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        break;
    case 4: //Millia
        compValue += REFLECT_INVOKE(compIntState, curCosts[costMiSFActive], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 7: //Chipp
        compValue += REFLECT_INVOKE(compIntState, curCosts[costChShuriken], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costChInvis], curGamestate->CharSpecific2[0] > 0, caseGamestate->CharSpecific2[0] > 0);
        break;
    case 8: //Eddie
        compValue += REFLECT_INVOKE(compInt, curCosts[costEdGauge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 1200);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costEdRecovering], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costEdType], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costEdActive], curGamestate->CharSpecific4[0], caseGamestate->CharSpecific4[0]);
        break;
    case 9: //Baiken
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaMovementSeal], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaCounterSeal], curGamestate->CharSpecific2[0] > 0, caseGamestate->CharSpecific2[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaGuardSeal], curGamestate->CharSpecific3[0] > 0, caseGamestate->CharSpecific3[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaPSeal], curGamestate->CharSpecific5[0] > 0, caseGamestate->CharSpecific5[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaKSeal], curGamestate->CharSpecific6[0] > 0, caseGamestate->CharSpecific6[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaSSeal], curGamestate->CharSpecific7[0] > 0, caseGamestate->CharSpecific7[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaHSeal], curGamestate->CharSpecific8[0] > 0, caseGamestate->CharSpecific8[0] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaDSeal], curGamestate->CharSpecific9[0] > 0, caseGamestate->CharSpecific9[0] > 0);
        break;
    case 10: //Faust
        compValue += REFLECT_INVOKE(compIntState, curCosts[costFaItemActive], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 11: //Testament
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTePuppets], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTeCrowAttackNo], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costTeCrowActive], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTeCrowPattern], curGamestate->CharSpecific4[0], caseGamestate->CharSpecific4[0]);
        break;
    case 12: //Jam
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiK], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiS], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiH], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0]);
        break;
    case 14: //Johnny
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJoMFlvl], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costJoCoins], curGamestate->CharSpecific2[0] < 8, caseGamestate->CharSpecific2[0] < 8);
        break;
    case 15: //Venom
        compValue += REFLECT_INVOKE(compInt, curCosts[costVeCharge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 20);
        break;
    case 19: //Zappa
        compValue += REFLECT_INVOKE(compIntState, curCosts[costZaSummon], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compInt, curCosts[costZaGauge], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0], 500);
        break;
    case 20: //Bridget
        compValue += REFLECT_INVOKE(compIntState, curCosts[costBrYoyo], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        break;
    case 21: //Robo-Ky
        compValue += REFLECT_INVOKE(compInt, curCosts[costRoHeat], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 3000);
        compValue += REFLECT_INVOKE(compInt, curCosts[costRoTimer], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0], 200);
        break;
    case 22: //A.B.A
        compValue += REFLECT_INVOKE(compIntState, curCosts[costAbMode], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costAbPacks], curGamestate->CharSpecific2[0], caseGamestate->CharSpecific2[0]);
        compValue += REFLECT_INVOKE(compInt, curCosts[costAbGauge], curGamestate->CharSpecific3[0], caseGamestate->CharSpecific3[0], 3000);
        break;
    case 23: //Order-Sol
        compValue += REFLECT_INVOKE(compInt, curCosts[costOsGauge], curGamestate->CharSpecific1[0], caseGamestate->CharSpecific1[0], 2000);
        break;
    case 25: //Justice
        compValue += REFLECT_INVOKE(compBool, curCosts[costJuInstall], curGamestate->CharSpecific1[0] > 0, caseGamestate->CharSpecific1[0] > 0);
        break;
    default:
        break;
    }
    switch (caseReplay.getCharIds()[1])
    {
    case 1: //Sol
        compValue += REFLECT_INVOKE(compBool, curCosts[costSoDIEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        break;
    case 4: //Millia
        compValue += REFLECT_INVOKE(compIntState, curCosts[costMiSFActiveEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 7: //Chipp
        compValue += REFLECT_INVOKE(compIntState, curCosts[costChShurikenEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costChInvisEnemy], curGamestate->CharSpecific2[1] > 0, caseGamestate->CharSpecific2[1] > 0);
        break;
    case 8: //Eddie
        compValue += REFLECT_INVOKE(compInt, curCosts[costEdGaugeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 1200);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costEdRecoveringEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costEdTypeEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costEdActiveEnemy], curGamestate->CharSpecific4[1], caseGamestate->CharSpecific4[1]);
        break;
    case 9: //Baiken
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaMovementSealEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaCounterSealEnemy], curGamestate->CharSpecific2[1] > 0, caseGamestate->CharSpecific2[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaGuardSealEnemy], curGamestate->CharSpecific3[1] > 0, caseGamestate->CharSpecific3[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaPSealEnemy], curGamestate->CharSpecific5[1] > 0, caseGamestate->CharSpecific5[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaKSealEnemy], curGamestate->CharSpecific6[1] > 0, caseGamestate->CharSpecific6[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaSSealEnemy], curGamestate->CharSpecific7[1] > 0, caseGamestate->CharSpecific7[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaHSealEnemy], curGamestate->CharSpecific8[1] > 0, caseGamestate->CharSpecific8[1] > 0);
        compValue += REFLECT_INVOKE(compBool, curCosts[costBaDSealEnemy], curGamestate->CharSpecific9[1] > 0, caseGamestate->CharSpecific9[1] > 0);
        break;
    case 10: //Faust
        compValue += REFLECT_INVOKE(compIntState, curCosts[costFaItemActiveEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 11: //Testament
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTePuppetsEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTeCrowAttackNoEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costTeCrowActiveEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costTeCrowPatternEnemy], curGamestate->CharSpecific4[1], caseGamestate->CharSpecific4[1]);
        break;
    case 12: //Jam
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiKEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiSEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJaAsanagiHEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1]);
        break;
    case 14: //Johnny
        compValue += REFLECT_INVOKE(compIntState, curCosts[costJoMFlvlEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compBool, curCosts[costJoCoinsEnemy], curGamestate->CharSpecific2[1] < 8, caseGamestate->CharSpecific2[1] < 8);
        break;
    case 15: //Venom
        compValue += REFLECT_INVOKE(compInt, curCosts[costVeChargeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 20);
        break;
    case 19: //Zappa
        compValue += REFLECT_INVOKE(compIntState, curCosts[costZaSummonEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compInt, curCosts[costZaGaugeEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1], 500);
        break;
    case 20: //Bridget
        compValue += REFLECT_INVOKE(compIntState, curCosts[costBrYoyoEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        break;
    case 21: //Robo-Ky
        compValue += REFLECT_INVOKE(compInt, curCosts[costRoHeatEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 3000);
        compValue += REFLECT_INVOKE(compInt, curCosts[costRoTimerEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1], 200);
        break;
    case 22: //A.B.A
        compValue += REFLECT_INVOKE(compIntState, curCosts[costAbModeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1]);
        compValue += REFLECT_INVOKE(compIntState, curCosts[costAbPacksEnemy], curGamestate->CharSpecific2[1], caseGamestate->CharSpecific2[1]);
        compValue += REFLECT_INVOKE(compInt, curCosts[costAbGaugeEnemy], curGamestate->CharSpecific3[1], caseGamestate->CharSpecific3[1], 3000);
        break;
    case 23: //Order-Sol
        compValue += REFLECT_INVOKE(compInt, curCosts[costOsGaugeEnemy], curGamestate->CharSpecific1[1], caseGamestate->CharSpecific1[1], 2000);
        break;
    case 25: //Justice
        compValue += REFLECT_INVOKE(compBool, curCosts[costJuInstallEnemy], curGamestate->CharSpecific1[1] > 0, caseGamestate->CharSpecific1[1] > 0);
        break;
    default:
        break;
    }
    return compValue;

    }

    inline float HelperComp(Metadata* curGamestate, Metadata* caseGamestate, Helper* curHelper, Helper* caseHelper, bool autoFail, bool opponent, std::array<float, 200>& curCosts) {
        float compValue = 0;
        double multiplier = 1;

        if (curHelper != nullptr && curHelper->proximityScale) {
            auto distance = fmin((float)abs(curGamestate->getPosX()[!opponent] - curHelper->posX) / maxXDistScreen, 1);
            multiplier = fmax(-0.01731221 + (0.9791619 - -0.01731221) / (1 + pow((distance / 0.364216), (4.017896))), 0);
        }

        if (autoFail || curHelper == nullptr) {
            return curCosts[costHelperSum] * multiplier;
        }
        compValue += compStateHash(curHelper->typeHash, caseHelper->typeHash) * curCosts[costHelperType];
        compValue += compRelativePosX(curGamestate->getPosX()[0], curHelper->posX, caseGamestate->getPosX()[0], caseHelper->posX) * curCosts[costHelperPosX];
        compValue += compRelativePosY(curGamestate->getPosY()[0], curHelper->posY, caseGamestate->getPosY()[0], caseHelper->posY) * curCosts[costHelperPosY];
        compValue += compAct(curHelper->actId, caseHelper->actId) * curCosts[costHelperState];
        compValue += compBool(curHelper->hit, caseHelper->hit) * curCosts[costHelperHit];
        compValue += compBool(curHelper->attack, caseHelper->attack) * curCosts[costHelperAttack];
        compValue += compHelperOrder(curGamestate->getPosX()[1], curGamestate->getPosX()[0], curHelper->posX, caseGamestate->getPosX()[1], caseGamestate->getPosX()[0], caseHelper->posX) * curCosts[costHelperOrder];
        compValue += compHelperOrder(curGamestate->getPosX()[0], curGamestate->getPosX()[1], curHelper->posX, caseGamestate->getPosX()[0], caseGamestate->getPosX()[1], caseHelper->posX) * curCosts[costHelperOrder];
        compValue = compValue * multiplier;

        return compValue;
    }

    inline float HelperCompMatch(Metadata* curGamestate, Metadata* caseGamestate, std::array<float, 200>& curCosts) {

        auto& p1HelpersCur = curGamestate->getPlayerHelpers(0);
        auto& p1HelpersCase = caseGamestate->getPlayerHelpers(0);
        float completeCost = 0;
        int curSize = p1HelpersCur.size();
        int caseSize = p1HelpersCase.size();
        bool skip = false;
        float costBuffer = 9999;
        int indexBuffer = -1;


        if (curSize <= 0 && caseSize <= 0) {
            completeCost += 0;
            skip = true;
        }
        if (skip == false && curSize <= 0 && caseSize > 0) {
            completeCost += curCosts[costHelperSum] * caseSize;
            skip = true;
        }
        if (skip == false && curSize <= 0 && caseSize > 0) {
            completeCost += curCosts[costHelperSum] * caseSize;
            skip = true;
        }

        static std::array<bool, 30> checkArr{};
        static std::array<bool, 30> checkArrP2{};
        if (!skip) {
            bool resetNeeded = false;
            for (int i = 0; i < curSize; i++) {
                costBuffer = 9999;
                indexBuffer = -1;
                for (int j = 0; j < caseSize; j++) {
                    if (checkArr[j] == false) {
                        auto cost = HelperComp(curGamestate, caseGamestate, p1HelpersCur[i].get(), p1HelpersCase[j].get(), false, false, curCosts);
                        if (cost < costBuffer) {
                            costBuffer = cost;
                            indexBuffer = j;
                        }
                    }
                }
                if (indexBuffer == -1) {
                    costBuffer = HelperComp(curGamestate, caseGamestate, nullptr, nullptr, true, false, curCosts);
                }
                else {
                    checkArr[indexBuffer] = true;
                }
                completeCost += costBuffer;
            }

            for (int i = 0; i < p1HelpersCase.size(); i++) {
                if (checkArr[i] == false) {
                    completeCost += HelperComp(curGamestate, caseGamestate, nullptr, nullptr, true, false, curCosts);
                }
            }
            for (int i = 0; i < 10; i++) {
                checkArr[i] = false;
            }
        }


        auto& p2HelpersCur = curGamestate->getPlayerHelpers(1);
        auto& p2HelpersCase = caseGamestate->getPlayerHelpers(1);
        curSize = p2HelpersCur.size();
        caseSize = p2HelpersCase.size();
        skip = false;
        if (curSize <= 0 && caseSize <= 0) {
            completeCost += 0;
            skip = true;
        }
        if (skip == false && curSize <= 0 && caseSize > 0) {
            completeCost += curCosts[costHelperSum] * caseSize;
            skip = true;
        }
        if (skip == false && curSize <= 0 && caseSize > 0) {
            completeCost += curCosts[costHelperSum] * caseSize;
            skip = true;
        }

        if (!skip) {
            for (int i = 0; i < p2HelpersCur.size(); i++) {
                costBuffer = 9999;
                indexBuffer = -1;
                for (int j = 0; j < p2HelpersCase.size(); j++) {
                    if (checkArrP2[j] == false) {
                        auto cost = HelperComp(curGamestate, caseGamestate, p2HelpersCur[i].get(), p2HelpersCase[j].get(), false, true, curCosts);
                        if (cost < costBuffer) {
                            costBuffer = cost;
                            indexBuffer = j;
                        }
                    }
                }
                if (indexBuffer == -1) {
                    costBuffer = HelperComp(curGamestate, caseGamestate, nullptr, nullptr, true, false, curCosts);
                }
                else {
                    checkArrP2[indexBuffer] = true;
                }
                completeCost += costBuffer;
            }

            for (int i = 0; i < p2HelpersCase.size(); i++) {
                if (checkArrP2[i] == false) {
                    completeCost += HelperComp(curGamestate, caseGamestate, nullptr, nullptr, true, true, curCosts);
                }
            }
            for (int i = 0; i < 10; i++) {
                checkArrP2[i] = false;
            }
        }


        return 0;
    }
inline float comparisonFunctionSlow(Metadata* curGamestate, Metadata* caseGamestate, CbrReplayFile& caseReplay, CbrCase* caseData, int replayIndex, int caseIndex, bool nextCaseCheck, std::array<float, 200> curCosts) {
    float compValue = 0;
    compValue += PURE_INVOKE(compDirectionHeld, curCosts[costButtonsHeld], curGamestate->inputFwd, curGamestate->inputBack, curGamestate->inputUp, curGamestate->inputDown, caseGamestate->inputFwd, caseGamestate->inputBack, caseGamestate->inputUp, caseGamestate->inputDown);
    compValue += PURE_INVOKE(compButtonsHeld, curCosts[costButtonsHeld], curGamestate->inputP, curGamestate->inputK, curGamestate->inputS, curGamestate->inputH, curGamestate->inputD, curGamestate->inputT, caseGamestate->inputP, caseGamestate->inputK, caseGamestate->inputS, caseGamestate->inputH, caseGamestate->inputD, caseGamestate->inputT);
    compValue += PURE_INVOKE(compNegativeEdge, curCosts[costNegativeEdge], curGamestate->inputP, curGamestate->inputK, curGamestate->inputS, curGamestate->inputH, curGamestate->inputD, caseGamestate->inputP, caseGamestate->inputK, caseGamestate->inputS, caseGamestate->inputH, caseGamestate->inputD, caseReplay.getCharIds()[0]);
    compValue += HelperCompMatch(curGamestate, caseGamestate, curCosts);
    return compValue;
}
inline float comparisonFunctionSlowDebug(Metadata* curGamestate, Metadata* caseGamestate, CbrReplayFile& caseReplay, CbrCase* caseData, int replayIndex, int caseIndex, bool nextCaseCheck, std::array<float, 200> curCosts, std::string& debugText) {
    float compValue = 0;
    compValue += REFLECT_INVOKE(compDirectionHeld, curCosts[costButtonsHeld], curGamestate->inputFwd, curGamestate->inputBack, curGamestate->inputUp, curGamestate->inputDown, caseGamestate->inputFwd, caseGamestate->inputBack, caseGamestate->inputUp, caseGamestate->inputDown);
    compValue += PURE_INVOKE(compButtonsHeld, curCosts[costButtonsHeld], curGamestate->inputP, curGamestate->inputK, curGamestate->inputS, curGamestate->inputH, curGamestate->inputD, curGamestate->inputT, caseGamestate->inputP, caseGamestate->inputK, caseGamestate->inputS, caseGamestate->inputH, caseGamestate->inputD, caseGamestate->inputT);
    compValue += PURE_INVOKE(compNegativeEdge, curCosts[costNegativeEdge], curGamestate->inputP, curGamestate->inputK, curGamestate->inputS, curGamestate->inputH, curGamestate->inputD, caseGamestate->inputP, caseGamestate->inputK, caseGamestate->inputS, caseGamestate->inputH, caseGamestate->inputD, caseReplay.getCharIds()[0]);



    float helperVal = HelperCompMatch(curGamestate, caseGamestate, curCosts);
    compValue += helperVal;
    if (helperVal > 0) {
        debugText += "\nHelper Cost: " + std::to_string(helperVal);
    }

    return compValue;
}

