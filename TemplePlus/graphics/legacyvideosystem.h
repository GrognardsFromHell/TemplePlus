
#pragma once

#include <memory>

class MainWindow;

namespace gfx {
	class RenderingDevice;
}

class LegacyVideoSystem {
public:
	LegacyVideoSystem(MainWindow &mainWindow, gfx::RenderingDevice &device);
	~LegacyVideoSystem();

private:
	std::unique_ptr<class LegacyResourceManager> mResources;
};

