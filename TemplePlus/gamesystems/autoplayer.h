#pragma once
#include "gamesystem.h"

class AutoplayerSystem : public GameSystem, /*public SaveGameAwareGameSystem, */ // savegame awareness breaks savegame backwards compatibility
	public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Autoplayer";
	AutoplayerSystem();
	~AutoplayerSystem();
	void Reset() override;
	/*bool SaveGame(TioFile* file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;*/
	void AdvanceTime(uint32_t time) override;
	const std::string& GetName() const override;
};