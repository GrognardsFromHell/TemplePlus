
#pragma once

#include <string>
#include <cstdint>

#include "gamesystem.h"

#include "legacy.h"
#include "obj.h"
#include "map/sector.h"

namespace gfx {
	class RenderingDevice;
}

class VagrantSystem : public GameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Vagrant";
	VagrantSystem(const GameSystemConf &config);
	~VagrantSystem();
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class DescriptionSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Description";
	DescriptionSystem(const GameSystemConf &config);
	~DescriptionSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	bool ReadCustomNames(GameSystemSaveFile* file, std::vector<std::string> &customNamesOut); // deserializes custom names from save game file
};

class ItemEffectSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "ItemEffect";
	ItemEffectSystem(const GameSystemConf &config);
	~ItemEffectSystem();
	void LoadModule() override;
	void UnloadModule() override;
	const std::string &GetName() const override;
};

class TeleportSystem : public GameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Teleport";
	TeleportSystem(const GameSystemConf &config);
	~TeleportSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class SectorSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Sector";
	SectorSystem(const GameSystemConf &config);
	~SectorSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	void SetLimits(uint64_t limitX, uint64_t limitY);
	bool ReadSectorTimes(GameSystemSaveFile *saveFile, std::vector<SectorTime> & sectorTimes);
};

class RandomSystem : public GameSystem {
public:
	static constexpr auto Name = "Random";
	RandomSystem(const GameSystemConf &config);
	const std::string &GetName() const override;
};

class CritterSystem : public GameSystem {
public:
	static constexpr auto Name = "Critter";
	CritterSystem(const GameSystemConf &config);
	~CritterSystem();
	const std::string &GetName() const override;

	/**
	 * Creates or refreshes the subdual and normal healing timers for all loaded NPCs.
	 */
	void UpdateNpcHealingTimers();
};

class ScriptNameSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "ScriptName";
	ScriptNameSystem(const GameSystemConf &config);
	~ScriptNameSystem();
	void LoadModule() override;
	void UnloadModule() override;
	const std::string &GetName() const override;
};

class PortraitSystem : public GameSystem {
public:
	static constexpr auto Name = "Portrait";
	PortraitSystem(const GameSystemConf &config);
	~PortraitSystem();
	const std::string &GetName() const override;

	bool GetFirstId(objHndl handle, int* idxOut) const; // gets first valid portrait from portraits.mes
	bool GetNextId(objHndl handle, int* idxOut) const;
	int GetKeyFromId(int id) const;
	std::string GetPortraitFileFromId(int id, int subId = 0);

	static bool IsPortraitFilenameValid(objHndl handle, const char* filename);
private:

	const int PORTRAIT_MAX_ID = 65536;
	
	struct PortraitPack{
		int key = 0;
		std::string path;
		std::map<int, std::string> packContents;
	};
	std::vector<PortraitPack> mPortraitPacks;
	MesHandle & mPortraitsMes = temple::GetRef<MesHandle>(0x10AB7368);
};

class SkillSystem : public GameSystem, public SaveGameAwareGameSystem {
public:
	static constexpr auto Name = "Skill";
	SkillSystem(const GameSystemConf &config);
	~SkillSystem();
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	bool ReadUnknown(GameSystemSaveFile *saveFile, int& unk); // not sure what it reads, appears unused
};

class FeatSystem : public GameSystem {
public:
	static constexpr auto Name = "Feat";
	FeatSystem(const GameSystemConf &config);
	~FeatSystem();
	const std::string &GetName() const override;
};

class SpellSystem : public GameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Spell";
	SpellSystem(const GameSystemConf &config);
	~SpellSystem();
	void Reset() override;
	const std::string &GetName() const override;
	
	bool Save(TioFile *file);
	bool Load(GameSystemSaveFile *file);
};

class StatSystem : public GameSystem {
public:
	static constexpr auto Name = "Stat";
	StatSystem(const GameSystemConf &config);
	~StatSystem();
	const std::string &GetName() const override;
};

class ScriptSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Script";
	ScriptSystem(const GameSystemConf &config);
	~ScriptSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	bool ReadGlobalVars(GameSystemSaveFile *saveFile, std::vector<int> & globalVars, std::vector<int> & globalFlagsData, int& storyState);
	bool ReadEncounterQueue(GameSystemSaveFile *saveFile, std::vector<int> & encounterQueue);
};

class LevelSystem : public GameSystem {
public:
	static constexpr auto Name = "Level";
	LevelSystem(const GameSystemConf &config);
	~LevelSystem();
	const std::string &GetName() const override;
};

