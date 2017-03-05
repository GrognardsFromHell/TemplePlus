
#include "stdafx.h"
#include "ui.h"
#include "ui_debug.h"
#include "tig/tig_startup.h"
#include <graphics/shaperenderer2d.h>

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

	if (widget->IsWindow()) {
		auto window = (LgcyWindow*)widget;
		for (size_t i = 0; i < window->childrenCount; i++) {
			DrawWidgetTreeNode(window->children[i]);
		}
	}

	ImGui::TreePop();

}

static void DrawWidgetTreeNode(int widgetId) {
	
	auto widget = uiManager->GetWidget(widgetId);
	DrawLegacyWidgetTreeNode(widget);

}

void UIRenderDebug()
{
	if (!debugUiVisible) {
		return;
	}
	ImGui::Begin("UI System", &debugUiVisible);

	if (ImGui::CollapsingHeader("Top Level Windows (Bottom to Top)")) {

		for (auto &widgetId : uiManager->GetActiveWindows()) {
			DrawWidgetTreeNode(widgetId);
		}

	}

	ImGui::End();
}
