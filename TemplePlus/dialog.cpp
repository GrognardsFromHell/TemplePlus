
#include "stdafx.h"
#include "dialog.h"
#include <fstream>
#include <gamesystems/gamesystems.h>
#include "gamesystems/timeevents.h"
#include "infrastructure/vfs.h"
#include <party.h>
#include <rng.h>
#include <critter.h>
#include <sound.h>
#include <gamesystems/objects/objsystem.h>
DialogScripts dialogScripts;

static struct DialogScriptsAddresses : temple::AddressTable {

	bool (__cdecl *GetFilename)(int scriptId, char *filename);

	bool (__cdecl *LoadScript)(const char *filename, int &handleOut);

	void (__cdecl *LoadNpcLine)(DialogState &state, bool force);

	void (__cdecl *FreeScript)(int handle);

	DialogScriptsAddresses() {
		rebase(GetFilename, 0x1007E270);
		rebase(LoadScript, 0x10036600);
		rebase(LoadNpcLine, 0x10038590);
		rebase(FreeScript, 0x100346F0);
	}
} addresses;


class DialogHooks : public TempleFix
{
public:
	
	static void MapFirstSeenBanter(int mapId);
	static void PlayVoice(objHndl handle, objHndl addressed, int soundId);
	
	void apply() override {

		static BOOL (__cdecl*orgLoadDialogFile)(char*, int*) = 
			replaceFunction<BOOL(__cdecl)(char*, int*)>(0x10036600, [](char* filename, int* dlgFileHandle){
			return dialogScripts.Load(filename, *dlgFileHandle)?TRUE:FALSE;
			//return orgLoadDialogFile(filename, dlgFileHandle);
		});

		// Fixes crash issue:
		// ObjHndl buffer overlow when more than 3 NPCs (leading to junk data handle being used)
		replaceFunction(0x10038470, MapFirstSeenBanter);

		//replaceFunction(0x100354A0, PlayVoice);
	}
} dialogHooks;


bool DialogLine::IsPcLine(){
	return this->minIq != 0;
}

bool DialogLine::IsNpcLine(){
	return !IsPcLine();
}

DialogLine &DialogLine::operator=(DialogLineNew &src){
	auto &dest = *this;

	dest.minIq = src.minIq;
	dest.answerLineId = src.answerLineId;
	dest.key = src.key;
	dest.txt = _strdup(src.txt.c_str());

	// copy testField if not blank
	auto testFieldOk = false;
	for (auto i = 0u; i < src.testField.size(); i++) {
		if (!isspace(src.testField[i])) {
			testFieldOk = true;
			break;
		}
	}
	dest.testField = testFieldOk ? _strdup(src.testField.c_str()) : nullptr;


	// copy effectfield if not blank
	auto effectFieldOk = false;
	for (auto i = 0u; i < src.effectField.size(); i++) {
		if (!isspace(src.effectField[i])) {
			effectFieldOk = true;
			break;
		}
	}
	dest.effectField = effectFieldOk ? _strdup(src.effectField.c_str()) : nullptr;

	if (src.IsNpcLine())
		dest.genderField = _strdup(src.genderField.c_str());
	else
		dest.genderField = (char*)-1;

	

	return dest;
}

string DialogScripts::GetFilename(int scriptId) {
	char filename[MAX_PATH];
	if (addresses.GetFilename(scriptId, filename)) {
		return filename;
	} else {
		return string();		
	}
}

bool DialogScripts::Load(const string& filename, int& dlgHandle) {
	DialogFile *&mFileEntries = temple::GetRef<DialogFile*>(0x108ED0CC);
	int &mFileEntryCount = temple::GetRef<int>(0x108ED0C4);

	if (GetFileEntryByFilename(filename, dlgHandle)){
		auto &entry = mFileEntries[dlgHandle];
		entry.refCount++;
		entry.lastTimeLoaded = gameSystems->GetTimeEvent().GetTime();
		return true;
	}

	DialogFile newEntry;
	strcpy(newEntry.filename, filename.c_str());
	newEntry.refCount = 1;
	newEntry.lineCapacity = newEntry.lineCount = 0;
	newEntry.lastTimeLoaded = gameSystems->GetTimeEvent().GetTime();
	newEntry.lines = nullptr;
	newEntry.unk11C = 0;

	newEntry.GetLinesFromFile();
	if (newEntry.lineCount ==0)
		return false;
	
	memcpy(&mFileEntries[dlgHandle], &newEntry, sizeof DialogFile);
	mFileEntryCount++;

	return true;
	//return addresses.LoadScript(filename.c_str(), dlgHandle);
}

