
#pragma once

#include <string>
#include <cstdint>

struct TioFile;
struct GameSystemSaveFile;
struct RebuildBufferInfo;

class NamedGameSystem {
public:
	virtual ~NamedGameSystem() = 0;

	virtual const std::string &GetName() const = 0;
};

inline NamedGameSystem::~NamedGameSystem() = default;

class ResetAwareGameSystem : virtual public NamedGameSystem {
public:
	virtual ~ResetAwareGameSystem() = 0;

	virtual void Reset() = 0;
};

inline ResetAwareGameSystem::~ResetAwareGameSystem() = default;

class ModuleAwareGameSystem : virtual public NamedGameSystem {
public:
	virtual ~ModuleAwareGameSystem() = 0;

	virtual void LoadModule() {}
	virtual void UnloadModule() {}
};

inline ModuleAwareGameSystem::~ModuleAwareGameSystem() = default;

class TimeAwareGameSystem : virtual public NamedGameSystem {
public:
	virtual ~TimeAwareGameSystem() = 0;

	virtual void AdvanceTime(uint32_t time) = 0;
};

inline TimeAwareGameSystem::~TimeAwareGameSystem() = default;

class SaveGameAwareGameSystem : virtual public NamedGameSystem {
public:
	virtual ~SaveGameAwareGameSystem() = 0;

	virtual bool SaveGame(TioFile *file) = 0;
	virtual bool LoadGame(GameSystemSaveFile* saveFile) = 0;
};

inline SaveGameAwareGameSystem::~SaveGameAwareGameSystem() = default;

class BufferResettingGameSystem : virtual public NamedGameSystem {
public:
	virtual ~BufferResettingGameSystem() = 0;

	virtual void ResetBuffers(const RebuildBufferInfo& rebuildInfo) = 0;
};

inline BufferResettingGameSystem::~BufferResettingGameSystem() = default;

class GameSystem : virtual public NamedGameSystem {
public:
	virtual ~GameSystem() = default;
};
