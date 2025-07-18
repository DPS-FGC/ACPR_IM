#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "CbrReplayFile.h"
#include "Metadata.h"
#include "AnnotatedReplay.h"



#define caseNeutralTime 12
#define caseMinTime 4


CbrReplayFile::CbrReplayFile()
{
}

CbrReplayFile::CbrReplayFile(std::string p1, std::string p2, int p1ID, int p2ID)
{
    characterName[0] = p1;
    characterName[1] = p2;
    characterId[0] = p1ID;
    characterId[1] = p2ID;
}
CbrReplayFile::CbrReplayFile(std::array< std::string, 2> arr, std::array< int, 2> arrId): characterName(arr), characterId(arrId)
{
}
void CbrReplayFile::CopyInput(std::vector<int> in)
{
    input = in;
}
void CbrReplayFile::AddCase(CbrCase c)
{
    cbrCase.push_back(std::move(c));
}

CbrCase* CbrReplayFile::getCase(int index) {
    if (index >= cbrCase.size()) {
        return &cbrCase[0];
    }
    return &cbrCase[index];
}
std::array<int, 2>& CbrReplayFile::getCharIds() {
    return characterId;
}
std::array<std::string, 2>& CbrReplayFile::getCharNames() {
    return characterName;
}

std::vector<CbrCase>* CbrReplayFile::getCaseBase() {
    return &cbrCase;
}

int CbrReplayFile::getCaseBaseLength() {
    return cbrCase.size();
}
int CbrReplayFile::getInput(int i) {
    return input[i];
}



