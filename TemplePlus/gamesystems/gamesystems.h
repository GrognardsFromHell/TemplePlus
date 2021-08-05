
#pragma once

#include <map>

#include <temple/dll.h>

#include "gamesystem.h"

#ifdef GetObject
#undef GetObject
#endif

#pragma pack(push, 1)
class LoadingScreen;

struct GameSystemConf {
	int editor;
	int width;
	int height;
	int viewportId;
	void *field_10;
	void *renderfunc;
};
#pragma pack(pop)

class TigBufferstuffInitializer {
public:
	TigBufferstuffInitializer();
	~TigBufferstuffInitializer();
	int bufferIdx() const {
		return mBufferIdx;
	}
private:
	int mBufferIdx = -1;
};

class TigInitializer;

namespace gfx {
	class AnimatedModelFactory;
}

class VagrantSystem;
class DescriptionSystem;
class ItemEffectSystem;
class TeleportSystem;
class SectorSystem;
class RandomSystem;
class CritterSystem;
class ScriptNameSystem;
class PortraitSystem;
class SkillSystem;
class FeatSystem;
class SpellSystem;
class StatSystem;
class ScriptSystem;
class LevelSystem;
class D20System;
class MapSystem;
class LightSchemeSystem;
class PlayerSystem;
class AreaSystem;
class DialogSystem;
class SoundMapSystem;
class SoundGameSystem;
class ItemSystem;
class CombatSystem;
class TimeEventSystem;
class RumorSystem;
class QuestSystem;
class AISystem;
class AnimSystem;
class AnimPrivateSystem;
class ReputationSystem;
class ReactionSystem;
class TileScriptSystem;
class SectorScriptSystem;
class WPSystem;
class InvenSourceSystem;
class TownMapSystem;
class GMovieSystem;
class BrightnessSystem;
class GFadeSystem;
class AntiTeleportSystem;
class TrapSystem;
class MonsterGenSystem;
class PartySystem;
class D20LoadSaveSystem;
class GameInitSystem;
class ObjFadeSystem;
class DeitySystem;
class UiArtManagerSystem;
class ParticleSysSystem;
class CheatsSystem;
class D20RollsSystem;
class SecretdoorSystem;
class MapFoggingSystem;
class RandomEncounterSystem;
class ObjectEventSystem;
class FormationSystem;
class ItemHighlightSystem;
class PathXSystem;
class ScrollSystem;
class LocationSystem;
class LightSystem;
class TileSystem;
class ONameSystem;
class ObjectNodeSystem;
class ObjSystem;
class ProtoSystem;
class ObjectSystem;
class MapSectorSystem;
class SectorVBSystem;
class TextBubbleSystem;
class TextFloaterSystem;
class JumpPointSystem;
class TerrainSystem;
class ClippingSystem;
class HeightSystem;
class GMeshSystem;
class PathNodeSystem;
class GameSystemLoadingScreen;
class PoisonSystem;

class GameSystems {
public:
	explicit GameSystems(TigInitializer &tig);
	~GameSystems();
	const GameSystemConf &GetConfig() const {
		return mConfig;
	}

