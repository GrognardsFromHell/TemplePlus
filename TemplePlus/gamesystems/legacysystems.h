
#pragma once

#include <string>
#include <cstdint>

#include "gamesystem.h"

#include "legacy.h"


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
};

class SkillSystem : public GameSystem, public SaveGameAwareGameSystem {
public:
	static constexpr auto Name = "Skill";
	SkillSystem(const GameSystemConf &config);
	~SkillSystem();
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
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
};

class MapSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Map";
	MapSystem(const GameSystemConf &config);
	~MapSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	void AdvanceTime(uint32_t time) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
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

class TimeEventSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "TimeEvent";
	TimeEventSystem(const GameSystemConf &config);
	~TimeEventSystem();
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
};

class AnimSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Anim";
	AnimSystem(const GameSystemConf &config);
	~AnimSystem();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;
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

class GroundSystem : public GameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Ground";
	GroundSystem(const GameSystemConf &config);
	~GroundSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
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

class ParticleSysSystem : public GameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "ParticleSys";
	ParticleSysSystem(const GameSystemConf &config);
	~ParticleSysSystem();
	void AdvanceTime(uint32_t time) override;
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
	MapFoggingSystem(const GameSystemConf &config);
	~MapFoggingSystem();
	void Reset() override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
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
