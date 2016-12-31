
#include "stdafx.h"
#include "tig_console.h"
#include "debugui.h"

Console::Console() : mLog(1024), mCommandHistory(100), mCommandBuf(1024, '\0') {
}

Console::~Console() {
}

void Console::Render()
{
	if (!mOpen) {
		return;
	}

	constexpr auto consoleWidgeFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

	auto size = ImGui::GetIO().DisplaySize;
	size.y /= 2;
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (!ImGui::Begin("Console", &mOpen, consoleWidgeFlags))
	{
		ImGui::End();
		return;
	}

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Selectable("Clear")) Clear();
		ImGui::EndPopup();
	}

	// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
	// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
	// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
	//     ImGuiListClipper clipper(Items.Size);
	//     while (clipper.Step())
	//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
	// However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
	// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
	// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
	// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

	for (auto it = mLog.begin(); it != mLog.end(); it++)
	{
		auto item = it->c_str();
		ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // A better implementation may store a type per-item. For the sample let's just parse the text.
		if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
		else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}

	if (mScrollToBottom)
		ImGui::SetScrollHere();
	mScrollToBottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	ImGui::PushItemWidth(-1);
	if (ImGui::InputText("Input", &mCommandBuf[0], mCommandBuf.size(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, Console::CommandEditCallback, this))
	{
		std::string command = mCommandBuf;
		command.resize(command.length()); // This discards trailing null-bytes
		trim(command);
		Execute(command);

		std::fill(mCommandBuf.begin(), mCommandBuf.end(), '\0'); // Clear command buffer

		// Refocus the control
		ImGui::SetKeyboardFocusHere(-1);
	}

	ImGui::End();
}

void Console::Clear()
{
	mLog.clear();
}

void Console::Execute(const std::string &command, bool skipHistory)
{
	if (!skipHistory) {
		// Remove it from the history first
		auto it = eastl::find(mCommandHistory.begin(), mCommandHistory.end(), command);
		if (it != mCommandHistory.end()) {
			mCommandHistory.erase(it);
		}

		// Append it at the end
		mCommandHistory.push_back(command);
		
		// Reset cur pos within the list
		mCommandHistoryPos = -1;
	}

	if (!ExecuteCheat(command)) {
		ExecuteScript(command);
	}

}

void Console::RunBatchFile(const std::string & path)
{
}

void Console::Show()
{
	mOpen = true;
}

void Console::Hide()
{
	mOpen = false;
}

void Console::Append(const char * text)
{
	std::string line(text);
	rtrim(line);
	mLog.push_back(line);
	ScrollToBottom();
}

void Console::SetCheats(Cheat * cheats, size_t count)
{
	mCheats.resize(count);
	for (size_t i = 0; i < count; i++) {
		mCheats[i] = cheats[i];
	}
}

void Console::SetCommandInterpreter(std::function<void(const std::string&)> interpreter)
{
	mInterpreter = interpreter;
}

bool Console::ExecuteCheat(const std::string & command)
{
	for (auto &cheat : mCheats) {

		if (!cheat.handler) {
			continue; // Dont bother if it doesn't have a handler
		}

		int nameLen = strlen(cheat.name);
		// This is equivalent to a case insensitive "startsWith"
		if (!_strnicmp(&command[0], cheat.name, nameLen)) {
			if (cheat.handler(command.c_str()) == 1) {
				return true;
			}
		}
	}

	return false;
}

void Console::ExecuteScript(const std::string & command)
{
	if (mInterpreter) {
		mInterpreter(command);
	}
}

int Console::CommandEditCallback(ImGuiTextEditCallbackData * data)
{
	Console *console = (Console*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
	{
		// Example of HISTORY
		const int prev_history_pos = console->mCommandHistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (console->mCommandHistoryPos == -1)
				console->mCommandHistoryPos = console->mCommandHistory.size() - 1;
			else if (console->mCommandHistoryPos > 0)
				console->mCommandHistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (console->mCommandHistoryPos != -1)
				if (++console->mCommandHistoryPos >= console->mCommandHistory.size())
					console->mCommandHistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != console->mCommandHistoryPos)
		{
			data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (console->mCommandHistoryPos >= 0) ? console->mCommandHistory[console->mCommandHistoryPos].c_str() : "");
			data->BufDirty = true;
		}
	}
	return 0;
}
