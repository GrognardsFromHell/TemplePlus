
#include "stdafx.h"

#include <infrastructure/exception.h>
#include <temple/dll.h>
#include "legacysystems.h"
#include <util/fixes.h>
#include "gamesystems.h"
#include "timeevents.h"
#include <graphics/device.h>
#include <graphics/camera.h>
#include <config/config.h>
#include <util/streams.h>
#include "objects/objevent.h"
#include <condition.h>
#include <sound.h>
#include <d20_level.h>
#include <damage.h>
#include <ui/ui_item_creation.h>
#include "d20/d20stats.h"
#include "deity/legacydeitysystem.h"
#include "ui/ui_systems.h"
#include "fade.h"
#include "objects/objsystem.h"
#include "infrastructure/vfs.h"
#include "infrastructure/elfhash.h"
#include "infrastructure/mesparser.h"
#include "legacymapsystems.h"


//*****************************************************************************
//* Vagrant
//*****************************************************************************

VagrantSystem::VagrantSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10086ae0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Vagrant");
	}
}
VagrantSystem::~VagrantSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10086b10);
	shutdown();
}
void VagrantSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10086cb0);
	advanceTime(time);
}
const std::string &VagrantSystem::GetName() const {
	static std::string name("Vagrant");
	return name;
}

//*****************************************************************************
//* Description
//*****************************************************************************

DescriptionSystem::DescriptionSystem(const GameSystemConf &config) {
	//auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100865d0);
	if (!description.Init(config)){
	//if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Description");
	}
}
DescriptionSystem::~DescriptionSystem() {
	/*auto shutdown = temple::GetPointer<void()>(0x10086670);
	shutdown();*/
	description.Exit();
}
void DescriptionSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10086710);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system Description");
	}
}
void DescriptionSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10086780);
	unloadModule();
}
void DescriptionSystem::Reset() {
	/*auto reset = temple::GetPointer<void()>(0x100866c0);
	reset();*/
	description.Reset();
}
bool DescriptionSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10086810);
	return save(file) == 1;
}
bool DescriptionSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100868b0);
	return load(saveFile) == 1;
}
const std::string &DescriptionSystem::GetName() const {
	static std::string name("Description");
	return name;
}

bool DescriptionSystem::ReadCustomNames(GameSystemSaveFile * file, std::vector<std::string>& customNamesOut){
	auto count = 0;
	if (!tio_fread(&count, sizeof(int), 1, file->file))
		return false;

	if (count <= 0)
		return true;

	for (auto i=0; i < count; i++){
		auto nameLen = 0;
		tio_fread(&nameLen, sizeof(int), 1, file->file);
		std::string tmpStr;
		tmpStr.resize(nameLen + 1);
		tio_fread(&tmpStr[0], sizeof(char), nameLen, file->file);
		tmpStr[nameLen] = 0;
		customNamesOut.push_back(tmpStr);
	}
	return true;

}

//*****************************************************************************
//* ItemEffect
//*****************************************************************************

ItemEffectSystem::ItemEffectSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100864d0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ItemEffect");
	}
}
ItemEffectSystem::~ItemEffectSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10086550);
	shutdown();
}
void ItemEffectSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10086560);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system ItemEffect");
	}
}
void ItemEffectSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x100865c0);
	unloadModule();
}
const std::string &ItemEffectSystem::GetName() const {
	static std::string name("ItemEffect");
	return name;
}

//*****************************************************************************
//* Teleport
//*****************************************************************************

TeleportSystem::TeleportSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10084a20);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Teleport");
	}
}
TeleportSystem::~TeleportSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10084fa0);
	shutdown();
}
void TeleportSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10084f60);
	reset();
}
void TeleportSystem::AdvanceTime(uint32_t time) {
	auto &fadeAndTeleportActive = temple::GetRef<BOOL>(0x10AB74C0);
	if (!fadeAndTeleportActive)
		return;

	auto &teleportProcessActive = temple::GetRef<BOOL>(0x10AB74B8);
	teleportProcessActive = 1;

	auto teleportProcess = temple::GetRef<void(__cdecl)(FadeAndTeleportArgs&)>(0x10085AA0);
	auto &teleportPacket = temple::GetRef<FadeAndTeleportArgs>(0x10AB74C8);
	teleportProcess(teleportPacket);

	teleportProcessActive = 0;
	fadeAndTeleportActive = 0;
	
	if (teleportPacket.flags & FadeAndTeleportFlags::ftf_unk80000000){
		if (teleportPacket.flags & FadeAndTeleportFlags::ftf_unk20){
			temple::GetRef<void(__cdecl)()>(0x100027E0)(); // GameEnableDrawing
		}
	}

	/*auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10086480);
	advanceTime(time);*/
}
const std::string &TeleportSystem::GetName() const {
	static std::string name("Teleport");
	return name;
}

//*****************************************************************************
//* Sector
//*****************************************************************************

SectorSystem::SectorSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10082db0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Sector");
	}
}
SectorSystem::~SectorSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10081bc0);
	shutdown();
}
void SectorSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10081bb0);
	reset();
}
bool SectorSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10081be0);
	return save(file) == 1;
}
bool SectorSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10081d20);
	return load(saveFile) == 1;
}
const std::string &SectorSystem::GetName() const {
	static std::string name("Sector");
	return name;
}

void SectorSystem::SetLimits(uint64_t limitX, uint64_t limitY)
{
	static auto set_sector_limit = temple::GetPointer<BOOL(uint64_t, uint64_t)>(0x10081940);
	set_sector_limit(limitX, limitY);
}

bool SectorSystem::ReadSectorTimes(GameSystemSaveFile * saveFile, std::vector<SectorTime>& sectorTimes){
	auto count = 0;
	if (!tio_fread(&count, sizeof(int), 1, saveFile->file))
		return false;
	if (count > 2 * config.sectorCacheSize)
		return false;
	sectorTimes.resize(count);
	if (!count)
		return true;

	if (tio_fread(&sectorTimes[0], sizeof(SectorTime), count, saveFile->file) != count)
		return false;
	return true;
}