class D20System : public GameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "D20";
	D20System(const GameSystemConf &config);
	~D20System();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;

	void RemoveDispatcher(objHndl obj);

	void ResetRadialMenus();
};

class LightSchemeSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "LightScheme";
	LightSchemeSystem(const GameSystemConf &config);
	~LightSchemeSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	void SetLightSchemeId(int schemeId);
	void SetLightScheme(int schemeId, int hour);
	int GetHourOfDay();


};

class PlayerSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Player";
	PlayerSystem(const GameSystemConf &config);
	~PlayerSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class AreaSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Area";
	AreaSystem(const GameSystemConf &config);
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class DialogSystem : public GameSystem {
public:
	static constexpr auto Name = "Dialog";
	DialogSystem(const GameSystemConf &config);
	~DialogSystem();
	const std::string &GetName() const override;
};

class SoundMapSystem : public GameSystem {
public:
	static constexpr auto Name = "SoundMap";
	SoundMapSystem(const GameSystemConf &config);
	const std::string &GetName() const override;
};

class SoundGameSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "SoundGame";
	SoundGameSystem(const GameSystemConf &config);
	~SoundGameSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;

	void SetSoundSchemeIds(int scheme1, int scheme2);

	// Used when starting the game
	void StopAll(bool flag);

};

class ItemSystem : public GameSystem, public BufferResettingGameSystem {
public:
	static constexpr auto Name = "Item";
	ItemSystem(const GameSystemConf &config);
	~ItemSystem();
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
};

class CombatSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Combat";
	CombatSystem(const GameSystemConf &config);
	~CombatSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class RumorSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "Rumor";
	RumorSystem(const GameSystemConf &config);
	~RumorSystem();
	void LoadModule() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class QuestSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Quest";
	QuestSystem(const GameSystemConf &config);
	~QuestSystem();
	void LoadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class AISystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "AI";
	AISystem(const GameSystemConf &config);
	void LoadModule() override;
	const std::string &GetName() const override;

	void AddAiTimer(objHndl handle);
};

class AnimPrivateSystem : public GameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "AnimPrivate";
	AnimPrivateSystem(const GameSystemConf &config);
	~AnimPrivateSystem();
	void Reset() override;
	const std::string &GetName() const override;
};

class ReputationSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Reputation";
	ReputationSystem(const GameSystemConf &config);
	~ReputationSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class ReactionSystem : public GameSystem {
public:
	static constexpr auto Name = "Reaction";
	ReactionSystem(const GameSystemConf &config);
	~ReactionSystem();
	const std::string &GetName() const override;
};

class TileScriptSystem : public GameSystem {
public:
	static constexpr auto Name = "TileScript";
	TileScriptSystem(const GameSystemConf &config);
	~TileScriptSystem();
	const std::string &GetName() const override;
};

class SectorScriptSystem : public GameSystem {
public:
	static constexpr auto Name = "SectorScript";
	SectorScriptSystem(const GameSystemConf &config);
	const std::string &GetName() const override;
};

class WPSystem : public GameSystem, public BufferResettingGameSystem {
public:
	static constexpr auto Name = "WP";
	WPSystem(const GameSystemConf &config);
	~WPSystem();
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
};

class InvenSourceSystem : public GameSystem {
public:
	static constexpr auto Name = "InvenSource";
	InvenSourceSystem(const GameSystemConf &config);
	~InvenSourceSystem();
	const std::string &GetName() const override;
};

class TownMapSystem : public GameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "TownMap";
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	const std::string &GetName() const override;
};

class GMovieSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "GMovie";
	void LoadModule() override;
	void UnloadModule() override;
	const std::string &GetName() const override;
};

class BrightnessSystem : public GameSystem {
public:
	static constexpr auto Name = "Brightness";
	BrightnessSystem(const GameSystemConf &config);
	const std::string &GetName() const override;
};

class GFadeSystem : public GameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "GFade";
	GFadeSystem(const GameSystemConf &config);
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class AntiTeleportSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "AntiTeleport";
	AntiTeleportSystem(const GameSystemConf &config);
	~AntiTeleportSystem();
	void LoadModule() override;
	void UnloadModule() override;
	const std::string &GetName() const override;
};

class TrapSystem : public GameSystem {
public:
	static constexpr auto Name = "Trap";
	TrapSystem(const GameSystemConf &config);
	~TrapSystem();
	const std::string &GetName() const override;
};

