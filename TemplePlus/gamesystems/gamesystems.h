
#pragma once

#pragma pack(push, 1)
struct GameSystemConf {
	int editor;
	int width;
	int height;
	int bufferstuffIdx;
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

class GameSystems {
public:
	explicit GameSystems(TigInitializer &tig);
	~GameSystems();
	const GameSystemConf &GetConfig() const {
		return mConfig;
	}

	// Makes a savegame.
	bool SaveGame(const std::string &filename, const std::string &displayName);
	
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

private:
	void RegisterDataFiles();
	void VerifyTemplePlusData();
	std::string GetLanguage();
	void PlayLegalMovies();
	void InitBufferStuff(const GameSystemConf& conf);
	void InitAnimationSystem();
	
	void ResizeScreen(int w, int h);
	
	TigInitializer &mTig;
	GameSystemConf mConfig;
	TigBufferstuffInitializer mTigBuffer;
	std::unique_ptr<class LegacyGameSystemResources> mLegacyResources;
};

extern GameSystems *gameSystems;
