#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "Metadata.h"



Metadata::Metadata()
{
}
//p1PosX, p2PosX, p1PosY, p2PosY, facing, P1Action, P2Action, P1Blockstun, P2Blockstun, P1Hitstun, P2Hitstun, P1AttackType, P2AttackType, p1Hitstop, p2Hitstop, p1ActionTimeNHS, p2ActionTimeNHS
Metadata::Metadata(int p1x, int p2x, int p1y, int p2y, bool b, std::string p1action, std::string p2action, uint16_t p1actId, uint16_t p2actId, uint32_t p1StateFlags, uint32_t p2StateFlags, uint32_t p1lastStateFlags, uint32_t p2lastStateFlags, int P1Blockstun, int P2Blockstun, int P1Hitstun, int P2Hitstun, int P1AttackType, int P2AttackType,int p1Hitstop, int p2Hitstop, int p1ActionTimeNHS, int p2ActionTimeNHS, char* p1LastAction, char* p2LastAction, uint16_t p1LastActId, uint16_t p2LastActId)
{
	posX[0] = p1x;
	posX[1] = p2x;
	posY[0] = p1y;
	posY[1] = p2y;
	facing = b;
	currentAction[0] = p1action;
	currentAction[1] = p2action;
	currentStateFlags[0] = p1StateFlags;
	currentStateFlags[1] = p2StateFlags;
	lastStateFlags[0] = p1lastStateFlags;
	lastStateFlags[1] = p2lastStateFlags;
	actId[0] = p1actId;
	actId[1] = p2actId;
	blockstun[0] = P1Blockstun;
	blockstun[1] = P2Blockstun;
	hitstun[0] = P1Hitstun;
	hitstun[1] = P2Hitstun;
	attackType[0] = P1AttackType;
	attackType[1] = P2AttackType;
	hitstop[0] = p1Hitstop;
	hitstop[1] = p2Hitstop;
	actionTimeNoHitstop[0] = p1ActionTimeNHS;
	actionTimeNoHitstop[1] = p2ActionTimeNHS;
	lastAction[0] = p1LastAction;
	lastAction[1] = p2LastAction;
	lastActId[0] = p1LastActId;
	lastActId[1] = p2LastActId;
}

void Metadata::SetComboVariables(int p1comboProration, int p2comboProration, int p1starterRating, int p2starterRating, int p1comboTime, int p2comboTime) {
	comboProration[0] = p1comboProration;
	comboProration[1] = p2comboProration;
	starterRating[0] = p1starterRating;
	starterRating[1] = p2starterRating;
	comboTime[0] = p1comboTime;
	comboTime[1] = p2comboTime;
}

void Metadata::addHelper(std::shared_ptr<Helper> h, int playerIndex) {
	helpers[playerIndex].push_back(h);
}
std::array<std::vector<std::shared_ptr<Helper>>, 2>& Metadata::getHelpers() {
	return helpers;
}
std::vector<std::shared_ptr<Helper>>& Metadata::getPlayerHelpers(int index) {
	return helpers[index];
}
void Metadata::SetFrameCount(int frameCount) {
	frame_count_minus_1 = frameCount;
}

int Metadata::getFrameCount() {
	return frame_count_minus_1;
}


void Metadata::setInputBufferActive(bool b) {
	inputBufferActive = b;
}
bool Metadata::getInputBufferActive() {
	return inputBufferActive;
}

std::array< int, 2> Metadata::getPosX() {
	return posX;
}
std::array< int, 2>Metadata::getPosY() {
	return posY;
}
bool Metadata::getFacing() {
	return facing;
}

std::array< std::string, 2>Metadata::getCurrentAction() {
	return currentAction;
}
std::array<uint16_t, 2>Metadata::getCurrentActId() {
	return actId;
}

std::array<uint16_t, 2>Metadata::getLastActId() {
	return lastActId;
}

std::array< std::string, 2>Metadata::getLastAction() {
	return lastAction;
}

std::array< size_t, 2>Metadata::getCurrentActionHash() {
	return currentActionHash;
}

std::array< size_t, 2>Metadata::getLastActionHash() {
	return lastActionHash;
}

std::array< int, 2> Metadata::getBlockstun() {
	return blockstun;
}
std::array< int, 2> Metadata::getHitstun() {
	return hitstun;
}
std::array< int, 2> Metadata::getAttackType() {
	return attackType;
}
std::array< int, 2> Metadata::getHitstop() {
	return hitstop;
}
std::array< int, 2> Metadata::getActionTimeNHS() {
	return actionTimeNoHitstop;
}
std::array< bool, 2> Metadata::getNeutral() {
	return neutral;
}
std::array< bool, 2> Metadata::getAttack() {
	return attack;
}
std::array< bool, 2> Metadata::getWakeup() {
	return wakeup;
}
std::array< bool, 2> Metadata::getBlocking() {
	return blocking;
}
std::array< bool, 2> Metadata::getHit() {
	return hit;
}
std::array< bool, 2> Metadata::getHitThisFrame() {
	return hitThisFrame;
}
std::array< bool, 2> Metadata::getBlockThisFrame() {
	return BlockThisFrame;
}
std::array< bool, 2> Metadata::getAir() {
	return air;
}
std::array< bool, 2> Metadata::getCrouching() {
	return crouching;
}
std::array< int, 2> Metadata::getComboProration() {
	return comboProration;
}
std::array< int, 2> Metadata::getStarterRating() {
	return starterRating;
}
std::array< int, 2> Metadata::getComboTime() {
	return comboTime;
}



