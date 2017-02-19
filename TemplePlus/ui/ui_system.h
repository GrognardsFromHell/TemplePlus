
#pragma once

#include <QObject>

struct TioFile;
struct UiResizeArgs;

struct UiSaveFile {
	uint32_t saveVersion;
	TioFile* file;
};

class SaveGameAwareUiSystem {
public:
	virtual ~SaveGameAwareUiSystem() = 0;

	virtual bool SaveGame(TioFile* tioFile) {
		return true;
	}

	virtual bool LoadGame(const UiSaveFile &saveGame) {
		return true;
	}
};

inline SaveGameAwareUiSystem::~SaveGameAwareUiSystem() = default;

class UiSystem : public QObject {
Q_OBJECT
public:
	virtual void Reset() {
	}
	virtual void LoadModule() {
	}
	virtual void UnloadModule() {
	}
	virtual void ResizeViewport(const UiResizeArgs &resizeArgs) {
	}

	virtual const std::string &GetName() const = 0;
};
