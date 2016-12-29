
#include "stdafx.h"
#include "ui_legacy.h"
#include "ui.h"
#include "tig/tig_mouse.h"
#include "tig/tig_msg.h"
#include "messages/messagequeue.h"
#include "infrastructure/exception.h"

UiLegacyManager *uiLegacyManager;

UiLegacyManager::UiLegacyManager()
{
	if (uiLegacyManager == nullptr) {
		uiLegacyManager = this;
	}
}

UiLegacyManager::~UiLegacyManager()
{
	if (uiLegacyManager == this) {
		uiLegacyManager = nullptr;
	}
}

LgcyWidgetId UiLegacyManager::AddWidget(const LgcyWidget *widget, const char *sourceFile, uint32_t sourceLine)
{
	// Assign a free id
	auto assignedId = mNextWidgetId++;

	// Make a private copy of the widget
	auto &activeWidget = mActiveWidgets[assignedId];

	activeWidget.sourceFile = sourceFile;
	activeWidget.sourceLine = sourceLine;
	
	// Make a copy and have it managed by the unique_ptr
	activeWidget.widget.reset(reinterpret_cast<LgcyWidget*>(malloc(widget->GetSize())));
	memcpy(activeWidget.widget.get(), widget, widget->GetSize());
	
	// Set some basic properties on the widget
	activeWidget.widget->size = widget->GetSize();
	activeWidget.widget->widgetId = assignedId;

	return assignedId;
}

void UiLegacyManager::AddWindow(LgcyWidgetId id)
{

	if (find(mActiveWindows.begin(), mActiveWindows.end(), id) != mActiveWindows.end()) {
		// Window is already in the list
		return;
	}

	auto window = GetWindow(id);
	if (!window) {
		throw new TempleException(format("Trying to add widget {} as a window which isn't a window.", id));
	}

	// Don't add it, if it's hidden
	if (window->IsHidden()) {
		return;
	}

	mActiveWindows.push_back(id);
	BringToFront(id);
}

void UiLegacyManager::RemoveWindow(LgcyWidgetId id)
{
	for (auto it = mActiveWindows.begin(); it != mActiveWindows.end(); ) {
		if (*it == id) {
			it = mActiveWindows.erase(it);
		} else {
			it++;
		}
	}
	SortWindows();
}

LgcyWidget * UiLegacyManager::GetWidget(LgcyWidgetId id)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		return it->second.widget.get();
	}
	return nullptr;
}

void UiLegacyManager::BringToFront(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MAX;
		SortWindows();
	}
}

void UiLegacyManager::SendToBack(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MIN;
		SortWindows();
	}
}

void UiLegacyManager::SetHidden(LgcyWidgetId id, bool hidden)
{
	auto widget = GetWidget(id);
	if (hidden) {
		widget->flags |= 1;
	} else {
		widget->flags &= (~1);
	}

	if (widget->IsWindow()) {
		if (hidden) {
			RemoveWindow(id);
		} else {
			AddWindow(id);
		}
	}
		
	RefreshMouseOverState();

}

void UiLegacyManager::RemoveWidget(LgcyWidgetId id)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		mActiveWidgets.erase(it);
	}
}

bool UiLegacyManager::AddChild(LgcyWidgetId parentId, LgcyWidgetId childId)
{
	auto parent = GetWindow(parentId);
	if (parent) {

		if (parent->childrenCount >= 127) {
			return false;
		}

		auto child = GetWidget(childId);
		if (child) {
			child->parentId = parentId;
		}
		parent->children[parent->childrenCount++] = childId;

		RefreshMouseOverState();
		return true;
	} else {
		return false;
	}
}

void UiLegacyManager::RemoveChildWidget(LgcyWidgetId id)
{
	auto widget = GetWidget(id);

	if (!widget || widget->parentId == -1) {
		return;
	}

	auto parent = GetWidget(widget->parentId);
	if (!parent) {
		return;
	}
	assert(parent->IsWindow());
	auto parentWindow = (LgcyWindow*)parent;

	for (uint32_t i = 0; i < parentWindow->childrenCount; ++i) {
		if (parentWindow->children[i] == widget->widgetId) {
			parentWindow->children[i] = -1;
			for (uint32_t j = i + 1; j < parentWindow->childrenCount; ++j) {
				parentWindow->children[j - 1] = parentWindow->children[j];
			}
			parentWindow->childrenCount--;
			return;
		}
	}

}