//*****************************************************************************
//* Random
//*****************************************************************************

RandomSystem::RandomSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10039040);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Random");
	}
}
const std::string &RandomSystem::GetName() const {
	static std::string name("Random");
	return name;
}

//*****************************************************************************
//* Critter
//*****************************************************************************

CritterSystem::CritterSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007e310);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Critter");
	}
}
CritterSystem::~CritterSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1007e3f0);
	shutdown();
}
const std::string &CritterSystem::GetName() const {
	static std::string name("Critter");
	return name;
}

void CritterSystem::UpdateNpcHealingTimers()
{
	auto updateHealing = temple::GetPointer<void()>(0x10080490);
	updateHealing();
}

//*****************************************************************************
//* ScriptName
//*****************************************************************************

ScriptNameSystem::ScriptNameSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007e000);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ScriptName");
	}
}
ScriptNameSystem::~ScriptNameSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1007e0c0);
	shutdown();
}
void ScriptNameSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1007e0e0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system ScriptName");
	}
}
void ScriptNameSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x1007e1b0);
	unloadModule();
}
const std::string &ScriptNameSystem::GetName() const {
	static std::string name("ScriptName");
	return name;
}

//*****************************************************************************
//* Portrait
//*****************************************************************************

PortraitSystem::PortraitSystem(const GameSystemConf &config) {
	//auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007de10);
	if (!mesFuncs.Open("art\\interface\\portraits\\portraits.mes", &mPortraitsMes)){
		throw TempleException("Unable to initialize game system Portrait");
	}

	TioFileList flist;
	tio_filelist_create(&flist, "art\\interface\\portraits\\*");

	for (auto i=0; i < flist.count; i++){
		auto &fentry = flist.files[i];
		if (!(fentry.attribs & TioFileAttribs::TFA_SUBDIR))
			continue;
		if (!_strcmpi(fentry.name, ".") || !_strcmpi(fentry.name, ".."))
			continue;

		PortraitPack porPackNew;
		porPackNew.path = fmt::format("art\\interface\\portraits\\{}", fentry.name);
		porPackNew.key = ElfHash::Hash(fentry.name);
		if (porPackNew.key > 0 && porPackNew.key <= PORTRAIT_MAX_ID){
			porPackNew.key = ElfHash::Hash(fmt::format("{}a{}b{}c{}",fentry.name, fentry.name, fentry.name, fentry.name));
		}


		auto portFname = fmt::format("{}\\portraits.mes", porPackNew.path);
		if (!tio_fileexists(portFname.c_str()))
			continue;

		auto mesContent = MesFile::ParseFile(portFname);

		TioFileList portraitTgaFiles;
		tio_filelist_create(&portraitTgaFiles, fmt::format("{}\\*.tga", porPackNew.path).c_str());
		auto lastIdx = 0;
		for (auto it: mesContent){
			auto portraitFname = fmt::format("{}\\{}",porPackNew.path, it.second);
			//if (!tio_fileexists(portraitFname.c_str())) // so it doesn't list non-existant entries (i.e. stuff that's not in the extension folder)
			//	continue;
			auto foundFile = false;
			for (auto j= 0 ; j < portraitTgaFiles.count; j++){
				if (!_strcmpi(portraitTgaFiles.files[ (j + lastIdx) % portraitTgaFiles.count].name, it.second.c_str())){
					foundFile = true;
					lastIdx = j + lastIdx + 1;
					break;
				}
			}
			if (!foundFile)
				continue;

			porPackNew.packContents[it.first] = it.second;
		}

		tio_filelist_destroy(&portraitTgaFiles);
		mPortraitPacks.push_back(porPackNew);
	}

	tio_filelist_destroy(&flist);
}

PortraitSystem::~PortraitSystem() {
	/*auto shutdown = temple::GetPointer<void()>(0x1007de30);
	shutdown();*/
	mesFuncs.Close(mPortraitsMes);
}
const std::string &PortraitSystem::GetName() const {
	static std::string name("Portrait");
	return name;
}

bool PortraitSystem::GetFirstId(objHndl handle, int* idxOut) const {
	*idxOut = 0;

	MesLine line;
	if (!mesFuncs.GetFirstLine(mPortraitsMes, &line))
		return false;
	
	auto mesFindLine = temple::GetRef<BOOL(__cdecl)(MesHandle, MesLine*)>(0x101E6650);

	while (line.key % 10 || !IsPortraitFilenameValid(handle, line.value)){
		if (!mesFindLine(mPortraitsMes, &line))
			return false;
	}

	*idxOut = line.key;
	return true;
}

bool PortraitSystem::GetNextId(objHndl handle, int* idxOut) const {
	MesLine line(*idxOut);

	auto findNextLine = temple::GetRef<BOOL(__cdecl)(MesHandle, MesLine*)>(0x101E6650);

	MesHandle mh = mPortraitsMes;
	auto packKey = GetKeyFromId(line.key);

	auto moveToFirstPortraitPack = [&](){ // moves to first portrait pack (if any is found) after exhausting the "normal" portraits
		if (!mPortraitPacks.size())
			return false;
		if (!mPortraitPacks[0].packContents.size())
			return false;
		
		packKey = mPortraitPacks[0].key;
		*idxOut = mPortraitPacks[0].packContents.begin()->first ^ packKey;
		return true;
	};

	// normal portraits retrieval
	if (packKey == 0){
		if (!findNextLine(mh, &line)){
			return moveToFirstPortraitPack();
		}
			
		while (line.key % 10 || !IsPortraitFilenameValid(handle, line.value)) {
			if (!findNextLine(mh, &line)){
				return moveToFirstPortraitPack();
			}			
		}

		*idxOut = (line.key ^ packKey);
		return true;
	}


	auto moveToNextPortraitPack=  [&](){
		auto isNextOne = false;
		for (auto it : mPortraitPacks){
			if (isNextOne) {
				if (!it.packContents.size())
					continue;

				/// todo verify is multiple of 10
				*idxOut = it.packContents.begin()->first ^ it.key;
				return true;
			}

			if (it.key == packKey){
				isNextOne = true;
				continue;
			}
			
		}
		return false;
	};

	if (packKey != 0){
		for (auto it: mPortraitPacks){
			if (it.key != packKey)
				continue;
			// found the portrait pack from the id
			
			auto foundPortrait = false;

			auto dekey = *idxOut ^ packKey;

			auto nextId = it.packContents.find(dekey);
			do	{
				std::advance(nextId, 1);
			} while (nextId != it.packContents.end() && (nextId->first % 10) );
				

			if (nextId == it.packContents.end()){
				return moveToNextPortraitPack();
			}
				

			auto result = nextId->first;

			*idxOut = result ^ packKey;
			return true;
		}
	}

	return false;
}

