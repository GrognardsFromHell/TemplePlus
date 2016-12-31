
#include "stdafx.h"
#include "tig_console.h"
#include "debugui.h"
#include "party.h"
#include "d20_level.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "critter.h"

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
	if (ImGui::Button("Levelup")){
		for (auto i = 0u; i < party.GroupListGetLen(); i++) {
			auto handle = party.GroupListGetMemberN(i);
			if (!party.ObjIsAIFollower(handle) && !d20LevelSys.CanLevelup(handle)) {
				auto obj = gameSystems->GetObj().GetObject(handle);

				auto curLvl = objects.StatLevelGet(handle, stat_level);
				auto xpReq = d20LevelSys.GetXpRequireForLevel(curLvl + 1);

				auto curXp = obj->GetInt32(obj_f_critter_experience);
				if ((int)xpReq > curXp)
					critterSys.AwardXp(handle, xpReq - curXp);
			}
		}
	}
	static char cheatsInput[256] = { 0, };
	ImGui::SameLine();
	if (ImGui::Button("Give")){
		int protoNum = 5001; // DO NOT USE arrow
		sscanf(cheatsInput, "%d", &protoNum);


		auto protoValid = true;
		if (protoNum < 1000 || protoNum > 20000) {
			logger->warn("Invalid proto for give command: {}", protoNum);
			protoValid = false;
		}


		auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoNum);
		if (!protoHandle)
			protoValid = false;

		auto leader = party.GetConsciousPartyLeader();
		auto loc = objects.GetLocation(leader);


		if (protoValid){
			auto handleNew = gameSystems->GetObj().CreateObject(protoHandle, loc);
			if (handleNew) {
				if (!inventory.SetItemParent(handleNew, leader, ItemInsertFlags::IIF_None)){
					objects.Destroy(handleNew);
				}
				else{
					auto obj = gameSystems->GetObj().GetObject(handleNew);
					obj->SetItemFlag(OIF_IDENTIFIED, 1);
				}
				
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Create")){
		int protoNum = 5001; // DO NOT USE arrow
		sscanf(cheatsInput, "%d", &protoNum);
		auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoNum);
		if (protoHandle){

			auto leader = party.GetConsciousPartyLeader();
			auto loc = objects.GetLocation(leader);

			auto handleNew = gameSystems->GetObj().CreateObject(protoHandle, loc);
			if (handleNew) {
				critterSys.GenerateHp(handleNew);
			}
			auto& consoleNewlyCreatedObj = temple::GetRef<objHndl>(0x10AA31B8);
			consoleNewlyCreatedObj = handleNew;
		}
	}

	static std::string cheatsInputDescr("Cheat Input");
	ImGui::SameLine();
	auto editCheatInputCallback = [](ImGuiTextEditCallbackData *self) -> int {
		int protoNum = 5001; // DO NOT USE arrow
		sscanf(cheatsInput, "%d", &protoNum);
		if (!strlen(cheatsInput))
			return 0;
		if (protoNum <= 1000 || protoNum >= 20000)
			return 0;
		auto protoHandle = gameSystems->GetObj().GetProtoHandle(protoNum);
		if (!protoHandle)
			return 0;

		cheatsInputDescr.clear();
		cheatsInputDescr.append(fmt::format("{}", description.getDisplayName(protoHandle)));
		return 0;
	};
	
	ImGui::PushItemWidth(100);
	if (ImGui::InputText(cheatsInputDescr.c_str(), &cheatsInput[0], 256, ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, editCheatInputCallback, (void*)this))
	{
		editCheatInputCallback(nullptr);
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

	// Command-line
	auto editCallback = [](ImGuiTextEditCallbackData *self) -> int {
		return 0;
	};

	ImGui::PushItemWidth(-1);
	if (ImGui::InputText("Input", &mCommandBuf[0], mCommandBuf.size(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, editCallback, (void*)this))
	{
		std::string command = mCommandBuf;
		command.resize(command.length()); // This discards trailing null-bytes
		trim(command);
		Execute(command);

		std::fill(mCommandBuf.begin(), mCommandBuf.end(), '\0'); // Clear command buffer
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
	}



}

void Console::Show()
{
	mOpen = true;
}

void Console::Toggle()
{
	mOpen = !mOpen;
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
