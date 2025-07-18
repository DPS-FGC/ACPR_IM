#include <fstream>
#include "CbrInterface.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>
#include <TlHelp32.h>
#include<ctype.h>
#include <thread>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include "CharacterStorage.h"
#include <boost/algorithm/string.hpp>
#include <Core/interfaces.h>
#include "Core/utils.h"


#define DebugPrint false
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>





void CbrInterface::saveDebugPrint() {
	if (!DebugPrint) { return; }
	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	{
		auto filename = ".\\CBRsave\\debugPrintText.ini";
		std::ofstream outfile(filename);
		boost::archive::text_oarchive archive(outfile);
		archive << debugPrintText;
	}
}


int CbrInterface::GetInput() const
{
	return input;
}

void CbrInterface::SetInput(int i)
{
	input = i;
}

void CbrInterface::setAnnotatedReplay(AnnotatedReplay r, int i) {
	aReplay[i] = r;
}
AnnotatedReplay* CbrInterface::getAnnotatedReplay(int i) {
	return &aReplay[i];
}

void CbrInterface::setCbrData(CbrData c, int i) {
	cbrData[i] = c;
}
void CbrInterface::mergeCbrData(CbrData c, int i) {
	auto vec = c.getReplayFiles();
	auto vec2 = cbrData[i].getReplayFiles();
	int start = vec2->size();;
	int end = start + vec->size();
	
	if (vec2->size() > 0) {
		vec2->insert(
			vec2->end(),
			vec->begin(),
			vec->end()
		);
	}
	else {
		vec->insert(
			vec->end(),
			vec2->begin(),
			vec2->end()
		);

		cbrData[i] = c;
	}
	cbrData[i].generateTreeFromOldReplayInRange(start, end);	
}

CbrData* CbrInterface::getCbrData(int i) {
	return &cbrData[i];
}

void CbrInterface::addReversalReplay(AnnotatedReplay ar) {
	reversalReplays.push_back(ar);
}
void CbrInterface::deleteReversalReplays() {
	reversalReplays.clear();
}
void CbrInterface::deleteReversalReplay(int i) {
	reversalReplays.erase(reversalReplays.begin() + i);
}
AnnotatedReplay* CbrInterface::getReversalReplay(int i) {
	return &reversalReplays[i];
}
/* 
#define D 128
#define C 64
#define B 32
#define A 16
#define forward 1100
#define back 1101
#define up 1102
#define down 1103
std::vector<CommandActions> generalCommands =
{
	{ "623", {6,-forward}},
	{ "236", {2,3}},
	{ "41236", {4,1}},
	{ "41236", {4,1}},
};
int CbrInterface::determineBufferedInput(std::array<FixedQueue<int, 5>, 2> inputBuffer) {

}
*/

int CbrInterface::randomReversalReplaySelection() {
	int combinedPropability = 0;
	for (int i = 0; i < reversalReplaysTriggerOdds.size(); i++) {
		reversalReplays[i].setPlaying(false);
		combinedPropability += reversalReplaysTriggerOdds[i];
	}
	float multi = static_cast<float>(100) / combinedPropability;
	//std::vector<float> probContainer;
	float probSave = 0;
	float rand = RandomFloat(0, 100);
	for (int i = 0; i < reversalReplaysTriggerOdds.size(); i++) {
		float max = reversalReplaysTriggerOdds[i] * multi;
		auto test = max + probSave;
		probSave += max;
		if (test > rand) {
			//reversalReplays[i].setPlaying(true);
			//reversalReplays[i].resetReplayIndex();
			return i;
			break;
		}
	}
	return -1;
}
void CbrInterface::ReplayActivation(int i) {
	reversalReplays[i].setPlaying(true);
	reversalReplays[i].resetReplayIndex();
}

void CbrInterface::disableAllReversalReplays() {
	for (int i = 0; i < reversalReplaysTriggerOdds.size(); i++) {
		reversalReplays[i].setPlaying(false);
		reversalReplays[i].resetReplayIndex();
	}
}
int CbrInterface::playBackActiveReversalReplay(bool facing) {
	for (int i = 0; i < reversalReplaysTriggerOdds.size(); i++) {
		if (reversalReplays[i].getPlaying()) {
			return reversalReplays[i].getNextInput(facing);
		}
	}
}
bool CbrInterface::isAnyPlayingReversalReplays() {
	for (int i = 0; i < reversalReplaysTriggerOdds.size(); i++) {
		if (reversalReplays[i].getPlaying()) {
			return true;
		}
	}
	return false;
}
CbrInterface::CbrInterface()
{
	//loadSettings(this);
	FILE* leakReportFile = fopen("leak_report.txt", "w");
	if (!leakReportFile) {
		// Handle error if file opening fails
		perror("Error opening file for output");
	}
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// Redirect the memory-leak report to the file
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, leakReportFile);
}
void CbrInterface::OnMatchInit(CharData& c1, CharData& c2, uint8_t gameMode) {
	loadSettings(this);
	autoRecordActive = false; 
	nameP1 = "";
	nameP2 = "";
	if (gameMode == GameMode_ReplayTheater)
	{
		SetPlayerNames(gameMode);

		if (autoRecordGameOwner || autoRecordAllOtherPlayers)
		{
			if (nameVersusP1[0] != '\0')
				nameP1 = std::string(nameVersusP1);
			if (nameVersusP2[0] != '\0')
				nameP2 = std::string(nameVersusP2);

			autoRecordActive = true;
			setAnnotatedReplay(AnnotatedReplay(nameP1, g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), c1.charIndex, c2.charIndex), 0);
			setAnnotatedReplay(AnnotatedReplay(nameP2, g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), c2.charIndex, c1.charIndex), 1);
		}
	}
	if (gameMode == GameMode_Online)
	{
		autoScanReplayFolder = false;
		SetPlayerNames(gameMode);
		if ((autoRecordGameOwner || autoRecordAllOtherPlayers) && nameP1 != "" && nameP2 != "") {
			autoRecordActive = true;
			setAnnotatedReplay(AnnotatedReplay(nameP1, g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), c1.charIndex, c2.charIndex), 0);
			setAnnotatedReplay(AnnotatedReplay(nameP2, g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), c2.charIndex, c1.charIndex), 1);
		}
	}
	if (gameMode == GameMode_Versus) {//gamemodeVersus
		if ((autoRecordGameOwner || autoRecordAllOtherPlayers)) {
			autoRecordActive = true;
			setAnnotatedReplay(AnnotatedReplay(nameVersusP1, g_interfaces.Player1.GetCharAbbr(), g_interfaces.Player2.GetCharAbbr(), c1.charIndex, c2.charIndex), 0);
			setAnnotatedReplay(AnnotatedReplay(nameVersusP2, g_interfaces.Player2.GetCharAbbr(), g_interfaces.Player1.GetCharAbbr(), c2.charIndex, c1.charIndex), 1);
		}
	}
	
}