int PortraitSystem::GetKeyFromId(int id) const{
	if (id < PORTRAIT_MAX_ID)
		return 0;

	for (auto it : mPortraitPacks){
		auto dekey = (int)(id ^ it.key);
		if (dekey > 0 && dekey < PORTRAIT_MAX_ID)
			return it.key;
	}
	return 0;
}

std::string PortraitSystem::GetPortraitFileFromId(int id, int subId){
	auto packKey = GetKeyFromId(id);
	auto result = fmt::format("art\\interface\\portraits\\");

	MesLine line(id + subId);
	if(!packKey){ // normal portraits.mes
		
		if (!mesFuncs.GetLine(mPortraitsMes, &line)) { // If not found, use TempMan
			line.key = 0 + subId; 
			mesFuncs.GetLine(mPortraitsMes, &line);
		}
		result.append(line.value);
		return result;
	}

	// get from new portrait pack
	for (auto it: mPortraitPacks){
		if (it.key != packKey)
			continue;
		auto dekey = id ^ packKey;
		auto portFind = it.packContents.find(dekey + subId);

		if (portFind == it.packContents.end()){ // not found, return TempMan
			line.key = 0 + subId;
			mesFuncs.GetLine(mPortraitsMes, &line);
			result.append(line.value);
			return result;
		}

		result = fmt::format("{}\\{}", it.path, portFind->second);
		return result;
	}

	// failsafe
	line.key = 0 + subId;
	mesFuncs.GetLine(mPortraitsMes, &line);
	result.append(line.value);
	return result;
}

bool PortraitSystem::IsPortraitFilenameValid(objHndl handle, const char* filename) {
	if (!filename || !*filename || !_strnicmp("TMP", filename, 3))
		return false;
	if (!_strnicmp("NPC", filename, 3) || !_strnicmp("MOO", filename, 3)) {
		return objSystem->GetObject(handle)->IsNPC();
	}
	return true;
}

//*****************************************************************************
//* Skill
//*****************************************************************************

SkillSystem::SkillSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007cfa0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Skill");
	}
}
SkillSystem::~SkillSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1007d0c0);
	shutdown();
}
bool SkillSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1007d110);
	return save(file) == 1;
}
bool SkillSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1007d0e0);
	return load(saveFile) == 1;
}
const std::string &SkillSystem::GetName() const {
	static std::string name("Skill");
	return name;
}

bool SkillSystem::ReadUnknown(GameSystemSaveFile * saveFile, int & unk){
	return tio_fread(&unk, sizeof(int), 1, saveFile->file) == 1;
}

//*****************************************************************************
//* Feat
//*****************************************************************************

FeatSystem::FeatSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007bfa0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Feat");
	}
}
FeatSystem::~FeatSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1007b900);
	shutdown();
}
const std::string &FeatSystem::GetName() const {
	static std::string name("Feat");
	return name;
}

//*****************************************************************************
//* Spell
//*****************************************************************************

SpellSystem::SpellSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1007b740);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Spell");
	}
	spellSys.Init(config);
}
SpellSystem::~SpellSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100791d0);
	mesFuncs.Close(spellSys.spellEnumsExt);
	mesFuncs.Close(spellSys.spellMesExt);
	shutdown();

}
void SpellSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100750f0);
	reset();
}
const std::string &SpellSystem::GetName() const {
	static std::string name("Spell");
	return name;
}

bool SpellSystem::Save(TioFile* file) {
	
	logger->debug("Saving Spells: {} spells initially in SpellsCastRegistry." , spellSys.spellCastIdxTable->itemCount);
	spellSys.SpellSavePruneInactive();
	logger->debug("Saving Spells: {} spells after pruning.", spellSys.spellCastIdxTable->itemCount);
	
	TioOutputStream tios(file);

	auto spellIdSerial = temple::GetPointer<int>(0x10AAF204);
	tios.WriteInt32(*spellIdSerial);
	// tio_fwrite(spellIdSerial, sizeof(int), 1, file);
	int numSpells = spellSys.spellCastIdxTable->itemCount;
	tios.WriteInt32(numSpells);
	/*if (!tio_fwrite(&numSpells, sizeof(int), 1, file))
		return FALSE;*/
	
	auto numSerialized = spellSys.SpellSave(tios);
	if (numSerialized != numSpells)
	{
		logger->error("Serialized wrong number of spells! SAVE IS CORRUPT! Serialized: {}, expected: {}", numSerialized, numSpells);
	}
	
	return TRUE;

	static auto spell_save = temple::GetPointer<BOOL(TioFile *)>(0x10079220);
	auto result = spell_save(file);
	return result == TRUE;
}