std::array<uint16_t, 13> neutralActions = {
	0, //Stand
	1, //Crouch
	2, //Stand2Crouch
	3, //Crouch2Stand
	4, //Prejump
	5, //FWalk
	6, //BWalk
	7, //JumpUpper
	8, //JumpDown
	21, //JumpLanding
	53, //Proximity guard standing
	55, //Not sure, but it has the proximity guard flag
	57  //Proximity guard crouching
};


void Metadata::computeMetaData() {
	attack[0] = attackType[0] > 0;
	attack[1] = attackType[1] > 0;
	wakeup[0] = currentStateFlags[0] & 0x8000;
	wakeup[1] = currentStateFlags[1] & 0x8000;
	blocking[0] = currentStateFlags[0] & 0x0200;
	blocking[1] = currentStateFlags[1] & 0x0200;
	hit[0] = currentStateFlags[0] & 0x0020;
	hit[1] = currentStateFlags[1] & 0x0020;
	neutral[0] = !blocking[0] && !hit[0] && CheckNeutralState(actId[0]);
	neutral[1] = !blocking[1] && !hit[1] && CheckNeutralState(actId[1]);
	hitThisFrame[0] = !(lastStateFlags[0] & 0x0020) && (currentStateFlags[0] & 0x0020);
	hitThisFrame[1] = !(lastStateFlags[1] & 0x0020) && (currentStateFlags[1] & 0x0020);
	BlockThisFrame[0] = !(lastStateFlags[0] & 0x0200) && (currentStateFlags[0] & 0x0200);
	BlockThisFrame[1] = !(lastStateFlags[1] & 0x0200) && (currentStateFlags[1] & 0x0200);
	air[0] = posY[0] < 0;
	air[1] = posY[1] < 0;
	crouching[0] = currentStateFlags[0] & 0x0400;
	crouching[1] = currentStateFlags[1] & 0x0400;
	currentActionHash[0] = std::hash<std::string>{}(currentAction[0]);
	currentActionHash[1] = std::hash<std::string>{}(currentAction[1]);
	lastActionHash[0] = std::hash<std::string>{}(lastAction[0]);
	lastActionHash[1] = std::hash<std::string>{}(lastAction[1]);

}

bool Metadata::CheckNeutralState(uint16_t actId) {
	for (std::size_t i = 0; i < neutralActions.size(); ++i) {
		if (actId == neutralActions[i]) {
			return true;
		}
	}
	return false;
}

std::string Metadata::PrintState() {
	std::string str = "";

	str += "PosX: " + std::to_string(posX[0]) + " - " + std::to_string(posX[1]) + " - " + std::to_string(abs(posX[0] - posX[1]))+ "\n";
	str += "PosY: " + std::to_string(posY[0]) + " - " + std::to_string(posY[1]) + " - " + std::to_string(abs(posY[0] - posY[1])) + "\n";
	str += "CurAction: " + currentAction[0] + " - " + currentAction[1] + "\n";
	str += "Air: " + std::to_string(air[0]) + " - " + std::to_string(air[1]) + "\n";
	str += "Attack: " + std::to_string(attack[0]) + " - " + std::to_string(attack[1]) + "\n";
	//str += "AtkType: " + std::to_string(attackType[0]) + " - " + std::to_string(attackType[1]) + "\n";
	str += "Crouching: " + std::to_string(crouching[0]) + " - " + std::to_string(crouching[1]) + "\n";
	str += "Neutral: " + std::to_string(neutral[0]) + " - " + std::to_string(neutral[1]) + "\n";
	str += "Wakeup: " + std::to_string(wakeup[0]) + " - " + std::to_string(wakeup[1]) + "\n";
	str += "Blocking: " + std::to_string(blocking[0]) + " - " + std::to_string(blocking[1]) + "\n";
	str += "BlockingTF: " + std::to_string(BlockThisFrame[0]) + " - " + std::to_string(BlockThisFrame[1]) + "\n";
	str += "Hit: " + std::to_string(hit[0]) + " - " + std::to_string(hit[1]) + "\n";
	str += "HitTF: " + std::to_string(hitThisFrame[0]) + " - " + std::to_string(hitThisFrame[1]) + "\n";
	str += "comboPror: " + std::to_string(comboProration[0]) + " - " + std::to_string(comboProration[1]) + "\n";
	str += "starterRating: " + std::to_string(starterRating[0]) + " - " + std::to_string(starterRating[1]) + "\n";
	str += "comboTime: " + std::to_string(comboTime[0]) + " - " + std::to_string(comboTime[1]) + "\n";

	
	return str;
}