CbrGenerationError CbrReplayFile::MakeCaseBase(AnnotatedReplay* ar, std::string charName, int startIndex, int endIndex, int insertPoint, bool instantLearning) {

    //Input command related initalization
    bool facingCommandStart = 0;
    auto commands = FetchCommandActions(charName);
    bool carlDoll = false;
    if (charName == "ca") {
        carlDoll = true;

    }
    input = ar->getInput();
    std::vector < inputMemory> inputResolve = {};
    std::vector < inputMemory> dollResolve = {};
    std::vector < inputMemory> debugResolve;
    uint16_t curState = ar->ViewMetadata(endIndex).getCurrentActId()[0];
    std::string curStateDoll = "";

    auto neutral = false;
    bool inputNewCaseTrigger = false;
    int framesIdle = 0; //frames character can be in a neutral state 
    int frameCount = ar->ViewMetadata(endIndex).getFrameCount();
    int meterRequirement = 0;
    int odRequirement = 0;
    
    //Making sure rollbacks dont cause issues by deleting rolled back frames
    int reduceAmount = 0;
    if (!instantLearning)
    {
        for (std::vector<int>::size_type i = endIndex;
            i != startIndex - 1; i--) {

            auto metaData = ar->getAllMetadata();
            if (endIndex != i && frameCount <= ar->ViewMetadata(i).getFrameCount()) {
                input.erase(input.begin() + i);
                metaData->erase(metaData->begin() + i);
                reduceAmount++;
            }
            frameCount = ar->ViewMetadata(i).getFrameCount();
        }
    }
    auto adjustedEndIndex = endIndex - reduceAmount;
    int iteratorIndex = adjustedEndIndex;//last index of the replay

    
    //Data structures for debugging
    CbrGenerationError retError;
    retError.errorCount = 0;
    retError.errorDetail = "";
    //int errorCounter = 0;
    int debugInt = 0;
    bool inputBuffering = false;
    std::string debugStructureStr = "";
    for (std::vector<int>::size_type i = startIndex;
        i < adjustedEndIndex - 1; i++) {
        debugStructureStr += ("Input: " + std::to_string(input[i]) + " Player: " + std::to_string(ar->ViewMetadata(i).getCurrentActId()[0]) + " - Opp: " + std::to_string(ar->ViewMetadata(i).getCurrentActId()[1]) + "\n");
    }

    //minimum attack distance var initalization
    int minx = -1;
    int miny = -1;
    bool nextMinDisResolve = false;

    //The actual creation loop
    for (std::vector<int>::size_type i = adjustedEndIndex;
        i != startIndex - 1; i--) {
        
        //check weather the AI is in a neutral state
        neutral = ar->ViewMetadata(i).getNeutral()[0];

        //If meter/OD decreased between frames store it as meter/OD requirement to be added to a case
        if (((int)i < ar->MetadataSize() - 1) && i >= 0) {
            auto difMeter = ar->ViewMetadata(i+1).heatMeter[0] - ar->ViewMetadata(i).heatMeter[0];
            if (difMeter < 0) {
                meterRequirement = difMeter * -1;
            }
            auto difOD = ar->ViewMetadata(i + 1).overdriveMeter[0] - ar->ViewMetadata(i).overdriveMeter[0];
            if (difOD < 0) {
                odRequirement = difOD * -1;
            }
        }

        //set minimum distance on hit to be saved when the case is generated.
        //we take i-1 from the frame the opponent is hit on because the opponents hurtbox expands in the first frame of hitstun already.
        if ((ar->ViewMetadata(i).getHitThisFrame()[1] || ar->ViewMetadata(i).getBlockThisFrame()[1]) && ar->ViewMetadata(i).hitMinX > 0 && (ar->ViewMetadata(i).hitMinX < minx || minx == -1) && i - 1 >= 0) {
            minx = ar->ViewMetadata(i).hitMinX; }
        if ((ar->ViewMetadata(i).getHitThisFrame()[1] || ar->ViewMetadata(i).getBlockThisFrame()[1]) && ar->ViewMetadata(i).hitMinY > 0 && (ar->ViewMetadata(i).hitMinY < miny || miny == -1) && i - 1 >= 0) {
            miny = ar->ViewMetadata(i).hitMinY; }

        //check if an input sequence is currently beeing executed and if so check every input against the sequence untill completion
        //All cases during an input sequence are marked to prevent them beeing played out of order
        if (inputResolve.size() > 0) {
            debugInt++;
            if (debugInt == 200) {
                retError.errorCount++;
                retError.errorDetail += "\nerror on move: " + std::to_string(inputResolveName) + "\n" + "Frame: " + std::to_string(i) + "\n Not resolved in 100 steps\n" ;
            }
            auto deconInputs = DeconstructInput(input[i], facingCommandStart);
            for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                inputResolve = CheckCommandExecution(deconInputs[j], inputResolve);//fix this inputResolve doesnt update from the function.

            }
        }

        //Same as with the input sequence but specifically for carls doll
        /*if (carlDoll && dollResolve.size() > 0) {
            debugInt++;
            if (debugInt == 100) {
                retError.errorCount++;
                retError.errorDetail += "error on doll move: " + std::to_string(inputResolveName) + "\n" + "Not resolved in 100 steps\n";
            }
            auto deconInputs = DeconstructInput(input[i], facingCommandStart);
            for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                dollResolve = CheckCommandExecution(deconInputs[j], dollResolve);//fix this inputResolve doesnt update from the function.

            }
        }*/

        //Making new cases when a case ends
        if (CheckCaseEnd(framesIdle, ar->ViewMetadata(i), curState, neutral, commands) ) {
            
            //Check for command execution in the new state and creat an input sequence that requires to be resolved
            if (stateChangeTrigger == true) {
                stateChangeTrigger = false;
                if (inputResolve.size() == 0) {
                    if (i+1 >= input.size()) {
                        facingCommandStart = ar->ViewMetadata(i).getFacing();
                    }
                    else {
                        facingCommandStart = ar->ViewMetadata(i + 1).getFacing();
                    }
                    
                    inputResolve = MakeInputArray(curState, commands, ar->ViewMetadata(i).getCurrentActId()[0]);
                    inputResolve = MakeInputArraySuperJump(curState, input, i, inputResolve);
                    if (inputResolve.size() > 0) {
                        debugInt = 0;
                        inputResolveName = curState;
                        inputBuffering = true;
                        if (i + 1 < input.size()) {
                            auto deconInputs = DeconstructInput(input[i + 1], facingCommandStart);
                            for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                                inputResolve = CheckCommandExecution(deconInputs[j], inputResolve);//fix this inputResolve doesnt update from the function.
                            }
                        }
                        auto deconInputs = DeconstructInput(input[i], facingCommandStart);
                        for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                            inputResolve = CheckCommandExecution(deconInputs[j], inputResolve);//fix this inputResolve doesnt update from the function.
                        }
                    }
                }

                /*if (carlDoll && dollResolve.size() == 0) {
                    facingCommandStart = ar->ViewMetadata(i + 1).getFacing();
                    for (int o = 0; o < ar->ViewMetadata(i + 1).getHelpers()[0].size(); o++) {
                        if (ar->ViewMetadata(i + 1).getHelpers()[0][o].get()->type == "Nirvana") {
                            curStateDoll = ar->ViewMetadata(i + 1).getHelpers()[0][o].get()->currentAction;
                            continue;
                        }
                    }
                    dollResolve = MakeInputArray(curStateDoll, FetchNirvanaCommandActions(), "none");
                    if (dollResolve.size() > 0) {
                        debugInt = 0;
                        inputResolveName = curState;
                        inputBuffering = true;
                        if (i + 1 < input.size()) {
                            auto deconInputs = DeconstructInput(input[i+1], facingCommandStart);
                            for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                                dollResolve = CheckCommandExecution(deconInputs[j], dollResolve);//fix this inputResolve doesnt update from the function.
                            }
                        }
                        auto deconInputs = DeconstructInput(input[i], facingCommandStart);
                        for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                            dollResolve = CheckCommandExecution(deconInputs[j], dollResolve);//fix this inputResolve doesnt update from the function.
                        }
                    }
                }*/

            }

            //Process variables that happen during the case but need to be stored in the case meta data.
            curState = ar->ViewMetadata(i).getCurrentActId()[0];
            cbrCase.insert(cbrCase.begin()+ insertPoint, CbrCase(std::move(ar->CopyMetadataPtr(i)), (int)i, iteratorIndex));
            if (inputResolve.size() == 0 && dollResolve.size() == 0) {
                inputBuffering = false;
            }
            cbrCase[insertPoint].setInputBufferSequence(inputBuffering);
            cbrCase[insertPoint].heatConsumed = meterRequirement;
            cbrCase[insertPoint].overDriveConsumed = odRequirement;
            if ((minx != -1 || miny != -1) &&nextMinDisResolve == true) {
                //if (cbrCase[insertPoint].getMetadata()->getNeutral()[0]) {
                    cbrCase[insertPoint].getMetadata()->hitMinX = minx;
                    cbrCase[insertPoint].getMetadata()->hitMinY = miny;
                    //Checking if min distance is truely only set in the proper conditions
                    if ((cbrCase[insertPoint].getMetadata()->hitMinX > 0 || cbrCase[insertPoint].getMetadata()->hitMinX > 0)) {
                        if (!cbrCase[insertPoint + 1].getMetadata()->getHitThisFrame()[1] && !cbrCase[insertPoint + 1].getMetadata()->getBlockThisFrame()[1]) {
                            retError.errorCount++;
                             retError.errorDetail += "minDist: \n";
                        }
                    }

                minx = -1; miny = -1;
                nextMinDisResolve = false;
            }
            else { 
                if ((minx != -1 || miny != -1)) {
                    nextMinDisResolve = true;
                }
                cbrCase[insertPoint].getMetadata()->hitMinX = -1;
                cbrCase[insertPoint].getMetadata()->hitMinY = -1;
            }
            

            //reset vars when case is created
            if (inputBuffering == false) { 
                meterRequirement = 0; 
                odRequirement = 0;
            }
            
            //if i is somehow bigger than the array its iterating create an error
            if ((int)i > iteratorIndex) {
                retError.errorCount++;
                retError.errorDetail += "Iterator error: " + std::to_string(inputResolveName) + "\n";
            }
            iteratorIndex = i - 1;
            framesIdle = -1;
            
        }
        framesIdle++;

        
        /* std::cout << someVector[i]; ... */
    }

    //The following is messy code to for finishing the case generation or to make instant learning work
    if (framesIdle > 0){
        if (framesIdle < caseMinTime && cbrCase.size() > 0 && insertPoint > 0) {
            int updatedInsertPoint = insertPoint - 1;
            cbrCase[updatedInsertPoint].SetEndIndex(cbrCase[updatedInsertPoint].getEndIndex() + framesIdle);
            if (inputResolve.size() == 0 && dollResolve.size() == 0) {
                inputBuffering = false;
            }
            cbrCase[updatedInsertPoint].setInputBufferSequence(inputBuffering);
            if (cbrCase[updatedInsertPoint].heatConsumed < meterRequirement) {
                cbrCase[updatedInsertPoint].heatConsumed = meterRequirement;
            }
            if (cbrCase[updatedInsertPoint].overDriveConsumed < odRequirement) {
                cbrCase[updatedInsertPoint].overDriveConsumed = odRequirement;
            }
            if ((minx != -1 || miny != -1) && nextMinDisResolve == true) {
                //if (cbrCase[updatedInsertPoint].getMetadata()->getNeutral()[0]) {
                    cbrCase[updatedInsertPoint].getMetadata()->hitMinX = minx;
                    cbrCase[updatedInsertPoint].getMetadata()->hitMinY = miny;
                    //Checking if min distance is truely only set in the proper conditions
                    if ((cbrCase[updatedInsertPoint].getMetadata()->hitMinX > 0 || cbrCase[updatedInsertPoint].getMetadata()->hitMinX > 0)) {
                        if (!cbrCase[updatedInsertPoint + 1].getMetadata()->getHitThisFrame()[1] && !cbrCase[updatedInsertPoint + 1].getMetadata()->getBlockThisFrame()[1]) {
                            retError.errorCount++;
                            retError.errorDetail += "minDist: \n";
                        }
                        //if (!cbrCase[updatedInsertPoint].getMetadata()->getNeutral()[0]) {
                        //    retError.errorCount++;
                            retError.errorDetail += "minDist: \n";
                        //}
                    }
                //}
                //else {
                //    cbrCase[updatedInsertPoint].getMetadata()->hitMinX = -1;
                //    cbrCase[updatedInsertPoint].getMetadata()->hitMinY = -1;
                //}
                minx = -1; miny = -1;
                nextMinDisResolve = false;
            }
            else {
                if ((minx != -1 || miny != -1)) {
                    nextMinDisResolve = true;
                }
                cbrCase[updatedInsertPoint].getMetadata()->hitMinX = -1;
                cbrCase[updatedInsertPoint].getMetadata()->hitMinY = -1;
            }
            
            //auto end = cbrCase[updatedInsertPoint].getEndIndex();
            //cbrCase.erase(cbrCase.begin() + updatedInsertPoint);
            //cbrCase.insert(cbrCase.begin() + updatedInsertPoint, CbrCase(std::move(ar->CopyMetadataPtr(startIndex)), (int)startIndex, end));
            if (cbrCase[updatedInsertPoint].getStartingIndex() > cbrCase[updatedInsertPoint].getEndIndex()) {
                retError.errorCount++;
                retError.errorDetail += "StartingIndex Error: \n";
            }
            auto caseBaseSize = cbrCase.size();
            auto testEnd = 0;
            for (std::size_t j = 0; j < caseBaseSize; ++j) {
                if (j == 0) {
                    testEnd = cbrCase[j].getEndIndex();
                }
                else {
                    if (testEnd + 1 != cbrCase[j].getStartingIndex()) {
                        retError.errorCount++;
                        retError.errorDetail += "testEnd error: \n";
                    }
                    testEnd = cbrCase[j].getEndIndex();
                }
            }
        }
        else {
            cbrCase.insert(cbrCase.begin() + insertPoint, CbrCase(std::move(ar->CopyMetadataPtr(startIndex)), (int)startIndex, iteratorIndex));
            if (cbrCase[insertPoint].heatConsumed < meterRequirement) {
                cbrCase[insertPoint].heatConsumed = meterRequirement;
            }
            if (cbrCase[insertPoint].overDriveConsumed < odRequirement) {
                cbrCase[insertPoint].overDriveConsumed = odRequirement;
            }
            if ((minx != -1 || miny != -1) && nextMinDisResolve == true) {
                //if (cbrCase[insertPoint].getMetadata()->getNeutral()[0]) {
                    cbrCase[insertPoint].getMetadata()->hitMinX = minx;
                    cbrCase[insertPoint].getMetadata()->hitMinY = miny;
                    //Checking if min distance is truely only set in the proper conditions
                    if ((cbrCase[insertPoint].getMetadata()->hitMinX > 0 || cbrCase[insertPoint].getMetadata()->hitMinX > 0)) {
                        if (!cbrCase[insertPoint + 1].getMetadata()->getHitThisFrame()[1] && !cbrCase[insertPoint + 1].getMetadata()->getBlockThisFrame()[1]) {
                            retError.errorCount++;
                            retError.errorDetail += "minDist: \n";
                        }
                        //if (!cbrCase[insertPoint].getMetadata()->getNeutral()[0]) {
                        //    retError.errorCount++;
                        //    retError.errorDetail += "minDist: \n";
                        //}
                    }
                //}
                //else {
                //    cbrCase[insertPoint].getMetadata()->hitMinX = -1;
                //    cbrCase[insertPoint].getMetadata()->hitMinY = -1;
                //}
                minx = -1; miny = -1;
                nextMinDisResolve = false;
            }
            else {
                if ((minx != -1 || miny != -1)) {
                    nextMinDisResolve = true;
                }
                cbrCase[insertPoint].getMetadata()->hitMinX = -1;
                cbrCase[insertPoint].getMetadata()->hitMinY = -1;
            }
            cbrCase[insertPoint].setInputBufferSequence(inputBuffering);
            if ((int)startIndex > iteratorIndex) {
                retError.errorCount++;
                retError.errorDetail += "startIndex error\n";
            }
            auto caseBaseSize = cbrCase.size();
            auto testEnd = 0;
            for (std::size_t j = 0; j < caseBaseSize; ++j) {
                if (j == 0) {
                    testEnd = cbrCase[j].getEndIndex();
                }
                else {
                    if (testEnd + 1 != cbrCase[j].getStartingIndex()) {
                        retError.errorCount++;
                        retError.errorDetail += "testend2nd error: \n";
                    }
                    testEnd = cbrCase[j].getEndIndex();
                }
            }
            
        }
        
    }

    if (inputResolve.size() > 0) {
        auto caseBaseSize = cbrCase.size();
        for (int i = insertPoint-1; i >= 0 && inputResolve.size() > 0; --i) {
            cbrCase[i].setInputBufferSequence(true);
            if (cbrCase[i].heatConsumed < meterRequirement) {
                cbrCase[i].heatConsumed = meterRequirement;
            }
            if (cbrCase[i].overDriveConsumed < odRequirement) {
                cbrCase[i].overDriveConsumed = odRequirement;
            }
            if ((minx != -1 || miny != -1) && nextMinDisResolve == true) {
                //if (cbrCase[i].getMetadata()->getNeutral()[0]) {
                    cbrCase[i].getMetadata()->hitMinX = minx;
                    cbrCase[i].getMetadata()->hitMinY = miny;
                    //Checking if min distance is truely only set in the proper conditions
                    if ((cbrCase[i].getMetadata()->hitMinX > 0 || cbrCase[i].getMetadata()->hitMinX > 0)) {
                        if (!cbrCase[i + 1].getMetadata()->getHitThisFrame()[1] && !cbrCase[i + 1].getMetadata()->getBlockThisFrame()[1]) {
                            retError.errorCount++;
                            retError.errorDetail += "minDist: \n";
                        }
                        //if (!cbrCase[i].getMetadata()->getNeutral()[0]) {
                        //    retError.errorCount++;
                        //    retError.errorDetail += "minDist: \n";
                        //}
                    }
               // }
                //else {
                //    cbrCase[i].getMetadata()->hitMinX = -1;
                //    cbrCase[i].getMetadata()->hitMinY = -1;
                //}
                minx = -1; miny = -1;
                nextMinDisResolve = false;
            }
            else {
                if ((minx != -1 || miny != -1)) {
                    nextMinDisResolve = true;
                }
                cbrCase[i].getMetadata()->hitMinX = -1;
                cbrCase[i].getMetadata()->hitMinY = -1;
            }
            for (int k = cbrCase[i].getEndIndex(); k >= cbrCase[i].getStartingIndex() && inputResolve.size() > 0; --k) {
                auto deconInputs = DeconstructInput(input[k], facingCommandStart);
                for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                    inputResolve = CheckCommandExecution(deconInputs[j], inputResolve);//fix this inputResolve doesnt update from the function.
                    if (inputResolve.size() == 0) {
                        cbrCase[i].setInputBufferSequence(false);
                        break;
                    }
                }
            }
        }
    }
    /*if (carlDoll && dollResolve.size() > 0) {
        auto caseBaseSize = cbrCase.size();
        for (int i = insertPoint - 1; i >= 0 && dollResolve.size() > 0; --i) {
            cbrCase[i].setInputBufferSequence(true);
            if (cbrCase[i].heatConsumed < meterRequirement) {
                cbrCase[i].heatConsumed = meterRequirement;
            }
            if (cbrCase[i].overDriveConsumed < odRequirement) {
                cbrCase[i].overDriveConsumed = odRequirement;
            }
            for (int k = cbrCase[i].getEndIndex(); k >= cbrCase[i].getStartingIndex() && dollResolve.size() > 0; --k) {
                auto deconInputs = DeconstructInput(input[k], facingCommandStart);
                for (std::vector<int>::size_type j = 0; j < deconInputs.size(); ++j) {
                    dollResolve = CheckCommandExecution(deconInputs[j], dollResolve);//fix this inputResolve doesnt update from the function.
                    if (dollResolve.size() == 0) {
                        cbrCase[i].setInputBufferSequence(false);
                        break;
                    }
                }
            }
        }
    }*/

    auto caseBaseSize = cbrCase.size();
    auto testEnd = 0;
    for (std::size_t j = 0; j < caseBaseSize; ++j) {
        if (j == 0) {
            testEnd = cbrCase[j].getEndIndex();
        }
        else {
            if (testEnd + 1 != cbrCase[j].getStartingIndex()) {
                retError.errorCount++;
                retError.errorDetail += "testend3nd error: \n";
            }
            testEnd = cbrCase[j].getEndIndex();
        }
        //Checking if min distance is truely only set in the proper conditions
        if ((cbrCase[j].getMetadata()->hitMinX > 0 || cbrCase[j].getMetadata()->hitMinX > 0)) {
            if (!cbrCase[j + 1].getMetadata()->getHitThisFrame()[1] && !cbrCase[j + 1].getMetadata()->getBlockThisFrame()[1]) {
                retError.errorCount++;
                retError.errorDetail += "minDist: \n";
            }
            //if (!cbrCase[j].getMetadata()->getNeutral()[0]) {
            //    retError.errorCount++;
            //    retError.errorDetail += "minDist: \n";
            //}
        }
  
    }

    if (retError.errorCount > 0) {
        for (std::vector<int>::size_type i = startIndex;
            i < adjustedEndIndex; i++) {
            if (ar->CopyMetadataPtr(i) != nullptr) {
                retError.errorDetail += " Frame: " + std::to_string(i) + " " + std::to_string(ar->CopyMetadataPtr(i)->getCurrentActId()[0]);
            }
            else {
                retError.errorDetail += " Frame: " + std::to_string(i) + " " + "ERROR PTR WAS NULL";
            }
        }
        retError.errorDetail += "\n";
        for (std::vector<int>::size_type i = startIndex;
            i < adjustedEndIndex; i++) {
            retError.errorDetail += " Frame: " + std::to_string(i) + " " + std::to_string(input[i]);
        }
    }

    retError.structure = charName + "\n" + debugStructureStr;
    return retError;
}


