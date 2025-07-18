#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <array>
#include "Helper.h"
#include <boost/serialization/shared_ptr.hpp>

class Metadata {
private:
    friend class boost::serialization::access;
public:
    int opponentId = 0;

    std::array< int, 2>posX;
    std::array< int, 2>posY;
    std::array< std::string, 2>currentAction;
    std::array< std::string, 2>lastAction;
    //std::array<uint16_t, 2>currentAction;
    //std::array<uint16_t, 2>lastAction;
    std::array<uint16_t, 2>actId;
    std::array<uint16_t, 2>lastActId;
    std::array<uint32_t, 2>currentStateFlags;
    std::array<uint32_t, 2>lastStateFlags;
    std::array< size_t, 2>currentActionHash;
    std::array< size_t, 2>lastActionHash;
    int frame_count_minus_1;

    std::array< int, 2>blockstun;
    std::array< int, 2>hitstun;
    std::array< int, 2>attackType;
    std::array< int, 2>hitstop;
    std::array< int, 2>actionTimeNoHitstop;

    std::array< int, 2>comboProration;
    std::array< int, 2>starterRating;
    std::array< int, 2>comboTime;
    std::array< bool, 2>neutral;
    std::array< bool, 2>attack;
    std::array< bool, 2>wakeup;
    std::array< bool, 2>blocking;
    std::array< bool, 2>hit;
    std::array< bool, 2>hitThisFrame;
    std::array< bool, 2>BlockThisFrame;
    std::array< bool, 2>air;
    std::array< bool, 2>crouching;


    bool facing;
    bool inputBufferActive = false;
    //helpers
    std::array<std::vector<std::shared_ptr<Helper>>, 2> helpers = { {} };
    std::array< int, 2>timeAfterRecovery;
    int matchState = -1;
    std::array< int, 2> heatMeter;
    std::array< int, 2> overdriveMeter;
    std::array< int, 2> overdriveTimeleft;
    std::array< int, 2> healthMeter = { 10000,10000 };
    int hitMinX = -1;
    int hitMinY = -1;

    bool inputA = false, inputB = false, inputC = false, inputD = false;
    bool inputP = false, inputK = false, inputS = false, inputH = false, inputT = false;
    bool inputFwd = false, inputUp = false, inputDown = false, inputBack = false;
    int inprocessInputSequence = -1;
    std::array< std::array< int, 2>, 2> velocity = { { { 0, 0 }, { 0, 0 } } };


    //CharacterSpecific
    std::array< int, 2> CharSpecific1 = { 0, 0 };
    std::array< int, 2> CharSpecific2 = { 0, 0 };
    std::array< int, 2> CharSpecific3 = { 0, 0 };
    std::array< int, 2> CharSpecific4 = { 0, 0 };
    std::array< int, 2> CharSpecific5 = { 0, 0 };
    std::array< int, 2> CharSpecific6 = { 0, 0 };
    std::array< int, 2> CharSpecific7 = { 0, 0 };
    std::array< int, 2> CharSpecific8 = { 0, 0 };
    std::array< int, 2> CharSpecific9 = { 0, 0 };


