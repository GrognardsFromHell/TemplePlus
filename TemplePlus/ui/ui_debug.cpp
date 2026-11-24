
#include "stdafx.h"
#include "ui.h"
#include "ui_debug.h"
#include "alert_debug.h"
#include "gameview.h"
#include "gamesystems/mapsystem.h"
#include "obj.h"
#include <graphics/textengine.h>
#include "ui/widgets/widget_styles.h"
#include "ui/widgets/widgets.h"
#include "tig/tig_startup.h"
#include "tig/tig_console.h"
#include "config/config.h"
#include <graphics/shaperenderer2d.h>
#include "animgoals/animgoals_debugrenderer.h"

#include <debugui.h>
#include <messages\messagequeue.h>
#include <tig\tig_keyboard.h>
#include <gamesystems/gamesystems.h>
#include <animgoals/anim.h>
#include <animgoals/anim_slot.h>
#include <gamesystems/objects/objsystem.h>

static bool debugUiVisible = false;

namespace alertDebug {
	struct Arc {
		XMFLOAT3 from, to;
		uint64_t expires;
		int map;
		std::string label;
	};
	static std::vector<Arc> arcs;
	static bool enabled = false;
	bool Enabled() { return enabled; }
	void SetEnabled(bool value) {
		enabled = value;
		arcs.clear();
	}
	void Record(objHndl center, objHndl ally, int range, const char* reason) {
		auto a = objects.GetLocationFull(center), b = objects.GetLocationFull(ally);
		auto from = a.ToInches3D(12), to = b.ToInches3D(12);
		auto distance = std::hypot(from.x - to.x, from.z - to.z) / INCH_PER_TILE;
		auto map = gameSystems->GetMap().GetCurrentMapId();
		logger->info("Ally alert [{}] map {}: {} ({},{}) -> {} ({},{}), {:.1f} tiles, search {}",
			reason, map, center, a.location.locx, a.location.locy, ally, b.location.locx, b.location.locy, distance, range);
		if (arcs.size() >= 64) arcs.erase(arcs.begin());
		arcs.push_back({from, to, GetTickCount64() + 5000, map, fmt::format("{:.1f} tiles", distance)});
	}
	void Render() {
		if (arcs.empty()) return;
		auto now = GetTickCount64();
		auto map = gameSystems->GetMap().GetCurrentMapId();
		auto style = widgetTextStyles->GetTextStyle("default-button-text");
		style.align = gfx::TextAlign::Center;
		style.dropShadow = true;
		for (auto it = arcs.begin(); it != arcs.end();) {
			if (it->expires <= now || it->map != map) { it = arcs.erase(it); continue; }
			auto a = gameView->WorldToScreenUi(it->from), b = gameView->WorldToScreenUi(it->to);
			float lift = std::min(60.f, 12.f + 0.15f * std::hypot(b.x - a.x, b.y - a.y));
			gfx::Line2d lines[18];
			auto prev = a;
			XMCOLOR color(1.f, 0.68f, 0.15f, 1.f);
			for (int i = 0; i < 16; ++i) {
				float t = (i+1)/16.f;
				XMFLOAT2 p(a.x + t*(b.x-a.x), a.y + t*(b.y-a.y) - 4*t*(1-t)*lift);
				lines[i] = gfx::Line2d(prev, p, color);
				prev = p;
			}
			float dx = b.x-lines[15].from.x, dy = b.y-lines[15].from.y;
			float scale = 8.f/std::max(0.01f, std::hypot(dx,dy));
			dx *= scale; dy *= scale;
			lines[16] = gfx::Line2d(b, {b.x-dx+dy/2, b.y-dy-dx/2}, color);
			lines[17] = gfx::Line2d(b, {b.x-dx-dy/2, b.y-dy+dx/2}, color);
			tig->GetShapeRenderer2d().DrawLines(lines);
			TigRect rect{int((a.x+b.x)/2)-55, int((a.y+b.y)/2-lift)-20, 110, 22};
			tig->GetRenderingDevice().GetTextEngine().RenderText(rect, style, it->label);
			++it;
		}
	}
}
static std::vector<std::string> autoplayerGoals;

void UISetAutoplayerGoals(const std::vector<std::string>& goals)
{
	autoplayerGoals = goals;
}