bool CbrReplayFile::CheckCaseEnd(int framesIdle, Metadata ar, uint16_t prevState, bool neutral, std::vector<CommandActionsAC>& commands) {
    //if states changed and minimum case time is reached change state
    auto curState = ar.getCurrentActId()[0];
    bool command = ContainsCommandState(prevState, commands);
    
    bool dashing = curState == 14 || curState == 16 || curState == 12 || curState == 13;

    if (curState != prevState && (framesIdle >= caseMinTime || !ar.getNeutral()[0] || command) ) {
        stateChangeTrigger = true;
        return true;
    }

    //if player is hit or hits the opponent also on guard.
    if ((ar.getHitThisFrame()[0] || ar.getBlockThisFrame()[0] || ar.getHitThisFrame()[1] || ar.getBlockThisFrame()[1])) {
        return true;
    }

    // if in neutral state for longer than caseNeutralTime end case
    if (neutral && !dashing && (framesIdle >= caseNeutralTime)) {
        return true;
    }
    return false;
}

CbrGenerationError CbrReplayFile::instantLearning(AnnotatedReplay* ar, std::string charName) {
    CbrGenerationError ret;
    ret.errorCount = 0;
    ret.errorDetail = "";
    int start = 0;
    int end = ar->getInput().size() - 1;
    if (end <= 1) { return ret; }
    if (cbrCase.size() > 0) {
        start = cbrCase[cbrCase.size() -1].getStartingIndex();
        cbrCase.pop_back();
        ret = MakeCaseBase(ar, charName, start, end, cbrCase.size(), true);

    }
    else {
        ret = MakeCaseBase(ar, charName, start, end, 0, true);
    }
    return ret;
}

void trimIdleInReplay(AnnotatedReplay* ar) {
    int start = ar->getInput().size() - 1;
    int end = start;
    for (int i = start; i > 0; i--) {
        if (ar->getInput()[i] != 5 || ar->ViewMetadata(i).getNeutral()[0] == false) {
            end = i;
            break;
        }
    }
    auto input = ar->getInputPtr();
    auto meta = ar->getAllMetadata();
    if (end != start) {
        input->erase(std::next(input->begin(), end + 1), std::next(input->begin(), start));
        meta->erase(std::next(meta->begin(), end + 1), std::next(meta->begin(), start));
    }

    start = input->size() - 1;
    end = 0;
    for (int i = 0; i <= start; i++) {
        if (input->at(i) != 5 || ar->ViewMetadata(i).getNeutral()[0] == false) {
            end = i;
            break;
        }
    }
    if (end != 0) {
        input->erase(input->begin(), input->begin() + end - 1);
        meta->erase(meta->begin(), meta->begin() + end - 1);
    }


}

CbrGenerationError CbrReplayFile::makeFullCaseBase(AnnotatedReplay* ar, std::string charName, bool trim) {

    if (trim) {
        trimIdleInReplay(ar);
    }
    

    CbrGenerationError ret;
    ret.errorCount = 0;
    ret.errorDetail = "";
    int start = 0;
    int end = ar->getInput().size() - 1;

    if (end <= 0) {
        ret.errorCount = 1;
        ret.errorDetail = "0 size replay";
        return ret;
    }



    auto testCopy = *ar;
    ret = MakeCaseBase(ar, charName, start, end, 0);
    if (ret.errorCount > 0) {
        ret = MakeCaseBase(&testCopy, charName, start, end, 0);
    }
    return ret;
}



#define TButton 512 //Taunt
#define DButton 256 //Dust
#define HButton 128 //Heavy slash
#define SButton 64 //Slash
#define KButton 32 //Kick
#define PButton 16 //Punch
#define T 512
#define D 256
#define H 128
#define S 64
#define K 32
#define P 16
#define forward 1100
#define back 1101
#define up 1102
#define down 1103
#define chargeBack back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back
#define chargeDown down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down
#define chargeDown40f down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down,down
#define chargeBack45f back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back,back
#define s360_1 2,back,up,6
#define s360_2 2,forward,up,4
#define s360_3 4,up,forward,2
#define s360_4 4,down,forward,8
#define s360_5 8,forward,down,4
#define s360_6 8,back,down,6
#define s360_7 6,down,back,8
#define s360_8 6,up,back,2


std::vector<int> CbrReplayFile::DeconstructInput(int input, bool facingSwitch) {
    std::vector<int> inputs;

    auto buffer = input;
    auto test = buffer - TButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(T);
    }
    test = buffer - DButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(D);
    }
    test = buffer - HButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(H);
    }
    test = buffer - SButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(S);
    }
    test = buffer - KButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(K);
    }
    test = buffer - PButton;
    if (test > 0) {
        buffer = test;
        inputs.push_back(P);
    }

    if (facingSwitch) {
        if (buffer == 6 || buffer == 3 || buffer == 9) {
            buffer = buffer - 2;
        }
        else {
            if (buffer == 4 || buffer == 7 || buffer == 1) {
                buffer = buffer + 2;
            }
        }


    }
    inputs.push_back(buffer);
    return inputs;
}
std::vector < inputMemory> CbrReplayFile::DeleteCompletedInputs(uint16_t name, std::vector < inputMemory>& inputBuffer) {
    for (int i = inputBuffer.size()-1; i >= 0 ; i--) {
        if (inputBuffer[i].name == name) {
            inputBuffer.erase(inputBuffer.begin() + i);
        }
    }
    return inputBuffer;
}
std::vector < inputMemory> CbrReplayFile::CheckCommandExecution(int input, std::vector < inputMemory>& inputBuffer)
{
    for (int i = 0; i < inputBuffer.size(); ++i) {
        auto test = inputBuffer[i].inputs.size() - 1;
        for (int j = inputBuffer[i].inputs.size() - 1; j >= 0; --j) {
            if (inputBuffer[i].inputs[j] < 0) {
                if (isDirectionInputs(-1 * inputBuffer[i].inputs[j])) {
                    if (!checkDirectionInputs(inputBuffer[i].inputs[j] * -1, input)) {
                        if (j == 0) {
                            return DeleteCompletedInputs(inputBuffer[i].name, inputBuffer);
                        }
                        else {
                            inputBuffer[i].inputs[j] = 0;
                        }
                    }
                }
                else {
                    if (input != inputBuffer[i].inputs[j] * -1) {
                        if (j == 0) {
                            return DeleteCompletedInputs(inputBuffer[i].name, inputBuffer);
                        }
                        else {
                            inputBuffer[i].inputs[j] = 0;
                        }
                    }
                }
                break;
            }
            if (inputBuffer[i].inputs[j] > 0) {
                if (isDirectionInputs( inputBuffer[i].inputs[j])) {
                    if (checkDirectionInputs(inputBuffer[i].inputs[j], input)) {
                        if (j == 0) {
                            return DeleteCompletedInputs(inputBuffer[i].name, inputBuffer);
                        }
                        else {
                            inputBuffer[i].inputs[j] = 0;
                        }
                    }
                }
                else {
                    if (input == inputBuffer[i].inputs[j] ) {
                        if (j == 0) {
                            return DeleteCompletedInputs(inputBuffer[i].name, inputBuffer);
                        }
                        else {
                            inputBuffer[i].inputs[j] = 0;
                        }
                    }
                }
                break;
            }
        }
    }
    return inputBuffer;
}

bool CbrReplayFile::isDirectionInputs(int direction) {
    if (direction >= 1100)return true;
    return false;
}

bool CbrReplayFile::checkDirectionInputs(int direction, int input) {
    if (direction == up) {
        if (input == 7)return true;
        if (input == 8)return true;
        if (input == 9)return true;
    }
    if (direction == forward) {
        if (input == 9)return true;
        if (input == 6)return true;
        if (input == 3)return true;
    }
    if (direction == down) {
        if (input == 3)return true;
        if (input == 2)return true;
        if (input == 1)return true;
    }
    if (direction == back) {
        if (input == 1)return true;
        if (input == 4)return true;
        if (input == 7)return true;
    }
    return false;
}
bool CbrReplayFile::ContainsCommandState(uint16_t move, std::vector<CommandActionsAC>& commands) {
    for (std::size_t i = 0; i < commands.size(); ++i) {
        if (commands[i].moveName == move) {
            return true;
        }
    }
    return false;
}
std::vector < inputMemory> CbrReplayFile::MakeInputArray(uint16_t move, std::vector<CommandActionsAC>& commands, uint16_t prevState) {
    int lastInput = -1;
    bool threeSixty = false;

    inputMemory inputsSequence;
    std::vector<inputMemory> container = { };

    for (std::size_t i = 0; i < commands.size(); ++i) {
        if (commands[i].moveName == move) {
            inputMemory inputsSequence;
            inputsSequence.name = move;
            if (commands[i].priorMove == prevState) {
                for (std::size_t j = 0; j < commands[i].altInputs.size(); ++j) {
                    inputsSequence.inputs.push_back(commands[i].altInputs[j]);
                }
            }
            else {
                for (std::size_t j = 0; j < commands[i].inputs.size(); ++j) {
                    inputsSequence.inputs.push_back(commands[i].inputs[j]);
                }
            }

            container.push_back(inputsSequence);
        }
    }
    return container;
}
std::vector<CommandActionsAC> superjumpCommands = {
{
    4, {down, up}}, //Pre jump
};
std::vector < inputMemory> CbrReplayFile::MakeInputArraySuperJump(uint16_t move, std::vector<int>& inputs, int startingIndex, std::vector < inputMemory>& inVec) {

    
        inputMemory container;

        if (move == superjumpCommands[0].moveName) {
            int foundUp = -1;
            for (int i = 0; i >= -2 && startingIndex + i >= 0; i--) {
                auto deconInputs = DeconstructInput(inputs[startingIndex + i], false);
                for (int o = 0; o < deconInputs.size(); o++) {
                    if (checkDirectionInputs(up, deconInputs[o])) {
                        foundUp = startingIndex + i;
                    }
                }
            }
            if (foundUp > -1) {
                int foundDown = 999;
                for (int i = 0; i >= -9; i--) {
                    if (foundUp + i < 0) { continue; }
                    auto deconInputs = DeconstructInput(inputs[foundUp + i], false);
                    for (int o = 0; o < deconInputs.size(); o++) {
                        if (checkDirectionInputs(down, deconInputs[o])) {
                            foundDown = startingIndex + foundUp + i;
                            container.inputs = superjumpCommands[0].inputs;
                            container.name = superjumpCommands[0].moveName;
                            inVec.push_back(container);
                            return inVec;
                        }
                    }
                }
            }
        }

    return inVec;
}

