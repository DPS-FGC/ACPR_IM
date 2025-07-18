#pragma once
#include "CBR/Metadata.h"
#include "CBR/CbrUtils.h"
#include "Core/interfaces.h"


bool placeHooks_cbr();

void InactiveMetaDataUpdate(bool PlayerIndex, int input);
std::shared_ptr<Metadata> RecordCbrMetaData(bool, int);

char ReplayCbrData();

int GGtoBBInput(uint16* input_gg, int playerNr);
uint16 BBtoGGInput(int input_bb, int playerNr);
bool CheckRoundStart(int round);
bool CheckRoundEnd(int round);
void CBRLogic(uint16* input, int hirarchy, bool readOnly, bool writeOnly, int playerNr, bool netplayMemory = false);
bool CBRLogic(int* input, int playerNr, int controllerNr, bool readOnly, bool writeOnly, bool netplayMemory = false);
bool CBRLogic(int* input, int hirarchy, int playerNr, int controllerNr, bool readOnly, bool writeOnly, bool netplayMemory = false);
void UpdateFrameMeter(int playerNr);
void reversalLogic(char* addr, int input, std::shared_ptr<Metadata> meta, int playerNR);
int reversalLogic(int input, int playerNR, bool readOnly, bool writeOnly);
void RecordCbrHelperData(std::shared_ptr<Metadata> me, bool PlayerIndex);
bool netaLogic(int* input, std::shared_ptr<Metadata> meta, int playerNR, bool readOnly, bool writeOnl);