    template<class Archive>
    void serialize(Archive& a, const unsigned version) {

        switch (version) {
        case 0:
            a & posX & posY & facing & currentAction & lastAction & actId & currentStateFlags & lastStateFlags & currentActionHash &
                lastActionHash & lastActId & blockstun & hitstun & attackType & hitstop & timeAfterRecovery &
                actionTimeNoHitstop & comboProration & starterRating & comboTime &
                neutral & attack & wakeup & blocking & hit & hitThisFrame & BlockThisFrame & air & crouching & inputBufferActive & helpers & matchState &
                heatMeter & overdriveMeter & overdriveTimeleft &
                CharSpecific1 & CharSpecific2 & CharSpecific3 & CharSpecific4 & CharSpecific5 & CharSpecific6 & CharSpecific7 & CharSpecific8 & CharSpecific9;
                break;
        case 1:
            a & posX & posY & facing & currentAction & lastAction & actId & currentStateFlags & lastStateFlags & currentActionHash &
                lastActionHash & lastActId & blockstun & hitstun & attackType & hitstop & timeAfterRecovery &
                actionTimeNoHitstop & comboProration & starterRating & comboTime &
                neutral & attack & wakeup & blocking & hit & hitThisFrame & BlockThisFrame & air & crouching & inputBufferActive & helpers & matchState &
                healthMeter & heatMeter & overdriveMeter & overdriveTimeleft &
                CharSpecific1 & CharSpecific2 & CharSpecific3 & CharSpecific4 & CharSpecific5 & CharSpecific6 & CharSpecific7 & CharSpecific8 & CharSpecific9;
                break;
        case 2:
            a & posX & posY & facing & currentAction & lastAction & actId & currentStateFlags & lastStateFlags & currentActionHash & hitMinX &
                lastActionHash & lastActId & blockstun & hitstun & attackType & hitstop & timeAfterRecovery &
                actionTimeNoHitstop & comboProration & starterRating & comboTime &
                neutral & attack & wakeup & blocking & hit & hitThisFrame & BlockThisFrame & air & crouching & inputBufferActive & helpers & matchState &
                healthMeter & heatMeter & overdriveMeter & overdriveTimeleft &
                CharSpecific1 & CharSpecific2 & CharSpecific3 & CharSpecific4 & CharSpecific5 & CharSpecific6 & CharSpecific7 & CharSpecific8 & CharSpecific9;
            break;
        case 3:
            a & posX & posY & facing & currentAction & lastAction & actId & currentStateFlags & lastStateFlags & currentActionHash & hitMinX &
                lastActionHash & lastActId & blockstun & hitstun & attackType & hitstop & timeAfterRecovery &
                actionTimeNoHitstop & comboProration & starterRating & comboTime &
                neutral & attack & wakeup & blocking & hit & hitThisFrame & BlockThisFrame & air & crouching & inputBufferActive & helpers & matchState &
                healthMeter & heatMeter & overdriveMeter & overdriveTimeleft &
                CharSpecific1 & CharSpecific2 & CharSpecific3 & CharSpecific4 & CharSpecific5 & CharSpecific6 & CharSpecific7 & CharSpecific8 & CharSpecific9 &
                inputA & inputB & inputC & inputD & inputFwd & inputUp & inputDown & inputBack  & inprocessInputSequence & velocity;
            
            break;
        }
        

        
    }
    Metadata();
    Metadata(int p1x, int p2x, int p1y, int p2y, bool b,
        std::string p1action, std::string p2action, uint16_t p1actId, uint16_t p2actId,
        uint32_t p1StateFlags, uint32_t p2StateFlags, uint32_t p1lastStateFlags, uint32_t p2lastStateFlags,
        int P1Blockstun, int P2Blockstun, int P1Hitstun, int P2Hitstun, int P1AttackType, int P2AttackType,
        int p1Hitstop, int p2Hitstop, int p1ActionTimeNHS, int p2ActionTimeNHS, char* p1LastAction, char* p2LastAction, uint16_t p1LastActId, uint16_t p2LastActId);

    void Metadata::addHelper(std::shared_ptr<Helper> h, int playerIndex);
    std::array<std::vector<std::shared_ptr<Helper>>, 2>& Metadata::getHelpers();
    std::vector<std::shared_ptr<Helper>>& Metadata::getPlayerHelpers(int index);
    void setInputBufferActive(bool);
    bool getInputBufferActive();
    void SetFrameCount(int frameCount);
    int getFrameCount();
    std::array< int, 2> getPosX();
    std::array< int, 2>getPosY();
    bool getFacing();
    std::array< std::string, 2>getCurrentAction();
    std::array< std::string, 2>getLastAction();
    std::array< uint16_t, 2>getCurrentActId();
    std::array< uint16_t, 2>getLastActId();
    std::array< size_t, 2>getCurrentActionHash();
    std::array< size_t, 2>getLastActionHash();
    std::array< int, 2> getBlockstun();
    std::array< int, 2> getHitstun();
    std::array< int, 2> getAttackType();
    std::array< int, 2> getHitstop();
    std::array< int, 2> getActionTimeNHS();
    std::array< bool, 2> getNeutral();
    std::array< bool, 2> getAttack();
    std::array< bool, 2> getWakeup();
    std::array< bool, 2> getBlocking();
    std::array< bool, 2> getHit();
    std::array< bool, 2> getHitThisFrame();
    std::array< bool, 2> getBlockThisFrame();
    std::array< bool, 2> getAir();
    std::array< bool, 2> getCrouching();
    bool CheckNeutralState(uint16_t);
    void computeMetaData();
    std::string PrintState();
    void SetComboVariables(int p1comboProration, int p2comboProration, int p1starterRating, int p2starterRating, int p1comboTime, int p2comboTime);
    std::array< int, 2> getComboProration();
    std::array< int, 2> getStarterRating();
    std::array< int, 2> getComboTime();

};
BOOST_CLASS_VERSION(Metadata, 3)