std::vector<CommandActionsAC> solCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {98, {2, 3, 6, P}}, //Gun flame
    {244, {2, 3, 6, P}}, //DI gun flame
    {186, {2, 1, 4, P}}, //Gun flame (feint)
    {100, {6, 2, 3, S}}, //Volcanic viper (ground, S-version)
    {104, {6, 2, 3, H}}, //Volcanic viper (ground, H-version)
    {110, {6, 2, 3, S}}, //Volcanic viper (air, S-version)
    {110, {6, 2, 9, 3, S}}, //Volcanic viper (air, S-version, tk)
    {112, {6, 2, 3, H}}, //Volcanic viper (air, H-version)
    {112, {6, 2, 9, 3, H}}, //Volcanic viper (air, H-version, tk)
    {117, {2, 1, 4, K}}, //Volcanic viper follow up
    {106, {2, 1, 4, S}}, //Grand viper
    {219, {2, 3, 6, K}}, //Bandit revolver (ground)
    {132, {2, 3, 6, K}}, //Bandit revolver (air)
    {132, {2, 3, 9, 6, K}}, //Bandit revolver (air, tk)
    {219, {2, 3, 6, K, K, K, K}}, //Bandit bringer
    {222, {2, 1, 4, K}}, //Riot stomp
    {136, {6, 2, 3, K}}, //Wild throw
    {272, {2, 3, 6, H}}, //Sidewinder (H)
    {256, {4, 1, 3, 6, H}}, //Fafnir (H)
    {256, {4, 2, 6, H}},
    {294, {6, 4, D}}, //Tyrant rave (Fafnir followup)
    {343, {2, 1, 4, K}}, //Slam (DI)
    {293, {4, 1, 3, 6, D}}, //Fafnir (D)
    {293, {4, 2, 6, D}}, 
    {340, {2, 3, 6, D}}, //Sidewinder (D)
    {96, {6, 3, 1, 4, 6, H}}, //Tyrand rave ver. beta
    {96, {6, 2, 4, 6, H}},
    {223, {2, 1, 4, 2, 1, 4, S}}, //Dragon install
    {296, {2, 1, 4, 2, 1, 4, 2, 1, 4, 2, 1, 4, P + H}}, //Dragon install 2nd
    {126, {2, 3, 6, 2, 3, 6, H}} //Napalm death (IK)
};

std::vector<CommandActionsAC> kyCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {186, {2, 3, 6, S}}, //Stun edge
    {230, {2, 3, 6, H}}, //Stun edge
    {181, {2, 3, 6, S}}, //Air stun edge
    {181, {2, 3, 9, S}}, //Air stun edge (TK)
    {190, {2, 3, 6, H}}, //Air stun edge
    {190, {2, 3, 9, H}}, //Air stun edge (TK)
    {98,  {6, 2, 3, S}}, //Vapor thrust
    {135, {6, 2, 3, H}}, //Vapor thrust
    {137, {6, 2, 3, S}}, //Air vapor thrust
    {137, {6, 2, 3, 9, S}},
    {137, {6, 2, 3, H}},
    {137, {6, 2, 9, 3, H}},
    {187, {2, 3, 6, K}}, //Stun dipper
    {103, {2, 1, 4, K}}, //Greed sever
    {253, {2, 2, 2, H}}, //Lightning strike
    {229, {2, 3, 6, D}}, //Air charged stun edge
    {229, {2, 3, 9, D}}, //Air charged stun edge (TK)
    {111, {2, 3, 6, D}}, //Charged stun edge
    {251, {4, D}}, //Lightning sphere
    {252, {4, 6, D}}, //Charge drive
    {255, {2, 1, 4, D}}, //Stun raising
    {307, {2, 1, 4, D}}, //Greed sever (FB)
    {102, {6, 3, 1, 4, H}}, //Ride the lightning
    {102, {6, 2, 4, H}},
    {170, {6, 3, 1, 4, H}}, //Air ride the lightning
    {170, {6, 2, 4, H}},
    {197, {2, 3, 6, 2, 3, 6, P}}, //Sacred edge
    {96,  {2, 3, 6, 2, 3, 6, H}} //Rising force (IK)
};

std::vector<CommandActionsAC> mayCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {131, {chargeBack, forward, S}}, //Mr. dolphin horizontal (S)
    {133, {chargeBack, forward, H}}, //Mr. dolphin horizontal (H)
    {260, {-6, 6, D}}, //Go Mr. dolphin horizontal (S)
    {262, {-6, 6, D}}, //Go Mr. doplhin horizontal (H)
    {126, {chargeDown, up, S}}, //Mr. dolphin vertical (S)
    {129, {chargeDown, up, H}}, //Mr. dolphin vertical (H)
    {264, {-8, 8, D}}, //Go Mr. dolphin vertical (S)
    {266, {-8, 8, D}}, //Go Mr. dolphin vertical (H)
    {103, {6, 2, 3, S}}, //Restive rolling (S)
    {236, {6, 2, 3, H}}, //Restive rolling (H)
    {153, {6, 2, 3, S}}, //Air restive rolling (S)
    {234, {6, 2, 3, H}}, //Air restive rolling (H)
    {114, {4, 1, 3, 6, P}}, //Applause for the victim
    {114, {4, 2, 6, P}},
    {114, {4, 1, 3, 6, K}},
    {114, {4, 2, 6, K}},
    {114, {4, 1, 3, 6, S}},
    {114, {4, 2, 6, S}},
    {114, {4, 1, 3, 6, H}},
    {114, {4, 2, 6, H}},
    {114, {4, 1, 3, 6, D}},
    {114, {4, 2, 6, D}},
    {107, {6, 3, 1, 4, K}}, //Overhead kiss
    {107, {6, 2, 4, K}}, //Overhead kiss
    {258, {2, 1, 4, D}}, //Jackhound
    {111, {6, 3, 1, 4, H}}, //Ultimate whiner
    {111, {6, 2, 4, H}},
    {135, {2, 3, 6, 2, 3, 6, S}}, //Great yamada attack
    {194, {6, 3, 1, 4, S}}, //Super screaming ultimate spinning whirlwind
    {194, {6, 2, 4, S}},
    {162, {4, 1, 3, 6, 4, 1, 3, 6, H}}, //May and the jolly crew (IK)
    {162, {4, 1, 3, 6, 4, 2, 6, H}},
    {162, {4, 2, 6, 4, 1, 3, 6, H}},
    {162, {4, 2, 6, 4, 2, 6, H}}
};

std::vector<CommandActionsAC> milliaCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {159, {2, 3, 6, K}}, //Turbo fall
    {159, {2, 3, 9, K}},
    {187, {2, 1, 4, K}}, //Forward roll
    {161, {2, 1, 4, H}}, //Secret garden
    {185, {2, 3, 6, S}}, //Tandem top (S)
    {115, {2, 3, 6, H}}, //Tandem top (H)
    {186, {2, 1, 4, P}}, //Iron savior
    {183, {2, 3, 6, P}}, //Bad moon
    {183, {2, 3, 9, P}},
    {287, {2, 1, 4, K}}, //Silent force (K)
    {287, {2, 1, 7, K}},
    {217, {2, 1, 4, S}}, //Silent force (S)
    {217, {2, 1, 7, S}},
    {231, {2, 1, 4, H}}, //Silent force (H)
    {231, {2, 1, 7, H}},
    {191, {2, 1, 4, S}}, //Lust shaker
    {253, {2, 1, 4, D}}, //Longinus
    {250, {2, 3, 6, D}}, //Pretty maze
    {251, {2, 3, 6, D}}, //Pretty maze (air)
    {251, {2, 3, 9, D}},
    {282, {2, 1, 4, D}}, //Secret garden (fb)
    {100, {2, 1, 4, 1, 3, 6, H}}, //Winger
    {100, {2, 1, 4, 2, 6, H}},
    {285, {2, 1, 4, 1, 3, 6, H}}, //Winger (air)
    {285, {2, 1, 4, 2, 6, H}},
    {285, {2, 1, 4, 2, 9, H}},
    {166, {2, 3, 6, 2, 3, 6, S}}, //Emerald rain
    {113, {2, 3, 6, 2, 3, 6, H}} //Iron maiden (IK)
};

std::vector<CommandActionsAC> axlCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {130, {6, 2, 3, S}}, //Benten gari (S)
    {131, {6, 2, 3, H}}, //Benten gari (H)
    {133, {6, 2, 3, H}}, //Follow up Axl bomber
    {136, {chargeBack, forward, S}}, //Rensen geki
    {140, {chargeBack, forward, H}}, //Rashou sen
    {198, {6, 2, 3, H}}, //Axl bomber
    {198, {6, 2, 9, 3, H}},
    {188, {2, 1, 4, P}}, //Tenhou seki upper
    {189, {2, 1, 4, K}}, //Tenhou seki low
    {208, {6, 2, 3, P}}, //Hachi subako
    {210, {4, 2, 1, D}}, //Shiranami no homura
    {145, {6, 3, 1, 4, S}}, //Raiei sageki (S)
    {145, {6, 2, 4, S}},
    {148, {6, 3, 1, 4, H}}, //Raiei sageki (H)
    {148, {6, 2, 4, H}}, //Raiei sageki (H)
    {244, {6, 3, 1, 4, S}}, //Kokuu geki
    {244, {6, 2, 4, S}},
    {244, {6, 3, 1, 7, S}},
    {244, {6, 2, 7, S}},
    {264, {6, 2, 3, D}}, //Axl bomber (fb)
    {264, {6, 2, 9, 3, D}},
    {296, {chargeBack, forward, D}}, //Kyokusa geki
    {157, {2, 3, 6, 3, 1, 4, H}}, //Byakue renshou
    {157, {2, 3, 6, 2, 4, H}},
    {129, {2, 3, 6, 2, 3, 6, H}} //Rensen ougi: midare gami (IK)
};

