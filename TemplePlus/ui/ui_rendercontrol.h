
#pragma once

#include <memory>

class QQuickWindow;
class QQuickView;

class UiRenderControl {
public:
	UiRenderControl();
	~UiRenderControl();

	void Render(QQuickWindow *view);
	void ProcessEvents();

	QQuickView *CreateView(const std::string &mainFile);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

