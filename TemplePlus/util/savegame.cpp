
#include "stdafx.h"

#include <infrastructure/exception.h>

#include "savegame.h"
#include <infrastructure/vfs.h>

enum class ArchiveEntryType : uint32_t {
	File = 0,
	StartFolder = 1,
	EndFolder = 2,
	End = 3
};

void SaveGameArchive::Create(const std::string& folder, const std::string& filename) {

	if (!vfs->DirExists(folder)) {
		throw TempleException("Cannot create savegame archive from {}, because the folder does not exist.",
			folder);
	}

	auto indexFilename = fmt::format("{}.tfai", filename);
	auto dataFilename = fmt::format("{}.tfaf", filename);

	VfsOutputStream indexStream(indexFilename);
	VfsOutputStream dataStream(dataFilename);

	AddFolder(folder, indexStream, dataStream);

	indexStream.WriteUInt32((uint32_t) ArchiveEntryType::End);

}

void SaveGameArchive::Unpack(const std::string& filename, const std::string& folder) {

	auto indexFilename = fmt::format("{}.tfai", filename);
	auto dataFilename = fmt::format("{}.tfaf", filename);

	auto indexContent = vfs->ReadAsBinary(indexFilename);
	auto dataContent = vfs->ReadAsBinary(dataFilename);

	MemoryInputStream indexStream(indexContent);
	MemoryInputStream dataStream(dataContent);
	
	auto entryType = (ArchiveEntryType)indexStream.ReadUInt32();
	while (entryType != ArchiveEntryType::End) {
		
		if (entryType == ArchiveEntryType::File) {
			UnpackFile(folder, indexStream, dataStream);
		} else if (entryType == ArchiveEntryType::StartFolder) {
			UnpackFolder(folder, indexStream, dataStream);
		} else {
			throw TempleException("Unexpected entry type {} in savegame archive", (int)entryType);
		}
		
		entryType = (ArchiveEntryType)indexStream.ReadUInt32();
	}

}

void SaveGameArchive::AddFolder(const std::string& folder, OutputStream& indexStream, OutputStream& dataStream) {

	auto globPattern = fmt::format("{}\\*.*", folder);
	for (auto result : vfs->Search(globPattern)) {				
		auto path = fmt::format("{}\\{}", folder, result.filename);

		if (result.dir) {
			indexStream.WriteUInt32((uint32_t)ArchiveEntryType::StartFolder);
			indexStream.WriteStringPrefixed(result.filename);

			// Recurse into the directories content
			AddFolder(path, indexStream, dataStream);

			indexStream.WriteUInt32((uint32_t)ArchiveEntryType::EndFolder);
		} else {
			indexStream.WriteUInt32((uint32_t)ArchiveEntryType::File);
			indexStream.WriteStringPrefixed(result.filename);
			indexStream.WriteUInt32(result.sizeInBytes);

			auto content = vfs->ReadAsBinary(path);
			dataStream.WriteBytes(content.data(), content.size());
		}
	}

}

void SaveGameArchive::UnpackFolder(const std::string& folder, InputStream& indexStream, InputStream& dataStream) {
	auto subfolder = indexStream.ReadStringPrefixed();
	auto path = fmt::format("{}\\{}", folder, subfolder);
	
	if (!vfs->DirExists(path) && !vfs->MkDir(path)) {
		throw TempleException("Cannot create directory {} while unpacking savegame archive.");
	}

	auto entryType = (ArchiveEntryType)indexStream.ReadUInt32();
	while (entryType != ArchiveEntryType::EndFolder) {

		if (entryType == ArchiveEntryType::File) {
			UnpackFile(path, indexStream, dataStream);
		} else if (entryType == ArchiveEntryType::StartFolder) {
			UnpackFolder(path, indexStream, dataStream);
		} else {
			throw TempleException("Unexpected entry type {} in savegame archive", (int)entryType);
		}

		entryType = (ArchiveEntryType)indexStream.ReadUInt32();
	}

}

void SaveGameArchive::UnpackFile(const std::string& folder, InputStream& indexStream, InputStream& dataStream) {
	auto file = indexStream.ReadStringPrefixed();
	auto path = fmt::format("{}\\{}", folder, file);
	size_t fileSize = indexStream.ReadUInt32();

	VfsOutputStream fileOut(path);
	dataStream.CopyTo(fileOut, fileSize);
}

void SaveGame::Load(const std::string &path) {

	auto dataSavFilename = fmt::format("{}\\data.sav", path);
	auto dataSavData = vfs->ReadAsBinary(dataSavFilename);
	
	MemoryInputStream dataSavInput(dataSavData);
	LoadDataSav(dataSavInput);

}