bool SaveDataComplete = false;
void CbrInterface::SaveCbrData(CbrData& cbr) {

	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	
	auto filenameReal = u8".\\CBRsave\\" + cbr.getCharName() + cbr.getPlayerName() + ".cbr";
	auto filename = u8".\\CBRsave\\SaveBuffer.cbr";
	

	auto opponentChar = getOpponentCharNameString(cbr);

	 {
		std::ofstream outfile(filename, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		std::string charName = cbr.getCharName();
		std::string playerName = cbr.getPlayerName();
		//std::string opponentChar = "all";
		int rCount = cbr.getReplayCount();
		archive << charName;
		archive << playerName;
		archive << opponentChar;
		archive << rCount;
		archive << cbr;
		
	}
	
	 auto filenameReal2 = "\\CBRsave\\" + cbr.getCharName() + cbr.getPlayerName() + ".cbr";
	 auto filename2 = "\\CBRsave\\SaveBuffer.cbr";
	 auto p = boost::filesystem::current_path() / filename2;
	 auto pReal = boost::filesystem::current_path() / filenameReal2;
	 
	 if (boost::filesystem::exists(pReal)) {
		 boost::filesystem::remove(pReal);
	 }
	 if (boost::filesystem::exists(p)) {
		 try
		 {
			 boost::filesystem::rename(p, pReal);
		 }
		 catch (const std::exception& e)
		 {
			 std::string s = "reversalName";
		 }
		 
	 }
	 SaveDataComplete = true;
	 
}


void SaveDataWithMetadata(const std::string& filename, const std::string& metadata, const std::string& compressedData) {
	std::ofstream outfile(filename, std::ios_base::binary);

	if (!outfile.is_open()) {
		std::cerr << "Error: Could not open the file.\n";
		return;
	}

	// Write metadata to the file
	outfile.write(metadata.c_str(), metadata.size());


	// Create a filtering stream for compression
	boost::iostreams::filtering_stream<boost::iostreams::output> f;
	f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
	f.push(outfile);

	// Write compressed data to the file
	f.write(compressedData.c_str(), compressedData.size());
}



void CbrInterface::SaveCbrDataExperiment(CbrData& cbr) {

	boost::filesystem::path dir(L"CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}

	boost::filesystem::path dir2(L"CBRsave\\Metadata");
	if (!(boost::filesystem::exists(dir2))) {
		boost::filesystem::create_directory(dir2);
	}

	//auto filenameReal = L".\\CBRsave\\" + 
	//	sanitize_filename(utf8_to_utf16(cbr.getCharName() + cbr.getPlayerName())) +
	//	L".cbr";
	auto filename = L".\\CBRsave\\SaveBuffer.cbr";
	auto filenameMeta = L".\\CBRsave\\Metadata\\SaveBuffer.met";

	// Serialize metadata to a string
	std::ostringstream metadataStream;
	{
		FileMetadata meta;
		auto opponentChar = getOpponentCharNameString(cbr);
		std::string charName = cbr.getCharName();
		std::string playerName = cbr.getPlayerName();
		int rCount = cbr.getReplayCount();
		meta.rCount = rCount;
		strcpy(meta.charName, charName.c_str());
		strcpy(meta.playerName, playerName.c_str());
		strcpy(meta.opponentChar, opponentChar.c_str());
		meta.rCount = rCount;

		std::ofstream outfile(filenameMeta, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << meta;
	}

	{
		std::ofstream outfile(filename, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << cbr;


	}
	auto filenameReal2Meta = L"\\CBRsave\\Metadata\\" + 
		sanitize_filename(utf8_to_utf16(cbr.getCharName() + cbr.getPlayerName())) + L".met";
	auto filenameReal2 = L"\\CBRsave\\" + 
		sanitize_filename(utf8_to_utf16(cbr.getCharName() + cbr.getPlayerName())) + L".cbr";
	auto filename2 = L"\\CBRsave\\SaveBuffer.cbr";
	auto filename2Meta = L"\\CBRsave\\Metadata\\SaveBuffer.met";
	auto p = boost::filesystem::current_path() / filename2;
	auto pReal = boost::filesystem::current_path() / filenameReal2;
	auto pMeta = boost::filesystem::current_path() / filename2Meta;
	auto pMetaReal = boost::filesystem::current_path() / filenameReal2Meta;

	if (boost::filesystem::exists(pReal)) {
		boost::filesystem::remove(pReal);

	}
	if (boost::filesystem::exists(pMetaReal)) {
		boost::filesystem::remove(pMetaReal);
	}
	if (boost::filesystem::exists(p)) {
		try
		{
			boost::filesystem::rename(p, pReal);

		}
		catch (const std::exception& e)
		{
			std::string s = "reversalName";
		}

	}
	if (boost::filesystem::exists(pMeta)) {
		try
		{
			boost::filesystem::rename(pMeta, pMetaReal);

		}
		catch (const std::exception& e)
		{
			std::string s = "reversalName";
		}

	}
	SaveDataComplete = true;

}
/*
void CbrInterface::SaveCbrDataExperiment(CbrData& cbr) {

	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}

	auto filenameReal = u8".\\CBRsave\\" + cbr.getCharName() + cbr.getPlayerName() + ".cbr";
	auto filename = u8".\\CBRsave\\SaveBuffer.cbr";

	// Serialize metadata to a string
	std::ostringstream metadataStream;
	{
		FileMetadata meta;
		auto opponentChar = getOpponentCharNameString(cbr);
		std::string charName = cbr.getCharName();
		std::string playerName = cbr.getPlayerName();
		int rCount = cbr.getReplayCount();
		meta.rCount = rCount;
		strcpy(meta.charName, charName.c_str());
		strcpy(meta.playerName, playerName.c_str());
		strcpy(meta.opponentChar, opponentChar.c_str());
		meta.rCount = rCount;

		boost::archive::text_oarchive metaArchive(metadataStream);
		metaArchive << meta;
	}

	{
		std::ofstream outfile(filename, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << cbr;

	}


	// Serialize compressed data to a string
	std::ostringstream compressedDataStream;
	{
		boost::archive::binary_oarchive dataArchive(compressedDataStream);
		dataArchive << cbr;
	}

	// Save metadata and compressed data to the file
	SaveDataWithMetadata(filename, metadataStream.str(), compressedDataStream.str());



	auto filenameReal2 = "\\CBRsave\\" + cbr.getCharName() + cbr.getPlayerName() + ".cbr";
	auto filename2 = "\\CBRsave\\SaveBuffer.cbr";
	auto p = boost::filesystem::current_path() / filename2;
	auto pReal = boost::filesystem::current_path() / filenameReal2;

	if (boost::filesystem::exists(pReal)) {
		boost::filesystem::remove(pReal);
	}
	if (boost::filesystem::exists(p)) {
		try
		{
			boost::filesystem::rename(p, pReal);
		}
		catch (const std::exception& e)
		{
			std::string s = "reversalName";
		}

	}
	SaveDataComplete = true;

}*/

boost::thread SaveDataThread;
void CbrInterface::SaveCbrDataThreaded(CbrData& cbr, bool run) {
	if (run && threadActive == false) {
		threadActive = true;
		SaveDataComplete = false;
		SaveDataThread = boost::thread(&CbrInterface::SaveCbrDataExperiment, this, cbr);
	}
	if (SaveDataComplete == true && threadActive == true && SaveDataThread.joinable()) {
		threadActive = false;
		SaveDataThread.join();
		SaveDataComplete = false;
	}
	return;
}

void CbrInterface::SaveReversal(AnnotatedReplay& rev) {

	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	std::string s = reversalName;
	auto filename = u8".\\CBRsave\\" + s + ".rev";
	std::ofstream outfile(filename, std::ios_base::binary);
	{
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << rev;
	}
}

AnnotatedReplay CbrInterface::LoadReversal(std::string in) {
	
	auto filename = u8".\\CBRsave\\" + in + ".rev";
	std::ifstream infile(filename, std::ios_base::binary);
	AnnotatedReplay insert;
	insert.debugFrameIndex = -1;
	if (infile.fail()) {
		//File does not exist code here
	}
	else {
		//std::string b = "";
		//infile >> b;
		
		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile);
		boost::archive::binary_iarchive archive(f);
		AnnotatedReplay insert;
		archive >> insert;
		insert.debugFrameIndex = 0;
		return insert;
	}
	return insert;
}

void CbrInterface::SaveWeights(costWeights cst) {

	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	std::string s = weightName;
	auto filename = u8".\\CBRsave\\" + s + ".cst";
	std::ofstream outfile(filename, std::ios_base::binary);
	{
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << cst;
	}
}
costWeights CbrInterface::LoadWeights(std::string in) {

	auto filename = u8".\\CBRsave\\" + in + ".cst";
	std::ifstream infile(filename, std::ios_base::binary);
	costWeights insert;
	insert.name[0] = "-1";
	if (infile.fail()) {
		//File does not exist code here
	}
	else {
		//std::string b = "";
		//infile >> b;

		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile);
		boost::archive::binary_iarchive archive(f);
		costWeights insert;
		archive >> insert;
		return insert;
	}
	return insert;
}
void CbrInterface::Testbase64(std::string cbr) {

	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	auto filename = ".\\CBRsave\\binaryZipString.txt";
	std::ofstream outfile(filename, std::ios_base::binary);
	{
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << cbr;
	}

	filename = ".\\CBRsave\\binaryString.txt";
	std::ofstream outfile2(filename, std::ios_base::binary);
	{
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		//f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile2);
		boost::archive::binary_oarchive archive(f);
		archive << cbr;
	}

	{
		std::stringstream outfile3;
		{
			boost::iostreams::filtering_stream<boost::iostreams::output> f;
			//f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
			f.push(outfile3);
			boost::archive::binary_oarchive archive(f);
			archive << cbr;
		}
		std::string s = outfile3.str();
		auto en64 = encode64(s);
		auto d = decode64(en64);


		filename = ".\\CBRsave\\encoded64String.txt";
		std::ofstream outfile4(filename, std::ios_base::binary);
		outfile4 << en64;

		filename = ".\\CBRsave\\decoded64String.txt";
		std::ofstream outfile5(filename, std::ios_base::binary);
		outfile5 << d;

		filename = ".\\CBRsave\\preEncode64String.txt";
		std::ofstream outfile6(filename, std::ios_base::binary);
		outfile6 << s;

	}
	{
		std::stringstream outfile3;
		{
			boost::iostreams::filtering_stream<boost::iostreams::output> f;
			f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
			f.push(outfile3);
			boost::archive::binary_oarchive archive(f);
			archive << cbr;
		}
		std::string s = outfile3.str();
		auto en64 = encode64(s);
		auto d = decode64(en64);

		if (s != d) {
			filename = ".\\CBRsave\\compressedEncoded64String.txt";
			std::ofstream outfile4(filename, std::ios_base::binary);
			outfile4 << en64;

			filename = ".\\CBRsave\\compressedDecoded64String.txt";
			std::ofstream outfile5(filename, std::ios_base::binary);
			outfile5 << d;

			filename = ".\\CBRsave\\preCompressedEncode64String.txt";
			std::ofstream outfile6(filename, std::ios_base::binary);
			outfile6 << s;

			filename = ".\\CBRsave\\inputString.txt";
			std::ofstream outfile7(filename);
			outfile4 << cbr;
		}
	}
}
bool LoadCbrComplete = false;
CbrData CbrInterface::LoadCbrDataExec(std::wstring filename, int playerNr) {
	auto insert = LoadCbrDataNoThread(filename);
	insert.generateTreeFromOldReplay();
	setCbrData(insert, playerNr);
	//pLoadCbrData.set_value(true);
	LoadCbrComplete = true;
	return insert;
}

//TODO: figure out why character and player name isent saved.
bool LoadNDeleteCbrDataComplete = false;
void CbrInterface::LoadnDeleteCbrDataExec(std::vector<std::wstring>& filename, bool upload, int deleteAmount) {
	for (int i = 0; i < filename.size(); i++) {
		auto insert = LoadCbrDataNoThread(filename[i]);
		if (insert.getPlayerName() == "-1") {
			LoadNDeleteCbrDataComplete = true;
			return;
		}
		insert.generateTreeFromOldReplay();
		//setCbrData(insert, playerNr);
		//pLoadCbrData.set_value(true);
		insert.deleteReplays(insert.getReplayCount()-1-deleteAmount, insert.getReplayCount()-1);
		SaveCbrDataExperiment(insert);
		/*if (upload) {
			auto j2 = convertCBRtoJson(insert, playerID);
			CbrHTTPPostNoThreat(j2);
		}*/
		
	}
	LoadNDeleteCbrDataComplete = true;
}

CbrData CbrInterface::LoadCbrDataNoThread(std::string playerName, std::string characterName) {
	std::wstring filename = makeFilenameCbr(playerName, characterName);
	return LoadCbrDataNoThread(filename);
}

CbrData CbrInterface::LoadCbrDataNoThread(std::wstring filename) {
	CbrData insert;
	FileMetadata meta;
	insert.setPlayerName("-1");
	auto filenameMeta = filename.substr(0, filename.size() - 3) + L"met";

	std::ifstream infile(filename, std::ios_base::binary);
	if (infile.fail()) {
		std::cerr << "Error: Could not open the file.\n";
		return insert;
	}

	// Read metadata
	try
	{

		//std::ifstream infile2(filenameMeta, std::ios_base::binary);
		//boost::iostreams::filtering_stream<boost::iostreams::input> f;
		//f.push(boost::iostreams::gzip_decompressor());
		//f.push(infile2);
		//boost::archive::binary_iarchive archive(f);
		//archive >> meta;

		boost::iostreams::filtering_stream<boost::iostreams::input> f2;
		f2.push(boost::iostreams::gzip_decompressor());
		f2.push(infile);
		boost::archive::binary_iarchive archive2(f2);
		CbrData insert;
		archive2 >> insert;
		return insert;
	}
	catch (const std::exception&)
	{
		std::ifstream infile2(filename, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile2);
		boost::archive::binary_iarchive archive(f);
		CbrData insert;
		std::string charName = "";
		std::string playerName = "";
		std::string opponentChar = "";
		int rCount = 0;
		archive >> charName;
		archive >> playerName;
		archive >> opponentChar;
		archive >> rCount;
		archive >> insert;
		return insert;
	}
	

	return insert;
}

/*CbrData CbrInterface::LoadCbrDataNoThread(std::string filename) {
	CbrData insert;
	insert.setPlayerName("-1");

	std::ifstream infile(filename, std::ios_base::binary);
	if (infile.fail()) {
		std::cerr << "Error: Could not open the file.\n";
		return insert;
	}

	// Read metadata
	std::string metadata;
	{
		char ch;
		// Read metadata
		while (infile.get(ch) && ch != '\n') {
			metadata += ch;
		}
	}

	// Create a filtering stream for decompression
	boost::iostreams::filtering_istream f;
	f.push(boost::iostreams::gzip_decompressor());
	f.push(infile);

	// Read compressed data
	std::string compressedData;
	char buffer[4096];  // Adjust buffer size as needed
	while (f) {
		f.read(buffer, sizeof(buffer));
		compressedData.append(buffer, f.gcount());
	}
	try
	{
		// Deserialize metadata
		std::istringstream metadataStream(metadata);
		FileMetadata meta;
		boost::archive::text_iarchive metaArchive(metadataStream);
		metaArchive >> meta;

		// Deserialize compressed data
		std::istringstream compressedDataStream(compressedData);
		{
			boost::archive::binary_iarchive archive(compressedDataStream);
			archive >> insert;
		}
	}
	catch (const std::exception&)
	{
		std::ifstream infile2(filename, std::ios_base::binary);
		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile2);
		boost::archive::binary_iarchive archive(f);
		CbrData insert;
		std::string charName = "";
		std::string playerName = "";
		std::string opponentChar = "";
		int rCount = 0;
		archive >> charName;
		archive >> playerName;
		archive >> opponentChar;
		archive >> rCount;
		archive >> insert;
		return insert;
	}
	

	return insert;
}*/

std::wstring CbrInterface::makeFilenameCbr(std::string playerName, std::string characterName) {
	std::wstring filename = L"CBRsave\\";
	filename = filename +  sanitize_filename(utf8_to_utf16(characterName + playerName)) + L".cbr";
	return filename;
}


void CbrInterface::LoadCbrData(std::string playerName, std::string characterName, bool run, int playerNr) {
	std::wstring filename = makeFilenameCbr(playerName, characterName);
	LoadCbrData(filename, run, playerNr);
}

boost::thread LoadCbrDataThread;
void CbrInterface::LoadCbrData(std::wstring filename, bool run, int playerNr) {
	
	if (run && threadActive == false) {
		threadActive = true;
		LoadCbrComplete = false;
		LoadCbrDataThread = boost::thread(&CbrInterface::LoadCbrDataExec, this, filename, playerNr);
	}
	if (LoadCbrComplete == true && threadActive == true && LoadCbrDataThread.joinable()) {
		threadActive = false;
		LoadCbrDataThread.join();
		LoadCbrComplete = false;
		
	}
	return;
}


boost::thread LoadNDeleteCbrDataThread;
void CbrInterface::LoadnDeleteCbrData(std::vector<std::wstring>& filename, bool run, bool upload, int deleteAmount) {

	if (run && threadActive == false) {
		threadActive = true;
		LoadNDeleteCbrDataComplete = false;
		LoadNDeleteCbrDataThread = boost::thread(&CbrInterface::LoadnDeleteCbrDataExec, this, filename, upload, deleteAmount);
	}
	if (LoadNDeleteCbrDataComplete == true && threadActive == true && LoadNDeleteCbrDataThread.joinable()) {
		threadActive = false;
		LoadNDeleteCbrDataThread.join();
		LoadNDeleteCbrDataComplete = false;

	}
	return;
}


bool MergeCbrComplete = false;
void CbrInterface::MergeCbrDataExec(std::wstring filename, int playerNr) {
	auto insert = LoadCbrDataNoThread(filename);
	mergeCbrData(insert, playerNr);
	MergeCbrComplete = true;
	return;
}


boost::thread MergeCbrDataThread;
void CbrInterface::MergeCbrDataThreaded(std::wstring filename, bool run, int playerNr) {

	if (run && threadActive == false) {
		threadActive = true;
		MergeCbrComplete = false;
		MergeCbrDataThread = boost::thread(&CbrInterface::MergeCbrDataExec, this, filename, playerNr);
	}
	if (MergeCbrComplete == true && threadActive == true && MergeCbrDataThread.joinable()) {
		threadActive = false;
		LoadCbrDataThread.join();
		MergeCbrComplete = false;
	}
	return;
}

bool LoadAndUploadComplete = false;
boost::thread LoadAndUploadDataThread;
void CbrInterface::LoadAndUploadData(std::wstring path) {
	auto test = LoadCbrDataNoThread(path);
	//auto j2 = convertCBRtoJson(test, playerID);
	//CbrHTTPPostNoThreat(j2);
	LoadAndUploadComplete = true;
}
void CbrInterface::LoadAndUploadDataThreaded(std::wstring path, bool run) {
	if (run && threadActive == false) {
		threadActive = true;
		LoadAndUploadComplete = false;
		LoadAndUploadDataThread = boost::thread(&CbrInterface::LoadAndUploadData, this, path);
	}
	if (LoadAndUploadComplete == true && threadActive == true && LoadAndUploadDataThread.joinable()) {
		threadActive = false;
		LoadAndUploadDataThread.join();
		LoadAndUploadComplete = false;
	}
	return;
}

CbrData CbrInterface::LoadCbrDataOld(std::string filename) {
	std::ifstream infile(filename, std::ios_base::binary);
	CbrData insert;
	insert.setPlayerName("-1");
	if (infile.fail()) {
		//File does not exist code here
	}
	else {
		//std::string b = "";
		//infile >> b;

		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile);
		boost::archive::binary_iarchive archive(f);
		CbrData insert;
		archive >> insert;
		return insert;
	}
	return insert;
}
std::vector<std::wstring> empty = { L"" };
void CbrInterface::clearThreads() {
	g_interfaces.cbrInterface.SaveCbrDataThreaded(*g_interfaces.cbrInterface.getCbrData(0), false);
	g_interfaces.cbrInterface.LoadAndUploadDataThreaded(L"", false);
	g_interfaces.cbrInterface.MergeCbrDataThreaded(L"", false, -1);
	g_interfaces.cbrInterface.LoadCbrData(L"", false, -1);
	g_interfaces.cbrInterface.LoadnDeleteCbrData(empty, false, false, false);

}

void CbrInterface::mergeCbrReplayBuffer(bool checkSkip = false) {
	/*if (!checkSkip && processingCBRdata) { return; }
	if (recordBufferP1Backup.size() > 0) {
		recordBufferP1.insert(recordBufferP1.end(), recordBufferP1Backup.begin(), recordBufferP1Backup.end());
	}
	if (recordBufferP2Backup.size() > 0) {
		recordBufferP2.insert(recordBufferP2.end(), recordBufferP2Backup.begin(), recordBufferP2Backup.end());
	}*/
}

int CbrInterface::getAutoRecordReplayAmount() {

	mergeCbrReplayBuffer();
	return recordBufferP1.size() + recordBufferP2.size();
}
void CbrInterface::clearAutomaticRecordReplays() {
	if (processingCBRdata) { return; }
	mergeCbrReplayBuffer();
	recordBufferP1.clear();
	recordBufferP2.clear();
}
void CbrInterface::deleteAutomaticRecordReplays(int deletionAmount) {
	if (processingCBRdata) { return; }
	mergeCbrReplayBuffer();
	recordBufferP1.erase(recordBufferP1.end() - deletionAmount, recordBufferP1.end());
	recordBufferP2.erase(recordBufferP2.end() - deletionAmount, recordBufferP2.end());
}
void CbrInterface::resetCbrInterface() {
	if (processingCBRdata) { return; }
	mergeCbrReplayBuffer();
	lastRecordedRound = 0;
	readDepth = { 99,99 };
	writeDepth = { -1,-1 };
	inputMemory = { 5,5 };
	controllerMemory = { -1,-1 };
	resetDepth = -1;
	resetPlayer = -1;
	writeMemory = { -1,-1 };
	netplayMemory = { };
	executionOrder = {};
	cbrData[0].resetReplayVariables();
	cbrData[1].resetReplayVariables();
}

void cbrDebugSave(CbrData& cbrData) {
	if (!DebugPrint) { return; }
	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	auto filename = ".\\CBRsave\\cbrDebug.txt";
	std::ofstream outfile(filename);
	for (std::size_t i = 0; i < cbrData.debugTextArr.size(); ++i) {
		outfile << cbrData.debugTextArr[i];
	}
	cbrData.debugTextArr.clear();
}

void CbrInterface::EndCbrActivities() {
	EndCbrActivities(2);
}
void CbrInterface::EndCbrActivities(int playerNr, bool trim) {
	resetCbrInterface();
	cbrData[0].resetCbr();
	cbrData[1].resetCbr();
	disableAllReversalReplays();

	if (autoRecordActive == true) {
		auto anReplay = getAnnotatedReplay(0);
		auto savedReplay = AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]);
		if (anReplay->getInputSize() >= 600) {
			if (processingCBRdata) { recordBufferP1Backup.push_back(std::move(*anReplay)); }
			else { recordBufferP1.push_back(std::move(*anReplay)); }
		}
		setAnnotatedReplay(savedReplay, 0);
		anReplay = getAnnotatedReplay(1);
		savedReplay = AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]);
		if (anReplay->getInputSize() >= 600) {
			if (processingCBRdata) { recordBufferP2Backup.push_back(std::move(*anReplay)); }
			else { recordBufferP2.push_back(std::move(*anReplay)); }
		}
		setAnnotatedReplay(savedReplay, 1);
		autoRecordActive = false;
		autoRecordFinished = true;
	}

	if ((playerNr == 0 || playerNr == 2) && Recording == true) {
		//EndRecording
		auto anReplay = getAnnotatedReplay(0);

		if (anReplay->getInputSize() >= 10) {
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName(), trim);
			debugErrorCounter[0] += err.errorCount;
			if(err.structure != ""){ saveStructureDebug(err.structure); }
			if(err.errorCount == 0){
				auto data = getCbrData(0);
				if (data->getCharName() != anReplay->getCharacterName()[0]) {
					setCbrData(CbrData(anReplay->getPlayerName(), anReplay->getPlayerName(), anReplay->getCharIds()[0]), 0);
					data->setCharName(anReplay->getCharacterName()[0]);
				}
				data->AddReplay(cbrReplay);
			}
			else {
				cbrCreationDebugStr += err.errorDetail;
				saveDebug();
			}
			setAnnotatedReplay(AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]), 0);
			
		}
		Recording = false;
	}

	if ((playerNr == 1 || playerNr == 2) && RecordingP2 == true) {
		//EndRecording
		auto anReplay = getAnnotatedReplay(1);
		if (anReplay->getInputSize() >= 10) {
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto  err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
			debugErrorCounter[1] += err.errorCount;
			if (err.structure != "") { saveStructureDebug(err.structure); }
			if (err.errorCount == 0) {
				auto data = getCbrData(1);
				if (data->getCharName() != anReplay->getCharacterName()[0]) {
					setCbrData(CbrData(anReplay->getPlayerName(), anReplay->getPlayerName(), anReplay->getCharIds()[0]), 1);
					data->setCharName(anReplay->getCharacterName()[0]);
				}
				data->AddReplay(cbrReplay);
			}else {
				cbrCreationDebugStr += err.errorDetail;
				saveDebug();
			}
			setAnnotatedReplay(AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]), 1);
		}
		RecordingP2 = false;
	}


	if ((playerNr == 0 || playerNr == 2) && Replaying == true) {
		cbrDebugSave(cbrData[0]);
		Replaying = false;
	}
	if ((playerNr == 1 || playerNr == 2) && ReplayingP2 == true) {
		cbrDebugSave(cbrData[1]);
		ReplayingP2 = false;
	}
	if (instantLearning == true) {
		auto data = getCbrData(1);
		auto anReplay = getAnnotatedReplay(0);
		if (data->getCharName() != anReplay->getCharacterName()[0]) {
			setCbrData(CbrData(anReplay->getPlayerName(), anReplay->getPlayerName(), anReplay->getCharIds()[0]), 0);
			data->setCharName(anReplay->getCharacterName()[0]);
		}

		//if (data->getReplayCount() > 0) {
		//	data->deleteReplays(data->getReplayCount() - 1, data->getReplayCount() - 1);
		//}
		data->resetInstantLearning();

		auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
		auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
		debugErrorCounter[0] += err.errorCount;
		if (err.structure != "") { saveStructureDebug(err.structure); }
		if (err.errorCount == 0) {
			auto data = getCbrData(1);
			if (data->getCharName() != anReplay->getCharacterName()[0]) {
				setCbrData(CbrData(anReplay->getPlayerName(), anReplay->getPlayerName(), anReplay->getCharIds()[0]), 1);
				data->setCharName(anReplay->getCharacterName()[0]);
			}
			data->AddReplay(cbrReplay);
		}
		else {
			cbrCreationDebugStr += err.errorDetail;
			saveDebug();
		}
		cbrDebugSave(*data);
		instantLearning = 0;
	}

	if (instantLearningP2 == true) {
		auto data = getCbrData(0);
		auto anReplay = getAnnotatedReplay(1);
		if (data->getCharName() != anReplay->getCharacterName()[0]) {
			setCbrData(CbrData(anReplay->getPlayerName(), anReplay->getPlayerName(), anReplay->getCharIds()[0]), 1);
		}

		//if (data->getReplayCount() > 0) {
		//	data->deleteReplays(data->getReplayCount() - 1, data->getReplayCount() - 1);
		//}
		data->resetInstantLearning();

		auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
		auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
		debugErrorCounter[1] += err.errorCount;
		if (err.structure != "") { saveStructureDebug(err.structure); }
		if (err.errorCount == 0) {
			data->AddReplay(cbrReplay);
		}
		else {
			cbrCreationDebugStr += err.errorDetail;
			saveDebug();
		}
		cbrDebugSave(*data);
		instantLearningP2 = 0;
	}

}
void CbrInterface::StartCbrRecording(char* p1charName, char* p2charName, int p1charId, int p2charId, int recordingSlot) {
	if (recordingSlot == 0) {
		Recording = true;
	}
	if (recordingSlot == 1) {
		RecordingP2 = true;
	}
	setAnnotatedReplay(AnnotatedReplay(playerName, p1charName, p2charName, p1charId, p2charId), recordingSlot);
}