bool SpellSystem::Load(GameSystemSaveFile* file) {
	logger->info("Loading Spells: {} spells initially in SpellsCastRegistry.", spellSys.spellCastIdxTable->itemCount);
	static auto spell__spell_load = temple::GetPointer<BOOL(GameSystemSaveFile*)>(0x100792a0);
	
	
	auto spellIdSerial = temple::GetPointer<uint32_t>(0x10AAF204);
	tio_fread(spellIdSerial, sizeof(int), 1, file->file);
	
	uint32_t numSpells;
	if (tio_fread(&numSpells, 4, 1, file->file) != 1)
		return FALSE;

	Expects(numSpells >= 0);
	Expects(*spellIdSerial >= numSpells);
	
	if (numSpells <= 0)
		return TRUE;

	uint32_t spellId;
	SpellPacket pkt;

	for (uint32_t i = 0; i < numSpells; i++)	{
		if (spellSys.LoadActiveSpellElement(file->file, spellId, pkt) != 1 ){
			logger->warn("Loading Spells: Failure! {} spells in SpellsCastRegistry after loading.", spellSys.spellCastIdxTable->itemCount);
			return FALSE;
		}
		if (! (spellId <= *spellIdSerial)){
			logger->warn("Invalid spellId {} detected, greater than spellIdSerial!", spellId);
		}
		if (!pkt.spellPktBody.caster)	{
			logger->warn("Null caster object!", spellId);
		}
		spellSys.SpellsCastRegistryPut(spellId, pkt);
	}
	logger->info("Loading Spells: {} spells in SpellsCastRegistry after loading.", spellSys.spellCastIdxTable->itemCount);
	return TRUE;
	
	auto result = spell__spell_load(file);
	
	return result == TRUE;
}

//*****************************************************************************
//* Stat
//*****************************************************************************

StatSystem::StatSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10073680);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Stat");
	}
	d20Stats.Init(config); // registers the T+ overrides
}
StatSystem::~StatSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100739b0);
	shutdown();
}
const std::string &StatSystem::GetName() const {
	static std::string name("Stat");
	return name;
}

//*****************************************************************************
//* Script
//*****************************************************************************

ScriptSystem::ScriptSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10006580);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Script");
	}
}
ScriptSystem::~ScriptSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10007b60);
	shutdown();
}
void ScriptSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10006630);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system Script");
	}
}
void ScriptSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10006650);
	unloadModule();
}
void ScriptSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10007ae0);
	reset();
}
bool ScriptSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100066e0);
	return save(file) == 1;
}
bool ScriptSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10006670);
	return load(saveFile) == 1;
}
const std::string &ScriptSystem::GetName() const {
	static std::string name("Script");
	return name;
}

bool ScriptSystem::ReadGlobalVars(GameSystemSaveFile * saveFile, std::vector<int>& globalVars, std::vector<int>& globalFlagsData, int & storyState){
	globalVars.resize(2000);
	globalFlagsData.resize(100);
	if (!tio_fread(&globalVars[0], sizeof(int) * 2000, 1, saveFile->file)
		|| !tio_fread(&globalFlagsData[0], sizeof(int) * 100, 1, saveFile->file)
		|| !tio_fread(&storyState, sizeof(int), 1, saveFile->file)
		)
		return false;

	return true;
}

bool ScriptSystem::ReadEncounterQueue(GameSystemSaveFile * saveFile, std::vector<int>& encounterQueue){
	int count = 0;
	if (!tio_fread(&count, sizeof(int), 1, saveFile->file))
		return false;
	encounterQueue.resize(count);
	if (!count)
		return true;

	if (tio_fread(&encounterQueue[0], sizeof(int), count, saveFile->file) != count)
		return false;
	return true;
}

//*****************************************************************************
//* Level
//*****************************************************************************

LevelSystem::LevelSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10072f50);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Level");
	}
}
LevelSystem::~LevelSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10073180);
	shutdown();
}
const std::string &LevelSystem::GetName() const {
	static std::string name("Level");
	return name;
}

//*****************************************************************************
//* D20
//*****************************************************************************

D20System::D20System(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004c8a0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system D20");
	}
	conds.RegisterNewConditions();
	d20ClassSys.GetClassSpecs();
	d20LevelSys.GenerateSpellsPerLevelTables();
	damage.Init();
	d20Sys.GetPythonActionSpecs();
}
D20System::~D20System() {
	auto shutdown = temple::GetPointer<void()>(0x1004c950);
	shutdown();
	damage.Exit();
}
void D20System::Reset() {
	auto reset = temple::GetPointer<void()>(0x1004c9b0);
	reset();
}
void D20System::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x1004fc40);
	advanceTime(time);
}
const std::string &D20System::GetName() const {
	static std::string name("D20");
	return name;
}

void D20System::RemoveDispatcher(objHndl obj)
{
	using FnRemoveDispatcher = void(objHndl);
	static FnRemoveDispatcher* removeDispatcher = temple::GetPointer<FnRemoveDispatcher>(0x1004FEE0);
	removeDispatcher(obj);
}

void D20System::ResetRadialMenus()
{
	static auto radialmenu_reset = temple::GetPointer<void()>(0x100eff40);
	radialmenu_reset();
}

//*****************************************************************************
//* LightScheme
//*****************************************************************************

LightSchemeSystem::LightSchemeSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1006ef30);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system LightScheme");
	}
}
LightSchemeSystem::~LightSchemeSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1006ef80);
	shutdown();
}
void LightSchemeSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1006f440);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system LightScheme");
	}
}
void LightSchemeSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x1006ef50);
	unloadModule();
}
void LightSchemeSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1006f430);
	reset();
}
bool LightSchemeSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1006ef90);
	return save(file) == 1;
}
bool LightSchemeSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1006f470);
	return load(saveFile) == 1;
}
const std::string &LightSchemeSystem::GetName() const {
	static std::string name("LightScheme");
	return name;
}

void LightSchemeSystem::SetLightSchemeId(int schemeId)
{
	static auto set_map_lightscheme_id = temple::GetPointer<BOOL(int)>(0x1006efd0);
	set_map_lightscheme_id(schemeId);
}

void LightSchemeSystem::SetLightScheme(int schemeId, int hour)
{
	static auto set_lightscheme = temple::GetPointer<signed int(int lightSchemeId, int hourOfDay)>(0x1006f350);
	set_lightscheme(schemeId, hour);
}

