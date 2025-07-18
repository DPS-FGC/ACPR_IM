#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <array>
#include <Game/Hitbox/Hitbox.h>
#include <Game/Hitbox/HitboxReader.h>

class Helper {
private:
    friend class boost::serialization::access;


public:
    std::string type;
    uint16_t id;
    size_t typeHash;
    int posX;
    int posY;
    std::string currentAction;
    size_t currentActionHash;
    uint16_t actId;
    bool proximityScale = false;

    int hitstun;
    int attackType;
    int hitstop;
    int actionTimeNoHitstop;
    uint32_t attackFlags;

    bool attack;
    bool hit;
    bool hitThisFrame;
    bool facing;
    template<class Archive>
    void serialize(Archive& a, const unsigned version) {
        a& type& typeHash& posX& posY& currentAction& currentActionHash& hitstun& attackType& hitstop& actionTimeNoHitstop& attack& hit& hitThisFrame& facing;
    }
    Helper();
    Helper(uint16_t p1id, int p1x, int p1y, bool b, std::string p1action, uint16_t p1actId, int P1Hitstun, int P1AttackType, int p1Hitstop, int p1ActionTimeNHS, uint32_t p1attackFlags);

    int getPosX();
    int getPosY();
    bool getFacing();
    std::string getCurrentAction();
    size_t getCurrentActionHash();
    int getHitstun();
    int getAttackType();
    int getHitstop();
    int getActionTimeNHS();
    bool getAttack();
    bool getHit();
    bool getHitThisFrame();

    void computeMetaData();
    std::string PrintState();;
};