void DialogScripts::LoadNpcLine(DialogState& state, bool force) {
	addresses.LoadNpcLine(state, force);
}

void DialogScripts::Free(int dlgHandle) {
	addresses.FreeScript(dlgHandle);
}

DialogFile* DialogScripts::GetDialogFileEntry(int dlgHandle){
	DialogFile *&mFileEntries = temple::GetRef<DialogFile*>(0x108ED0CC);
	int &mFileEntryCount = temple::GetRef<int>(0x108ED0C4);

	if (dlgHandle >= mFileEntryCount || dlgHandle < 0)
		return nullptr;
	return &mFileEntries[dlgHandle];
}

ostream & operator<<(ostream & result, DialogLine &line){
	/*
	{1}{NPC txt}{Gender}{     }{speechId }{        }{}
	{2}{PC txt }{      }{minIq}{testField}{answerId}{effectField}
	*/
	if (line.IsPcLine()) {
		result << "{" << line.key << "}";
		if (line.txt)
			result << "{" << line.txt << "}";
		else
			result << "{}";
		result << "{}"; // blank gender field

		result << "{" << line.minIq << "}";
		if (line.testField)
			result << "{" << line.testField << "}";
		else
			result << "{}";
		result << "{" << line.answerLineId << "}";

		if (line.effectField)
			result << "{" << line.effectField << "}";
		else
			result << "{}";

		//result = fmt::format("\{{}\}\{{}\}\{\}\{{}\}\{{}\}\{{}\}\{{}\}", 
		//	line.key, line.txt, line.minIq, line.testField, line.answerLineId, line.effectField );
	}
	else {
			
		result << "{" << line.key << "}";
		if (line.txt)
			result << "{" << line.txt << "}";
		else
			result << "{}";
		
		if (line.genderField)
			result << "{" << line.genderField << "}";
		else
			result << "{}";

		result << "{}";
		result << "{}";
		result << "{}";

		if (line.effectField)
			result << "{" << line.effectField << "}";
		else
			result << "{}";

		//result = fmt::format("\{{}\}\{{}\}\{{}\}\{\}\{\}\{\}\{\}",
		//	line.key, line.txt, line.genderField, line.effectField);
	}
	return result;
}

void DialogScripts::SaveToFile(int dlgHandle){
	auto fileEntry = GetDialogFileEntry(dlgHandle);
	if (!fileEntry)
		return;

	std::string newFileName(fmt::format("", fileEntry->filename));
	//auto newFile = fopen("shit.mes", "wb");
	ofstream newFile("shit.mes");

	for (auto i=0; i < fileEntry->lineCount; i++){
		auto line = fileEntry->lines[i];

		if (i && line.IsNpcLine() && line.key < 10000) {
			newFile << std::endl;
			if (!(line.key % 10))
				newFile << std::endl;
		}
		newFile << line << std::endl;
	}
}

bool DialogScripts::GetFileEntryByFilename(const std::string & filename, int &dlgHandleOut){

	DialogFile *&mFileEntries = temple::GetRef<DialogFile*>(0x108ED0CC);
	int &mFileEntryCapacity = temple::GetRef<int>(0x108ED0C8);

	auto oldestFreeHandle = -1;

	for (auto i=0; i < mFileEntryCapacity; i++){

		auto &entry = mFileEntries[i];

		if (!_stricmp(filename.c_str(), entry.filename)){
			dlgHandleOut = i;
			return true;
		}
		if (!entry.refCount){
			if (oldestFreeHandle == -1){
				oldestFreeHandle = i;
			}
			else{
				auto &lastEntry = mFileEntries[oldestFreeHandle];
				if (lastEntry.filename[0]
				&& (!entry.filename[0])
					|| GameTime::Compare(lastEntry.lastTimeLoaded, 
						entry.lastTimeLoaded) > 0){
					oldestFreeHandle = i;
			    }
				
			}
		}
	}

	if (oldestFreeHandle != -1){
		dlgHandleOut = oldestFreeHandle;
		if (mFileEntries[oldestFreeHandle].filename[0])
			Free(oldestFreeHandle);
		return false;
	}

	auto prevCount = mFileEntryCapacity;
	dlgHandleOut = prevCount;
	// allocate 10 more entries
	mFileEntryCapacity += 10;
	mFileEntries = (DialogFile*)realloc(mFileEntries, mFileEntryCapacity * sizeof DialogFile);
	for (auto i=prevCount; i < mFileEntryCapacity; i++){
		auto &entry = mFileEntries[i];
		entry.filename[0] = 0;
		entry.refCount = 0;
	}
	return false;
}


