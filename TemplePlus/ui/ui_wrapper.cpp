
#include "stdafx.h"

#include "tig/tig_msg.h"
#include "ui.h"
#include "util/fixes.h"

static int ui_add_window(LgcyWindow *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut, const char *sourceFile, uint32_t lineNumber) {
	widget->type = LgcyWidgetType::Window;
	assert(widget->GetSize() == widgetSize);
	
	auto assignedId = uiManager->AddWidget(widget, sourceFile, lineNumber);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	uiManager->AddWindow(assignedId);
	return 0;
}

static int ui_scrollbar_add(LgcyScrollBar *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut) {
	widget->type = LgcyWidgetType::Scrollbar;
	assert(widget->GetSize() == widgetSize);

	auto assignedId = uiManager->AddWidget(widget, __FILE__, __LINE__);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	return 0;
}

static int ui_add_button(LgcyButton *widget, size_t widgetSize, LgcyWidgetId *assignedIdOut, const char *sourceFile, uint32_t lineNumber) {
	widget->type = LgcyWidgetType::Button;
	assert(widget->GetSize() == widgetSize);

	auto assignedId = uiManager->AddWidget(widget, sourceFile, lineNumber);
	if (assignedIdOut) {
		*assignedIdOut = assignedId;
	}
	return 0;
}

static int ui_copy_widget(LgcyWidgetId id, LgcyWidget *widgetOut) {
	auto widget = uiManager->GetWidget(id);
	if (!widget) {
		throw new TempleException(format("Trying to access widget id {} which does not exist.", id));
	}
	memcpy(widgetOut, widget, widget->size);
	return 0;
}

static int ui_widget_set(LgcyWidgetId id, const LgcyWidget *widgetData) {
	auto widget = uiManager->GetWidget(id);
	if (!widget) {
		throw new TempleException(format("Trying to access widget id {} which does not exist.", id));
	}
	assert(widgetData->size == widget->size);
	memcpy(widget, widgetData, widget->size);
	return 0;
}

static LgcyWidget *ui_widget_get(LgcyWidgetId id) {
	return uiManager->GetWidget(id);
}

static int ui_widget_set_hidden(LgcyWidgetId id, BOOL hidden) {
	uiManager->SetHidden(id, hidden == TRUE);
	return 0;
}

static BOOL is_widget_hidden(LgcyWidgetId id) {
	return uiManager->IsHidden(id) ? TRUE : FALSE;
}

static BOOL ui_widget_contains_point(LgcyWidgetId widgetId, int x, int y) {
	return uiManager->DoesWidgetContain(widgetId, x, y) ? TRUE : FALSE;
}

static LgcyWidgetId ui_widget_get_at_incl_children(int x, int y) {
	// -1 means no widget
	return uiManager->GetWidgetAt(x, y);	
}

static int ui_widget_remove(LgcyWidgetId id) {
	uiManager->RemoveWidget(id);
	return 0;
}

static int ui_widget_remove_regard_parent(LgcyWidgetId id) {
	uiManager->RemoveChildWidget(id);
	return 0;
}

static int ui_widgets_render() {
	uiManager->Render();
	return 0;
}

static int ui_widget_and_window_remove(LgcyWidgetId id) {
	uiManager->RemoveWindow(id);
	uiManager->RemoveWidget(id);
	return 0;
}

static int ui_widget_bring_to_front(LgcyWidgetId id) {
	uiManager->BringToFront(id);
	return 0;
}

static int ui_widget_send_to_back(LgcyWidgetId id) {
	uiManager->SendToBack(id);
	return 0;
}

static int ui_widgetlist_indexof(LgcyWidgetId widId, LgcyWidgetId* widlist, int size) {
	for (int i = 0; i < size; i++) {
		if (widlist[i] == widId)
			return i;
	}

	return -1;
}

static LgcyButton *ui_get_button(LgcyWidgetId widId) {
	LgcyButton * result = (LgcyButton *)ui_widget_get(widId);
	if (!result || result->type != LgcyWidgetType::Button)
		return nullptr;
	return result;

}

static int ui_widget_add_child(LgcyWidgetId parentId, LgcyWidgetId childId) {
	return uiManager->AddChild(parentId, childId) ? FALSE : TRUE;
}

/*
	Replaces all functions that access global widget state with wrappers around our
	legacy UI manager class.
*/
static class UiWrapper : public TempleFix {
public:

	void apply() override
	{
		replaceFunction(0x101F8950, ui_widget_add_child);
		replaceFunction(0x101F8FD0, ui_add_window);		
		replaceFunction(0x101F9080, ui_copy_widget);
		replaceFunction(0x101F90B0, ui_widget_set);
		replaceFunction(0x101F90E0, ui_widget_get);
		replaceFunction(0x101F9570, ui_get_button);
		replaceFunction(0x101F9100, ui_widget_set_hidden);
		replaceFunction(0x101F9180, is_widget_hidden);
		replaceFunction(0x101F9240, ui_widget_contains_point);
		replaceFunction(0x101F9290, ui_widget_get_at_incl_children);
		replaceFunction(0x101F9420, ui_widget_remove);
		replaceFunction(0x101F9460, ui_add_button);
		replaceFunction(0x101F94D0, ui_widget_remove_regard_parent);		
		replaceFunction(0x101F9010, ui_widget_and_window_remove);
		replaceFunction(0x1011DFE0, ui_widgetlist_indexof);

		replaceFunction(0x101F8D10, ui_widgets_render);
				
		replaceFunction(0x101F8E40, ui_widget_bring_to_front);
		replaceFunction(0x101F8EA0, ui_widget_send_to_back);

		replaceFunction(0x101F9D20, ui_scrollbar_add);

		replaceFunction<BOOL(TigMouseMsg*)>(0x101F9970, [](TigMouseMsg *msg) {
			return uiManager->TranslateMouseMessage(*msg) ? TRUE : FALSE;
		});

		replaceFunction<BOOL(TigMsg*)>(0x101F8A80, [](TigMsg *msg) {
			return uiManager->ProcessMessage(*msg) ? TRUE : FALSE;
		});
		
		// Only called by functions we've already replaced
		// replaceFunction(0x101F91A0, ui_active_ctrl_get_free_id);
		// replaceFunction(0x101F8DB0, ui_windows_sort);
		
	}

} uiWrapper;