void CbrInterface::StartCbrInstantLearning(char* p1charName, char* p2charName, int p1charId, int p2charId, int recordingSlot) {
	
	int ann = 0;
	int cbr = 1;
	if (recordingSlot == 0) {
		ann = 0;
		cbr = 1;
		instantLearning = true;
	}
	if (recordingSlot == 1) {
		ann = 1;
		cbr = 0;
		instantLearningP2 = true;
	}
	setAnnotatedReplay(AnnotatedReplay(playerName, p1charName, p2charName, p1charId, p2charId), ann);
	if (!getCbrData(cbr)->getEnabled()) {
		setCbrData(CbrData(getAnnotatedReplay(ann)->getPlayerName(), getAnnotatedReplay(ann)->getFocusCharName(), getAnnotatedReplay(ann)->getCharIds()[0]), cbr);
	}
	auto cbrReplay = CbrReplayFile(getAnnotatedReplay(ann)->getCharacterName(), getAnnotatedReplay(ann)->getCharIds());
	getCbrData(cbr)->setInstantLearnReplay(cbrReplay);
}



void CbrInterface::threadSaveReplay(bool save) {
	
	//saveReplayDataInMenu();
	if (!threadActive) {
		if (processingStoredRecordingsThread.joinable()) {
			processingStoredRecordingsThread.join();
			//threadActive = false;
	}
	threadActive = true;
	processingStoredRecordingsThread = boost::thread(&CbrInterface::saveReplayDataInMenu, this);
	}
}