template<typename T>
static map<int32_t, T> LoadIdxTable(InputStream& stream) {
	auto magicNumber = stream.ReadUInt32();
	if (magicNumber != 0xAB1EE1BA) {
		throw TempleException("Unable to read the idx table header");
	}

	auto bucketCount = stream.ReadUInt32();
	auto dataSize = stream.ReadUInt32();
	
	std::map<int32_t, T> result;
	for (size_t bucket = 0; bucket < bucketCount; ++bucket) {
		size_t nodeCount = stream.ReadUInt32();

		for (size_t i = 0; i < nodeCount; ++i) {
			auto key = stream.ReadInt32();
			T value;
			stream.ReadBytes(reinterpret_cast<uint8_t*>(&value), sizeof(value));
			result[key] = value;
		}
	}

	magicNumber = stream.ReadUInt32();
	if (magicNumber != 0xE1BAAB1E) {
		throw TempleException("Unable to read the idx table footer");
	}

	return result;

}

static bool EndsWithPythonEndSentinel(std::vector<char> &str) {
	if (str.size() < 4) {
		return false;
	}
	auto len = str.size();
	return str[len - 1] == 0x9B
		&& str[len - 2] == 0xBA
		&& str[len - 3] == 0xDD
		&& str[len - 4] == 0xD5;
}

