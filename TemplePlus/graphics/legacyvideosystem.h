
#pragma once

#include <memory>

class MainWindow;
class Graphics;

class LegacyVideoSystem {
public:
	LegacyVideoSystem(MainWindow &mainWindow, Graphics &graphics);
	~LegacyVideoSystem();

private:
	std::unique_ptr<class LegacyResourceManager> mResources;
};