class MonsterGenSystem : public GameSystem, public SaveGameAwareGameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "MonsterGen";
	MonsterGenSystem(const GameSystemConf &config);
	~MonsterGenSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
};

class PartySystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Party";
	PartySystem(const GameSystemConf &config);
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	void SaveCurrent();
	void RestoreCurrent();

	bool IsInParty(objHndl obj) const;

	void ForEachInParty(std::function<void(objHndl)> callback);
};

class D20LoadSaveSystem : public GameSystem, public SaveGameAwareGameSystem {
public:
	static constexpr auto Name = "D20LoadSave";
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class GameInitSystem : public GameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "GameInit";
	GameInitSystem(const GameSystemConf &config);
	~GameInitSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	const std::string &GetName() const override;
};

class ObjFadeSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "ObjFade";
	ObjFadeSystem(const GameSystemConf &config);
	~ObjFadeSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class DeitySystem : public GameSystem {
public:
	static constexpr auto Name = "Deity";
	DeitySystem(const GameSystemConf &config);
	~DeitySystem();
	const std::string &GetName() const override;
};

class UiArtManagerSystem : public GameSystem {
public:
	static constexpr auto Name = "UiArtManager";
	UiArtManagerSystem(const GameSystemConf &config);
	~UiArtManagerSystem();
	const std::string &GetName() const override;
};

class CheatsSystem : public GameSystem {
public:
	static constexpr auto Name = "Cheats";
	CheatsSystem(const GameSystemConf &config);
	const std::string &GetName() const override;
};

class D20RollsSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "D20Rolls";
	D20RollsSystem(const GameSystemConf &config);
	~D20RollsSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class SecretdoorSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Secretdoor";
	SecretdoorSystem(const GameSystemConf &config);
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class MapFoggingSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "MapFogging";
	MapFoggingSystem(gfx::RenderingDevice &device);
	~MapFoggingSystem();
	void Reset() override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;

	void LoadFogColor(const std::string &dataDir);

	void Enable();
	void Disable();

	// The ETD data seems to be used for the townmap
	void LoadExploredTileData(int mapId);
	void SaveExploredTileData(int mapId);
	
	void SaveEsd();
	void PerformCheckForCritter(objHndl handle, int idx);

private:

	gfx::RenderingDevice &mDevice;

	void InitScreenBuffers();

	static constexpr size_t sFogBufferDim = 102;

	uint64_t& mFogMinX = temple::GetRef<uint64_t>(0x10824468);
	uint64_t& mFogMinY = temple::GetRef<uint64_t>(0x108EC4C8);
	uint64_t& mSubtilesX = temple::GetRef<uint64_t>(0x10820458);
	uint64_t& mSubtilesY = temple::GetRef<uint64_t>(0x10824490);

	BOOL& mFoggingEnabled = temple::GetRef<BOOL>(0x108254A0);
	uint8_t*& mFogCheckData = temple::GetRef<uint8_t*>(0x108A5498);
	void** mFogBuffers = temple::GetPointer<void*>(0x10824470); // 8 entries, one for each controllable party member
	

	SectorLoc* mEsdSectorLocs = temple::GetPointer<SectorLoc>(0x108EC598); // 32 entries
	uint32_t& mEsdLoaded = temple::GetRef<uint32_t>(0x108EC6B0);
	BOOL& mDoFullUpdate = temple::GetRef<BOOL>(0x108EC590);
	uint32_t& mFogChecks = temple::GetRef<uint32_t>(0x102ACEFC);

	int& mScreenWidth = temple::GetRef<int>(0x108EC6A0);
	int& mScreenHeight = temple::GetRef<int>(0x108EC6A4);
};

class RandomEncounterSystem : public GameSystem, public SaveGameAwareGameSystem {
public:
	static constexpr auto Name = "RandomEncounter";
	RandomEncounterSystem(const GameSystemConf &config);
	~RandomEncounterSystem();
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class ObjectEventSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "ObjectEvent";
	ObjectEventSystem(const GameSystemConf &config);
	~ObjectEventSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class FormationSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Formation";
	FormationSystem(const GameSystemConf &config);
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
};

class ItemHighlightSystem : public GameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "ItemHighlight";
	ItemHighlightSystem(const GameSystemConf &config);
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	const std::string &GetName() const override;
};

class PathXSystem : public GameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "PathX";
	PathXSystem(const GameSystemConf &config);
	void Reset() override;
	const std::string &GetName() const override;
};