bool CbrInterface::threadCheckSaving() {
	bool ret = false;
	//saveReplayDataInMenu();

	if (!threadActive && processingStoredRecordingsThread.joinable()) {
		processingStoredRecordingsThread.join();
		//threadActive = false;
		ret = true;
	}
	return ret;
}

std::string CbrInterface::threadStatus() {
	std::string sRet = "Active:";

	if (processingStoredRecordingsThread.joinable()) {
		sRet += " processingStoredRecordingsThread";
	}

	return sRet;
}

bool CbrInterface::threadActiveCheck() {
	return threadActive;
}

void CbrInterface::saveSettings() {
	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	{
		auto filename = ".\\CBRsave\\CbrSettings.ini";
		std::ofstream outfile(filename);
		boost::archive::text_oarchive archive(outfile);
		archive << autoRecordGameOwner << autoRecordAllOtherPlayers << autoUploadOwnData << autoRecordConfirmation << myName;
	}
}
void CbrInterface::loadSettings(CbrInterface* cbrI) {
	if (cbrSettingsLoaded == true) { return; }
	cbrSettingsLoaded = true;
	try
	{
		cbrI->autoRecordGameOwner = false;
		cbrI->autoRecordAllOtherPlayers = false;
		std::string filename = ".\\CBRsave\\CbrSettings.ini";
		std::ifstream infile(filename);
		if (infile.fail()) {
			//File does not exist code here
		}
		else {
			boost::archive::text_iarchive archive(infile);
			debugPrintText += "LoadOk0\n";
			archive >> cbrI->autoRecordGameOwner;
			archive >> cbrI->autoRecordAllOtherPlayers;
			archive >> cbrI->autoUploadOwnData;
			archive >> cbrI->autoRecordConfirmation;
			archive >> cbrI->myName;
		}
	}
	catch (const std::exception&)
	{
		cbrI->autoRecordGameOwner = true;
		cbrI->autoRecordAllOtherPlayers = true;
		cbrI->autoUploadOwnData = false;
		cbrI->autoRecordConfirmation = true;
	}

}
void CbrInterface::saveDebug() {
	boost::filesystem::path dir("CBRsave");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	{
		auto filename = ".\\CBRsave\\CbrErrorReport.txt";
		std::ofstream outfile(filename);
		//boost::archive::text_oarchive archive(outfile);
		outfile << cbrCreationDebugStr;
	}
}