int LightSchemeSystem::GetHourOfDay()
{
	static auto lightscheme_get_hour = temple::GetPointer<int()>(0x1006f0b0);
	return lightscheme_get_hour();
}

//*****************************************************************************
//* Player
//*****************************************************************************

PlayerSystem::PlayerSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1006ede0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Player");
	}
}
PlayerSystem::~PlayerSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1006ee40);
	shutdown();
}
void PlayerSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1006ee00);
	reset();
}
bool PlayerSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x101f5850);
	return save(file) == 1;
}
bool PlayerSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x101f5850);
	return load(saveFile) == 1;
}
const std::string &PlayerSystem::GetName() const {
	static std::string name("Player");
	return name;
}

//*****************************************************************************
//* Area
//*****************************************************************************

AreaSystem::AreaSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1006e550);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Area");
	}
}
void AreaSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1006e590);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system Area");
	}
}
void AreaSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x1006e860);
	unloadModule();
}
void AreaSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1006e560);
	reset();
}
bool AreaSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1006e920);
	return save(file) == 1;
}
bool AreaSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1006e8d0);
	return load(saveFile) == 1;
}
const std::string &AreaSystem::GetName() const {
	static std::string name("Area");
	return name;
}

//*****************************************************************************
//* Dialog
//*****************************************************************************

DialogSystem::DialogSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10036040);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Dialog");
	}
}
DialogSystem::~DialogSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10036080);
	shutdown();
}
const std::string &DialogSystem::GetName() const {
	static std::string name("Dialog");
	return name;
}

//*****************************************************************************
//* SoundMap
//*****************************************************************************

SoundMapSystem::SoundMapSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1006ded0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system SoundMap");
	}
}
const std::string &SoundMapSystem::GetName() const {
	static std::string name("SoundMap");
	return name;
}

//*****************************************************************************
//* SoundGame
//*****************************************************************************

SoundGameSystem::SoundGameSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1003d4a0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system SoundGame");
	}

	sound.Init(); // init user_sounds extensions
}
SoundGameSystem::~SoundGameSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1003bb10);
	shutdown();
}
void SoundGameSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1003bb80);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system SoundGame");
	}

	mesFuncs.Open("tpmes\\sounds.mes", &sound.tpSounds);
}
void SoundGameSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x1003bbc0);
	unloadModule();

	mesFuncs.Close(sound.tpSounds);
}
void SoundGameSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1003cb30);
	reset();
}
bool SoundGameSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1003bbd0);
	return save(file) == 1;
}
bool SoundGameSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1003cb70);
	return load(saveFile) == 1;
}
void SoundGameSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x1003dc50);
	advanceTime(time);
}
const std::string &SoundGameSystem::GetName() const {
	static std::string name("SoundGame");
	return name;
}

void SoundGameSystem::SetSoundSchemeIds(int scheme1, int scheme2)
{
	static auto soundscheme_set = temple::GetPointer<void(int, int)>(0x1003c4d0);
	soundscheme_set(scheme1, scheme2);
}

void SoundGameSystem::StopAll(bool flag)
{
	static auto soundgame_stop_all = temple::GetPointer<void(int a1)>(0x1003c5b0);
	soundgame_stop_all(flag ? TRUE : FALSE);
}

//*****************************************************************************
//* Item
//*****************************************************************************

ItemSystem::ItemSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10063c70);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Item");
	}
}
ItemSystem::~ItemSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10063dc0);
	shutdown();
}
void ItemSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10063df0);
	resetBuffers(&rebuildInfo);
}
const std::string &ItemSystem::GetName() const {
	static std::string name("Item");
	return name;
}

//*****************************************************************************
//* Combat
//*****************************************************************************

CombatSystem::CombatSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10063ba0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Combat");
	}
}
CombatSystem::~CombatSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10062eb0);
	shutdown();
}
void CombatSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10062ed0);
	reset();
}
bool CombatSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10062440);
	return save(file) == 1;
}
bool CombatSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10062470);
	return load(saveFile) == 1;
}
void CombatSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10062e20);
	advanceTime(time);
}
const std::string &CombatSystem::GetName() const {
	static std::string name("Combat");
	return name;
}

//*****************************************************************************
//* Rumor
//*****************************************************************************

RumorSystem::RumorSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1005f960);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Rumor");
	}
}
RumorSystem::~RumorSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1005f9d0);
	shutdown();
}
void RumorSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x101f5850);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system Rumor");
	}
}
bool RumorSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x101f5850);
	return save(file) == 1;
}
bool RumorSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x101f5850);
	return load(saveFile) == 1;
}
const std::string &RumorSystem::GetName() const {
	static std::string name("Rumor");
	return name;
}

//*****************************************************************************
//* Quest
//*****************************************************************************

QuestSystem::QuestSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1005f660);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Quest");
	}
}
QuestSystem::~QuestSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1005f2d0);
	shutdown();
}
void QuestSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1005f310);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system Quest");
	}
}
void QuestSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1005f2a0);
	reset();
}
bool QuestSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1005f3a0);
	return save(file) == 1;
}
bool QuestSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1005f320);
	return load(saveFile) == 1;
}
const std::string &QuestSystem::GetName() const {
	static std::string name("Quest");
	return name;
}

//*****************************************************************************
//* AI
//*****************************************************************************

AISystem::AISystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10056d50);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system AI");
	}
}
void AISystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10056e30);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system AI");
	}
}
const std::string &AISystem::GetName() const {
	static std::string name("AI");
	return name;
}

void AISystem::AddAiTimer(objHndl handle)
{
	static auto ai_schedule_npc_timer = temple::GetPointer<void(objHndl)>(0x1005d5e0);
	ai_schedule_npc_timer(handle);
}

//*****************************************************************************
//* AnimPrivate
//*****************************************************************************

AnimPrivateSystem::AnimPrivateSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10055280);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system AnimPrivate");
	}
}
AnimPrivateSystem::~AnimPrivateSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100552f0);
	shutdown();
}
void AnimPrivateSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10054dd0);
	reset();
}
const std::string &AnimPrivateSystem::GetName() const {
	static std::string name("AnimPrivate");
	return name;
}

