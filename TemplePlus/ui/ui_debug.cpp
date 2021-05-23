
#include "stdafx.h"
#include "ui.h"
#include "ui_debug.h"
#include "ui/widgets/widgets.h"
#include "tig/tig_startup.h"
#include "config/config.h"
#include <graphics/shaperenderer2d.h>
#include "animgoals/animgoals_debugrenderer.h"

#include <debugui.h>
#include <messages\messagequeue.h>
#include <tig\tig_keyboard.h>

static bool debugUiVisible = false;

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
			{ (float) widget->x, (float) widget->y },
			{ (float) widget->x + widget->width, (float) widget->y + widget->height },
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

void DrawContentTreeNode(WidgetContent* content) {
	std::string textEntry;
	if (content->IsText()) {
		textEntry += "[txt]: ";
		textEntry += ((WidgetText*)content)->GetText();
	}
	else if (content->IsImage()) {
		textEntry += "[img] ";
	}
	else {
		textEntry += "[unk] ";
	}
	ImGui::BulletText(textEntry.c_str());
	ImGui::BulletText(" X:%d Y:%d FixedW:%d FixedH:%d", content->GetX(), content->GetY(), content->GetFixedWidth(), content->GetFixedHeight());
	auto &contentArea = content->GetContentArea();
	ImGui::BulletText("Content area: X:%d Y:%d W:%d H:%d", contentArea.x, contentArea.y, contentArea.width, contentArea.height);
	
}

static void DrawAdvWidgetTreeNode(WidgetBase *widget) {
	
	std::string textEntry;
	if (widget->IsContainer()) {
		textEntry += "[cnt] ";
	} else if (widget->IsButton()) {
		textEntry += "[btn] ";
	} else {
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
			{ (float) contentArea.x, (float) contentArea.y },
			{ (float) contentArea.x + contentArea.width, (float) contentArea.y + contentArea.height },
			XMCOLOR(1, 1, 1, 1)
		);
	}

	if (!opened) {
		return;
	}

	ImGui::BulletText(" X:%d Y:%d W:%d H:%d", widget->GetX(), widget->GetY(), widget->GetWidth(), widget->GetHeight());
	if (!widget->IsVisible()) {
		ImGui::SameLine();
		ImGui::TextColored({1, 0, 0, 1}, "[Hidden]");
	}

	if (widget->IsContainer()) {
		auto window = (WidgetContainer*)widget;
		for (auto &child : window->GetChildren()) {
			DrawAdvWidgetTreeNode(child.get());
		}
	}

	for (auto& content : widget->GetContent()) {
		DrawContentTreeNode(content.get());
	}

	ImGui::TreePop();

}

static void DrawWidgetTreeNode(int widgetId) {
	
	auto widget = uiManager->GetWidget(widgetId);
	auto advWidget = uiManager->GetAdvancedWidget(widgetId);

	if (advWidget) {
		DrawAdvWidgetTreeNode(advWidget);
	} else {
		DrawLegacyWidgetTreeNode(widget);
	}

}

void UIRenderDebug()
{
	if (!debugUiVisible) {
		return;
	}
	ImGui::Begin("Debug Info", &debugUiVisible);

	if (ImGui::CollapsingHeader("Top Level Windows (Bottom to Top)")) {

		for (auto &widgetId : uiManager->GetActiveWindows()) {
			DrawWidgetTreeNode(widgetId);
		}

	}

	if (ImGui::CollapsingHeader("Anim Goals Debugging")) {
		static bool showGoalsChecked;
		ImGui::Checkbox("Render Current Goals", &showGoalsChecked);
		AnimGoalsDebugRenderer::Enable(showGoalsChecked);

		static bool showNamesChecked;
		ImGui::Checkbox("Render Object Names", &showNamesChecked);
		AnimGoalsDebugRenderer::EnableObjectNames(showNamesChecked);
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