void CbrInterface::saveStructureDebug(std::string text) {
	boost::filesystem::path dir("CBRsave\\Structure");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	{
		auto time = NowToString();
		
		std::string bufferName = "";
		for (int i = 0; i < 23; ++i) {
			if (time[i] != 0) {
				std::string tests = "";
				tests += time[i];
				if (boost::filesystem::windows_name(tests)) {
					bufferName += time[i];
				}
			}
		}
		auto filename = ".\\CBRsave\\Structure\\" + bufferName + ".txt";
		std::ofstream outfile(filename);
		//boost::archive::text_oarchive archive(outfile);
		outfile << text;
	}
}

void CbrInterface::saveReplayDataInMenu() {
	processingCBRdata = true;
	mergeCbrReplayBuffer(true);
	auto cbrData = CbrData();
	bool ranOnce = false;
	std::string charName = "";
	std::string playerName = "";
	if (playerID == "") {
		playerID = GetPlayerID();
	}
	autoRecordDeletionAmount[0] = 0;
	autoRecordDeletionAmount[1] = 0;
	for (int i = 0; i < recordBufferP1.size(); ++i) {
		auto& anReplay = recordBufferP1[i];
		if (i == 0 || charName != anReplay.getFocusCharName() || playerName != anReplay.getPlayerName()) {
			charName = anReplay.getFocusCharName();
			playerName = anReplay.getPlayerName();
			autoRecordSaveCompletedName[0] = playerName;
			autoRecordSaveCompletedChar[0] = charName;
			if (ranOnce) { 
				SaveCbrDataExperiment(cbrData);
				//if (autoUploadOwnData && cbrData.getPlayerName() == playerID){ CbrHTTPPostNoThreat(convertCBRtoJson(cbrData, playerID)); }
				cbrData = CbrData();
			}
			ranOnce = true;
			cbrData = LoadCbrDataNoThread(anReplay.getPlayerName(), anReplay.getCharacterName()[0]);
			if (cbrData.getPlayerName() == "-1") {
				cbrData = CbrData(anReplay.getPlayerName(), anReplay.getCharacterName()[0], anReplay.getCharIds()[0]);
			}
		}
		auto cbrReplay = CbrReplayFile(anReplay.getCharacterName(), anReplay.getCharIds());
		auto err = cbrReplay.makeFullCaseBase(&anReplay, anReplay.getFocusCharName());

		if (err.structure != "") { saveStructureDebug(err.structure); }
		debugErrorCounter[0] += err.errorCount;
		if (err.errorCount == 0) { 
			cbrData.AddReplay(cbrReplay); 
			autoRecordDeletionAmount[0]++;
		}
		else {
			cbrCreationDebugStr += err.errorDetail;
			saveDebug();
		}
		
	}
	if(ranOnce){ 
		SaveCbrDataExperiment(cbrData);
		//if (autoUploadOwnData && cbrData.getPlayerName() == playerID) { CbrHTTPPostNoThreat(convertCBRtoJson(cbrData, playerID)); }
	}
	ranOnce = false;

	cbrData = CbrData();
	for (int i = 0; i < recordBufferP2.size(); ++i) {
		auto& anReplay = recordBufferP2[i];
		if (i == 0 || charName != anReplay.getFocusCharName() || playerName != anReplay.getPlayerName()) {
			charName = anReplay.getFocusCharName();
			playerName = anReplay.getPlayerName();
			autoRecordSaveCompletedName[1] = playerName;
			autoRecordSaveCompletedChar[1] = charName;
			if (ranOnce) { 
				SaveCbrDataExperiment(cbrData);
				//if (autoUploadOwnData && cbrData.getPlayerName() == playerID) { CbrHTTPPostNoThreat(convertCBRtoJson(cbrData, playerID)); }
				cbrData = CbrData();
			}
			ranOnce = true;
			cbrData = LoadCbrDataNoThread(anReplay.getPlayerName(), anReplay.getCharacterName()[0]);
			if (cbrData.getPlayerName() == "-1") {
				cbrData = CbrData(anReplay.getPlayerName(), anReplay.getCharacterName()[0], anReplay.getCharIds()[0]);
			}
		}
		auto cbrReplay = CbrReplayFile(anReplay.getCharacterName(), anReplay.getCharIds());
		auto err = cbrReplay.makeFullCaseBase(&anReplay, anReplay.getFocusCharName());
		if (err.structure != "") { saveStructureDebug(err.structure); }
		debugErrorCounter[1] += err.errorCount;
		if (err.errorCount == 0) { 
			cbrData.AddReplay(cbrReplay); 
			autoRecordDeletionAmount[1]++;
		}
		else {
			cbrCreationDebugStr += err.errorDetail;
			saveDebug();
		}
		
	}
	if (ranOnce) { 
		SaveCbrDataExperiment(cbrData);
		//if (autoUploadOwnData && cbrData.getPlayerName() == playerID) { CbrHTTPPostNoThreat(convertCBRtoJson(cbrData, playerID)); }
	}
	ranOnce = false;
	recordBufferP1.clear();
	recordBufferP2.clear();
	autoRecordSaveCompleted = true;
	
	processingCBRdata = false;
	threadActive = false;
}
std::string CbrInterface::WriteAiInterfaceState() {
	std::string str = "Ai Activities:";

	if (Recording) {
		str += " - RecordingP1";
	}
	if (RecordingP2) {
		str += " - RecordingP2";
	}
	if (Replaying) {
		str += " - ReplayingP1";
	}
	if (ReplayingP2) {
		str += " - ReplayingP2";
	}
	if (instantLearning) {
		str += " - InstantLearningP1";
	}
	if (instantLearningP2) {
		str += " - InstantLearningP2";
	}
	str += "\nFA1: " + std::to_string(cbrData[0].framesActive) + " - " + std::to_string(cbrData[1].framesActive);
	return str;
}
void CbrInterface::RestartCbrActivities(char* p1charName, char* p2charName, int p1charId, int p2charId) {
	resetCbrInterface();
	cbrData[0].resetCbr();
	cbrData[1].resetCbr();
	disableAllReversalReplays();
	if (autoRecordActive == true) {
		auto anReplay = getAnnotatedReplay(0);
		auto savedReplay = AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]);
		if (anReplay->getInputSize() >= 900) {

			if (processingCBRdata) { recordBufferP1Backup.push_back(std::move(*anReplay)); }
			else { recordBufferP1.push_back(std::move(*anReplay)); }
		}
		setAnnotatedReplay(savedReplay, 0);
		anReplay = getAnnotatedReplay(1);
		savedReplay = AnnotatedReplay(anReplay->getPlayerName(), anReplay->getCharacterName()[0], anReplay->getCharacterName()[1], anReplay->getCharIds()[0], anReplay->getCharIds()[1]);
		if (anReplay->getInputSize() >= 900) {
			if (processingCBRdata) { recordBufferP2Backup.push_back(std::move(*anReplay)); }
			else{ recordBufferP2.push_back(std::move(*anReplay)); }
			
		}
		setAnnotatedReplay(savedReplay, 1);
	}

	if (Recording == true) {
		auto data = getCbrData(0);
		auto anReplay = getAnnotatedReplay(0);
		//EndRecording
		if (anReplay->getInputSize() >= 10) {
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
			if (err.structure != "") { saveStructureDebug(err.structure); }
			debugErrorCounter[0] += err.errorCount;
			if (err.errorCount == 0) { data->AddReplay(cbrReplay); }
			else {
				cbrCreationDebugStr += err.errorDetail;
				saveDebug();
			}
			Recording = false;
		}
		StartCbrRecording(p1charName, p2charName, p1charId, p2charId,0);
	}

	if (RecordingP2 == true) {
		auto data = getCbrData(1);
		auto anReplay = getAnnotatedReplay(1);
		//EndRecording
		if (anReplay->getInputSize() >= 10) {
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
			debugErrorCounter[0] += err.errorCount;
			if (err.errorCount == 0) { data->AddReplay(cbrReplay); }
			if (err.structure != "") { saveStructureDebug(err.structure); }
			cbrCreationDebugStr += err.errorDetail;
			saveDebug();
			RecordingP2 = false;
		}
		StartCbrRecording(p2charName, p1charName, p2charId, p1charId, 1);
	}

	if (Replaying == true) {
		auto data = getCbrData(0);
		auto anReplay = getAnnotatedReplay(0);
		cbrDebugSave(*data);
	}

	if (ReplayingP2 == true) {
		auto data = getCbrData(1);
		auto anReplay = getAnnotatedReplay(1);
		cbrDebugSave(*data);
	}

	if (instantLearning == true) {
		auto data = getCbrData(1);
		auto anReplay = getAnnotatedReplay(0);
		if (data->getInstantLearnReplay()->getCaseBaseLength() > 0) {
			//data->deleteReplays(data->getReplayCount() - 1, data->getReplayCount() - 1);
			data->resetInstantLearning();
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
			if (err.structure != "") { saveStructureDebug(err.structure); }
			debugErrorCounter[0] += err.errorCount;
			if (err.errorCount == 0) { data->AddReplay(cbrReplay); }
			else {
				cbrCreationDebugStr += err.errorDetail;
				saveDebug();
			}
		}
		cbrDebugSave(*data);
		StartCbrInstantLearning(p1charName, p2charName, p1charId, p2charId,0);
	}

	if (instantLearningP2 == true) {
		auto data = getCbrData(0);
		auto anReplay = getAnnotatedReplay(1);
		if (data->getInstantLearnReplay()->getCaseBaseLength() > 0) {
			//data->deleteReplays(data->getReplayCount() - 1, data->getReplayCount() - 1);
			data->resetInstantLearning();
			auto cbrReplay = CbrReplayFile(anReplay->getCharacterName(), anReplay->getCharIds());
			auto err = cbrReplay.makeFullCaseBase(anReplay, anReplay->getFocusCharName());
			if (err.structure != "") { saveStructureDebug(err.structure); }
			debugErrorCounter[1] += err.errorCount;
			if (err.errorCount == 0) { data->AddReplay(cbrReplay); }
			else {
				cbrCreationDebugStr += err.errorDetail;
				saveDebug();
			}
		}
		cbrDebugSave(*data);
		StartCbrInstantLearning(p2charName, p1charName, p2charId, p1charId, 1);
	}
}

