
#include "stdafx.h"

#include "ui_legacy.h"
#include "util/fixes.h"

static UiLegacyManager &manager = uiLegacyManager;

static int ui_add_window(LgcyWindow *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut, const char *sourceFile, uint32_t lineNumber) {
	widget->type = LWT_WINDOW;
	assert(widget->GetSize() == widgetSize);
	
	auto assignedId = manager.AddWidget(widget, sourceFile, lineNumber);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	return 0;
}

static int ui_scrollbar_add(LgcyScrollBar *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut) {
	widget->type = LWT_SCROLLBAR;
	assert(widget->GetSize() == widgetSize);

	auto assignedId = manager.AddWidget(widget, __FILE__, __LINE__);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	return 0;
}

static int ui_add_button(LgcyButton *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut, const char *sourceFile, uint32_t lineNumber) {
	widget->type = LWT_BUTTON;
	assert(widget->GetSize() == widgetSize);

	auto assignedId = manager.AddWidget(widget, sourceFile, lineNumber);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	return 0;
}

static int ui_copy_widget(LgcyWidgetId id, LgcyWidget *widgetOut) {
	auto widget = manager.GetWidget(id);
	if (!widget) {
		throw new TempleException(format("Trying to access widget id {} which does not exist.", id));
	}
	memcpy(widgetOut, widget, widget->size);
	return 0;
}

static int ui_widget_set(LgcyWidgetId id, const LgcyWidget *widgetData) {
	auto widget = manager.GetWidget(id);
	if (!widget) {
		throw new TempleException(format("Trying to access widget id {} which does not exist.", id));
	}
	assert(widgetData->size == widget->size);
	memcpy(widget, widgetData, widget->size);
	return 0;
}

static LgcyWidget *ui_widget_get(LgcyWidgetId id) {
	return manager.GetWidget(id);
}

static int ui_widget_set_hidden(LgcyWidgetId id, BOOL hidden) {
	manager.SetHidden(id, hidden == TRUE);
	return 0;
}

static BOOL is_widget_hidden(LgcyWidgetId id) {
	auto widget = manager.GetWidget(id);
	return widget->IsHidden() ? TRUE : FALSE;
}

static bool ui_widget_contains_point(LgcyWidgetId widgetId, int x, int y) {
	return 0;
}

static LgcyWidgetId ui_widget_get_at_incl_children(int x, int y) {
	// -1 means no widget
	return -1;
}

static int ui_widget_remove(LgcyWidgetId id) {
	manager.RemoveWidget(id);
	return 0;
}

static int ui_widget_remove_regard_parent(LgcyWidgetId id) {
	manager.RemoveChildWidget(id);
	return 0;
}

static int ui_widgets_render() {
	manager.Render();
	return 0;
}

static int ui_widget_and_window_remove(LgcyWidgetId id) {
	manager.RemoveWindow(id);
	manager.RemoveWidget(id);
	return 0;
}

static int ui_widget_bring_to_front(LgcyWidgetId id) {
	manager.BringToFront(id);
	return 0;
}

static int ui_widget_send_to_back(LgcyWidgetId id) {
	manager.SendToBack(id);
	return 0;
}

/*
	Replaces all functions that access global widget state with wrappers around our
	legeacy UI manager class.
*/
static class UiLegacyWrapper : public TempleFix {
public:

	void apply() override
	{
		replaceFunction(0x101F8FD0, ui_add_window);
		replaceFunction(0x101F9080, ui_copy_widget);
		replaceFunction(0x101F90B0, ui_widget_set);
		replaceFunction(0x101F90E0, ui_widget_get);
		replaceFunction(0x101F9100, ui_widget_set_hidden);
		replaceFunction(0x101F9180, is_widget_hidden);
		replaceFunction(0x101F9240, ui_widget_contains_point);
		replaceFunction(0x101F9290, ui_widget_get_at_incl_children);
		replaceFunction(0x101F9420, ui_widget_remove);
		replaceFunction(0x101F9460, ui_add_button);
		replaceFunction(0x101F94D0, ui_widget_remove_regard_parent);
		replaceFunction(0x101F9010, ui_widget_and_window_remove);

		replaceFunction(0x101F8D10, ui_widgets_render);
				
		replaceFunction(0x101F8E40, ui_widget_bring_to_front);
		replaceFunction(0x101F8EA0, ui_widget_send_to_back);

		replaceFunction(0x101F9D20, ui_scrollbar_add);

		// Only called by functions we've already replaced
		// replaceFunction(0x101F91A0, ui_active_ctrl_get_free_id);
		// replaceFunction(0x101F8DB0, ui_windows_sort);
		
	}

} uiLegacyWrapper;