//*****************************************************************************
//* Reputation
//*****************************************************************************

ReputationSystem::ReputationSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10054b00);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Reputation");
	}
}
ReputationSystem::~ReputationSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10054240);
	shutdown();
}
void ReputationSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100542a0);
	reset();
}
bool ReputationSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100542d0);
	return save(file) == 1;
}
bool ReputationSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100542f0);
	return load(saveFile) == 1;
}
const std::string &ReputationSystem::GetName() const {
	static std::string name("Reputation");
	return name;
}

//*****************************************************************************
//* Reaction
//*****************************************************************************

ReactionSystem::ReactionSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10053bd0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Reaction");
	}
}
ReactionSystem::~ReactionSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10053c50);
	shutdown();
}
const std::string &ReactionSystem::GetName() const {
	static std::string name("Reaction");
	return name;
}

//*****************************************************************************
//* TileScript
//*****************************************************************************

TileScriptSystem::TileScriptSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10053980);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system TileScript");
	}
}
TileScriptSystem::~TileScriptSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100539d0);
	shutdown();
}
const std::string &TileScriptSystem::GetName() const {
	static std::string name("TileScript");
	return name;
}

//*****************************************************************************
//* SectorScript
//*****************************************************************************

SectorScriptSystem::SectorScriptSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x101f5850);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system SectorScript");
	}
}
const std::string &SectorScriptSystem::GetName() const {
	static std::string name("SectorScript");
	return name;
}

//*****************************************************************************
//* WP
//*****************************************************************************

WPSystem::WPSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100533c0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system WP");
	}
}
WPSystem::~WPSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10053410);
	shutdown();
}
void WPSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10053430);
	resetBuffers(&rebuildInfo);
}
const std::string &WPSystem::GetName() const {
	static std::string name("WP");
	return name;
}

//*****************************************************************************
//* InvenSource
//*****************************************************************************

InvenSourceSystem::InvenSourceSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10053220);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system InvenSource");
	}
}
InvenSourceSystem::~InvenSourceSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100525f0);
	shutdown();
}
const std::string &InvenSourceSystem::GetName() const {
	static std::string name("InvenSource");
	return name;
}

//*****************************************************************************
//* TownMap
//*****************************************************************************

void TownMapSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10051cd0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system TownMap");
	}
}
void TownMapSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10052130);
	unloadModule();
}
void TownMapSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10052100);
	reset();
}
const std::string &TownMapSystem::GetName() const {
	static std::string name("TownMap");
	return name;
}

//*****************************************************************************
//* GMovie
//*****************************************************************************

void GMovieSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x10033d90);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system GMovie");
	}
}
void GMovieSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10033dc0);
	unloadModule();
}
const std::string &GMovieSystem::GetName() const {
	static std::string name("GMovie");
	return name;
}

//*****************************************************************************
//* Brightness
//*****************************************************************************

BrightnessSystem::BrightnessSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10051ca0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Brightness");
	}
}
const std::string &BrightnessSystem::GetName() const {
	static std::string name("Brightness");
	return name;
}

//*****************************************************************************
//* GFade
//*****************************************************************************

GFadeSystem::GFadeSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100519e0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system GFade");
	}
}
void GFadeSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10051a10);
	advanceTime(time);
}
const std::string &GFadeSystem::GetName() const {
	static std::string name("GFade");
	return name;
}

//*****************************************************************************
//* AntiTeleport
//*****************************************************************************

AntiTeleportSystem::AntiTeleportSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10051830);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system AntiTeleport");
	}
}
AntiTeleportSystem::~AntiTeleportSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10051870);
	shutdown();
}
void AntiTeleportSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x100518c0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system AntiTeleport");
	}
}
void AntiTeleportSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10051990);
	unloadModule();
}
const std::string &AntiTeleportSystem::GetName() const {
	static std::string name("AntiTeleport");
	return name;
}

//*****************************************************************************
//* Trap
//*****************************************************************************

TrapSystem::TrapSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10050da0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Trap");
	}
}
TrapSystem::~TrapSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10050940);
	shutdown();
}
const std::string &TrapSystem::GetName() const {
	static std::string name("Trap");
	return name;
}

//*****************************************************************************
//* MonsterGen
//*****************************************************************************

MonsterGenSystem::MonsterGenSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100500c0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system MonsterGen");
	}
}
MonsterGenSystem::~MonsterGenSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10050160);
	shutdown();
}
void MonsterGenSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10050170);
	resetBuffers(&rebuildInfo);
}
void MonsterGenSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10050140);
	reset();
}
bool MonsterGenSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100501d0);
	return save(file) == 1;
}
bool MonsterGenSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100501a0);
	return load(saveFile) == 1;
}
const std::string &MonsterGenSystem::GetName() const {
	static std::string name("MonsterGen");
	return name;
}

//*****************************************************************************
//* Party
//*****************************************************************************

PartySystem::PartySystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1002b9d0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Party");
	}
}
void PartySystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1002ac00);
	reset();
}
bool PartySystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1002ac70);
	return save(file) == 1;
}
bool PartySystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1002ad80);
	return load(saveFile) == 1;
}
const std::string &PartySystem::GetName() const {
	static std::string name("Party");
	return name;
}

void PartySystem::SaveCurrent()
{
	auto saveCurrent = temple::GetPointer<void()>(0x1002BA40);
	saveCurrent();
}

void PartySystem::RestoreCurrent()
{
	auto restoreCurrent = temple::GetPointer<void()>(0x1002AEA0);
	restoreCurrent();
}

bool PartySystem::IsInParty(objHndl obj) const
{
	static auto IsInParty = temple::GetPointer<BOOL(objHndl)>(0x1002b1b0);
	return IsInParty(obj) == TRUE;
}

