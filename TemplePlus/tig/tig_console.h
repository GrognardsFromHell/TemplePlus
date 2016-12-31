
#pragma once

#include <EASTL/bonus/ring_buffer.h>
#include <EASTL/vector.h>
#include <string>
#include <functional>

struct ImGuiTextEditCallbackData;

// Returns 1 if it can handle the command
typedef int(__cdecl *CheatFn)(const char *command);
typedef void(__cdecl *CheatSetStateFn)(const char *command, int state);

const int cheatCount = 40;

#pragma pack(push, 1)
struct Cheat {
	char *name;
	int unk1;
	CheatSetStateFn setState;
	CheatFn handler;
};
#pragma pack(pop)

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

	void RunBatchFile(const std::string &path);

	void Show();
	void Hide();
	void Append(const char *text);

	void SetCheats(Cheat *cheats, size_t count);
	void SetCommandInterpreter(std::function<void(const std::string&)> interpreter);

private:
	bool mOpen = false;
	bool mScrollToBottom = false;
	std::string mCommandBuf;
	std::function<void(const std::string&)> mInterpreter;

	eastl::vector<Cheat> mCheats;

	eastl::ring_buffer<std::string, eastl::vector<std::string>> mLog;
	eastl::ring_buffer<std::string, eastl::vector<std::string>> mCommandHistory;
	int mCommandHistoryPos = -1;

	bool ExecuteCheat(const std::string &command);
	void ExecuteScript(const std::string &command);

	static int CommandEditCallback(ImGuiTextEditCallbackData *data);

};