static void RenderAutoplayerGoals()
{
	if (autoplayerGoals.empty()) return;
	const auto screen = ImGui::GetIO().DisplaySize;
	const float margin = 12.0f;
	const float width = std::min(300.0f, screen.x - 2 * margin);
	if (width <= 0) return;

	const auto& style = ImGui::GetStyle();
	const float contentWidth = std::max(1.0f, width - 2 * style.WindowPadding.x - 2);
	float height = 2 * style.WindowPadding.y + ImGui::GetTextLineHeight() + 2;
	for (const auto& goal : autoplayerGoals) {
		height += ImGui::CalcTextSize(goal.c_str(), nullptr, false, contentWidth).y + style.ItemSpacing.y;
	}
	ImGui::SetNextWindowPos(ImVec2(screen.x - width - margin, margin));
	ImGui::SetNextWindowSize(ImVec2(width, height));
	const auto flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.09f, 0.76f));
	if (ImGui::Begin("##autoplayerGoals", nullptr, flags)) {
		ImGui::TextColored(ImVec4(0.93f, 0.79f, 0.52f, 1), "AUTOPLAYER");
		ImGui::PushTextWrapPos(0);
		for (const auto& goal : autoplayerGoals) {
			ImGui::TextUnformatted(goal.c_str());
		}
		ImGui::PopTextWrapPos();
	}
	ImGui::End();
	ImGui::PopStyleColor();
}

void UIShowDebug()
{
	debugUiVisible = true;
}

static void DrawWidgetTreeNode(int widgetId);

static bool HasName(LgcyWidget *widget) {
	if (!widget->name[0]) {
		return false;
	}

	// Some names are just filled with garbage... We try to catch that here by checking
	// if any of the name's chars is a null terminator
	for (int i = 0; i < 64; i++) {
		if (!widget->name[i]) {
			return true;
		}
	}

	return false;
	
}

static void DrawLegacyWidgetTreeNode(LgcyWidget *widget) {

	std::string textEntry;
	if (widget->IsWindow()) {
		textEntry += "[wnd] ";
	} else if (widget->IsButton()) {
		textEntry += "[btn] ";
	} else if (widget->IsScrollBar()) {
		textEntry += "[scr] ";
	}

	textEntry += std::to_string(widget->widgetId);

	if (HasName(widget)) {
		textEntry.push_back(' ');
		textEntry.append(widget->name);
	}

	bool opened = ImGui::TreeNode(textEntry.c_str());

// Draw the widget outline regardless of whether the tree node is opend
if (ImGui::IsItemHovered()) {
	tig->GetShapeRenderer2d().DrawRectangleOutline(
		{ (float)widget->x, (float)widget->y },
		{ (float)widget->x + widget->width, (float)widget->y + widget->height },
		XMCOLOR(1, 1, 1, 1)
	);
}

if (!opened) {
	return;
}

ImGui::BulletText(" X:%d Y:%d W:%d H:%d", widget->x, widget->y, widget->width, widget->height);
ImGui::BulletText(" Renderer: %x", widget->render);
ImGui::BulletText(" Msg Handler: %x", widget->handleMessage);

if (widget->IsWindow()) {
	auto window = (LgcyWindow*)widget;
	for (size_t i = 0; i < window->childrenCount; i++) {
		DrawWidgetTreeNode(window->children[i]);
	}
}

ImGui::TreePop();

}

static void DrawAdvWidgetTreeNode(WidgetBase* widget) {

	std::string textEntry;
	if (widget->IsContainer()) {
		textEntry += "[cnt] ";
	}
	else if (widget->IsButton()) {
		textEntry += "[btn] ";
	}
	else {
		textEntry += "[unk] ";
	}

	textEntry += fmt::format("{} ", widget->GetWidgetId());

	if (!widget->GetId().empty()) {
		textEntry += widget->GetId();
		textEntry.append(" ");
	}

	textEntry += widget->GetSourceURI();

	bool opened = ImGui::TreeNode(textEntry.c_str());

	// Draw the widget outline regardless of whether the tree node is opend
	if (ImGui::IsItemHovered()) {
		auto contentArea = widget->GetContentArea();
		tig->GetShapeRenderer2d().DrawRectangleOutline(
			{ (float)contentArea.x, (float)contentArea.y },
			{ (float)contentArea.x + contentArea.width, (float)contentArea.y + contentArea.height },
			XMCOLOR(1, 1, 1, 1)
		);
	}

	if (!opened) {
		return;
	}

	ImGui::BulletText(" X:%d Y:%d W:%d H:%d", widget->GetX(), widget->GetY(), widget->GetWidth(), widget->GetHeight());
	if (!widget->IsVisible()) {
		ImGui::SameLine();
		ImGui::TextColored({ 1, 0, 0, 1 }, "[Hidden]");
	}

	if (widget->IsContainer()) {
		auto window = (WidgetContainer*)widget;
		for (auto& child : window->GetChildren()) {
			DrawAdvWidgetTreeNode(child.get());
		}
	}

	ImGui::TreePop();

}

