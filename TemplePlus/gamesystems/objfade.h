#pragma once
#include "gametime.h"
#include "gamesystem.h"
struct GameSystemConf;


struct ObjFadeArgs
{
	int id;
	int initialOpacity;
	int goalOpacity;
	int tickMs;
	int tickQuantum;
	int flags;
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

	void SetValidationObj(objHndl handle);
	objHndl GetValidationObj();
	int AppendToTable(int quantum, int initialOpacity, int goalOpacity, int tickTimeMs, int flags);
	void RemoveFromTable(int id);
};