	VagrantSystem& GetVagrant() const {
		assert(!!mVagrant);
		return *mVagrant;
	}
	DescriptionSystem& GetDescription() const {
		assert(!!mDescription);
		return *mDescription;
	}
	ItemEffectSystem& GetItemEffect() const {
		assert(!!mItemEffect);
		return *mItemEffect;
	}
	TeleportSystem& GetTeleport() const {
		assert(!!mTeleport);
		return *mTeleport;
	}
	SectorSystem& GetSector() const {
		assert(!!mSector);
		return *mSector;
	}
	RandomSystem& GetRandom() const {
		assert(!!mRandom);
		return *mRandom;
	}
	CritterSystem& GetCritter() const {
		assert(!!mCritter);
		return *mCritter;
	}
	ScriptNameSystem& GetScriptName() const {
		assert(!!mScriptName);
		return *mScriptName;
	}
	PortraitSystem& GetPortrait() const {
		assert(!!mPortrait);
		return *mPortrait;
	}
	SkillSystem& GetSkill() const {
		assert(!!mSkill);
		return *mSkill;
	}
	FeatSystem& GetFeat() const {
		assert(!!mFeat);
		return *mFeat;
	}
	SpellSystem& GetSpell() const {
		assert(!!mSpell);
		return *mSpell;
	}
	StatSystem& GetStat() const {
		assert(!!mStat);
		return *mStat;
	}
	ScriptSystem& GetScript() const {
		assert(!!mScript);
		return *mScript;
	}
	LevelSystem& GetLevel() const {
		assert(!!mLevel);
		return *mLevel;
	}
	D20System& GetD20() const {
		assert(!!mD20);
		return *mD20;
	}
	MapSystem& GetMap() const {
		assert(!!mMap);
		return *mMap;
	}
	ScrollSystem& GetScroll() const {
		assert(!!mScroll);
		return *mScroll;
	}
	LocationSystem& GetLocation() const {
		assert(!!mLocation);
		return *mLocation;
	}
	LightSystem& GetLight() const {
		assert(!!mLight);
		return *mLight;
	}
	TileSystem& GetTile() const {
		assert(!!mTile);
		return *mTile;
	}
	ONameSystem& GetOName() const {
		assert(!!mOName);
		return *mOName;
	}
	ObjectNodeSystem& GetObjectNode() const {
		assert(!!mObjectNode);
		return *mObjectNode;
	}
	ObjSystem& GetObj() const {
		assert(!!mObj);
		return *mObj;
	}
	ProtoSystem& GetProto() const {
		assert(!!mProto);
		return *mProto;
	}
	ObjectSystem& GetObject() const {
		assert(!!mObject);
		return *mObject;
	}
	MapSectorSystem& GetMapSector() const {
		assert(!!mMapSector);
		return *mMapSector;
	}
	SectorVBSystem& GetSectorVB() const {
		assert(!!mSectorVB);
		return *mSectorVB;
	}
	TextBubbleSystem& GetTextBubble() const {
		assert(!!mTextBubble);
		return *mTextBubble;
	}
	TextFloaterSystem& GetTextFloater() const {
		assert(!!mTextFloater);
		return *mTextFloater;
	}
	JumpPointSystem& GetJumpPoint() const {
		assert(!!mJumpPoint);
		return *mJumpPoint;
	}
	TerrainSystem& GetTerrain() const {
		assert(!!mTerrain);
		return *mTerrain;
	}
	ClippingSystem& GetClipping() const {
		assert(!!mClipping);
		return *mClipping;
	}
	HeightSystem& GetHeight() const {
		assert(!!mHeight);
		return *mHeight;
	}
	GMeshSystem& GetGMesh() const {
		assert(!!mGMesh);
		return *mGMesh;
	}
	PathNodeSystem& GetPathNode() const {
		assert(!!mPathNode);
		return *mPathNode;
	}
	LightSchemeSystem& GetLightScheme() const {
		assert(!!mLightScheme);
		return *mLightScheme;
	}
	PlayerSystem& GetPlayer() const {
		assert(!!mPlayer);
		return *mPlayer;
	}
	AreaSystem& GetArea() const {
		assert(!!mArea);
		return *mArea;
	}
	DialogSystem& GetDialog() const {
		assert(!!mDialog);
		return *mDialog;
	}
	SoundMapSystem& GetSoundMap() const {
		assert(!!mSoundMap);
		return *mSoundMap;
	}
	SoundGameSystem& GetSoundGame() const {
		assert(!!mSoundGame);
		return *mSoundGame;
	}
	ItemSystem& GetItem() const {
		assert(!!mItem);
		return *mItem;
	}
	CombatSystem& GetCombat() const {
		assert(!!mCombat);
		return *mCombat;
	}
	TimeEventSystem& GetTimeEvent() const {
		assert(!!mTimeEvent);
		return *mTimeEvent;
	}
	RumorSystem& GetRumor() const {
		assert(!!mRumor);
		return *mRumor;
	}
	QuestSystem& GetQuest() const {
		assert(!!mQuest);
		return *mQuest;
	}
	AISystem& GetAI() const {
		assert(!!mAI);
		return *mAI;
	}
	AnimSystem& GetAnim() const {
		assert(!!mAnim);
		return *mAnim;
	}
	AnimPrivateSystem& GetAnimPrivate() const {
		assert(!!mAnimPrivate);
		return *mAnimPrivate;
	}
	ReputationSystem& GetReputation() const {
		assert(!!mReputation);
		return *mReputation;
	}
	ReactionSystem& GetReaction() const {
		assert(!!mReaction);
		return *mReaction;
	}
	TileScriptSystem& GetTileScript() const {
		assert(!!mTileScript);
		return *mTileScript;
	}
	SectorScriptSystem& GetSectorScript() const {
		assert(!!mSectorScript);
		return *mSectorScript;
	}
	WPSystem& GetWP() const {
		assert(!!mWP);
		return *mWP;
	}
	InvenSourceSystem& GetInvenSource() const {
		assert(!!mInvenSource);
		return *mInvenSource;
	}
	TownMapSystem& GetTownMap() const {
		assert(!!mTownMap);
		return *mTownMap;
	}
	GMovieSystem& GetGMovie() const {
		assert(!!mGMovie);
		return *mGMovie;
	}
	BrightnessSystem& GetBrightness() const {
		assert(!!mBrightness);
		return *mBrightness;
	}
	GFadeSystem& GetGFade() const {
		assert(!!mGFade);
		return *mGFade;
	}
	AntiTeleportSystem& GetAntiTeleport() const {
		assert(!!mAntiTeleport);
		return *mAntiTeleport;
	}
	TrapSystem& GetTrap() const {
		assert(!!mTrap);
		return *mTrap;
	}
	MonsterGenSystem& GetMonsterGen() const {
		assert(!!mMonsterGen);
		return *mMonsterGen;
	}
	PartySystem& GetParty() const {
		assert(!!mParty);
		return *mParty;
	}
	D20LoadSaveSystem& GetD20LoadSave() const {
		assert(!!mD20LoadSave);
		return *mD20LoadSave;
	}
	GameInitSystem& GetGameInit() const {
		assert(!!mGameInit);
		return *mGameInit;
	}
	ObjFadeSystem& GetObjFade() const {
		assert(!!mObjFade);
		return *mObjFade;
	}
	DeitySystem& GetDeity() const {
		assert(!!mDeity);
		return *mDeity;
	}
	UiArtManagerSystem& GetUiArtManager() const {
		assert(!!mUiArtManager);
		return *mUiArtManager;
	}
	ParticleSysSystem& GetParticleSys() const {
		assert(!!mParticleSys);
		return *mParticleSys;
	}
	CheatsSystem& GetCheats() const {
		assert(!!mCheats);
		return *mCheats;
	}
	D20RollsSystem& GetD20Rolls() const {
		assert(!!mD20Rolls);
		return *mD20Rolls;
	}
	SecretdoorSystem& GetSecretdoor() const {
		assert(!!mSecretdoor);
		return *mSecretdoor;
	}
	MapFoggingSystem& GetMapFogging() const {
		assert(!!mMapFogging);
		return *mMapFogging;
	}
	RandomEncounterSystem& GetRandomEncounter() const {
		assert(!!mRandomEncounter);
		return *mRandomEncounter;
	}
	ObjectEventSystem& GetObjectEvent() const {
		assert(!!mObjectEvent);
		return *mObjectEvent;
	}
	FormationSystem& GetFormation() const {
		assert(!!mFormation);
		return *mFormation;
	}
	ItemHighlightSystem& GetItemHighlight() const {
		assert(!!mItemHighlight);
		return *mItemHighlight;
	}
	PathXSystem& GetPathX() const {
		assert(!!mPathX);
		return *mPathX;
	}
	PoisonSystem& GetPoison() const {
		assert(!!mPoison);
		return *mPoison;
	}