void CbrInterface::Checkinputs() {

	if (hProcess == 0) {
		DWORD procId = GetProcId(L"BBCF.exe");
		//Getmodulebaseaddress
		moduleBase = GetModuleBaseAddress(procId, L"BBCF.exe");
		hProcess = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

		uintptr_t input4Ptr = moduleBase + 0xD61658;
		uintptr_t input55Ptr = 0;
		ReadProcessMemory(hProcess, (BYTE*)input4Ptr, &input55Ptr, sizeof(input55Ptr), 0);
		input55Ptr = input55Ptr + 0x4;

		uintptr_t input6Ptr = 0;
		ReadProcessMemory(hProcess, (BYTE*)input55Ptr, &input6Ptr, sizeof(input6Ptr), 0);
		input5Ptr = input6Ptr + 0x10DC;
		
	}
	uintptr_t input1Ptr = moduleBase + 0xD61584;
	uintptr_t input2Ptr = moduleBase + 0xD615E8;
	uintptr_t input3Ptr = moduleBase + 0xE19888;
	int input1 = 0;
	int input2 = 0;
	int input3 = 0;
	int input4 = 0;

	ReadProcessMemory(hProcess, (BYTE*)input1Ptr, &input1, sizeof(input1), 0);
	ReadProcessMemory(hProcess, (BYTE*)input2Ptr, &input2, sizeof(input2), 0);
	ReadProcessMemory(hProcess, (BYTE*)input3Ptr, &input3, sizeof(input3), 0);
	ReadProcessMemory(hProcess, (BYTE*)input5Ptr, &input4, sizeof(input4), 0);
	
	debugPrint1 = input1;
	debugPrint2 = input2;
	debugPrint3 = input3;
	debugPrint4 = input4;
	if (input1 != input3) {
		input1 = input1;
	}
}

