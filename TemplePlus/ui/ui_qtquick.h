
#pragma once

#include <memory>

#include "ui.h"

class UiQtQuick {
	friend struct Impl;
public:
	UiQtQuick();
	~UiQtQuick();

	LgcyWidgetId LoadWindow(int x, int y, int width, int height, const std::string &path);
private:
	struct Impl;
	std::unique_ptr<Impl> mImpl;
};
