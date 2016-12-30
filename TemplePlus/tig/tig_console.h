
#pragma once

#include <EASTL/bonus/ring_buffer.h>
#include <EASTL/vector.h>
#include <string>

class Console {
public:
	Console();
	~Console();
	
	void Render();

	bool IsOpen() const {
		return mOpen;
	}

	void Clear();

	void ScrollToBottom() {
		mScrollToBottom = true;
	}

	void Execute(const std::string &command, bool skipHistory = false);

	void Show();
	void Hide();
	void Append(const char *text);

private:
	bool mOpen = false;
	bool mScrollToBottom = false;
	std::string mCommandBuf;

	eastl::ring_buffer<std::string, eastl::vector<std::string>> mLog;
	eastl::ring_buffer<std::string, eastl::vector<std::string>> mCommandHistory;

};