struct DlgParser {
	stringstream ss;
	string lineBuf; // buffer for storing a line's content

	stringstream bracketStream;
	string bracketContent;
	

	bool GetSingleLine(DialogLineNew &line, int&fileLine);
	bool GetBracketContent(int &fileLine);

	DlgParser(string & fileContents):ss(fileContents){};
};

bool DlgParser::GetSingleLine(DialogLineNew& line, int& fileLine){
	// parse buf for bracket stuff until a line is complete	
	if (!GetBracketContent(fileLine))
		return false;
	line.key = atol(bracketContent.c_str());

	if (!GetBracketContent(fileLine)){
		logger->error("Missing text on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	line.txt = bracketContent;

	if (!GetBracketContent(fileLine)) {
		logger->error("Missing gender field on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	line.genderField = bracketContent;

	if (!GetBracketContent(fileLine)) {
		logger->error("Missing minimum IQ value on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	auto minIq = atol(bracketContent.c_str());
	if (!minIq && bracketContent.size()) {
		logger->error("Invalid minimum IQ value on line: {} (dialog line {}). Must be blank (for an NPC) or non-zero (for a PC)", fileLine, line.key);
		return false;
	}
	line.minIq = minIq;
	
	if (!GetBracketContent(fileLine)) {
		logger->error("Missing test field on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	line.testField = bracketContent;

	if (!GetBracketContent(fileLine)) {
		logger->error("Missing response value on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	if (bracketContent[0] == '#'){
		logger->error("Saw a # in a response value on line: {} (dialog line {})", fileLine, line.key);
		return 0;
	}
	line.answerLineId = atol(bracketContent.c_str());


	if (!GetBracketContent(fileLine)) {
		logger->error("Missing effect field on line: {} (dialog line {})", fileLine, line.key);
		return false;
	}
	line.effectField = bracketContent;

	// check non-blank gender field for NPC lines
	if (line.IsNpcLine()){

		auto fieldIsBlank= true;
		for (auto i = 0u; i < line.genderField.size(); i++) {
			if (!isspace(line.genderField[i])) {
				fieldIsBlank = false;
				break;
			}
		}

		// if it's blank, check that the txt field isn't also blank (e.g. when using the picker stuff)
		if (fieldIsBlank){

			fieldIsBlank = true;
			for (auto i = 0u; i < line.txt.size(); i++) {
				if (!isspace(line.txt[i])) {
					fieldIsBlank = false;
					break;
				}
			}

			if (!fieldIsBlank)
				logger->warn("Missing NPC response line for females: {} (dialog line {})", fileLine, line.key);
		}
	}
	
	return true;
}

bool DlgParser::GetBracketContent(int &fileLine){
	string brBuf;
	const int MAX_TEXT_SIZE = 1000;

	bool validLineFound = false;
	while (!validLineFound) {
		getline(ss, brBuf, '{');
		if (ss.eof()) { // no new brackets found
			return false;
		}
		validLineFound = true;

		//Check for a commented out line so it can be ignored
		auto eolnLocation = brBuf.rfind('\n');
		if (eolnLocation == std::string::npos) {
			eolnLocation = 0;
		}
		auto commentCharLocation = brBuf.find('/', eolnLocation);
		if (commentCharLocation != std::string::npos) {
			if (brBuf[commentCharLocation + 1] == '/') {

				//Fix the line count, stip the rest of the line and keep looking for more
				validLineFound = false;
				for (auto nl = strchr(brBuf.c_str(), '\n'); nl; fileLine++) {
					nl = strchr(nl + 1, '\n');
				}
				getline(ss, brBuf, '\n');
				if (ss.eof()) {  // no new brackets found
					return false;
				}
			}
		}
	}

	// count how many newlines
	for (auto nl = strchr(brBuf.c_str(), '\n'); nl; fileLine++) {
		nl = strchr(nl + 1, '\n');
	}
	auto rb = strchr(brBuf.c_str(), '}');
	if (rb){
		logger->warn("Possible missing left brace on or before line {}", fileLine);
	}
	
	
	// get bracket content
	getline(ss, brBuf, '}');
	if (ss.eof()) {
		logger->error("Unclosed bracket found! Line {}", fileLine);
		return false;
	}
	if (ss.gcount() >= MAX_TEXT_SIZE){
		logger->error("String too long on line: {} (max is {})", fileLine, MAX_TEXT_SIZE);
		return false;
	}
	// count how many newlines inside bracket
	for (auto nl = strchr(brBuf.c_str(), '\n'); nl; fileLine++) {
		nl = strchr(nl + 1, '\n');
	}
	
	bracketContent = brBuf;
	return true;
}

void DialogFile::GetLinesFromFile()
{
	if (!vfs->FileExists(this->filename))
		return;

	auto fileContents = vfs->ReadAsString(this->filename);
	DlgParser dlgParser(fileContents);
	
	int fileLine = 1;
	DialogLineNew lineTmp;
	while (dlgParser.GetSingleLine(lineTmp, fileLine)){

		if (this->lineCount == this->lineCapacity){
			this->lineCapacity += 10;
			this->lines = (DialogLine*)realloc(lines, lineCapacity * sizeof DialogLine);
		}
		this->lines[lineCount++] = lineTmp;

	}

	std::sort(this->lines, &this->lines[lineCount-1], [](DialogLine &a, DialogLine &b)->bool{
		return a.key < b.key;
	});
}

bool DialogLineNew::IsPcLine() {
	return minIq != 0;
}

bool DialogLineNew::IsNpcLine(){
	return !IsPcLine();
}

void DialogHooks::MapFirstSeenBanter(int mapId)
{
	auto npcCount = party.GroupNPCFollowersLen();
	char text[1000];
	int soundId = -1;
	static auto GetMapFirstSeenVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, int, char [], int&)>(0x10037FD0);

	// Temple+: now uses vector instead of fixes size[3] array
	std::vector<objHndl> commentNpcs;
	for (auto i = 0; i < npcCount; ++i) {
		auto npc = party.GroupNPCFollowersGetMemberN(i);
		auto fellowPc = party.GetFellowPc(npc);
		GetMapFirstSeenVoiceLine(npc, fellowPc, mapId, text, soundId);
		if (text[0]) {
			commentNpcs.push_back(npc);
		}
	}

	auto talker = objHndl::null;
	if (commentNpcs.size()) {
		auto idx = rngSys.GetInt(0, commentNpcs.size() - 1);
		talker = commentNpcs[idx];
	}
	else {
		auto idx = rngSys.GetInt(0, party.GroupPCsLen() - 1); // Temple+: was (0, N) instead of (0, N-1) in vanilla
		talker = party.GroupPCsGetMemberN(idx);
	}

	if (talker) {
		auto fellowPc = party.GetFellowPc(talker);
		GetMapFirstSeenVoiceLine(talker, fellowPc, mapId, text, soundId);
		logger->trace("Playing sound {} for talker {}, text {}", soundId, talker, text);
		critterSys.PlayCritterVoiceLine(talker, fellowPc, text, soundId);
	}

	// Append to seen map list
	static auto SeenMaplistAppend = temple::GetRef<void(__cdecl)(objHndl, int)>(0x10080390);
	auto partySize = party.GroupListGetLen();
	for (auto i = 0; i < partySize; ++i) {
		auto partyMem = party.GroupListGetMemberN(i);
		SeenMaplistAppend(partyMem, mapId);
	}
}

void DialogHooks::PlayVoice(objHndl handle, objHndl addressed, int soundId)
{
	if (soundId == -1) {
		return;
	}
	if (!objSystem->IsValidHandle(handle)) {
		return;
	}

	auto mActiveSpeech = temple::GetRef<int>(0x108EC85C);
	if (mActiveSpeech != -1) {
		sound.MssFreeStream(mActiveSpeech);
		mActiveSpeech = -1;
	}


	char filename[260] = { 0, };
	if (critterSys.IsPC(handle)) {
		auto voiceIdx = objects.getInt32(handle, obj_f_pc_voice_idx);
		sprintf(filename, "sound\\speech\\pcvoice\\%.2d\\%d.mp3", voiceIdx, soundId);
	}
	else {
		auto scriptId = (soundId >> 16) & 0x7FFF;
		auto mfChar = objects.StatLevelGet(addressed, stat_gender) != 0 ? 'm' : 'f';
		sprintf(filename, "sound\\speech\\%.5d\\v%d_%c.mp3", scriptId, (soundId & 0xFFFF) , mfChar);
		if (mfChar == 'f' && !tio_fileexists(filename, 0))
			sprintf(filename, "sound\\speech\\%.5d\\v%d_%c.mp3", scriptId, (unsigned __int16)soundId, 'm');
	}

	
	logger->trace("Playing sound file: {}", filename);
	static auto PlayVoiceFile = temple::GetRef<int(__cdecl)(const char*, int timeToFade)>(0x1003C5F0);
	if (tio_fileexists(filename, nullptr)) {
		mActiveSpeech = PlayVoiceFile(filename, 0);
	}
	else {
		logger->trace("Sound file not found!");
	}
}
