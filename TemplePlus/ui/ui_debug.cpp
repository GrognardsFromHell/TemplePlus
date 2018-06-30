
#include "stdafx.h"
#include "ui.h"
#include "ui_debug.h"
#include "ui/widgets/widgets.h"
#include "tig/tig_startup.h"
#include "config/config.h"
#include <graphics/shaperenderer2d.h>
#include "animgoals/animgoals_debugrenderer.h"

#include <debugui.h>

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
		static bool checked;
		ImGui::Checkbox("Render Current Goals", &checked);
		AnimGoalsDebugRenderer::Enable(checked);
	}

	if (ImGui::CollapsingHeader("Rendering Debugging")) {
		ImGui::Checkbox("Draw Cylinder Hitboxes", &config.drawObjCylinders);
	}

	ImGui::End();
}