static void DrawWidgetTreeNode(int widgetId) {

	auto widget = uiManager->GetWidget(widgetId);
	auto advWidget = uiManager->GetAdvancedWidget(widgetId);

	if (advWidget) {
		DrawAdvWidgetTreeNode(advWidget);
	}
	else {
		DrawLegacyWidgetTreeNode(widget);
	}

}

static void DrawAnimSlots();

void UIRenderDebug()
{
	RenderAutoplayerGoals();
	if (!debugUiVisible) {
		return;
	}
	ImGui::Begin("Debug Info", &debugUiVisible);
	if (ImGui::Button("Console")) {
		tig->GetConsole().Toggle();
	}
	if (ImGui::CollapsingHeader("Top Level Windows (Bottom to Top)")) {

		for (auto &widgetId : uiManager->GetActiveWindows()) {
			DrawWidgetTreeNode(widgetId);
		}

	}

	if (ImGui::CollapsingHeader("Viewports")) {
		ImGui::Text(fmt::format("Cur idx: {}", temple::GetRef<int>(0x10BD3B44)).c_str());
		for (auto i = 0; i < 10; ++i) {
			ImGui::Text(fmt::format("{}: {}", i, temple::GetRef<int[10]>(0x10BD3AF8)[i]).c_str());
		}
	}

	if (ImGui::CollapsingHeader("Anim Goals Debugging")) {
		static bool showGoalsChecked;
		ImGui::Checkbox("Render Current Goals", &showGoalsChecked);
		AnimGoalsDebugRenderer::Enable(showGoalsChecked);

		static bool showNamesChecked;
		ImGui::Checkbox("Render Object Names", &showNamesChecked);
		AnimGoalsDebugRenderer::EnableObjectNames(showNamesChecked);
		DrawAnimSlots();
	}

	if (ImGui::CollapsingHeader("Rendering Debugging")) {
		ImGui::Checkbox("Debug Clipping", &config.debugClipping);
		ImGui::Checkbox("Debug Particle Systems", &config.debugPartSys);
		ImGui::Checkbox("Draw Cylinder Hitboxes", &config.drawObjCylinders);
	}

	if (messageQueue)
	if (ImGui::CollapsingHeader("Message Debugging")) {
		static bool debugMsgs;
		ImGui::Checkbox("Debug Messages", &debugMsgs);
		if (debugMsgs) {
			auto& dbgQueue = messageQueue->GetDebugMsgs();
			for (auto& msg : dbgQueue) {
				std::string text;
				text += "Time " + std::to_string(msg.createdMs);
				switch (msg.type) {
				case TigMsgType::MOUSE:
					text += fmt::format(" Mouse ({}, {}) {} flags: 0x{:x}", msg.arg1, msg.arg2, msg.arg3, msg.arg4);
					break;
				case TigMsgType::CHAR:
					text += fmt::format(" Char 0x{:x} {} {} {}", ((TigCharMsg&)msg).charCode, ((TigCharMsg&)msg)._unused1, ((TigCharMsg&)msg)._unused2, ((TigCharMsg&)msg)._unused3);
					break;
				case TigMsgType::KEYSTATECHANGE:
					text += fmt::format(" Keystate 0x{:x} down: {}", ((TigKeyStateChangeMsg&)msg).key, ((TigKeyStateChangeMsg&)msg).down);
					break;
				case TigMsgType::KEYDOWN:
					text += fmt::format(" Key Down ({}, {})", msg.arg1, msg.arg2);
					break;
				case TigMsgType::WIDGET:
					text += fmt::format(" Widget {}, {}, {}, {}", msg.arg1, msg.arg2, msg.arg3, msg.arg4);
					break;
					
				}
				ImGui::BulletText(text.c_str());
				
			}
		}
	}

	ImGui::End();
}

void DrawAnimSlots()
{
	auto& anim = gameSystems->GetAnim();
	for (auto i = 0; i < AnimSlotCount; ++i) {
		AnimSlot &slot = anim.GetRunSlot(i);
		if (!slot.flags) {
			//if (!objSystem->IsValidHandle(slot.animObj))
				continue;
		}
		if(ImGui::TreeNode(fmt::format("{}: {} {:x}", i, slot.animObj, slot.flags).c_str())) {
			ImGui::Text(fmt::format("Current goal {}", slot.currentGoal).c_str());
			ImGui::TreePop();
		}
	}
	
}