void PartySystem::ForEachInParty(std::function<void(objHndl)> callback) {
	static auto party_size = temple::GetPointer<size_t()>(0x1002b2b0);
	static auto party_get = temple::GetPointer<objHndl(size_t)>(0x1002b150);

	auto count = party_size();
	for (size_t i = 0; i < count; ++i) {
		auto handle = party_get(i);
		callback(handle);
	}

}

//*****************************************************************************
//* D20LoadSave
//*****************************************************************************

bool D20LoadSaveSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1004fb70);
	return save(file) == 1;
}
bool D20LoadSaveSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1004fbd0);
	return load(saveFile) == 1;
}
const std::string &D20LoadSaveSystem::GetName() const {
	static std::string name("D20LoadSave");
	return name;
}

//*****************************************************************************
//* GameInit
//*****************************************************************************

GameInitSystem::GameInitSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004c610);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system GameInit");
	}
}
GameInitSystem::~GameInitSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1004c690);
	shutdown();
}
void GameInitSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x1004c6a0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system GameInit");
	}
}
void GameInitSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x1004c850);
	unloadModule();
}
void GameInitSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1004c660);
	reset();
}
const std::string &GameInitSystem::GetName() const {
	static std::string name("GameInit");
	return name;
}

//*****************************************************************************
//* ObjFade
//*****************************************************************************

ObjFadeSystem::ObjFadeSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004c130);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ObjFade");
	}
}
ObjFadeSystem::~ObjFadeSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1004c170);
	shutdown();
}
void ObjFadeSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1004c190);
	reset();
}
bool ObjFadeSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1004c1c0);
	return save(file) == 1;
}
bool ObjFadeSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1004c220);
	return load(saveFile) == 1;
}
const std::string &ObjFadeSystem::GetName() const {
	static std::string name("ObjFade");
	return name;
}

//*****************************************************************************
//* Deity
//*****************************************************************************

DeitySystem::DeitySystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004a760);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Deity");
	}
	deitySys.Init();
}
DeitySystem::~DeitySystem() {
	auto shutdown = temple::GetPointer<void()>(0x1004a800);
	shutdown();
}
const std::string &DeitySystem::GetName() const {
	static std::string name("Deity");
	return name;
}

//*****************************************************************************
//* UiArtManager
//*****************************************************************************

UiArtManagerSystem::UiArtManagerSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004a610);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system UiArtManager");
	}
}
UiArtManagerSystem::~UiArtManagerSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1004a250);
	shutdown();
}
const std::string &UiArtManagerSystem::GetName() const {
	static std::string name("UiArtManager");
	return name;
}

//*****************************************************************************
//* Cheats
//*****************************************************************************

CheatsSystem::CheatsSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10048a60);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Cheats");
	}
}
const std::string &CheatsSystem::GetName() const {
	static std::string name("Cheats");
	return name;
}

//*****************************************************************************
//* D20Rolls
//*****************************************************************************

D20RollsSystem::D20RollsSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100475d0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system D20Rolls");
	}
}
D20RollsSystem::~D20RollsSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10047150);
	shutdown();
}
void D20RollsSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10047160);
	reset();
}
bool D20RollsSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100471a0);
	return save(file) == 1;
}
bool D20RollsSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100471e0);
	return load(saveFile) == 1;
}
const std::string &D20RollsSystem::GetName() const {
	static std::string name("D20Rolls");
	return name;
}

//*****************************************************************************
//* Secretdoor
//*****************************************************************************

SecretdoorSystem::SecretdoorSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10046370);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Secretdoor");
	}
}
void SecretdoorSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10046390);
	reset();
}
bool SecretdoorSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100463b0);
	return save(file) == 1;
}
bool SecretdoorSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10046400);
	return load(saveFile) == 1;
}
const std::string &SecretdoorSystem::GetName() const {
	static std::string name("Secretdoor");
	return name;
}

//*****************************************************************************
//* MapFogging
//*****************************************************************************

MapFoggingSystem::MapFoggingSystem(gfx::RenderingDevice &device) : mDevice(device) {

	mFogCheckData = nullptr;
	
	mFoggingEnabled = true;
	for (size_t i = 0; i < 8; i++) {
		mFogBuffers[i] = malloc(16 * sFogBufferDim * sFogBufferDim);
	}

	InitScreenBuffers();
	
	mEsdLoaded = 0;
	memset(&mEsdSectorLocs[0], 0, 32 * sizeof(uint64_t));

	config.AddVanillaSetting("fog checks", "1", [&]() {
		mFogChecks = config.GetVanillaInt("fog checks");
	});
}

MapFoggingSystem::~MapFoggingSystem() {
	if (mFoggingEnabled) {
		free(mFogCheckData);
		for (size_t i = 0; i < 8; ++i) {
			free(mFogBuffers[i]);
		}
	}
}
void MapFoggingSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	InitScreenBuffers();
}

void MapFoggingSystem::Reset() {
	// Previously: 1002EBD0
	mEsdLoaded = 0;
	memset(&mEsdSectorLocs[0], 0, 32 * sizeof(uint64_t));
	mDoFullUpdate = false;
}

const std::string &MapFoggingSystem::GetName() const {
	static std::string name("MapFogging");
	return name;
}

void MapFoggingSystem::LoadFogColor(const std::string & dataDir)
{
	static auto loadFogColor = temple::GetPointer<void(const char*)>(0x10030BF0);
	loadFogColor(dataDir.c_str());
}

void MapFoggingSystem::Enable()
{
	static auto map_fogging_enable = temple::GetPointer<void()>(0x1002ec80);
	map_fogging_enable();
}

void MapFoggingSystem::Disable()
{
	static auto map_fogging_disable = temple::GetPointer<void()>(0x1002ec90);
	map_fogging_disable();
}

void MapFoggingSystem::LoadExploredTileData(int mapId)
{
	static auto map_fogging_load_etd = temple::GetPointer<void(int)>(0x10030d10);
	map_fogging_load_etd(mapId);
}

void MapFoggingSystem::SaveExploredTileData(int mapId) {
	static auto map_fogging_save_etd = temple::GetPointer<void(int)>(0x10030e20);
	map_fogging_save_etd(mapId);
}