std::vector<CommandActionsAC> potemkinCommands =
{
    {16, {-4, 4, -back, 4}}, //Back dash
    {182, {2, 3, 6, P}}, //Megafist forward
    {182, {2, 3, 9, P}},
    {185, {2, 1, 4, P}}, //Megafist backward
    {185, {2, 1, 7, P}},
    {181, {2, 3, 6, S}}, //Slide head
    {175, {6, 3, 1, 4, S}}, //F.D.B
    {175, {6, 2, 4, S}},
    {120, {chargeBack, 6, H}}, //Hammerfall
    {120, {chargeBack, 9, H}},
    {122, {6, 3, 1, 4, 6, P}}, //Potemkin buster
    {122, {6, 2, 4, 6, P}}, //Potemkin buster
    {114, {6, 2, 3, H}}, //Heat knuckle
    {118, {6, 3, 1, 4, H}}, //Heat extend
    {118, {6, 2, 4, H}},
    {236, {6, 3, 1, 4, D}}, //Judge gauntlet
    {236, {6, 2, 4, D}}, //Judge gauntlet
    {257, {6, 3, 1, 4, 6, D}}, //Air Potemkin buster
    {257, {6, 2, 4, 6, D}},
    {107, {6, 3, 1, 4, 6, H}}, //Giganter
    {107, {6, 2, 4, 6, H}},
    {109, {4, 1, 3, 6, 4, 1, 3, 6, P}}, //Gigantic bullet
    {109, {4, 1, 3, 6, 4, 2, 6, P}},
    {109, {4, 2, 6, 4, 1, 3, 6, P}},
    {109, {4, 2, 6, 4, 2, 6, P}},
    {130, {2, 3, 6, 2, 3, 6, S}}, //Heavenly Potemkin buster
    {150, {2, 3, 6, 2, 3, 6, H}} //Magnum opera (IK)
};

std::vector<CommandActionsAC> chippCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {113, {2, 3, 6, P}}, //Alpha blade
    {133, {2, 3, 6, P}}, //Alpha blade (air)
    {133, {2, 3, 9, P}},
    {212, {6, 2, 3, S}}, //Beta blade
    {106, {6, 2, 3, S}}, //Beta blade (air)
    {106, {6, 2, 9, 3, S}},
    {104, {4, 1, 3, 6, H}}, //Gamma blade
    {104, {4, 2, 6, H}}, 
    {119, {2, 1, 4, K}}, //Tsuyoshi-shiki meisei
    {142, {4, 1, 3, 6, K}}, //Genrou zan
    {142, {4, 2, 6, K}},
    {177, {2, 3, 6, S}}, //Resshou
    {178, {2, 3, 6, S}}, //Rokusai
    {179, {2, 3, 6, K}}, //Senshuu
    {117, {-2, 2, -down, 2, P}}, //Tsuyoshi-shiki: Ten'i (P)
    {185, {-2, 2, -down, 2, K}}, //Tsuyoshi-shiki: Ten'i (K)
    {183, {-2, 2, -down, 2, S}}, //Tsuyoshi-shiki: Ten'i (S)
    {115, {-2, 2, -down, 2, H}}, //Tsuyoshi-shiki: Ten'i (H)
    {261, {-2, 2, -down, 2, D}}, //Tsuyoshi-shiki: Ten'i (D)
    {202, {2, 1, 4, P}}, //Aerial taunt (slow)
    {202, {2, 1, 7, P}},
    {258, {2, 1, 4, P}}, //Aerial taunt (fast)
    {258, {2, 1, 7, P}},
    {335, {6, 2, 3, D}}, //Beta blade (fb)
    {336, {6, 2, 3, D}}, //Beta blade (air, fb)
    {336, {6, 2, 9, 3, D}},
    {339, {4, 1, 3, 6, D}}, //Gamma blade (fb)
    {339, {4, 2, 6, D}},
    {223, {6, 3, 1, 4, 6, H}}, //Zensei rouga
    {223, {6, 2, 4, 6, H}},
    {138, {2, 3, 6, 2, 3, 6, K}}, //Banki messai
    {151, {2, 3, 6, 2, 3, 6, H}} //Delta end (IK)
};

std::vector<CommandActionsAC> eddieCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {183, {-2, 2, -down, 2, S}}, //Invite Hell (S)
    {113, {-2, 2, -down, 2, H}}, //Invite Hell (H)
    {101, {2, 1, 4, K}}, //Break the law
    {227, {4, 1, 3, 6, S}}, //Shadow gallery
    {227, {4, 2, 6, S}},
    {181, {2, 3, 6, P}}, //Summon Eddie (P)
    {182, {2, 3, 6, P}}, //Recall Eddie (P)
    {181, {2, 3, 6, K}}, //Summon Eddie (K)
    {182, {2, 3, 6, K}}, //Recall Eddie (K)
    {181, {2, 3, 6, S}}, //Summon Eddie (S)
    {182, {2, 3, 6, S}}, //Recall Eddie (S)
    {181, {2, 3, 6, H}}, //Summon Eddie (H)
    {182, {2, 3, 6, H}}, //Recall Eddie (H)
    {262, {2, 1, 4, P}}, //Traversing
    {275, {2, 1, 4, H}}, //Eddie summoning vice
    {124, {6, 2, 3, S}}, //Damned fang
    {98, {2, 1, 4, S}}, //Drunkard shade
    {163, {4, 1, 3, 6, S}}, //Shadow gallery (air)
    {163, {4, 2, 6, S}},
    {163, {4, 1, 3, 9, S}},
    {163, {4, 2, 9, S}},
    {113, {-2, 2, -down, 2, D}}, //Drill special
    {249, {2, 3, 6, D}}, //Exhaustion
    {106, {6, 3, 1, 4, 6, H}}, //Amorphous
    {106, {6, 2, 4, 6, H}},
    {184, {2, 3, 6, 2, 3, 6, S}}, //Executor-X
    {184, {2, 3, 6, 2, 3, 9, S}},
    {191, {2, 3, 6, 2, 3, 6, H}} //Black in mind (IK)
};

std::vector<CommandActionsAC> baikenCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {187, {2, 3, 6, K}}, //Tatami gaeshi
    {158, {2, 3, 6, K}}, //Tatami gaeshi (air)
    {158, {2, 3, 9, K}},
    {185, {6, 2, 3, S}}, //Youzansen
    {185, {6, 2, 9, 3, S}},
    {182, {6, 2, 3, P}}, //Youshijin
    {287, {4, 1, 2, P}}, //Zakuro
    {188, {4, 1, 2, K}}, //Mawarikomi
    {180, {4, 1, 2, S}}, //Sakura
    {249, {4, 1, 2, H}}, //Ouren
    {142, {6, 3, 1, 4, K}}, //Suzuran
    {142, {6, 2, 4, K}},
    {167, {4, 1, 3, 6, H}}, //Kabari
    {167, {4, 2, 6, H}},
    {252, {4, 1, 2, D}}, //Baku
    {286, {2, 3, 6, D}}, //Triple tatami gaeshi
    {186, {2, 3, 6, 2, 3, 6, S}}, //Tsurane sanzu watashi
    {101, {2, 3, 6, 2, 3, 6, H}} //Garyou tensei (IK)
};

std::vector<CommandActionsAC> faustCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {121, {4, 1, 3, 6, K}}, //Re-re-re no tsuki
    {121, {4, 2, 6, K}},
    {130, {2, 3, 6, P}}, //Holler
    {133, {2, 3, 6, P}}, //Can't hear you
    {134, {2, 3, 6, P}}, //Holler again!
    {286, {2, 3, 6, D}}, //Getting' to the good part
    {287, {2, 3, 6, D}}, //This one's on the house
    {214, {2, 3, 6, S}}, //Souten enshin ranbu
    {165, {-6, 6, -forward, 6}}, //Forwards
    {166, {-4, 4, -back, 4}}, //Backwards
    {221, {2, 3, 6, P}}, //Love
    {221, {2, 3, 9, P}},
    {190, {2, 1, 4, H}}, //Hack'n slash
    {123, {2, 1, 4, P}}, //From the front
    {168, {2, 1, 4, K}}, //From behind
    {125, {2, 1, 4, S}}, //From above
    {96, {2, 3, 6, P}}, //What could this be?
    {222, {2, 3, 6, H}}, //Going my way
    {222, {2, 3, 9, H}},
    {285, {2, 1, 4, D}}, //BAM! outt'a nowhere
    {284, {2, 3, 6, D}}, //Re-re-re no cho-tsuki
    {284, {2, 3, 9, D}},
    {200, {2, 3, 6, 2, 3, 6, P}}, //W-W-What could this be?
    {105, {2, 3, 6, 2, 3, 6, S}}, //Shigekiteki zetsumei ken
    {102, {2, 3, 6, 2, 3, 6, H}} //This week's yamaban (IK)
};

std::vector<CommandActionsAC> testamentCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {136, {2, 3, 6, P}}, //Phantom soul (P)
    {146, {2, 3, 6, K}}, //Phantom soul (K)
    {241, {2, 1, 4, P}}, //Badlands
    {243, {2, 1, 4, P}}, //Badlands (air)
    {243, {2, 1, 7, P}},
    {178, {2, 1, 4, H}}, //Zeinest
    {177, {2, 1, 4, H}}, //Zeinest (air)
    {177, {2, 1, 7, H}},
    {176, {2, 1, 4, S}}, //HITOMI
    {133, {4, 1, 3, 6, S}}, //EXE beast (S)
    {133, {4, 2, 6, S}},
    {137, {4, 1, 3, 6, H}}, //EXE beast (H)
    {137, {4, 2, 6, H}},
    {142, {2, 1, 4, K}}, //Warrant
    {262, {-2, 2, -down, 2, P}}, //Lucht warrant (P)
    {262, {-2, 2, -down, 2, K}}, //Lucht warrant (K)
    {167, {2, 1, 4, D}}, //Grave digger
    {168, {2, 1, 4, D}}, //Grave digger (air)
    {168, {2, 1, 7, D}},
    {261, {6, 3, 1, 4, 6, S}}, //Nightmare circular (S)
    {261, {6, 2, 4, 6, S}},
    {149, {6, 3, 1, 4, 6, H}}, //Nightmare circular (H)
    {149, {6, 2, 4, 6, H}},
    {183, {2, 3, 6, 2, 3, 6, H}}, //Master of puppet
    {174, {2, 3, 6, 2, 3, 6, H}} //Seventh sign (IK)
};

