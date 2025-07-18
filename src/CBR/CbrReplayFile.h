#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "Metadata.h"
#include "CbrCase.h"
#include "AnnotatedReplay.h"

struct CbrGenerationError
{
    std::string structure;
    std::string errorDetail;
    int errorCount;
};

struct CommandActions
{
    std::string moveName;
    std::vector<int> inputs;
    std::string priorMove;
    std::vector<int> altInputs;
};

struct CommandActionsAC
{
    uint16_t moveName;
    std::vector<int> inputs;
    uint16_t priorMove = 0xFFFF;
    std::vector<int> altInputs;
};

struct inputMemory 
{
    std::vector<signed int> inputs;
    uint16_t name;
};

class CbrReplayFile {
private:
    //friend class boost::serialization::access;
    std::vector<CbrCase> cbrCase;
    std::vector<int> input;
    std::array<std::string, 2> characterName;
    std::array<int, 2> characterId;
    static std::array< std::string, 13> neutralActions;
    bool stateChangeTrigger = false;
    bool commandInputNeedsResolving = false;
 
    uint16_t inputResolveName = -1;

public:
    template<class Archive>
    void serialize(Archive& a, const unsigned version) {
        a& cbrCase& characterName & input &  stateChangeTrigger & commandInputNeedsResolving & characterId;
    }
    CbrReplayFile();
    //p1Charname p2Charname
    CbrReplayFile::CbrReplayFile(std::string p1, std::string p2, int p1ID, int p2ID);
    CbrReplayFile::CbrReplayFile(std::array< std::string, 2> arr, std::array< int, 2> arrId);
    void CopyInput(std::vector<int>);
    void AddCase(CbrCase);
    CbrCase* getCase(int);
    std::vector<CbrCase>* getCaseBase();
    int getCaseBaseLength();
    int getInput(int);

    CbrGenerationError MakeCaseBase(AnnotatedReplay*, std::string, int, int, int, bool instantLearning = false);
    bool CbrReplayFile::CheckCaseEnd(int framesIdle, Metadata ar, uint16_t prevState, bool neutral, std::vector<CommandActionsAC>& commands);
    bool CheckNeutralState(std::string);
    std::vector<int> DeconstructInput(int, bool);
    std::vector < inputMemory> CbrReplayFile::CheckCommandExecution(int input, std::vector < inputMemory>& inputBuffer);
    std::vector < inputMemory> CbrReplayFile::MakeInputArray(uint16_t move, std::vector<CommandActionsAC>& commands, uint16_t prevState);
    std::vector<CommandActionsAC> FetchCommandActions(std::string&);
    std::vector<CommandActions> CbrReplayFile::FetchNirvanaCommandActions();
    CbrGenerationError instantLearning(AnnotatedReplay*, std::string);
    CbrGenerationError CbrReplayFile::makeFullCaseBase(AnnotatedReplay*, std::string, bool trim = false);
    bool checkDirectionInputs(int direction, int input);
    bool isDirectionInputs(int direction);
    std::array<int, 2>& CbrReplayFile::getCharIds();
    std::array<std::string, 2>& CbrReplayFile::getCharNames();
    std::vector < inputMemory> CbrReplayFile::MakeInputArraySuperJump(uint16_t move, std::vector<int>& inputs, int startingIndex, std::vector < inputMemory>& inVec);
    std::vector < inputMemory> CbrReplayFile::DeleteCompletedInputs(uint16_t name, std::vector < inputMemory>& inputBuffer);
    bool CbrReplayFile::ContainsCommandState(uint16_t move, std::vector<CommandActionsAC>& commands);
};