	// All systems that want to listen to map events
	const std::vector<MapCloseAwareGameSystem*> &GetMapCloseAwareSystems() const {
		return mMapCloseAwareSystems;
	}

	gfx::AnimatedModelFactory& GetAAS() const {
		assert(!!mAAS);
		return *mAAS;
	}

	// Makes a savegame.
	bool SaveGame(const std::string &filename, const std::string &displayName);

	bool SaveGameIronman();
	
	// Loads a game.
	bool LoadGame(const std::string &filename);

	/*
	Call this before loading a game. Use not yet known.
	TODO I do NOT think this is used, should be checked. Seems like leftovers from even before arcanum
	*/
	void DestroyPlayerObject();

	// Ends the game, resets the game systems and returns to the main menu.
	void EndGame();

	void AdvanceTime();

	void LoadModule(const std::string &moduleName);
	void AddModulePaths(const std::string &moduleName);

	void UnloadModule();
	void RemoveModulePaths();

	void ResetGame();
	bool IsResetting() const {
		return mResetting;
	}

	/**
	 * Creates the screenshots that will be used in case the game is saved.
	 */
	void TakeSaveScreenshots();

	bool IsIronman() const {
		return mIronmanFlag != FALSE;
	}
	void SetIronman(bool enable) {
		mIronmanFlag = enable ? TRUE : FALSE;
	}

private:
	BOOL &mIronmanFlag = temple::GetRef<BOOL>(0x103072B8);
	int& mIronmanSaveNumber = temple::GetRef<int>(0x10306F44);
	char *&mIronmanSaveName = temple::GetRef<char*>(0x103072C0);

