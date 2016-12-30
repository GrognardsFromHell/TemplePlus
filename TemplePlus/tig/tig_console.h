
#pragma once

#include <EASTL/fixed_list.h>
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

	void Show();
	void Hide();
	void Append(const char *text);

private:
	bool mOpen = false;
	bool mScrollToBottom = false;
	std::string mCommandBuf;

	eastl::fixed_list<std::string, 1024, false> mLines;
};