void MapFoggingSystem::SaveEsd() {
	static auto map_flush_esd = temple::GetPointer<void()>(0x10030f40);
	map_flush_esd();
}

void MapFoggingSystem::PerformCheckForCritter(objHndl handle, int idx){
	memset(&mFogBuffers[idx], 0, 4 * sFogBufferDim* sFogBufferDim);
	auto obj = objSystem->GetObject(handle);
	auto objLoc = obj->GetLocation();

	auto fogBufferDim_div3 = sFogBufferDim / 3;
	int64_t (& objsRelX)[] = temple::GetRef<int64_t[]>(0x1080FB88);
	int64_t (& objsRelY)[] = temple::GetRef<int64_t[]>(0x108EC550);

	objsRelX[idx] = objLoc.locx - fogBufferDim_div3;
	objsRelY[idx] = objLoc.locy - fogBufferDim_div3;

	TileRect tiles;
	tiles.x1 = objLoc.locx - fogBufferDim_div3;
	tiles.x2 = objLoc.locx + fogBufferDim_div3;
	tiles.y1 = objLoc.locy - fogBufferDim_div3;
	tiles.y2 = objLoc.locy + fogBufferDim_div3;

	auto sectorList = sectorSys.BuildSectorList(&tiles);
	if (!sectorList)
		return;

	auto listNode = sectorList;

	while (listNode){

		Sector *sect;
		if (!sectorSys.SectorLock(listNode->sector, &sect)){
			listNode = listNode->next;
			continue;
		}
		auto secLoc = listNode->sector;
		auto baseTile=secLoc.GetBaseTile();

		auto svb = gameSystems->GetSectorVB().GetSvb(secLoc);
		auto relX = objsRelX[idx];
		auto relY = objsRelY[idx];


		// unlock sector TODO svb
		sectorSys.SectorUnlock(listNode->sector);

		listNode = listNode->next;
	}

	sectorSys.SectorListReturnToPool(sectorList);

}

void MapFoggingSystem::InitScreenBuffers() {

	mScreenWidth = config.renderWidth;
	mScreenHeight = config.renderHeight;

	// Calculate the tile locations in each corner of the screen
	auto topLeftLoc = mDevice.GetCamera().ScreenToTile(0, 0);
	auto topRightLoc = mDevice.GetCamera().ScreenToTile(mScreenWidth, 0);
	auto bottomLeftLoc = mDevice.GetCamera().ScreenToTile(0, mScreenHeight);
	auto bottomRightLoc = mDevice.GetCamera().ScreenToTile(mScreenWidth, mScreenHeight);

	mFogMinX = topRightLoc.location.locx;
	mFogMinY = topLeftLoc.location.locy;

	// Whatever the point of this may be ...
	if (topLeftLoc.off_y < topLeftLoc.off_x || topLeftLoc.off_y < -topLeftLoc.off_x) {
		mFogMinY--;
	}

	mSubtilesX = (bottomLeftLoc.location.locx - mFogMinX + 3) * 3;
	mSubtilesY = (bottomRightLoc.location.locy - mFogMinY + 3) * 3;

	mFogCheckData = (uint8_t*)malloc((size_t)(mSubtilesX * mSubtilesY));
	memset(mFogCheckData, 0, (size_t)(mSubtilesX * mSubtilesY));
	
	mDoFullUpdate = TRUE;
}

//*****************************************************************************
//* RandomEncounter
//*****************************************************************************

RandomEncounterSystem::RandomEncounterSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100457b0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system RandomEncounter");
	}
}
RandomEncounterSystem::~RandomEncounterSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10045830);
	shutdown();
}
bool RandomEncounterSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x101f5850);
	return save(file) == 1;
}
bool RandomEncounterSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100458c0);
	return load(saveFile) == 1;
}
const std::string &RandomEncounterSystem::GetName() const {
	static std::string name("RandomEncounter");
	return name;
}

//*****************************************************************************
//* ObjectEvent
//*****************************************************************************

ObjectEventSystem::ObjectEventSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10045110);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ObjectEvent");
	}
}
ObjectEventSystem::~ObjectEventSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10045140);
	shutdown();
}
void ObjectEventSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10045160);
	reset();
}
bool ObjectEventSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100456d0);
	return save(file) == 1;
}
bool ObjectEventSystem::LoadGame(GameSystemSaveFile* saveFile) {
	/*auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100451b0);
	auto result  = load(saveFile) == 1;*/

	return objEvents.ObjEventLoadGame(saveFile) == TRUE;
}
void ObjectEventSystem::AdvanceTime(uint32_t time) {
	//auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10045740);
	//advanceTime(time);
	objEvents.AdvanceTime();
}
const std::string &ObjectEventSystem::GetName() const {
	static std::string name("ObjectEvent");
	return name;
}

//*****************************************************************************
//* Formation
//*****************************************************************************

FormationSystem::FormationSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100437c0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Formation");
	}
}
void FormationSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10043250);
	reset();
}
bool FormationSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x10043270);
	return save(file) == 1;
}
bool FormationSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x100432e0);
	return load(saveFile) == 1;
}
const std::string &FormationSystem::GetName() const {
	static std::string name("Formation");
	return name;
}

//*****************************************************************************
//* ItemHighlight
//*****************************************************************************

ItemHighlightSystem::ItemHighlightSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100431b0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ItemHighlight");
	}
}
void ItemHighlightSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100431d0);
	reset();
}
void ItemHighlightSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100431f0);
	advanceTime(time);
}
const std::string &ItemHighlightSystem::GetName() const {
	static std::string name("ItemHighlight");
	return name;
}

//*****************************************************************************
//* PathX
//*****************************************************************************

PathXSystem::PathXSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10042a90);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system PathX");
	}
}
void PathXSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10042aa0);
	reset();
}
const std::string &PathXSystem::GetName() const {
	static std::string name("PathX");
	return name;
}