void SaveGame::LoadDataSav(InputStream &in) {

	auto version = in.ReadUInt32();
	if (version != 0) {
		throw TempleException("Savegame has invalid version: {}", version);
	}

	auto ironman = in.ReadUInt32();
	mIronman = (ironman != 0);
	if (mIronman) {
		mIronmanSaveNumber = in.ReadUInt32();
		mIronmanSaveName = in.ReadZStringPrefixed();
	}

	// Gamesystem "description"
	auto descCount = in.ReadUInt32();
	mCustomDescriptions.reserve(descCount);
	for (size_t i = 0; i < descCount; i++) {
		mCustomDescriptions.push_back(in.ReadStringPrefixed());
	}
	ReadSystemFooter(in, "description");

	// Gamesystem "sector"
	auto secTimesCount = in.ReadUInt32();
	mSectorTimes.reserve(secTimesCount);
	for (size_t i = 0; i < secTimesCount; ++i) {
		SectorTime record;
		record.secLoc.raw = in.ReadUInt64();
		record.gameTime.timeInDays = in.ReadUInt32();
		record.gameTime.timeInMs = in.ReadUInt32();
		mSectorTimes.push_back(record);
	}
	ReadSystemFooter(in, "sector");

	// Gamesystem "skill"
	in.ReadUInt32(); // Seems to be an unused value

	ReadSystemFooter(in, "skill");

	// Gamesystem "script"
	mGlobalVars.resize(8000);
	in.ReadBytes(&mGlobalVars[0], 8000);
	mGlobalFlags.resize(400);
	in.ReadBytes(&mGlobalFlags[0], 400);
	mStoryState = in.ReadUInt32();

	auto encounterCount = in.ReadUInt32();
	mEncounterQueue.reserve(encounterCount);
	for (size_t i = 0; i < encounterCount; ++i) {
		mEncounterQueue.push_back(in.ReadInt32());
	}

	ReadSystemFooter(in, "script");

	mMapDataPath = in.ReadLine(260);
	mMapSavePath = in.ReadLine(260);

	// Visited Maps
	auto visitedMaps = LoadIdxTable<uint32_t>(in);
	for (auto& entry : visitedMaps) {
		mVisitedMaps.insert(entry.first);
	}

	// Read Spells
	mNextSpellId = in.ReadUInt32();
	auto spellCount = in.ReadUInt32();
	for (size_t i = 0; i < spellCount; ++i) {
		SpellSave spell;

		spell.spellId = in.ReadUInt32();
		spell.spellIdx = in.ReadUInt32();
		spell.active = in.ReadUInt32();

		spell.spellEnum = in.ReadUInt32();
		spell.spellEnumOrg = in.ReadUInt32();
		spell.flagsSth = in.ReadUInt32();
		spell.caster = in.ReadObjectId();
		spell.casterPartSysName = in.ReadUInt32();

		spell.spellClass = in.ReadUInt32();
		spell.spellLevel = in.ReadUInt32();
		spell.baseCasterLevel = in.ReadUInt32();
		spell.spellDc = in.ReadUInt32();
		spell.spellObjCount = in.ReadUInt32();
		spell.unknownObj = in.ReadObjectId();
		for (size_t i = 0; i < 128; ++i) {
			spell.spellObjs[i].id = in.ReadObjectId();
			spell.spellObjs[i].casterPartSysName = in.ReadUInt32();
		}

		spell.targetListNumItemsCopy = in.ReadUInt32();
		spell.targetListNumItems = in.ReadUInt32();
		for (size_t i = 0; i < 32; ++i) {
			spell.targets[i].id = in.ReadObjectId();
		}
		for (size_t i = 0; i < 32; ++i) {
			spell.targets[i].casterPartSysName = in.ReadUInt32();
		}
		spell.numProjectiles = in.ReadUInt32();
		for (size_t i = 0; i < 5; ++i) {
			spell.projectiles[i] = in.ReadObjectId();
		}
		spell.aoeCenter = in.ReadLocFull();
		spell.duration = in.ReadUInt32();
		spell.durationRemaining = in.ReadUInt32();
		spell.spellRange = in.ReadUInt32();
		spell.savingThrowResult = in.ReadUInt32();
		spell.metaMagic = in.ReadUInt32();
		spell.spellId2 = in.ReadUInt32();

		mSpells.push_back(spell);
	}

	// TODO: Map Flee Data

	ReadSystemFooter(in, "map");

	// Gamesystem "lightscheme"
	mLightSchemeId = in.ReadUInt32();
	mLightSchemeHourOfDay = in.ReadUInt32();
	ReadSystemFooter(in, "lightscheme");

	// Gamesystem "player" (doesn't actually save anything)
	ReadSystemFooter(in, "player");

	// Gamesystem "area"
	mKnownAreas.clear();
	// Vanilla ToEE has 13 areas
	// Co8 has more, so this needs adjustment
	for (size_t i = 0; i < 13; ++i) {
		if (in.ReadUInt8()) {
			mKnownAreas.insert(i);
		}
	}
	in.ReadUInt32(); // Last discovered area, never used

	ReadSystemFooter(in, "area");

	// Gamesystem "sound"
	mSoundSchemeIds.clear();
	auto schemeId = in.ReadUInt32();
	if (schemeId) {
		mSoundSchemeIds.push_back(schemeId);
	}
	schemeId = in.ReadUInt32();
	if (schemeId) {
		mSoundSchemeIds.push_back(schemeId);
	}

	mSoundGameOverSound = (in.ReadUInt32() != 0);
	mSoundGameOverStash = { in.ReadUInt32(), in.ReadUInt32() };
	
	mSoundGameInCombat = (in.ReadUInt32() != 0);
	mSoundGameCombatStash = { in.ReadUInt32(), in.ReadUInt32() };

	ReadSystemFooter(in, "sound");

	mInCombat = (in.ReadUInt32() != 0);
	ReadSystemFooter(in, "combat");
	
	mRealTime.timeInDays = in.ReadUInt32();
	mRealTime.timeInMs = in.ReadUInt32();
	mGameTime.timeInDays = in.ReadUInt32();
	mGameTime.timeInMs = in.ReadUInt32();
	mAnimTime.timeInDays = in.ReadUInt32();
	mAnimTime.timeInMs = in.ReadUInt32();

	std::vector<char> pickledObj;
	mTimeEvents.clear();

	// The events for the three clock types are stored separately
	for (size_t clockId = 0; clockId < 3; clockId++) {
		auto eventCount = in.ReadUInt32();
		mTimeEvents.reserve(mTimeEvents.size() + eventCount);
		for (size_t i = 0; i < eventCount; i++) {
			TimeEventSave event;
			event.expiresAt.timeInDays = in.ReadUInt32();
			event.expiresAt.timeInMs = in.ReadUInt32();
			event.type = (TimeEventType)in.ReadUInt32();

			if (event.type > TimeEventType::PythonRealtime) {
				throw TempleException("Invalid event type read");
			}

			auto &spec = GetTimeEventTypeSpec(event.type);
			for (size_t i = 0; i < spec.argTypes.size(); i++) {
				TimeEventArgSave arg;

				switch (spec.argTypes[i]) {
				case TimeEventArgType::Int:
					arg.intVal = in.ReadInt32();
					break;
				case TimeEventArgType::Float:
					arg.floatVal = in.ReadFloat();
					break;
				case TimeEventArgType::Object:
					arg.objectRefVal = ReadObjRef(in);
					break;
				case TimeEventArgType::PythonObject:
					// This is complex
					if (in.ReadUInt32() != 0xADD2DECA) {
						throw TempleException("Unable to read start sentinel for Python object");
					}

					// read into buffer until we reach the end sentinel
					pickledObj.clear();
					pickledObj.reserve(4096);
					// This is flimsy at best, but it's neither null terminated, nor is it
					// prefixed with the real length
					while (!EndsWithPythonEndSentinel(pickledObj)) {
						pickledObj.push_back(in.ReadUInt8());
					}
					// Truncate the sentinel
					pickledObj.resize(pickledObj.size() - 4);
					arg.pythonObj.assign(pickledObj.data(), pickledObj.size());
					break;
				case TimeEventArgType::Location:
					arg.locVal = in.ReadLoc();
					break;
				default:
					continue;
				}

				event.args.push_back(arg);
			}
			mTimeEvents.emplace_back(event);
		}
	}

	ReadSystemFooter(in, "timeevent");

	// No actual data saved
	ReadSystemFooter(in, "rumor");
	
	// Quests
	mQuests.resize(200);
	for (size_t i = 0; i < 200; ++i) {
		mQuests[i].acceptedTime.timeInDays = in.ReadUInt32();
		mQuests[i].acceptedTime.timeInMs = in.ReadUInt32();
		mQuests[i].state = in.ReadUInt32();
		mQuests[i].acceptedArea = in.ReadUInt32();
	}
	ReadSystemFooter(in, "quest");

	// Gamesystem "anims"
	mNextAnimId = in.ReadUInt32();
	mActiveGoalCount = in.ReadUInt32();
	mAnimCatchup = (in.ReadUInt32() != 0);
	mAnimActionId = in.ReadUInt32();
	mUnkAnimVals.resize(15);
	for (size_t i = 0; i < 15; ++i) {
		mUnkAnimVals[i] = in.ReadUInt32();
	}

	auto animSlotCount = in.ReadUInt32();
	if (animSlotCount != 512) {
		throw TempleException("Expected anim slot count to be 512, but was {}", animSlotCount);
	}

	mAnimSlots.clear();
	size_t slotsRead = 0;
	while (slotsRead < animSlotCount) {
		auto runLength = in.ReadInt32();
		// Negative count indicates unused slots
		if (runLength < 0) {
			runLength = - runLength;
			slotsRead += runLength;
			continue;
		}

		for (int i = 0; i < runLength; ++i) {
			mAnimSlots[slotsRead] = ReadAnimSlot(in);
			slotsRead++;
		}
	}

	ReadSystemFooter(in, "anim");

}

