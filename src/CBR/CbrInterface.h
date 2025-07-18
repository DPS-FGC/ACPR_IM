#pragma once
#include <deque>
#include <CBR/AnnotatedReplay.h>
#include <CBR/CbrData.h>
#include <Windows.h>
#include "Game/CharData.h"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <imgui.h>
#include <Overlay/Window/HitboxOverlay.h>
#include <Game/Hitbox/Hitbox.h>
#include <Game/Hitbox/HitboxReader.h>
#include <CBR/CbrUtils.h>

class CbrInterface
{
public:
	CbrInterface();

	std::string debugPrintText = "";

	ImVec2 CbrInterface::GetMinDistance(const CharData* charObj, const CharData* charObjOpp);
	ImVec2 CbrInterface::RotatePoint(ImVec2 center, float angleInRad, ImVec2 point);
	bool CbrInterface::WorldToScreen(LPDIRECT3DDEVICE9 pDevice, D3DXMATRIX* view, D3DXMATRIX* proj, D3DXVECTOR3* pos, D3DXVECTOR3* out);

	void netRequestTest();
	void saveDebugPrint();

	void CbrInterface::OnMatchInit(CharData& c1, CharData& c2, uint8_t gameMode);
	int GetInput() const;
	void SetInput(int);

	void setAnnotatedReplay(AnnotatedReplay, int i);
	AnnotatedReplay* getAnnotatedReplay(int i);
	void CbrInterface::mergeCbrData(CbrData c, int i);
	void setCbrData(CbrData, int i);
	CbrData* getCbrData(int i);
	void CbrInterface::SetPlayerNames(uint8_t gameMode);
	bool CbrInterface::CheckGameOwner(int playerNr);
	std::string CbrInterface::GetPlayerID();
	uintptr_t CbrInterface::GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
	DWORD CbrInterface::GetProcId(const wchar_t* procName);

	int getAutoRecordReplayAmount();
	void clearAutomaticRecordReplays();
	void deleteAutomaticRecordReplays(int deletionAmount);
	void CbrInterface::threadSaveReplay(bool save);
	std::string CbrInterface::threadStatus();

	bool threadCheckSaving();

	std::wstring CbrInterface::makeFilenameCbr(std::string playerName, std::string characterName);
	void CbrInterface::clearThreads();
	void CbrInterface::SaveCbrData(CbrData& cbr);
	void CbrInterface::SaveCbrDataExperiment(CbrData& cbr);
	void CbrInterface::SaveCbrDataThreaded(CbrData& cbr, bool run);
	void CbrInterface::LoadCbrData(std::string playerName, std::string characterName, bool run, int playerNr);
	void CbrInterface::LoadCbrData(std::wstring filename, bool run, int playerNr);
	void CbrInterface::LoadnDeleteCbrDataExec(std::vector<std::wstring>& filename, bool upload, int deleteAmount);
	void CbrInterface::LoadnDeleteCbrData(std::vector<std::wstring>& filename, bool run, bool upload, int deleteAmount);
	CbrData CbrInterface::LoadCbrDataExec(std::wstring filename, int playerNr);
	CbrData CbrInterface::LoadCbrDataNoThread(std::wstring filename);
	CbrData CbrInterface::LoadCbrDataNoThread(std::string playerName, std::string characterName);
	CbrData CbrInterface::LoadCbrDataOld(std::string filename);
	void CbrInterface::MergeCbrDataThreaded(std::wstring filename, bool run, int playerNr);
	void CbrInterface::MergeCbrDataExec(std::wstring filename, int playerNr);
	void CbrInterface::LoadAndUploadData(std::wstring path);
	void CbrInterface::LoadAndUploadDataThreaded(std::wstring path, bool run);
	int CbrInterface::GetNumberOfReplayMenuItems();


	std::string CbrInterface::WriteAiInterfaceState();

	int windowLoadNr = -1;
	bool windowReload = false;

	std::string playerID = "";

	std::string nameP1 = "";
	std::string nameP2 = "";
	char nameVersusP1[128] = u8"";
	char nameVersusP2[128] = u8"";


	int deletionStart = 0;
	int deletionEnd = 0;

	bool cbrSettingsLoaded = false;
	bool autoRecordGameOwner = false;
	bool autoRecordAllOtherPlayers = false;
	bool autoRecordConfirmation = true;
	bool autoRecordActive = false;
	bool autoRecordFinished = false;
	bool autoUploadOwnData = false;
	bool autoRecordSaveCompleted = false;
	char myName[128] = u8"";
	bool autoScanReplayFolder = false;
	bool stopAutoScan = false;
	int currentReplayItem = 0;
	bool currentReplayWatched = false;
	int replayInputCooldown = 0;
	int replayRangeStart = 0;
	int replayRangeEnd = 0;
	std::array <std::string, 2> autoRecordSaveCompletedName = { "","" };
	std::array <std::string, 2> autoRecordSaveCompletedChar = { "","" };
	std::array <int, 2>  autoRecordDeletionAmount = { 0,0 };

