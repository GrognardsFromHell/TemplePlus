
#pragma once

struct UiTextPrivate;

class UiText {
public:
	void Initialize();
	void Uninitialize();
	void Update();
	void Render();

	bool HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);
private:
	UiTextPrivate *d = nullptr;
};

extern UiText uiText;