std::vector<CommandActionsAC> jamCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {230, {-2, 2, -down, 2, K}}, //Breath of asanagi (K)
    {113, {-2, 2, -down, 2, S}}, //Breath of asanagi (S)
    {114, {-2, 2, -down, 2, H}}, //Breath of asanagi (H)
    {103, {2, 3, 6, K}}, //Ryujin
    {204, {2, 3, 6, K}}, //Ryujin (air)
    {204, {2, 3, 9, K}},
    {182, {2, 1, 4, K}}, //Gekirin
    {206, {2, 1, 4, K}}, //Gekirin (air)
    {206, {2, 1, 7, K}},
    {189, {6, 2, 3, K}}, //Kenroukaku
    {208, {6, 2, 3, K}}, //Kenroukaku (air)
    {208, {6, 2, 9, 3, K}},
    {97, {2, 3, 6, S}}, //Bakushuu
    {267, {2, 3, 6, P}}, //Choujin
    {269, {2, 3, 6, P}}, //Choujin (air)
    {269, {2, 3, 9, P}},
    {288, {-2, 2, -down, 2, D}}, //Breath of asanagi (fb)
    {238, {6, 3, 1, 4, 6, H}}, //Renhoukyaku
    {238, {6, 2, 4, 6, H}},
    {133, {6, 3, 1, 4, 6, S}}, //Choukyaku hououshou
    {133, {6, 2, 4, 6, S}},
    {120, {2, 3, 6, 2, 3, 6, H}}, //Geki: saishinshou
    {290, {6, 4, 6, 4, 1, 3, 6, P + K}}, //Tousai hyakuretsuken
    {290, {6, 4, 6, 4, 2, 6, P + K}},
    {101, {2, 3, 6, 2, 3, 6, H}} //Gasenkotsu (IK)
    
};

std::vector<CommandActionsAC> anjiCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {186, {2, 3, 6, P}}, //Shitsu
    {188, {2, 3, 6, S}}, //Fuujin (S)
    {175, {2, 3, 6, H}}, //Fuujin (H)
    {144, {2, 1, 4, P}}, //Shin: Nishiki
    {144, {2, 1, 9, P}},
    {170, {6, 2, 3, H}}, //On
    {172, {2, 1, 4, P}}, //Kai (P)
    {102, {2, 1, 4, K}}, //Kai (K)
    {186, {2, 3, 6, D}}, //Shitstu (fb)
    {300, {6, 2, 3, D}}, //On (fb)
    {185, {6, 3, 1, 4, 6, H}}, //Issei Ougi: Sai
    {185, {6, 2, 4, 6, H}},
    {221, {chargeDown40f, up, K}}, //Tenjinkyaku
    {142, {6, 3, 1, 4, S}}, //Kachoufuugetsu
    {142, {6, 2, 4, S}},
    {101, {2, 3, 6, 2, 3, 6, H}} //Zetsu (IK)
};

std::vector<CommandActionsAC> johnnyCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {276, {3, H}}, //3H
    {201, {2, 3, 6, P}}, // Mist stance 
    {201, {2, 3, 6, K}}, // Mist stance 
    {201, {2, 3, 6, S}}, // Mist stance 
    {205, {2, 3, 6, S}}, // lower mist finer 1,2
    {104, {2, 3, 6, S}}, //lower mist finer 3
    {98,  {2, 3, 6, K}}, // middle mist finer 1,2
    {102, {2, 3, 6, K}}, //middle mist finer 3
    {204, {2, 3, 6, P}}, // upper mist finer 1,2
    {100, {2, 3, 6, P}}, //upper mist finer 3
    {234, {-6, 6, -forward, 6}}, // Mist step forward
    {235, {-4, 4, -back, 4}}, // Mist step back
//{ 171 , {H} }, //Mist cancel
    {246, {2, 1, 4, D}}, //Jackhound
    {247, {2, 1, 4, D}}, // stance Jackhound
    {248, {2, 1, 4, D}}, //stance dash Jackhound
    {275, {2, 3, 6, D}}, /// Return Jack
    {179, {2, 3, 6, H}}, // Glitter is Gold
    {316, {2, 1, 4, H}}, //Glitter is Gold (Up)
    {105, {2, 1, 4, P}}, //Bacchus Sigh
    {208, {4, 1, 2, 3,6,H}}, //Ensenga 
    {208, {4, 1, 2, 3,6,9,H}}, //Ensenga (TK)
    {166, {6, 2, 3, S}}, //Divine blade transport
//{168, {S} }, //Transport Divine Blade
    {266, {2, 4, 1, S}}, //Killer Joker Transport
//{268, {S} }, //Transport Killer Joker
    {273, {2, 1, 4, K}}, //Killer Joker (Air)
    {273, {2, 1, 4, 9, K}}, //Killer Joker (Air, TK) 
    {200, {6, 2, 3, K}}, //Divine Blade (Air)
    {200, {6, 2, 3, 9, K}}, //Divine Blade (Air, TK)
    {120, {6, 3, 1, 4, 6, H}}, //That's My Name
    {120, {6, 2, 4, 6, H}},
    {305, {2, 3, 6, 2, 3, 6, H}}, //Uncho's Iai (Air)
    {305, {2, 3, 6, 2, 3, 6, 9, H}}, //Uncho's Iai (Air, TK)
    {124, {2, 3, 6, 2, 3, 6, H}} //Joker Trick (IK)
};

std::vector<CommandActionsAC> venomCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {256, {chargeBack45f, forward, S}}, //Stinger aim (S)
    {258, {chargeBack45f, forward, H}}, //Stinger aim (H)
    {260, {chargeDown40f, up, S}}, //Carcas raid (S)
    {262, {chargeDown40f, up, H}}, //Carcas raid (H)
    {97, {6, 2, 3, S}}, //Double head morbid (S)
    {98, {6, 2, 3, H}}, //Double head morbid (H)
    {187, {6, 2, 3, K}}, //Teleport
    {184, {2, 3, 6, S}}, //Mad struggle (S)
    {184, {2, 3, 9, S}},
    {114, {2, 3, 6, H}}, //Mad struggle (H)
    {114, {2, 3, 9, H}},
    {186, {2, 1, 4, P}}, //Summoning ball (P)
    {101, {2, 1, 4, K}}, //Summoning ball (K)
    {102, {2, 1, 4, S}}, //Summoning ball (S)
    {103, {2, 1, 4, H}}, //Summoning ball (H)
    {235, {2, 1, 4, D}}, //Summoning ball (D)
    {173, {2, 1, 4, P}}, //Aerial summoning ball (P)
    {173, {2, 1, 7, P}},
    {200, {2, 1, 4, K}}, //Aerial summoning ball (K)
    {200, {2, 1, 7, K}},
    {201, {2, 1, 4, S}}, //Aerial summoning ball (S)
    {201, {2, 1, 7, S}},
    {202, {2, 1, 4, H}}, //Aerial summoning ball (H)
    {202, {2, 1, 7, H}},
    {236, {2, 1, 4, D}}, //Aerial summoning ball (D)
    {236, {2, 1, 7, D}},
    {196, {4, 2, 1, P}}, //Dubious curve (P)
    {197, {4, 2, 1, K}}, //Dubious curve (K)
    {198, {4, 2, 1, S}}, //Dubious curve (S)
    {199, {4, 2, 1, H}}, //Dubious curve (H)
    {237, {4, 2, 1, D}}, //Dubious curve (D)
    {258, {chargeBack45f, forward, D}}, //Stinger aim (fb)
    {262, {chargeDown40f, up, D}}, //Carcass raid (fb)
    {98, {6, 2, 3, D}}, //Double head morbid (fb)
    {255, {2, 3, 6, D}}, //Mad struggle (fb)
    {255, {2, 3, 9, D}},
    {112, {2, 1, 4, 1, 3, 6, S}}, //Dark angel
    {112, {2, 1, 4, 2, 6, S}},
    {166, {2, 3, 6, 2, 3, 6, H}}, //Red hail
    {166, {2, 3, 6, 2, 3, 9, H}},
    {289, {6, 3, 1, 4, 6, P}}, //Tactical arch (P)
    {289, {6, 2, 4, 6, P}},
    {290, {6, 3, 1, 4, 6, K}}, //Tactical arch (K)
    {290, {6, 2, 4, 6, K}},
    {291, {6, 3, 1, 4, 6, S}}, //Tactical arch (S)
    {291, {6, 2, 4, 6, S}},
    {292, {6, 3, 1, 4, 6, H}}, //Tactical arch (H)
    {292, {6, 2, 4, 6, H}},
    {293, {6, 3, 1, 4, 6, D}}, //Tactical arch (D)
    {293, {6, 2, 4, 6, D}},
    {108, {2, 3, 6, 2, 3, 6, H}} //Dimmu borgir (IK)
};

std::vector<CommandActionsAC> dizzyCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {167, {2, 3, 6, H}}, //I use this to fish (H)
    {277, {2, 3, 6, D}}, //I use this to fish (D)
    {166, {2, 1, 4, P}}, //My talking buddies (P)
    {148, {2, 1, 4, K}}, //My talking buddies (K)
    {149, {2, 1, 4, S}}, //My talking buddies (S)
    {150, {2, 1, 4, H}}, //My talking buddies (H)
    {278, {2, 1, 4, D}}, //My talking buddies (D)
    {175, {4, 2, 1, S}}, //I use this to get fruit
    {215, {2, 3, 6, K}}, //It started out as just light (K)
    {165, {2, 3, 6, S}}, //It started out as just light (S)
    {153, {2, 1, 4, P}}, //Please leave me alone (P)
    {153, {2, 1, 7, P}},
    {154, {2, 1, 4, K}}, //Please leave me alone (K)
    {154, {2, 1, 7, K}},
    {213, {2, 1, 4, S}}, //Please leave me alone (S)
    {213, {2, 1, 7, S}},
    {214, {2, 1, 4, D}}, //Please leave me alone (fb)
    {214, {2, 1, 7, D}},
    {276, {4, 2, 1, D}}, //I use this to get fruit (fb)
    {174, {6, 3, 1, 4, 6, S}}, //Imperial ray
    {174, {6, 2, 4, 6, S}},
    {97, {6, 3, 1, 4, 6, P}}, //Necro unleashed
    {97, {6, 2, 4, 6, P}},
    {96, {6, 4, 6, 4, 1, 3, 6, H}}, //Gamma ray
    {96, {6, 4, 6, 4, 2, 6, H}},
    {237, {chargeDown, 8, 4, 6, 2, P + H}} //I can't...contain my strength (IK)
};

std::vector<CommandActionsAC> slayerCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {98, {2, 3, 6, P}}, //Mappa hunch (P)
    {102, {2, 3, 6, K}}, //Mappa hunch (K)
    {130, {2, 1, 4, K}}, //Foot loose journey
    {130, {2, 1, 7, K}},
    {161, {2, 1, 4, P}}, //Dandy step (P)
    {162, {2, 1, 4, K}}, //Dandy step (K)
    {262, {2, 1, 4, S}}, //Dandy step (S)
    {298, {2, 1, 4, H}}, //Dandy step (H)
    {107, {6, 3, 1, 4, 6, P}}, //Undertow
    {107, {6, 2, 4, 6, P}},
    {112, {2, 3, 6, H}}, //Blodsucking univierse
    {261, {2, 3, 6, D}}, //Big band upper
    {297, {2, 1, 4, D}}, //Dandy step (fb)
    {165, {6, 3, 1, 4, 6, S}}, //Dead on time
    {165, {6, 2, 4, 6, S}},
    {104, {2, 3, 6, 2, 3, 6, H}}, //Eternal wings
    {163, {2, 1, 4, 2, 1, 4, S}}, //Up and close dandy
    {163, {2, 1, 4, 2, 1, 7, S}},
    {126, {2, 3, 6, 2, 3, 6, H}} //All dead
};

