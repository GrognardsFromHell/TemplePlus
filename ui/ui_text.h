
#pragma once

struct UiTextPrivate;

class UiText {
public:
	void Initialize();
	void Uninitialize();
	void Update();
	void Render();
private:
	UiTextPrivate *d = nullptr;
};

extern UiText uiText;
