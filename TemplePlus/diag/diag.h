#pragma once

#include <memory>

namespace gfx {
	class RenderingDevice;
}

class GameSystems;
class GameRenderer;

class DiagScreen {
public:
	explicit DiagScreen(gfx::RenderingDevice& device,
		GameSystems &gameSystems,
		GameRenderer &gameRenderer);
	~DiagScreen();
		
	void Render();

	void SetEnabled(bool enabled) {
		mEnabled = enabled;
	}

	bool IsEnabled() const {
		return mEnabled;
	}

	void Toggle();
private:
	class Impl;
	std::unique_ptr<Impl> mImpl;
	gfx::RenderingDevice& mDevice;
	GameSystems &mGameSystems;
	GameRenderer &mGameRenderer;

	std::string FormatMemSize(size_t memory);

	bool mEnabled = false;
};

extern DiagScreen *diagScreen;