std::vector<CommandActionsAC> inoCommands =
{
    {15, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {98, {4, 1, 3, 6, S}}, //Stroke the big tree (S)
    {98, {4, 2, 6, S}},
    {100, {4, 1, 3, 6, H}}, //Stroke the big tree (H)
    {100, {4, 2, 6, H}},
    {117, {2, 1, 4, P}}, //Antidepressant scale (P)
    {269, {2, 1, 4, H}}, //Antidepressant scale (H)
    {124, {2, 1, 4, P}}, //Antidepressant scale (air, P)
    {124, {2, 1, 7, P}},
    {270, {2, 1, 4, H}}, //Antidepressant scale (air, H)
    {270, {2, 1, 7, H}},
    {101, {2, 3, 6, P}}, //Sultry perrformance (P)
    {101, {2, 3, 9, P}},
    {110, {2, 3, 6, K}}, //Sultry perrformance (K)
    {110, {2, 3, 9, K}},
    {107, {2, 3, 6, S}}, //Sultry perrformance (S)
    {107, {2, 3, 9, S}},
    {104, {2, 3, 6, H}}, //Sultry perrformance (H)
    {104, {2, 3, 9, H}},
    {134, {6, 3, 1, 4, 6, K}}, //Chemical love (horizontal)
    {134, {6, 2, 4, 6, K}},
    {136, {6, 3, 1, 4, 6, K}}, //Chemical love (horizontal, air)
    {136, {6, 2, 4, 6, K}},
    {136, {6, 3, 1, 4, 9, K}},
    {136, {6, 2, 4, 9, K}},
    {241, {6, 3, 1, 4, 6, S}}, //Chemical love (vertical)
    {241, {6, 2, 4, 6, S}},
    {243, {6, 3, 1, 4, 6, S}}, //Chemical love (vertical, air)
    {243, {6, 2, 4, 6, S}},
    {243, {6, 3, 1, 4, 9, S}},
    {243, {6, 2, 4, 9, S}},
    {263, {2, 3, 6, D}}, //Sultry perrformance (fb)
    {263, {2, 3, 9, D}},
    {117, {2, 1, 4, D}}, //Antidepressant scale (fb)
    {124, {2, 1, 4, D}}, //Antidepressant scale (air, fb)
    {124, {2, 1, 7, D}},
    {113, {6, 3, 1, 4, 6, H}}, //Longing desperation
    {113, {6, 2, 4, 6, H}},
    {133, {2, 3, 6, 3, 1, 4, S}}, //Ultimate fortissimo
    {133, {2, 3, 6, 2, 4, S}},
    {133, {2, 3, 6, 3, 1, 7, S}},
    {133, {2, 3, 6, 2, 7, S}},
    {126, {2, 3, 6, 2, 3, 6, H}} //Last will and testament
};

std::vector<CommandActionsAC> zappaCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {129, {2, 3, 6, P}}, //Hello, 3 Centipedes
    {286, {2, 1, 4, D}}, //Etiquette starts here
    {129, {6, 3, 2, 1, 4,}}, //Birth!
    {126, {2, 3, 6, 2, 3, 6, H}}, //I'm Scared (IK)
//Sword commands
    {186, {6, 2, 3, H}}, //Please Fall
    {286, {2, 3, 6, H}}, //Coming Through (Air)
    {286, {2, 3, 9, H}},
    {229, {6, 3, 1, 4, H}}, //Come close, and I'll kill you
    {229, {6, 2, 4, H}},
    {214, {2, 3, 6, S}}, //This Has Gotta Hurt
//Triplets commands
    {294, {2, 3, 6, P}}, //Please Don't Come back (P)
    {251, {2, 3, 6, K}}, //Please Don't Come back (K)
    {204, {2, 3, 6, S}}, //Please Don't Come back (S)
    {227, {2, 3, 6, H}}, //Please Don't Come back (H)
    {253, {2, 3, 6, D}}, //Please Don't Come back (D)
    {295, {2, 3, 6, P}}, //Please Don't Come back (P, air)
    {252, {2, 3, 6, K}}, //Please Don't Come back (K, air)
    {205, {2, 3, 6, S}}, //Please Don't Come back (S, air)
    {228, {2, 3, 6, H}}, //Please Don't Come back (H, air)
    {254, {2, 3, 6, D}}, //Please Don't Come back (D, air)
    {295, {2, 3, 9, P}},
    {252, {2, 3, 9, K}},
    {205, {2, 3, 9, S}},
    {228, {2, 3, 9, H}},
    {254, {2, 3, 9, D}},
    {341, {2, 1, 4, P}},
//Raoh
    {162, {2, 3, 6, S}}, //Darkness Anthem
    {326, {2, 3, 6, S}}, //Darkness Anthem (Air)
    {326, {2, 3, 9, S}},
    {164, {2, 1, 4, S}}, //Last Edguy
    {332, {2, 1, 4, S}}, //Last Edguy (Air)
    {332, {2, 1, 7, S}},
    {165, {6, 3, 1, 4, 6, S}}, //Bestows Malice
    {165, {6, 2, 4, 6, S}}
};

std::vector<CommandActionsAC> bridgetCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
//Bridget commands
    {132, {2, 3, 6, K}}, //Kick Start My Heart
    {131, {6, 2, 3, P}}, //Starship
    {124, {6, 2, 3, P}}, //starship (Air)
    {124, {6, 2, 9, 3, P}},
    {154, {6, 3, 1, 4, 6, S}}, //Loop the Loop
    {154, {6, 2, 4, 6, S}},
    {156, {4, 1, 3, 6, 4, 1, 3, 6, S}}, //Maintenence Disaster
    {156, {4, 1, 3, 6, 4, 2, 6, S}},
    {156, {4, 2, 6, 4, 1, 3, 6, S}},
    {156, {4, 2, 6, 4, 2, 6, S}},
    {155, {6, 3, 1, 4, 6, H}}, //Me and My Killing Machine
    {155, {6, 2, 4, 6, H}},
    {301, {4, 1, 3, 6, 4, H}}, //Me and My Killing Machine (Delay)
    {301, {4, 2, 6, 4, H}},
    {126, {2, 3, 6, 2, 3, 6, H}}, //Shoot the Moon (IK)
//Roger
    {128, {2, 3, 6, H}}, //Roger Rush
    {128, {2, 3, 6, D}}, //Roger Rush (fb)
    {298, {2, 3, 6, H}}, //Roger Rush (air)
    {298, {2, 3, 9, H}},
    {297, {2, 3, 6, D}}, //Roger Rush (fb, air)
    {297, {2, 3, 9, D}},
    {129, {2, 1, 4, H}}, //Jagged Roger
    {129, {2, 1, 4, D}}, //Jagged Roger (fb)
    {203, {2, 1, 4, H}}, //Jagged Roger (air)
    {203, {2, 1, 7, H}},
    {203, {2, 1, 4, D}}, //Jagged Roger (fb, air)
    {203, {2, 1, 7, D}},
    {98, {6, 2, 3, H}}, //Roger Hug
    {130, {2, 1, 4, K}}, //Rolling Movement
    {130, {2, 1, 7, K}},
    {246, {4, 2, 1, H}}, //Roger Get
    {247, {4, 2, 1, H}}, //Roger Get (Air)
    {247, {4, 2, 7, 1, H}}
};

std::vector<CommandActionsAC> robokyCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {186, {2, 3, 6, S}}, //Don't get coc-KY, LV1
    {187, {2, 3, 6, S}}, //Don't get coc-KY, LV2
    {218, {2, 3, 6, S}}, //Don't get coc-KY, LV3
    {229, {2, 1, 4, S}}, //Fun-KY, LV1
    {232, {2, 1, 4, S}}, //Fun-KY, LV2
    {235, {2, 1, 4, S}}, //Fun-KY, LV3
    {97, {6, 2, 3, H}}, //Hun-KY homerun, LV1
    {99, {6, 2, 3, H}}, //Hun-KY homerun, LV2
    {135, {6, 2, 3, H}}, //Hun-KY homerun, LV3
    {349, {6, 2, 3, H}}, //Hun-KY homerun (air), LV1
    {349, {6, 2, 9, 3, H}},
    {351, {6, 2, 3, H}}, //Hun-KY homerun (air), LV2
    {351, {6, 2, 9, 3, H}},
    {353, {6, 2, 3, H}}, //Hun-KY homerun (air), LV2
    {353, {6, 2, 9, 3, H}},
    {181, {2, 3, 6, S}}, //Jun-Ky bargain, LV1
    {181, {2, 3, 9, S}},
    {188, {2, 3, 6, S}}, //Jun-Ky bargain, LV2
    {188, {2, 3, 9, S}},
    {190, {2, 3, 6, S}}, //Jun-Ky bargain, LV3
    {190, {2, 3, 9, S}},
    {102, {-6, 6, -forward, 6, P}}, //Spi-KY (P)
    {102, {-6, 6, -forward, 6, K}}, //Spi-KY (K)
    {102, {-6, 6, -forward, 6, H}}, //Spi-KY (S)
    {102, {-6, 6, -forward, 6, H}}, //Spi-KY (H)
    {223, {6, 3, 1, 4, K}}, //S-KY-line
    {223, {6, 2, 4, K}},
    {272, {2, 1, 4, D}}, //Today's S-KY's beautiful
    {272, {2, 1, 7, D}},
    {170, {2, 3, 6, 2, 3, 6, S}}, //What's useless will always be so
    {252, {2, 3, 6, S}}, //Follow-up head butt
    {197, {2, 3, 6, 2, 3, 6, P}}, //Ris-KY lovers
    {96, {2, 3, 6, 2, 3, 6, H}} //13 luck-KY (IK)
};