ObjRefSave SaveGame::ReadObjRef(InputStream& in) {
	ObjRefSave result;
	result.id = in.ReadObjectId();
	result.location = locXY::fromField(in.ReadUInt64());
	result.mapId = in.ReadUInt32();
	return result;
}

AnimSlotSave SaveGame::ReadAnimSlot(InputStream& in) {
	AnimSlotSave result;

	result.id.slotIndex = in.ReadUInt32();
	result.id.uniqueId = in.ReadUInt32();
	result.id.field_8 = in.ReadUInt32();

	result.flags = in.ReadUInt32();
	result.currentState = in.ReadInt32();
	result.field14 = in.ReadInt32();
	
	result.animObj = ReadObjRef(in);
	auto currentGoalIdx = in.ReadInt32();

	for (auto i = 0; i <= currentGoalIdx; ++i) {
		AnimGoalSave goal;
		goal.type = (AnimGoalType)in.ReadUInt32();
		goal.selfObj = ReadObjRef(in);
		goal.targetObj = ReadObjRef(in);
		goal.blockObj = ReadObjRef(in);
		goal.scratchObj = ReadObjRef(in);
		goal.parentObj = ReadObjRef(in);
		goal.targetLoc = locXY::fromField(in.ReadUInt64());
		goal.range = locXY::fromField(in.ReadUInt64());
		goal.animId = in.ReadInt32();
		goal.animIdPrev = in.ReadInt32();
		goal.animData = in.ReadInt32();
		goal.animData2 = in.ReadInt32();
		goal.spellData = in.ReadInt32();
		goal.skillData = in.ReadInt32();
		goal.flagsData = in.ReadInt32();
		goal.scratchVal1 = in.ReadInt32();
		goal.scratchVal2 = in.ReadInt32();
		goal.scratchVal3 = in.ReadInt32();
		goal.scratchVal4 = in.ReadInt32();
		goal.scratchVal5 = in.ReadInt32();
		goal.scratchVal6 = in.ReadInt32();
		goal.soundHandle = in.ReadInt64() >> 32;
		result.goals.push_back(goal);
	}

	in.ReadBytes(&result.unk[0], result.unk.size());
	result.pathInfo = in.ReadUInt64();
	result.field18 = in.ReadUInt64();
	result.someTime.timeInDays = in.ReadUInt32();
	result.someTime.timeInMs = in.ReadUInt32();
	result.field2c90 = in.ReadUInt32();

	return result;
}

void SaveGame::ReadSystemFooter(InputStream& in, const char *system) {
	auto footer = in.ReadUInt32();
	if (footer != 0xBEEFCAFE) {
		throw TempleException("Invalid footer after game system {}: {:x}", system, footer);
	}
}