std::string CbrInterface::GetPlayerID() {
	try
	{
		//Get ProcId of the target process
		DWORD procId = GetProcId(L"BBCF.exe");
		//Getmodulebaseaddress
		uintptr_t moduleBase = GetModuleBaseAddress(procId, L"BBCF.exe");
		uintptr_t p1NamePtr = moduleBase + 0x662740;
		HANDLE hProcess = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

		char playerID[35]{ 0 };

		ReadProcessMemory(hProcess, (BYTE*)p1NamePtr, &playerID, 34, 0);
		nameP1 = "";

		for (int i = 0; i < 35; i++) {
			debugPrintText += playerID[i];
		}
		std::string bufferName = "";
		for (int i = 0; i < 35; ++i) {
			if (playerID[i] != 0) {
				bufferName = nameP1 + playerID[i];
				if (boost::filesystem::windows_name(bufferName)) {
					nameP1 += playerID[i];
				}

			}
			else {
				if (i + 1 >= 35 || playerID[i + 1] == 0) {
					break;
				}
			}

		}
		return nameP1;
	}
	catch (const std::exception&)
	{

	}
	return "";
}

void CbrInterface::SetPlayerNames(uint8_t gameMode) {
	try
	{
		char p1CharName[32]{ 0 };
		char p2CharName[32]{ 0 };

		if (gameMode == GameMode_Online)
		{
			memcpy(p1CharName, g_gameVals.pPlayerNameOnline[0], 32);
			memcpy(p2CharName, g_gameVals.pPlayerNameOnline[1], 32);
		}
		else
		{
			memcpy(p1CharName, g_gameVals.pPlayerNameReplay[0], 32);
			memcpy(p2CharName, g_gameVals.pPlayerNameReplay[1], 32);
		}

		nameP1 = "";
		nameP2 = "";

		for (int i = 0; i < 32; i++) {
			debugPrintText += p1CharName[i];
		}
		debugPrintText += "\n";
		for (int i = 0; i < 32; i++) {
			debugPrintText += p2CharName[i];
		}
		debugPrintText += "\n";

		std::string bufferName = "";
		for (int i = 0; i < 32; ++i) {
			if (p1CharName[i] != 0) {
				bufferName = nameP1 + p1CharName[i];
				if (boost::filesystem::windows_name(bufferName)) {
					nameP1 += p1CharName[i];
				}

			}
			else {
				if (i + 1 >= 32 || p1CharName[i + 1] == 0) {
					break;
				}
			}

		}
		debugPrintText += nameP1 + "\n";

		for (int i = 0; i < 32; ++i) {
			bufferName = nameP2 + p2CharName[i];
			if (p2CharName[i] != 0) {
				if (boost::filesystem::windows_name(bufferName)) {
					nameP2 += p2CharName[i];
				}
			}
			else {
				if (i + 1 >= 32 || p2CharName[i + 1] == 0) {
					break;
				}
			}

		}
		debugPrintText += nameP2 + "\n";
		saveDebugPrint();
	}
	catch (const std::exception&)
	{

	}
}

bool CbrInterface::CheckGameOwner(int playerNr)
{
	if (g_gameVals.GetGameMode() == GameMode_ReplayTheater)
	{
		//Local replays save the steam name in the player slot of the player who activated vs 2P.
		if (playerNr == 0 && (nameP1 != "" && nameP2 == ""))
			return true;
		if (playerNr == 1 && (nameP1 == "" && nameP2 != ""))
			return true;

		if (playerNr == 0)
		{
			if (nameP1 != "")
				return strcmp(myName, nameP1.c_str()) == 0;
			else if (nameVersusP1[0] != '\0')
				return strcmp(myName, nameVersusP1) == 0;
		}
		if (playerNr == 1)
		{
			if (nameP2 != "")
				return strcmp(myName, nameP2.c_str()) == 0;
			else if (nameVersusP2[0] != '\0')
				return strcmp(myName, nameVersusP2) == 0;
		}
			
	}
	else if (g_gameVals.GetGameMode() == GameMode_Online)
	{
		if (playerNr == 0)
			return strcmp(myName, nameP1.c_str()) == 0;
		if (playerNr == 1)
			return strcmp(myName, nameP2.c_str()) == 0;
	}
	else if (g_gameVals.GetGameMode() == GameMode_Versus)
	{
		if (playerNr == 0 && nameVersusP1[0] != '\0')
			return strcmp(myName, nameVersusP1) == 0;
		if (playerNr == 1 && nameVersusP2[0] != '\0')
			return strcmp(myName, nameVersusP2) == 0;
	}
	return playerNr == 0;
}

DWORD CbrInterface::GetProcId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));

		}
	}
	CloseHandle(hSnap);
	return procId;
}

uintptr_t CbrInterface::GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}
/*
void CbrInterface::UpdateOldCbrMetadata() {
	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	//boost::filesystem::path dir("CBRsave");
	for (const auto& dirEntry : recursive_directory_iterator("CBRsave")) {
		auto& t = dirEntry;
		std::string s = dirEntry.path().string();
		auto found = std::string::npos != s.find(".cbr");
		if (found) {
			auto cbr = LoadCbrDataOld(s);
			SaveCbrData(cbr);
		}
	}
}*/

// Returns true if two rectangles (l1, r1) and (l2, r2)
// overlap

ImVec2 doOverlap(ImVec2 l1, ImVec2 r1, ImVec2 l2, ImVec2 r2, bool swap)
{
	// if rectangle has area 0, no overlap
	if (l1.x == r1.x || l1.y == r1.y || r2.x == l2.x || l2.y == r2.y)
		return ImVec2(-1, -1);

	// If one rectangle is on left side of other
	if (l1.x > r2.x || l2.x > r1.x)
		return ImVec2(-1, -1);;

	// If one rectangle is above other
	if (r1.y > l2.y || r2.y > l1.y)
		return ImVec2(-1, -1);;

	if(swap){ return ImVec2(r2.x - l1.x, l1.y - r2.y); }
	return ImVec2(r1.x - l2.x, l1.y - r2.y);
}