	void VerifyTemplePlusData();
	std::string GetLanguage();
	void PlayLegalMovies();
	void InitBufferStuff(const GameSystemConf& conf);
	void InitAnimationSystem();
	std::string ResolveSkaFile(int meshId) const;
	std::string ResolveSkmFile(int meshId) const;
	int ResolveMaterial(const std::string &material) const;
	
	void ResizeScreen(int w, int h);

	void InitializeSystems(LoadingScreen &);
	
	template<typename T, typename... TArgs>
	std::unique_ptr<T> InitializeSystem(LoadingScreen &loadingScreen, TArgs&&... args);
	
	TigInitializer &mTig;
	GameSystemConf mConfig;
	TigBufferstuffInitializer mTigBuffer;

	std::unique_ptr<gfx::AnimatedModelFactory> mAAS;
	std::map<int, std::string> mMeshesById;

	bool mResetting = false;
	
	std::vector<class TimeAwareGameSystem*> mTimeAwareSystems;
	std::vector<class ModuleAwareGameSystem*> mModuleAwareSystems;
	std::vector<class ResetAwareGameSystem*> mResetAwareSystems;
	std::vector<class BufferResettingGameSystem*> mBufferResettingSystems;
	std::vector<class SaveGameAwareGameSystem*> mSaveGameAwareSystems;
	std::vector<class MapCloseAwareGameSystem*> mMapCloseAwareSystems;