	int resetDepth = -1;
	int resetPlayer = -1;
	std::unordered_map<int, bool> executionOrder = {};
	std::array <int, 2> readDepth = { 99,99 };
	std::array <int, 2> writeDepth = { -1,-1 };
	std::array <int, 2> inputMemory = { 5,5 };
	std::array <int, 2> inputMemoryHirarchy = { -1,-1 };
	std::array <int, 2> controllerMemory = { -1,-1 };
	std::array <int, 2> writeMemory = { -1,-1 };
	std::array <std::shared_ptr<Metadata>, 2> netplayMemory = { };

	//int cbrTriggeredThisFrame = 99;
	//int cbrTriggeredThisFrameP2 = 99;

	bool Recording = false;
	bool Replaying = false;
	bool RecordingP2 = false;
	bool ReplayingP2 = false;
	bool instantLearning = false;
	bool instantLearningP2 = false;

	bool firstInputParser = false;
	int debugReplayNr = 0;
	std::deque<char> inputs;
	int input;
	int inputP2;
	std::array<int, 2> debugErrorCounter = { 0,0 };
	int pMatchState = 0;
	int lastRecordedRound = 0;
	int debugNr = -1;
	std::string cbrCreationDebugStr = "";

	void addReversalReplay(AnnotatedReplay);
	void deleteReversalReplays();
	void deleteReversalReplay(int);
	AnnotatedReplay* getReversalReplay(int);
	int CbrInterface::randomReversalReplaySelection();
	void CbrInterface::disableAllReversalReplays();
	int CbrInterface::playBackActiveReversalReplay(bool facing);
	bool CbrInterface::isAnyPlayingReversalReplays();
	void CbrInterface::ReplayActivation(int i);

	char playerName[128] = "";
	char reversalName[128] = "";
	char weightName[128] = "";
	void CbrInterface::SaveReversal(AnnotatedReplay& rev);
	AnnotatedReplay CbrInterface::LoadReversal(std::string filename);
	void CbrInterface::SaveWeights(costWeights cst);
	costWeights CbrInterface::LoadWeights(std::string in);

	bool reversalRecording;
	bool reversalRecordingActive;
	bool reversalActive;
	int reversalReplayNr = 0;
	bool blockStanding = false;
	bool blockCrouching = false;
	bool barrierReversal = false;
	int reversalInput = 5;
	int reversalReplaySelected = -1;

	std::vector<int> reversalReplaysTriggerOdds;
	std::vector<int> reversalBuffer;

	void CbrInterface::saveSettings();
	void CbrInterface::loadSettings(CbrInterface* cbrI);
	void CbrInterface::saveDebug();
	void CbrInterface::saveStructureDebug(std::string text);

	void CbrInterface::saveReplayDataInMenu();
	void EndCbrActivities();
	void CbrInterface::StartCbrRecording(char* p1charName, char* p2charName, int p1charId, int p2charId, int recordingSlot);
	void CbrInterface::StartCbrInstantLearning(char* p1charName, char* p2charName, int p1charId, int p2charId, int recordingSlot);
	void CbrInterface::RestartCbrActivities(char* p1charName, char* p2charName, int p1charId, int p2charId);
	void CbrInterface::resetCbrInterface();

	void EndCbrActivities(int playerNr, bool trim = false);

	void CbrInterface::UpdateOldCbrMetadata();

	void CbrInterface::Checkinputs();
	int debugPrint1;
	int debugPrint2;
	int debugPrint3;
	int debugPrint4;
	HANDLE hProcess = 0;
	uintptr_t moduleBase = 0;
	uintptr_t input5Ptr = 0;

	std::vector<AnnotatedReplay> recordBufferP1{};
	std::vector<AnnotatedReplay> recordBufferP2{};
	std::vector<AnnotatedReplay> recordBufferP1Backup{};
	std::vector<AnnotatedReplay> recordBufferP2Backup{};

	bool netaRecording = false;
	bool netaPlaying = false;
	bool netaOpen = false;
	int netaReplayCounter = 0;
	
	void CbrInterface::Testbase64(std::string cbr);

	int CbrInterface::determineBufferedInput(std::array<FixedQueue<int, 5>, 2>& inputBuffer);

	bool CbrInterface::threadActiveCheck();

	template<class Archive>
	void serialize(Archive& a, const unsigned version) {
		a& autoRecordGameOwner& autoRecordAllOtherPlayers;
	}
private:
	
	std::array<AnnotatedReplay,2> aReplay;
	std::array < CbrData,2> cbrData;
	std::vector<AnnotatedReplay> reversalReplays;
	
	
	bool threadActive = false;
	bool processingCBRdata = false;
	boost::thread processingStoredRecordingsThread;

	void CbrInterface::mergeCbrReplayBuffer(bool checkSkip);
};