float Playerdistance = 99999999999;
float hitboxdistance = 99999999999;
float minDistance = 999999999;
ImVec2 CbrInterface::GetMinDistance(const CharData* charObj, const CharData* charObjOpp)
{
	std::vector<Hitbox> entries = HitboxReader::getHitboxes(charObj);
	float minx = 0;
	float miny = 0;
	for (const Hitbox& entry : entries)
	{
		if (entry.type == HitboxType_Hitbox) {
			ImVec2 playerWorldPos(charObj->posX, -charObj->posY); //Flip y coordinate to use the same convention as BB
			float scaleX = charObj->scaleX;
			float scaleY = charObj->scaleY;
			//scaling logic taken from ACPR hitbox viewer, not sure if this is correct here
			if (scaleX < 0 && scaleY < 0)
			{
				scaleX = 1000.0f;
				scaleY = 1000.0f;
			}
			else
			{
				scaleX = scaleX < 0 ? scaleY : scaleX;
				scaleY = scaleY < 0 ? scaleX : scaleY;
			}

			float offsetX = floor(entry.offsetX * 100 * scaleX / 1000.0f);
			float offsetY = -floor(entry.offsetY * 100 * scaleY / 1000.0f);
			float width = floor(entry.width * 100 * scaleX / 1000.0f);
			float height = -floor(entry.height * 100 * scaleY / 1000.0f);

			if (charObj->facingRight)
			{
				offsetX = -offsetX;
				width = -width;
			}

			ImVec2 pointA(playerWorldPos.x + offsetX, playerWorldPos.y + offsetY);
			ImVec2 pointB(playerWorldPos.x + offsetX + width, playerWorldPos.y + offsetY);
			ImVec2 pointC(playerWorldPos.x + offsetX + width, playerWorldPos.y + offsetY + height);
			ImVec2 pointD(playerWorldPos.x + offsetX, playerWorldPos.y + offsetY + height);

			ImVec2 pointLU(9999999999, -9999999999);
			ImVec2 pointRD(-9999999999, 9999999999);
			
			
			if (pointA.x < pointLU.x) { pointLU.x = pointA.x; }
			if (pointA.y > pointLU.y) { pointLU.y = pointA.y; }
			if (pointA.x > pointRD.x) { pointRD.x = pointA.x; }
			if (pointA.y < pointRD.y) { pointRD.y = pointA.y; }

			if (pointB.x < pointLU.x) { pointLU.x = pointB.x; }
			if (pointB.y > pointLU.y) { pointLU.y = pointB.y; }
			if (pointB.x > pointRD.x) { pointRD.x = pointB.x; }
			if (pointB.y < pointRD.y) { pointRD.y = pointB.y; }

			if (pointC.x < pointLU.x) { pointLU.x = pointC.x; }
			if (pointC.y > pointLU.y) { pointLU.y = pointC.y; }
			if (pointC.x > pointRD.x) { pointRD.x = pointC.x; }
			if (pointC.y < pointRD.y) { pointRD.y = pointC.y; }

			if (pointD.x < pointLU.x) { pointLU.x = pointD.x; }
			if (pointD.y > pointLU.y) { pointLU.y = pointD.y; }
			if (pointD.x > pointRD.x) { pointRD.x = pointD.x; }
			if (pointD.y < pointRD.y) { pointRD.y = pointD.y; }


			std::vector<Hitbox> entriesOpp = HitboxReader::getHitboxes(charObjOpp);
			for (const Hitbox& entryOpp : entriesOpp)
			{
				if (entryOpp.type == HitboxType_Hurtbox) {
					ImVec2 playerWorldPosOpp(charObjOpp->posX, -charObjOpp->posY);
					float scaleXOpp = charObjOpp->scaleX;
					float scaleYOpp = charObjOpp->scaleY;

					if (scaleXOpp < 0 && scaleYOpp < 0)
					{
						scaleXOpp = 1000.0f;
						scaleYOpp = 1000.0f;
					}
					else
					{
						scaleXOpp = scaleXOpp < 0 ? scaleYOpp : scaleXOpp;
						scaleYOpp = scaleYOpp < 0 ? scaleXOpp : scaleYOpp;
					}

					float offsetXOpp = floor(entryOpp.offsetX * 100 * scaleXOpp / 1000.0f);
					float offsetYOpp = -floor(entryOpp.offsetY * 100 * scaleYOpp / 1000.0f);
					float widthOpp = floor(entryOpp.width * 100 * scaleXOpp / 1000.0f);
					float heightOpp = -floor(entryOpp.height * 100 * scaleYOpp / 1000.0f);

					if (charObjOpp->facingRight)
					{
						offsetXOpp = -offsetXOpp;
						widthOpp = -widthOpp;
					}

					ImVec2 pointAOpp(playerWorldPosOpp.x + offsetXOpp, playerWorldPosOpp.y + offsetYOpp);
					ImVec2 pointBOpp(playerWorldPosOpp.x + offsetXOpp + widthOpp, playerWorldPosOpp.y + offsetYOpp);
					ImVec2 pointCOpp(playerWorldPosOpp.x + offsetXOpp + widthOpp, playerWorldPosOpp.y + offsetYOpp + heightOpp);
					ImVec2 pointDOpp(playerWorldPosOpp.x + offsetXOpp, playerWorldPosOpp.y + offsetYOpp + heightOpp);

					ImVec2 pointLU2(9999999999, -9999999999);
					ImVec2 pointRD2(-9999999999, 9999999999);

					if (pointAOpp.x < pointLU2.x) { pointLU2.x = pointAOpp.x; }
					if (pointAOpp.y > pointLU2.y) { pointLU2.y = pointAOpp.y; }
					if (pointAOpp.x > pointRD2.x) { pointRD2.x = pointAOpp.x; }
					if (pointAOpp.y < pointRD2.y) { pointRD2.y = pointAOpp.y; }

					if (pointBOpp.x < pointLU2.x) { pointLU2.x = pointBOpp.x; }
					if (pointBOpp.y > pointLU2.y) { pointLU2.y = pointBOpp.y; }
					if (pointBOpp.x > pointRD2.x) { pointRD2.x = pointBOpp.x; }
					if (pointBOpp.y < pointRD2.y) { pointRD2.y = pointBOpp.y; }

					if (pointCOpp.x < pointLU2.x) { pointLU2.x = pointCOpp.x; }
					if (pointCOpp.y > pointLU2.y) { pointLU2.y = pointCOpp.y; }
					if (pointCOpp.x > pointRD2.x) { pointRD2.x = pointCOpp.x; }
					if (pointCOpp.y < pointRD2.y) { pointRD2.y = pointCOpp.y; }

					if (pointDOpp.x < pointLU2.x) { pointLU2.x = pointDOpp.x; }
					if (pointDOpp.y > pointLU2.y) { pointLU2.y = pointDOpp.y; }
					if (pointDOpp.x > pointRD2.x) { pointRD2.x = pointDOpp.x; }
					if (pointDOpp.y < pointRD2.y) { pointRD2.y = pointDOpp.y; }
					
					auto distance = doOverlap(pointLU, pointRD, pointLU2, pointRD2, charObj->posX > charObjOpp->posX);
					if (distance.x != -1) {
						if (distance.x > minx) { minx = distance.x; }
						if (distance.y > miny) { miny = distance.y; }
					}

				}


			}
		}
		
	}
	return ImVec2(minx * 0.8, miny * 0.8);
}

ImVec2 CbrInterface::RotatePoint(ImVec2 center, float angleInRad, ImVec2 point)
{
	if (!angleInRad)
	{
		return point;
	}

	// translate point back to origin:
	point.x -= center.x;
	point.y -= center.y;

	float s = sin(angleInRad);
	float c = cos(angleInRad);

	// rotate point
	float xNew = point.x * c - point.y * s;
	float yNew = point.x * s + point.y * c;

	// translate point back:
	point.x = xNew + center.x;
	point.y = yNew + center.y;
	return point;
}

bool CbrInterface::WorldToScreen(LPDIRECT3DDEVICE9 pDevice, D3DXMATRIX* view, D3DXMATRIX* proj, D3DXVECTOR3* pos, D3DXVECTOR3* out)
{
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX world;

	pDevice->GetViewport(&viewPort);
	D3DXMatrixIdentity(&world);

	D3DXVec3Project(out, pos, &viewPort, proj, view, &world);
	if (out->z < 1) {
		return true;
	}
	return false;
}


//Logic replicates the logic used by the game to calculate the number of items
int CbrInterface::GetNumberOfReplayMenuItems()
{
	int64_t result64 = (int64_t)(*g_gameVals.pLastReplayItemAddr - *g_gameVals.pFirstReplayItemAddr);
	result64 *= 0x67B23A55;
	int result = (int)(result64 >> 32) >> 7;
	uint32_t copy = (uint32_t)result >> 31;
	return (int)(result + copy - 1);
}