void UiLegacyManager::Render()
{
	// Make a copy here since some vanilla logic will show/hide windows in their render callbacks
	auto activeWindows(mActiveWindows);

	for (auto windowId : activeWindows) {
		auto window = GetWindow(windowId);
		if (window->IsHidden()) {
			continue;
		}

		// Render the widget itself
		auto renderFunc = window->render;
		if (renderFunc) {
			renderFunc(windowId);
		}

		// Render all child widgets
		for (uint32_t i = 0; i < window->childrenCount; ++i) {
			auto childId = window->children[i];
			auto child = GetWidget(childId);
			if (!child->IsHidden() && child->render) {
				child->render(childId);
			}
		}
	}
}

LgcyWindow * UiLegacyManager::GetWindowAt(int x, int y)
{
	// Backwards because of render order (rendered last is really on top)
	for (int i = mActiveWindows.size() - 1; i >= 0; --i) {
		auto windowId = mActiveWindows[i];
		auto window = GetWindow(windowId);
		if (!window->IsHidden() && DoesWidgetContain(windowId, x, y)) {
			return window;
		}
	}
	return nullptr;
}

LgcyWidgetId UiLegacyManager::GetWidgetAt(int x, int y)
{
	LgcyWidgetId result = -1;

	// Backwards because of render order (rendered last is really on top)
	for (int i = mActiveWindows.size() - 1; i >= 0; --i) {
		auto windowId = mActiveWindows[i];
		auto window = GetWindow(windowId);
		if (!window->IsHidden() && DoesWidgetContain(windowId, x, y)) {
			result = windowId;

			// Also in reverse order
			for (int j = window->childrenCount - 1; j >= 0; --j) {
				auto childId = window->children[j];
				auto child = GetWidget(childId);
				if (child && !child->IsHidden() && DoesWidgetContain(childId, x, y)) {
					result = childId;
					break;
				}
			}

			break;
		}
	}
	return result;
}

bool UiLegacyManager::DoesWidgetContain(LgcyWidgetId id, int x, int y)
{
	if (id == -1) {
		return false;
	}

	auto widget = GetWidget(id);
	
	return widget
		&& x >= widget->x 
		&& y >= widget->y
		&& x < (int)(widget->x + widget->width)
		&& y < (int)(widget->y + widget->height);
}

void UiLegacyManager::RefreshMouseOverState()
{
	TigMouseState state;
	mouseFuncs.GetState(&state);
	
	TigMouseMsg msg;
	msg.x = state.x;
	msg.y = state.y;
	msg.mouseStateField24 = state.field24;
	msg.flags = 0x1000;
	ui.TranslateMouseMessage(&msg);
}

void UiLegacyManager::SortWindows()
{
	// Sort Windows by Z-Index
	sort(mActiveWindows.begin(), mActiveWindows.end(), [this](LgcyWidgetId idA, LgcyWidgetId idB) {
		auto windowA = GetWindow(idA);
		auto windowB = GetWindow(idB);
		return windowA->zIndex < windowB->zIndex;
	});

	// Reassign a zindex in monotonous order
	for (size_t i = 0; i < mActiveWindows.size(); ++i) {
		auto window = GetWindow(mActiveWindows[i]);
		window->zIndex = i;
	}
}

size_t LgcyWidget::GetSize() const
{
	switch (type) {
	case LgcyWidgetType::Window:
		return 0x294;
	case LgcyWidgetType::Button:
		return 0xBC;
	case LgcyWidgetType::Scrollbar:
		return 0xB0;
	default:
		throw new TempleException("Cannot determine size of a widget that has an unknown type.");
	}
}

LgcyWindow::LgcyWindow()
{
	widgetId = -1;
	parentId = -1;
	type = LgcyWidgetType::Window;
	this->size = sizeof(LgcyWindow);
	x = 0;
	y = 0;
	xrelated = 0;
	yrelated = 0;
	height = 0;
	width = 0;
	render = 0;
	handleMessage = 0;
	renderTooltip = 0;
	childrenCount = 0;
	mouseState = LgcyWindowMouseState::Outside;
	field_28c = 0;
	zIndex = 0;
	flags = 0;
}

LgcyWindow::LgcyWindow(int x, int y, int w, int h) : LgcyWindow()
{
	this->x = x;
	this->y = y;
	this->xrelated = x;
	this->yrelated = y;
	this->width = w;
	this->height = h;
}

LgcyButton::LgcyButton() {
	memset(this, 0, sizeof(LgcyButton));
	type = LgcyWidgetType::Button;
}

void LgcyWidgetDeleter::operator()(LgcyWidget* p) {
	free(p);
}
