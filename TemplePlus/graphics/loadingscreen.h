
#pragma once

#include <memory>
#include <string>
#include <cstdint>

class Graphics;

class LoadingScreen {
public:
	explicit LoadingScreen(Graphics &g);
	~LoadingScreen();

	void SetProgress(float progress);
	float GetProgress() const;

	void SetMessageId(uint32_t messageId);
	void SetMessage(const std::string &message);

	void SetImage(const std::string &imagePath);

	void Render();
private:
	struct Impl;
	std::unique_ptr<Impl> mImpl;
};