std::vector<CommandActionsAC> abaCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {278, {6, 3, 1, 4, H}}, //Bonding (Air)(Normal>Sacrifice)
    {278, {6, 2, 4, H}},
    {278, {6, 3, 1, 7, H}}, //Bonding (Air, TK)(Normal>Sacrifice)
    {278, {6, 2, 7, H}},
    {279, {6, 3, 1, 4, H}}, //Bonding (Air)(Sacrifice>Normal)
    {279, {6, 2, 4, H}},
    {279, {6, 3, 1, 7, H}}, //Bonding (Air, TK)(Sacrifice>Normal)
    {279, {6, 2, 7, H}},
    {274, {6, 3, 1, 4, H}}, //Bonding (Normal>Sacrifice)
    {274, {6, 2, 4, H}},
    {275, {6, 3, 1, 4, H}}, //Bonding (Sacrifice>Normal)
    {275, {6, 2, 4, H}},
    {289, {2, 3, 6, S }}, //Deletion
    {290, {4, 6, S}}, //Ruin
    {291, {6, 3, 1, 4, S}}, //Condemnationn
    {291, {6, 2, 4, S}},
    {292, {2, 3, 6, K}}, //Retribution (normal mode)
    {299, {2, 3, 6, K}}, //Blunder (Sacrifice mode)
    {299, {4, 1, 3, 6, K}}, //Blunder (Sacrifice mode)
    {299, {4, 2, 6, K}},
    {297, {2, 3, 6, H}}, //Avoidance (normal mode)
    {297, {2, 3, 6, D}}, //Avoidance (sacrifice mode)(FB)
    {304, {2, 3, 6, P}}, //Dragging (normal mode)
    {307, {2, 3, 6, D}}, //Dragging (normal mode)(FB)
    {294, {6, 3, 1, 4, P}}, //Injection (normal>sacrifice)
    {294, {6, 2, 4, P}},
    {296, {6, 3, 1, 4, P }}, //Displacement (sacrifice>normal)
    {296, {6, 2, 4, P }},
    {300, {6, 3, 1, 4, 6, H}}, //Evidence: Concealment (normal mode)
    {300, {6, 2, 4, 6, H}},
    {331, {2, 3, 6, H}}, //Evidence: Concealment (normal mode)
    {332, {2, 3, 6, H}}, //Evidence: Concealment (normal mode)
    {302, {6, 3, 1, 4, 6, P}}, //Evidence: Destruction (Departure)
    {302, {6, 2, 4, 6, P}},
    {302, {6, 3, 1, 4, 9, P}},
    {302, {6, 2, 4, 9, P}},
    {303, {2, 1, 4, K}}, //Evidence: Destruction (Arrival)(Sacrifice mode)
    {298, {6, 2, 3, H}}, //Judgement (Sacrifice Mode)
    {298, {6, 2, 3, D}}, //Judgement (Sacrifice mode)(FB)
    {301, {4, 1, 3, 6, S}}, //Eradication (Sacrifice mode)(Air)
    {301, {4, 2, 6, S}},
    {301, {4, 1, 3, 9, S}},
    {301, {4, 2, 9, S}},
    {309, {4, 2, 1, H}}, //Engorgement (Sacrifice mode)    
    {295, {6, 3, 1, 4, 6, 3, 1, 4, S}}, //Altercation (Sacrifice Mode)
    {295, {6, 3, 1, 4, 6, 2, 4, S}},
    {295, {6, 2, 4, 6, 3, 1, 4, S}},
    {295, {6, 2, 4, 6, 2, 4, S}},
    {103, {2, 3, 6, 2, 3, 6, H}}, //Darkness Of Ignorance (IK)
};

std::vector<CommandActionsAC> ordersolCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    //{274 {command special, D}}, //Action Charge 
    //{275 {command special, D}}, //Action Charge (Air)
    {277, {2, 3, 6, P}}, //Block Head Buster level 1 
    {278, {2, 3, 6, P}}, //Block Head Buster level 2
    {279, {2, 3, 6, P}}, //Block Head Buster level 3
    {281, {2, 1, 4, S}}, //Gun Blaze level 1
    {282, {2, 1, 4, S}}, //Gun Blaze level 2
    {283, {2, 1, 4, S}}, //Gun Blaze level 3
    {292, {6, 2, 3, S}}, //Storm Viper level 1
    {293, {6, 2, 3, S}}, //Storm Viper level 2
    {294, {6, 2, 3, S}}, //Storm Viper level 3
    {299, {6, 2, 3, S}}, //Storm Viper level 1(Air)
    {299, {6, 2, 3, 9, S}}, //Storm Viper level 1(Air, TK)
    {300, {6, 2, 3, S}}, //Storm Viper level 2(Air)
    {300, {6, 2, 3, 9, S}}, //Storm Viper level 2(Air, TK)
    {301, {6, 2, 3, S}}, //Storm Viper level 3(Air)
    {301, {6, 2, 3, 9, S}}, //Storm Viper level 3(Air, TK)
    {306, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 1
    {307, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 2
    {308, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 3
    {327, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 1(Air)
    {327, {2, 3, 6, 9, K}}, //Bandit Revolver Prototype Level 1(Air)
    {328, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 2(Air)
    {328, {2, 3, 6, 9, K}}, //Bandit Revolver Prototype Level 2(Air)
    {329, {2, 3, 6, K}}, //Bandit Revolver Prototype Level 3(Air)
    {329, {2, 3, 6, 9, K}}, //Bandit Revolver Prototype Level 3(Air)
    {285, {2, 3, 6, S}}, //Rock It Level 1
    {286, {2, 3, 6, S}}, //Rock It Level 2
    {287, {2, 3, 6, S}}, //Rock It Level 3
    {271, {2, 1, 4, D}}, //Charge
    //{273, {During Charge, H}}, //Charge Burst
    {372, {4, 1, 3, 6, D}}, //Fafnir
    {372, {4, 2, 6, D}},
    {313, {6, 3, 1, 4, 6, H}}, //Tyrant Rave Ver.Omega level 1
    {313, {6, 2, 4, 6, H}},
    {314, {6, 3, 1, 4, 6, H}}, //Tyrant Rave Ver.Omega level 2
    {314, {6, 2, 4, 6, H}},
    {316, {6, 3, 1, 4, 6, H}}, //Tyrant Rave Ver.Omega level 3
    {316, {6, 2, 4, 6, H}},
    {321, {6, 3, 1, 4, P}}, //Savage Fang level 1
    {321, {6, 2, 4, P}},
    {322, {6, 3, 1, 4, P}}, //Savage Fang level 2
    {322, {6, 2, 4, P}},
    {323, {6, 3, 1, 4, P}}, //Savage Fang level 3
    {323, {6, 2, 4, P}},
    {334, {6, 3, 1, 4, 6, S}}, //Dragon Install
    {334, {6, 2, 4, 6, S}},
    //Morbid World
    //{336, {P}}, //
    //{337, {K}}, //
    //{339, {S}}, //
    //{343, {H}}, //
    //{343, {D}}, //
    //{344, {K}}, //
    //{345, {S}}, //
    //Morbid World(Cont..)
    {346, {6, 3, 2, 1, 4, 6, H}}, //
    {349, {6, 3, 2, 1, 4, 6, H}}, //IK version
};

std::vector<CommandActionsAC> kliffCommands =
{
    {14, {-6, 6, -forward, 6}}, //Forward Dash
    {16, {-4, 4, -back, 4}}, //Back dash
    {12, {-6, 6, -forward, 6}}, //Air forward dash
    {12, {-9, 9, -forward, 6}},
    {13, {-4, 4, -back, 4}}, //Air back dash
    {13, {-7, 7, -back, 4}},
    {108, {2, 3, 6, P}}, //Bellowing Roar
    {109, {2, 3, 6, S}}, //Bellowing Roar
    {114, {2, 1, 4, S}}, //Skull Crusher (grounded)
    {246, {2, 1, 4, S}}, //Skull Crusher (Air)
    {246, {2, 1, 4, 9, S}}, //Skull Crusher (Air, TK)
    {111, {2, 1, 4, K}}, //Nape Saddle
    {116, {6, 2, 3, H}}, //Scale Ripper
    {208, {4, 1, 3, 6, S}}, //Limb Severer (Air)
    {208, {4, 2, 6, S}},
    {208, {4, 1, 3, 6, 9, S}}, //Limb Severer (Air, TK)
    {208, {4, 2, 6, 9, S}},
    {119, {2, 1, 4, P}}, //2-Steps Forward    (121 for broke back)
    //{120, {2 - Steps Forward, P}}, //Hellish Charge
    //{253, {2 - Steps Forward, D}}, //Hellish Charge (FB)
    {248, {2, 3, 6, D}}, //Bellowing Roar (FB, Air)
    {248, {2, 3, 6, 9, D}}, //Bellowing Roar (FB, Air, TK)
    {110, {2, 3, 6, 2, 3, 6, H}}, //Reflex Roar
    {118, {6, 3, 1, 4, 6, H}}, //Sole Survivor
    {118, {6, 2, 4, 6, H}},
    {103, {2, 3, 6, 2, 3, 6, H}}, //Pulverizing Dragon's Roar(IK)
};

std::vector<CommandActionsAC> justiceCommands =
{
    {16, {-4, 4, -back, 4}}, //Back dash
    {103, {2, 1, 4, P}}, //Valkyrie Arc
    {245, {4, 1, 3, 6, S}}, //Micheal Sword
    {245, {4, 2, 6, S}},
    {246, {4, 1, 3, 6, S}}, //Micheal Sword (Air)
    {246, {4, 2, 6, S}},
    {246, {4, 1, 3, 6, 9,S}}, //Micheal Sword (Air, TK)
    {246, {4, 2, 6, 9, S}},
    {105, {4, 1, 3, 6, H}}, //Micheal Sword
    {105, {4, 2, 6, H}},
    {247, {4, 1, 3, 6, H}}, //Micheal Sword (Air)
    {247, {4, 2, 6, H}},
    {247, {4, 1, 3, 6, 9, H}}, //Micheal Sword (Air, TK)
    {247, {4, 2, 6, 9, H}},
    {106, {6, 2, 3, K}}, //S.B.T
    {248, {2, 2, P}}, //N.B.
    {249, {2, 2, K}}, //N.B.
    {114, {2, 2, S}}, //N.B.
    {115, {2, 2, H}}, //N.B.
    {250, {2, 2, D}}, //N.B.
    {242, {2, 3, 6, D}}, //Saperia Trance
    {243, {2, 3, 6, D}}, //Saperia Trance(Air)
    {243, {2, 3, 6, 9, D}}, //Saperia Trance(Air, TK)
    {107, {6, 3, 1, 4, 6, S}}, //Imperial Ray
    {107, {6, 2, 4, 6, S}},
    {211, {4, 6, 4, 6, 3, 1, 4, H}}, //Gamma Ray
    {211, {4, 6, 4, 6, 2, 4, H}},
    {116, {6, 3, 1, 4, 6, H}}, //Micheal Blade
    {116, {6, 2, 4, 6, H}},
    {253, {4, 6, 4, 6, 3, 1, 4, S}}, //Omega Shift
    {253, {4, 6, 4, 6, 2, 4, S}}, //Omega Shift
    {109, {2, 3, 6, 2, 3, 6, H}} // X Laser(IK)
};

std::vector<CommandActionsAC> CbrReplayFile::FetchCommandActions(std::string& charName) {
    if (charName == "so") { return solCommands; }
    if (charName == "ky") { return kyCommands; }
    if (charName == "ma") { return mayCommands; }
    if (charName == "mi") { return milliaCommands; }
    if (charName == "ax") { return axlCommands; }
    if (charName == "po") { return potemkinCommands; }
    if (charName == "ch") { return chippCommands; }
    if (charName == "ed") { return eddieCommands; }
    if (charName == "ba") { return baikenCommands; }
    if (charName == "fa") { return faustCommands; }
    if (charName == "te") { return testamentCommands; }
    if (charName == "ja") { return jamCommands; }
    if (charName == "an") { return anjiCommands; }
    if (charName == "jo") { return johnnyCommands; }
    if (charName == "ve") { return venomCommands; }
    if (charName == "di") { return dizzyCommands; }
    if (charName == "sl") { return slayerCommands; }
    if (charName == "in") { return inoCommands; }
    if (charName == "za") { return zappaCommands; }
    if (charName == "br") { return bridgetCommands; }
    if (charName == "ro") { return robokyCommands; }
    if (charName == "ab") { return abaCommands; }
    if (charName == "os") { return ordersolCommands; }
    if (charName == "kl") { return kliffCommands; }
    return justiceCommands;
}