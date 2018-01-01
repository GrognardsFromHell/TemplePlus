
#include "stdafx.h"

#include <temple/dll.h>

#include "scripting.h"
#include "legacy.h"
#include "../gametime.h"
#include "python/python_embed.h"
#include "gamesystems.h"
#include "objects/gameobject.h"

static constexpr size_t sGlobalVarCount = 2000;
static constexpr size_t sGlobalFlagsCount = 100;

ScriptingSystem::ScriptingSystem() : 
	global_vars_(sGlobalVarCount, 0), 
	global_flags_(sGlobalFlagsCount, 0), 
	python_(std::make_unique<PythonScripting>())
{
	// Previously implemented @ 0x10006580
	temple::GetRef<int>(0x103073B4) = 0; // this is the editor_mode flag, if 1, all scripting is disabled
}

ScriptingSystem::~ScriptingSystem() {
	// Previously implemented @ 0x10007b60
}

void ScriptingSystem::LoadModule() {
	// Previously implemented @ 0x10006630
	// Previously mes\StoryState.mes was loaded here. Supposedly it was displayed in the main menu,
	// but it is only used by the old-style Arcanum menu, not the ToEE menu
}

void ScriptingSystem::UnloadModule() {
	// Previously implemented @ 0x10006650
}

void ScriptingSystem::Reset() {
	// Previously implemented @ 0x10007ae0
	std::fill(global_vars_.begin(), global_vars_.end(), 0);
	std::fill(global_flags_.begin(), global_flags_.end(), 0);

	python_->Reset();
}

bool ScriptingSystem::SaveGame(TioFile *file) {

	// Previously implemented @ 0x100066e0

	return tio_fwrite(&global_vars_[0], sizeof(int) * global_vars_.size(), 1, file) == 1
		&& tio_fwrite(&global_flags_[0], sizeof(int) * global_flags_.size(), 1, file) == 1
		&& tio_fwrite(&story_state_, sizeof(int), 1, file) == 1
		&& python_->SaveGame(file);

}

bool ScriptingSystem::LoadGame(GameSystemSaveFile* saveFile) {

	// Previously implemented @ 0x10006670

	return tio_fread(&global_vars_[0], sizeof(int) * global_vars_.size(), 1, saveFile->file) == 1
		&& tio_fread(&global_flags_[0], sizeof(int) * global_flags_.size(), 1, saveFile->file) == 1
		&& tio_fread(&story_state_, sizeof(int), 1, saveFile->file) == 1
		&& python_->LoadGame(saveFile);

}

const std::string &ScriptingSystem::GetName() const {
	static std::string name("Script");
	return name;
}

bool ScriptingSystem::ReadGlobalVars(GameSystemSaveFile * saveFile, std::vector<int>& globalVars, std::vector<int>& globalFlagsData, int & storyState) {
	globalVars.resize(2000);
	globalFlagsData.resize(100);
	if (!tio_fread(&globalVars[0], sizeof(int) * 2000, 1, saveFile->file)
		|| !tio_fread(&globalFlagsData[0], sizeof(int) * 100, 1, saveFile->file)
		|| !tio_fread(&storyState, sizeof(int), 1, saveFile->file)
		)
		return false;

	return true;
}

bool ScriptingSystem::ReadEncounterQueue(GameSystemSaveFile * saveFile, std::vector<int>& encounterQueue) {
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

int ScriptingSystem::GetGlobalFlag(int flag_idx) {
	// Previously implemented in 0x10006790
	auto word_idx = flag_idx / 32;
	auto bit_idx = flag_idx % 32;
	return (global_flags_[word_idx] >> bit_idx) & 1;
}

void ScriptingSystem::SetGlobalFlag(int flag_idx, bool value) {
	// Previously implemented in 0x100067C0
	auto word_idx = flag_idx / 32;
	auto bit_idx = flag_idx % 32;
	if (value) {
		global_flags_[word_idx] |= 1 << bit_idx;
	} else {
		global_flags_[word_idx] &= ~(1 << bit_idx);
	}
}

int ScriptingSystem::GetGlobalVar(int var_idx) {
	// Previously implemented in 0x10006760
	return global_vars_[var_idx];
}

void ScriptingSystem::SetGlobalVar(int var_idx, int value) {
	// Previously implemented in 0x10006770
	global_vars_[var_idx] = value;
}

int ScriptingSystem::GetStoryState() {
	// Previously implemented in 0x10006A20
	return story_state_;
}

void ScriptingSystem::SetStoryState(int story_state)
{
	// Previously implemented in 0x10006A3C

	// Only change the story state if it advances, never change it backwards
	if (story_state > story_state_) {
		story_state_ = story_state;
	}
}

int ScriptingSystem::InvokeScript(const ScriptInvocation &invocation)
{
	if (python_->IsPythonScriptId(invocation.script->scriptId)) {
		return python_->InvokePythonScript(invocation);
	}

	logger->info("Ignoring invocation for non-python script {}", invocation.script->scriptId);

	return 0;
}

struct ScriptingReplacement : public TempleFix {
public:
	
	virtual void apply() override
	{
		// script_invoke
		replaceFunction<int(ScriptInvocation *)>(0x1000bb60, [](ScriptInvocation *invocation) {
			return gameSystems->GetScript().InvokeScript(*invocation);
		});

		// script_get_story_state (used somewhere in the dialog system and save info)
		replaceFunction<int()>(0x10006a20, []() {
			return gameSystems->GetScript().GetStoryState();
		});

		// set story state was only used in legacy scripting and python and thus hasn't been replaced

		// global vars state was only ever accessed from python

		// script_global_flag_get
		replaceFunction<int(int)>(0x10006790, [](int flag_idx) {
			return gameSystems->GetScript().GetGlobalFlag(flag_idx) ? 1 : 0;
		});

		// script_global_flag_set
		replaceFunction<void(int, int)>(0x100067c0, [](int flag_idx, int value) {
			gameSystems->GetScript().SetGlobalFlag(flag_idx, value == 1);
		});

	}

} scriptingReplacement;
