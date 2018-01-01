
#pragma once

#include <vector>

#include "gamesystem.h"
#include "../obj_structs.h"

class PythonScripting;
struct ObjectScript;

enum class ObjScriptEvent : uint32_t {
	Examine = 0,
	Use,
	Destroy,
	Unlock,
	Get,
	Drop,
	Throw,
	Hit,
	Miss,
	Dialog = 9,
	FirstHeartbeat,
	CatchingThiefPc,
	Dying,
	EnterCombat,
	ExitCombat,
	StartCombat,
	EndCombat,
	BuyObject,
	Resurrect,
	Heartbeat,
	LeaderKilling,
	InsertItem,
	WillKos,
	TakingDamage,
	WieldOn,
	WieldOff,
	CritterHits,
	NewSector,
	RemoveItem,
	LeaderSleeping,
	Bust,
	DialogOverride,
	Transfer,
	CaughtThief,
	CriticalHit,
	CriticalMiss,
	Join,
	Disband,
	NewMap,
	Trap,
	TrueSeeing,
	SpellCast,
	UnlockAttempt
};

struct ScriptInvocation {
	ObjectScript *script;
	int start_instruction = 0; // Only used for legacy system
	objHndl triggerer = objHndl::null;
	objHndl attachee = objHndl::null;
	int spell_id = 0;
	int arg4 = 0;
	ObjScriptEvent event;
};

class ScriptingSystem : public GameSystem, public SaveGameAwareGameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "Script";
	ScriptingSystem();
	~ScriptingSystem();
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string &GetName() const override;

	bool ReadGlobalVars(GameSystemSaveFile *saveFile, std::vector<int> & globalVars, std::vector<int> & globalFlagsData, int& storyState);
	bool ReadEncounterQueue(GameSystemSaveFile *saveFile, std::vector<int> & encounterQueue);

	int GetGlobalFlag(int flagIdx);
	int GetGlobalVar(int varIdx);
	void SetGlobalFlag(int flagIdx, bool enable);
	void SetGlobalVar(int flagIdx, int value);

	int GetStoryState();
	void SetStoryState(int story_state);

	int InvokeScript(const ScriptInvocation &invocation);

private:
	struct PythonImpl;

	static constexpr size_t SCRIPT_CACHE_COUNT = 100;
	std::vector<int> global_vars_;
	std::vector<int> global_flags_;
	int story_state_ = 0;

	std::unique_ptr<PythonScripting> python_;

};