	std::unique_ptr<VagrantSystem> mVagrant;
	std::unique_ptr<DescriptionSystem> mDescription;
	std::unique_ptr<ItemEffectSystem> mItemEffect;
	std::unique_ptr<TeleportSystem> mTeleport;
	std::unique_ptr<SectorSystem> mSector;
	std::unique_ptr<RandomSystem> mRandom;
	std::unique_ptr<CritterSystem> mCritter;
	std::unique_ptr<ScriptNameSystem> mScriptName;
	std::unique_ptr<PortraitSystem> mPortrait;
	std::unique_ptr<SkillSystem> mSkill;
	std::unique_ptr<FeatSystem> mFeat;
	std::unique_ptr<SpellSystem> mSpell;
	std::unique_ptr<StatSystem> mStat;
	std::unique_ptr<ScriptSystem> mScript;
	std::unique_ptr<LevelSystem> mLevel;
	// The D20 system actually holds on to object handles until it's destroyed,
	// which is why this system has to be here instead of where it originally was
	std::unique_ptr<ObjSystem> mObj;
	std::unique_ptr<D20System> mD20;
	std::unique_ptr<ParticleSysSystem> mParticleSys;
	std::unique_ptr<MapSystem> mMap;
	std::unique_ptr<ScrollSystem> mScroll;
	std::unique_ptr<LocationSystem> mLocation;
	std::unique_ptr<LightSystem> mLight;
	std::unique_ptr<TileSystem> mTile;
	std::unique_ptr<ONameSystem> mOName;
	std::unique_ptr<ObjectNodeSystem> mObjectNode;	
	std::unique_ptr<ProtoSystem> mProto;
	std::unique_ptr<ObjectSystem> mObject;
	std::unique_ptr<MapSectorSystem> mMapSector;
	std::unique_ptr<SectorVBSystem> mSectorVB;
	std::unique_ptr<TextBubbleSystem> mTextBubble;
	std::unique_ptr<TextFloaterSystem> mTextFloater;
	std::unique_ptr<JumpPointSystem> mJumpPoint;
	std::unique_ptr<TerrainSystem> mTerrain;
	std::unique_ptr<ClippingSystem> mClipping;	
	std::unique_ptr<HeightSystem> mHeight;
	std::unique_ptr<GMeshSystem> mGMesh;
	std::unique_ptr<PathNodeSystem> mPathNode;
	std::unique_ptr<LightSchemeSystem> mLightScheme;
	std::unique_ptr<PlayerSystem> mPlayer;
	std::unique_ptr<AreaSystem> mArea;
	std::unique_ptr<DialogSystem> mDialog;
	std::unique_ptr<SoundMapSystem> mSoundMap;
	std::unique_ptr<SoundGameSystem> mSoundGame;
	std::unique_ptr<ItemSystem> mItem;
	std::unique_ptr<CombatSystem> mCombat;
	std::unique_ptr<TimeEventSystem> mTimeEvent;
	std::unique_ptr<RumorSystem> mRumor;
	std::unique_ptr<QuestSystem> mQuest;
	std::unique_ptr<AISystem> mAI;
	std::unique_ptr<AnimSystem> mAnim;
	std::unique_ptr<AnimPrivateSystem> mAnimPrivate;
	std::unique_ptr<ReputationSystem> mReputation;
	std::unique_ptr<ReactionSystem> mReaction;
	std::unique_ptr<TileScriptSystem> mTileScript;
	std::unique_ptr<SectorScriptSystem> mSectorScript;
	std::unique_ptr<WPSystem> mWP;
	std::unique_ptr<InvenSourceSystem> mInvenSource;
	std::unique_ptr<TownMapSystem> mTownMap;
	std::unique_ptr<GMovieSystem> mGMovie;
	std::unique_ptr<BrightnessSystem> mBrightness;
	std::unique_ptr<GFadeSystem> mGFade;
	std::unique_ptr<AntiTeleportSystem> mAntiTeleport;
	std::unique_ptr<TrapSystem> mTrap;
	std::unique_ptr<MonsterGenSystem> mMonsterGen;
	std::unique_ptr<PartySystem> mParty;
	std::unique_ptr<D20LoadSaveSystem> mD20LoadSave;
	std::unique_ptr<GameInitSystem> mGameInit;
	std::unique_ptr<ObjFadeSystem> mObjFade;
	std::unique_ptr<DeitySystem> mDeity;
	std::unique_ptr<UiArtManagerSystem> mUiArtManager;
	std::unique_ptr<CheatsSystem> mCheats;
	std::unique_ptr<D20RollsSystem> mD20Rolls;
	std::unique_ptr<SecretdoorSystem> mSecretdoor;
	std::unique_ptr<MapFoggingSystem> mMapFogging;
	std::unique_ptr<RandomEncounterSystem> mRandomEncounter;
	std::unique_ptr<ObjectEventSystem> mObjectEvent;
	std::unique_ptr<FormationSystem> mFormation;
	std::unique_ptr<ItemHighlightSystem> mItemHighlight;
	std::unique_ptr<PathXSystem> mPathX;
	std::unique_ptr<PoisonSystem> mPoison;

	std::unique_ptr<class LegacyGameSystemResources> mLegacyResources;

	GUID mModuleGuid;
	std::string mModuleArchivePath;
	std::string mModuleDirPath;
};

extern GameSystems *gameSystems;
