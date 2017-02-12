
#pragma once

#include <memory>

class QQuickWindow;

class UiRenderControl {
public:
	UiRenderControl();
	~UiRenderControl();

	void Render(QQuickWindow *view);
	void ProcessEvents();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

