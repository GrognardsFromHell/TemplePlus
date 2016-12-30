
#include "stdafx.h"
#include "tig_console.h"
#include "debugui.h"

Console::Console() : mCommandBuf(1024, '\0') {
}

Console::~Console() {
}

void Console::Render()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);

	if (!ImGui::Begin("Console", &mOpen))
	{
		ImGui::End();
		return;
	}
	
	ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
	ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

	// TODO: display items starting from the bottom

	if (ImGui::SmallButton("Add Dummy Text")) {  } ImGui::SameLine();
	if (ImGui::SmallButton("Add Dummy Error")) {} ImGui::SameLine();
	if (ImGui::SmallButton("Clear")) Clear(); ImGui::SameLine();
	if (ImGui::SmallButton("Scroll to bottom")) ScrollToBottom();

	ImGui::Separator();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::PopStyleVar();
	ImGui::Separator();

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

	// TODO: Children

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
	if (ImGui::InputText("Input", &mCommandBuf[0], mCommandBuf.size(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, editCallback, (void*)this))
	{
		/*char* input_end = InputBuf + strlen(InputBuf);
		while (input_end > InputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
		if (InputBuf[0])
			ExecCommand(InputBuf);
		strcpy(InputBuf, "");*/
	}

	// Demonstrate keeping auto focus on the input box
	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	ImGui::End();
}

void Console::Clear()
{
}

void Console::Show()
{
}

void Console::Hide()
{
}

void Console::Append(const char * text)
{
	mLines.push_back(text);
}
