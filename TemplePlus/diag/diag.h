#pragma once

#include <memory>

class Graphics;

class DiagScreen {
public:
	explicit DiagScreen(Graphics& g);
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
	Graphics& mGraphics;

	std::string FormatMemSize(size_t memory);

	bool mEnabled = false;
};

extern DiagScreen *diagScreen;
