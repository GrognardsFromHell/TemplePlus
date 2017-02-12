#include "stdafx.h"
#include "ui.h"
#include "tig/tig.h"
#include <util/fixes.h>
#include <tig/tig_msg.h>
#include <sound.h>
#include <tig/tig_font.h>
#include <tig/tig_mouse.h>
#include "messages/messagequeue.h"
#include "widgets/widgets.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QQuickItem>

UiManager *uiManager;

/**
 * Handles button click triggering.
 * When a button receives a mouse-down event, there are two possible behaviors:
 * - If the button is non-repeat, the button captures the mouse input
 *   - If the mouse button is released while the mouse is over the button, 
 *     an OnClick message is dispatched to that button.
 * - If the button is repeat, the button also captures the mouse input, and an OnClick
 *   msg is immediately dispatched.
 *  - The next trigger time is set to current-time + 500ms
 *  - When the mouse leaves the button's client area, the auto-repeat is suspended (next action time set to 0)
 *  - When the mouse button is released (regardless of where), the auto repeat is suspended
 *  - When the time update event is received, and the time for the next press elapsed, trigger an onclick msg
 */
class ButtonClickBehavior {
public:
		
	void UpdateTime(int newTime);
	
private:
	int mNextClick = 0;
	int mX;
	int mY;
	int mButtonDown = false;
	LgcyWidgetId mReceiver = -1;
};

static ButtonClickBehavior buttonClickBehavior;

#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>

#include "ui_rendercontrol.h"

class EventFilter : public QObject {
public:
	bool eventFilter(QObject *watched, QEvent *event) override {

		if (event->type() != QEvent::MouseMove
			&& event->type() != QEvent::MouseButtonPress
			&& event->type() != QEvent::MouseButtonRelease) {
			return false;
		}

		QString objName = QString::fromStdString(fmt::format("{}", (void*)watched));
		if (!watched->objectName().isEmpty()) {
			objName = watched->objectName();
		}

		qDebug() << "Event to " << watched->metaObject()->className() << " (" << objName << "): " << event;
		
		return false;
	}
};

struct NewImpl {
	int argc = 0;
	
	std::unique_ptr<QGuiApplication> guiApp;
	std::unique_ptr<UiRenderControl> renderControl;
	std::unique_ptr<QQmlEngine> engine;
	
	NewImpl() {

		char *argv[] = {
			"",
			"-platform",
			"offscreen"
		};
		argc = 3;

		guiApp = std::make_unique<QGuiApplication>(argc, &argv[0]);
		guiApp->installEventFilter(new EventFilter);

		renderControl = std::make_unique<UiRenderControl>();

		engine = std::make_unique<QQmlEngine>();

		engine->setBaseUrl(QUrl::fromLocalFile("C:/TemplePlus/TemplePlus/tpdata/ui/"));
		
		QObject::connect(engine.get(), &QQmlEngine::warnings, [=](const QList<QQmlError> &warnings) {
			for (auto &error : warnings) {
				logger->warn("{}", error.toString().toStdString());
			}
		});
		
		auto view = new QQuickView(engine.get(), nullptr);

		QObject::connect(view, &QQuickView::statusChanged, [=](QQuickView::Status status) {
			if (status == QQuickView::Error) {
				for (auto &error : view->errors()) {
					logger->warn("{}", error.toString().toStdString());
				}
			}
		});

		guiApp->processEvents();

	}
	
};

static std::unique_ptr<NewImpl> newImpl;

UiManager::UiManager() {
	Expects(uiManager == nullptr);
	uiManager = this;
	
	newImpl = std::make_unique<NewImpl>();	
}

UiManager::~UiManager() {
	if (uiManager == this) {
		uiManager = nullptr;
	}
}

void UiManager::SetAdvancedWidget(LgcyWidgetId id, WidgetBase * widget)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		it->second.advancedWidget = widget;
	}
}

WidgetBase * UiManager::GetAdvancedWidget(LgcyWidgetId id) const
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		return it->second.advancedWidget;
	}
	return nullptr;
}

class UiReplacement : TempleFix
{
public:

	void apply() override
	{
		 /*
		 Hook for missing portraits (annoying logspam!)
		 */
		 static int(__cdecl*orgGetAsset)(int, int, void*, int) = replaceFunction<int(__cdecl)(int, int, void*, int)>(0x1004A360, [](int assetType, int id, void* out, int subId)
		 {
			 // portraits
			 if (assetType == 0) {
				 MesLine line;
				 line.key = id + subId;
				 auto uiArtMgrMesFiles = temple::GetRef<MesHandle*>(0x10AA31C8);

				 if (!mesFuncs.GetLine(uiArtMgrMesFiles[assetType], &line)){
					 return orgGetAsset(assetType, 0, out, subId);
				 }
			 }
			 return orgGetAsset(assetType, id, out, subId);
		 });

		 // DrawTextInWidget hook
		 replaceFunction<bool(__cdecl)(int, char*, TigRect&, TigTextStyle&)>(0x101F87C0, [](int widgetId ,char* text, TigRect& rect, TigTextStyle& style)->bool
		 {
			 auto wid = uiManager->GetWidget(widgetId);
			 if (!wid)
				 return 1;
			 if (*text == 0)
				 return 1;

			

			 TigRect extents(rect.x + wid->x, rect.y + wid->y, rect.width, rect.height);
			 if (rect.x < 0 || rect.x > 10000 || rect.width > 10000 || rect.width < 0)
			 {
				 extents.x = wid->x;
				 extents.width = wid->width;

				 //auto tigRects = temple::GetPointer<TigRect>(0x10C7C638);
				 //TigRect rects[20];
				 //memcpy(rects, tigRects, sizeof(rects));
				 //
				 //auto uiCharEditorSkillsWnd = temple::GetPointer<LgcyWindow>(0x10C7B628);
				 //int dummy = 1;


			 }
			 
			 

			 return tigFont.Draw(text, extents, style) != 0;

		 });


	}
} uiReplacement;

LgcyButton::LgcyButton(char* ButtonName, int ParentId, int X, int Y, int Width, int Height){
	if (ButtonName){
		auto pos = name;
		while( *ButtonName && (pos - name < 63) ){
			*pos = *ButtonName;
			pos++; ButtonName++;
		}
		*pos = 0;
	}

	x = X;
	y = Y;
	width = Width;
	height = Height;
	parentId = ParentId;
	yrelated = Y;
	xrelated = X;
	flags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = LgcyButtonState::Normal;
	field98 = 0;
	type = LgcyWidgetType::Button;
	widgetId = -1;
	field8C = -1;
	field90 = -1;
	field84 = -1;
	field88 = -1;
	sndDown = -1;
	sndClick = -1;
	sndHoverOn = -1;
	sndHoverOff = -1;
}

LgcyButton::LgcyButton(char* ButtonName, int ParentId, TigRect& rect){
	if (ButtonName) {
		auto pos = name;
		while (*ButtonName && (pos - name < 63)) {
			*pos = *ButtonName;
			pos++; ButtonName++;
		}
		*pos = 0;
	}

	x = rect.x;
	y = rect.y;
	width = rect.width;
	height = rect.height;
	parentId = ParentId;
	yrelated = rect.y;
	xrelated = rect.x;
	flags = 0;
	renderTooltip = nullptr;
	render = nullptr;
	handleMessage = nullptr;
	buttonState = LgcyButtonState::Normal;
	field98 = 0;
	type = LgcyWidgetType::Button;
	widgetId = -1;
	field8C = -1;
	field90 = -1;
	field84 = -1;
	field88 = -1;
	sndDown = -1;
	sndClick = -1;
	sndHoverOn = -1;
	sndHoverOff = -1;
}

void LgcyButton::SetDefaultSounds()
{
	sndDown = 3012;
	sndClick = 3013;
	sndHoverOn = 3010;
	sndHoverOff = 3011;
}

int LgcyScrollBar::GetY()
{
	auto getY = temple::GetRef<int(__cdecl)(int id, int& yOut)>(0x101FA150);
	int y = 0;
	if (getY(this->widgetId, y)){
		logger->warn("scrollbar error!");
		y = -1;
	}
	return y;
}

bool LgcyScrollBar::Init(int X, int Y, int Height){

	x = X;
	y = Y;
	type = LgcyWidgetType::Scrollbar;
	parentId = -1;
	widgetId = -1;
	flags = 0;
	size = 176;
	width = 13;
	height = Height;
	field90 = 0;
	render = temple::GetRef<void(__cdecl)(int)>(0x101FA1B0);
	handleMessage = temple::GetRef<BOOL(__cdecl)(int, TigMsg*)>(0x101FA410);
	renderTooltip = nullptr;
	yMin = 0;
	yMax = 100;
	scrollQuantum = 1;
	field8C = 5;
	scrollbarY = 0;
	field98= 0;
	field9C = 0;
	fieldAC = 0;
	fieldA8 = 0;
	fieldA4 = 0;
	fieldA0 = 0;
	field94 = 0;
	return false;
}

bool LgcyScrollBar::Init(int x, int y, int height, int parentId)
{
	Init(x, y, height);
	this->parentId = parentId;
	auto p = uiManager->GetWindow(parentId);
	this->x += p->x;
	this->y += p->y;
	return false;
}

LgcyWidgetId UiManager::AddWindow(LgcyWindow& widget)
{
	widget.type = LgcyWidgetType::Window;

	auto widgetId = AddWidget(&widget, __FILE__, __LINE__);
	AddWindow(widgetId);
	return widgetId;
}

BOOL UiManager::ButtonInit(LgcyButton* widg, char* buttonName, int parentId, int x, int y, int width, int height)
{
	if (buttonName)
	{
		char * c = buttonName;
		memcpy(widg->name, buttonName, min(sizeof(widg->name), strlen(buttonName)));
	}
	widg->x = x;
	widg->xrelated = x;
	widg->y = y;
	widg->yrelated = y;
	widg->parentId = parentId;
	widg->width = width;
	widg->height = height;
	widg->flags = 0;
	widg->buttonState = LgcyButtonState::Normal;
	widg->renderTooltip = 0;
	widg->field98 = 0;
	widg->type = LgcyWidgetType::Button;
	widg->widgetId = -1;
	widg->field8C = -1;
	widg->field90 = -1;
	widg->field84 = -1;
	widg->field88 = -1;
	widg->sndDown = -1;
	widg->sndClick = -1;
	widg->sndHoverOn = -1;
	widg->sndHoverOff = -1;
	return 0;
}

LgcyWidgetId UiManager::AddButton(LgcyButton& button)
{
	button.type = LgcyWidgetType::Button;
	return AddWidget(&button, __FILE__, __LINE__);
}

LgcyWidgetId UiManager::AddButton(LgcyButton &button, LgcyWidgetId parentId)
{
	auto buttonId = AddButton(button);
	AddChild(parentId, buttonId);
	return buttonId;
}

LgcyWidgetId UiManager::AddScrollBar(LgcyScrollBar& scrollBar)
{
	scrollBar.type = LgcyWidgetType::Scrollbar;
	return AddWidget(&scrollBar, __FILE__, __LINE__);
}

LgcyWidgetId UiManager::AddScrollBar(LgcyScrollBar& scrollBar, LgcyWidgetId parentId)
{
	auto scrollBarId = AddScrollBar(scrollBar);
	AddChild(parentId, scrollBarId);
	return scrollBarId;
}

void UiManager::SetButtonState(LgcyWidgetId widgetId, LgcyButtonState newState)
{
	auto button = GetButton(widgetId);
	Expects(button);
	button->buttonState = newState;
}

LgcyButtonState UiManager::GetButtonState(LgcyWidgetId widId) {
	auto button = GetButton(widId);
	Expects(button);
	return button->buttonState;
}

bool UiManager::ScrollbarGetY(int widId, int * scrollbarY) {
	auto widget = (LgcyScrollBar*) GetWidget(widId);
	if (!widget || widget->type != LgcyWidgetType::Scrollbar)
		return true;

	if (!scrollbarY)
		return false;
	auto y = 0;
	auto ymax = widget->yMax;
	auto ymin = widget->yMin;

	if (ymax > ymin) {
		auto field90 = widget->field90;
		if (!field90) {
			y = widget->scrollbarY;
		}
		else {
			y = (ymax + widget->field8C - ymin)
				* ( ((widget->height - 44) * widget->scrollbarY) / (ymax + widget->field8C - ymin) - field90)
				/ (widget->height - 44);
		}
	}
	

	if (y > ymax) {
		*scrollbarY = ymax;
		return false;
	}
	
	if (y < ymin) {
		*scrollbarY = ymin;
		return false;
	}

	*scrollbarY = y;
	return false;
}

void UiManager::ScrollbarSetYmax(LgcyWidgetId widId, int yMax)
{
	LgcyScrollBar *widg = GetScrollBar(widId);
	Expects(widg);
	widg->yMax = yMax;
}

void UiManager::ScrollbarSetY(LgcyWidgetId widId, int value) {
	auto scrollbar = GetScrollBar(widId);

	scrollbar->scrollbarY = value;

	TigMsg msg;
	msg.createdMs = temple::GetRef<int>(0x11E74578);
	msg.type = TigMsgType::WIDGET;
	msg.arg2 = 5;
	msg.arg1 = scrollbar->parentId;
	messageQueue->Enqueue(msg);
}


LgcyWidgetId UiManager::AddWidget(const LgcyWidget *widget, const char *sourceFile, uint32_t sourceLine)
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

void UiManager::AddWindow(LgcyWidgetId id)
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

void UiManager::RemoveWindow(LgcyWidgetId id)
{
	for (auto it = mActiveWindows.begin(); it != mActiveWindows.end(); ) {
		if (*it == id) {
			it = mActiveWindows.erase(it);
		}
		else {
			it++;
		}
	}
	SortWindows();
}

LgcyWidget * UiManager::GetWidget(LgcyWidgetId id) const
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		return it->second.widget.get();
	}
	return nullptr;
}

LgcyWindow* UiManager::GetWindow(LgcyWidgetId widId) const
{
	LgcyWidget* result = GetWidget(widId);
	if (!result)
		return nullptr;
	if (result->type == LgcyWidgetType::Window) {
		return static_cast<LgcyWindow*>(result);
	}
	return nullptr;
}

LgcyButton* UiManager::GetButton(LgcyWidgetId widId) const
{
	auto result = GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Button) {
		return nullptr;
	}
	return static_cast<LgcyButton*>(result);
}

LgcyScrollBar* UiManager::GetScrollBar(LgcyWidgetId widId) const
{
	auto result = GetWidget(widId);
	if (!result || result->type != LgcyWidgetType::Scrollbar)
		return nullptr;
	return static_cast<LgcyScrollBar*>(result);
}

void UiManager::BringToFront(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MAX;
		SortWindows();
	}
}

void UiManager::SendToBack(LgcyWidgetId id)
{
	auto window = GetWindow(id);
	if (window) {
		window->zIndex = INT32_MIN;
		SortWindows();
	}
}

void UiManager::SetHidden(LgcyWidgetId id, bool hidden)
{
	auto widget = GetWidget(id);
	if (hidden) {
		widget->flags |= 1;
	}
	else {
		widget->flags &= (~1);
	} 

	// New widgets are rendered recursively and dont need to be 
	// in the top-level window list to be rendered if they are nested
	// children
	bool isNewWidget = GetAdvancedWidget(id) != nullptr;

	// Update the top-level window list
	if (widget->IsWindow() && (!isNewWidget || widget->parentId == -1)) {
		if (hidden) {
			RemoveWindow(id);
		} else {
			AddWindow(id);
		}
	}

	RefreshMouseOverState();

}

void UiManager::RemoveWidget(LgcyWidgetId id)
{
	auto it = mActiveWidgets.find(id);
	if (it != mActiveWidgets.end()) {
		if (it->second.widget->IsWindow()) {
			RemoveWindow(id);
		}
		mActiveWidgets.erase(it);
	}
}

bool UiManager::AddChild(LgcyWidgetId parentId, LgcyWidgetId childId)
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
	}
	else {
		return false;
	}
}

void UiManager::RemoveChildWidget(LgcyWidgetId id)
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

void UiManager::Render()
{

	newImpl->renderControl->ProcessEvents();
	
	// Make a copy here since some vanilla logic will show/hide windows in their render callbacks
	auto activeWindows(mActiveWindows);

	for (auto windowId : activeWindows) {

		auto view = mActiveWidgets[windowId].view;
		if (view) {
			newImpl->renderControl->Render(view);
			continue;
		}

		// Our new widget system handles rendering itself
		auto advWidget = GetAdvancedWidget(windowId);
		if (advWidget) {
			advWidget->Render();
			continue;
		}
		
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

LgcyWindow * UiManager::GetWindowAt(int x, int y)
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

LgcyWidgetId UiManager::GetWidgetAt(int x, int y)
{
	LgcyWidgetId result = -1;

	// Backwards because of render order (rendered last is really on top)
	for (int i = mActiveWindows.size() - 1; i >= 0; --i) {
		auto windowId = mActiveWindows[i];
		auto window = GetWindow(windowId);
		if (!window->IsHidden() && DoesWidgetContain(windowId, x, y)) {
			result = windowId;
			
			auto advWidget = GetAdvancedWidget(windowId);
			if (advWidget) {
				int localX = x - window->x;
				int localY = y - window->y;

				auto in = advWidget->PickWidget(localX, localY);
				if (in) {
					return in->GetWidgetId();
				}
				break;
			}

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

bool UiManager::DoesWidgetContain(LgcyWidgetId id, int x, int y)
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

void UiManager::RefreshMouseOverState()
{
	TigMouseState state;
	mouseFuncs.GetState(&state);

	TigMouseMsg msg;
	msg.x = state.x;
	msg.y = state.y;
	msg.mouseStateField24 = state.field24;
	msg.flags = MouseStateFlags::MSF_POS_CHANGE;
	TranslateMouseMessage(msg);
}

void UiManager::SortWindows()
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

#include <QtGui/5.8.0/QtGui/qpa/qwindowsysteminterface.h>

static Qt::KeyboardModifiers keyStateToModifiers(int wParam)
{
	Qt::KeyboardModifiers mods(Qt::NoModifier);
	if (GetKeyState(VK_CONTROL) < 0)
		mods |= Qt::ControlModifier;
	if (GetKeyState(VK_SHIFT) < 0)
		mods |= Qt::ShiftModifier;
	if (GetKeyState(VK_MENU) < 0)
		mods |= Qt::AltModifier;
	return mods;
}

bool UiManager::TranslateMouseMessage(const TigMouseMsg& mouseMsg)
{
	int flags = mouseMsg.flags;
	int x = mouseMsg.x, y = mouseMsg.y;
	TigMsgWidget newTigMsg;
	newTigMsg.createdMs = temple::GetRef<int>(0x11E74578); // system time as recorded by the Msg Process function
	newTigMsg.type = TigMsgType::WIDGET;
	newTigMsg.x = x;
	newTigMsg.y = y;

	int widIdAtCursor = GetWidgetAt(x, y);

	int globalWidId = mWidgetMouseHandlerWidgetId;

	// moused widget changed
	if ((flags & MSF_POS_CHANGE) && widIdAtCursor != globalWidId)
	{
		if (widIdAtCursor != -1 && mouseFuncs.GetCursorDrawCallbackId() == (uint32_t)mMouseMsgHandlerRenderTooltipCallback)
		{
			mouseFuncs.SetCursorDrawCallback(nullptr, 0);
		}

		if (globalWidId != -1)
		{
			bool enqueueExited = false;
			auto globalWid = GetWidget(globalWidId);
			// if window
			if (globalWid->IsWindow())
			{
				auto prevHoveredWindow = (LgcyWindow*)globalWid;
				if (prevHoveredWindow->mouseState == LgcyWindowMouseState::Pressed) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::PressedOutside;
				}
				else if (prevHoveredWindow->mouseState != LgcyWindowMouseState::PressedOutside) {
					prevHoveredWindow->mouseState = LgcyWindowMouseState::Outside;
				}
				enqueueExited = true;
			}
			// button
			else if (globalWid->IsButton() && !globalWid->IsHidden())
			{
				auto buttonWid = GetButton(globalWidId);
				switch (buttonWid->buttonState) {
				case LgcyButtonState::Hovered:
					// Unhover
					buttonWid->buttonState = LgcyButtonState::Normal;
					sound.MssPlaySound(buttonWid->sndHoverOff);
					break;
				case LgcyButtonState::Down:
					// Down -> Released without click event
					buttonWid->buttonState = LgcyButtonState::Released;
					break;
				}
				if (!GetWidget(globalWid->parentId)->IsHidden())
				{
					enqueueExited = true;
				}
			}
			// scrollbar
			else if (globalWid->IsScrollBar())
			{
				if (globalWid->IsHidden() || GetWidget(globalWid->parentId)->IsHidden())
					enqueueExited = false;
				else
					enqueueExited = true;
			}

			if (enqueueExited)
			{
				newTigMsg.widgetId = globalWidId;
				newTigMsg.widgetEventType = TigMsgWidgetEvent::Exited;
				messageQueue->Enqueue(newTigMsg);
			}
		}

		if (widIdAtCursor != -1)
		{
			auto widAtCursor = GetWidget(widIdAtCursor);
			if (widAtCursor->type == LgcyWidgetType::Window)
			{
				auto widAtCursorWindow = GetWindow(widIdAtCursor);
				if (widAtCursorWindow->mouseState == LgcyWindowMouseState::PressedOutside) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Pressed;
				}
				else if (widAtCursorWindow->mouseState != LgcyWindowMouseState::Pressed) {
					widAtCursorWindow->mouseState = LgcyWindowMouseState::Hovered;
				}
			}
			else if (widAtCursor->type == LgcyWidgetType::Button) {
				auto buttonWid = GetButton(widIdAtCursor);
				if (buttonWid->buttonState != LgcyButtonState::Normal)
				{
					if (buttonWid->buttonState == LgcyButtonState::Released)
					{
						buttonWid->buttonState = LgcyButtonState::Down;
					}
				}
				else
				{
					buttonWid->buttonState = LgcyButtonState::Hovered;
					sound.MssPlaySound(buttonWid->sndHoverOn);
				}
			}
			newTigMsg.widgetId = widIdAtCursor;
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Entered;
			messageQueue->Enqueue(newTigMsg);
		}
		globalWidId = mWidgetMouseHandlerWidgetId = widIdAtCursor;
	}

	if (mouseMsg.flags & MouseStateFlags::MSF_POS_CHANGE_SLOW && globalWidId != -1 && !mouseFuncs.GetCursorDrawCallbackId())
	{
		mouseFuncs.SetCursorDrawCallback([&](int x, int y) {
			mMouseMsgHandlerRenderTooltipCallback(x, y, &mWidgetMouseHandlerWidgetId);
		}, (uint32_t)mMouseMsgHandlerRenderTooltipCallback);
	}

	if (mouseMsg.flags & (MouseStateFlags::MSF_LMB_DOWN | MouseStateFlags::MSF_MMB_DOWN | MouseStateFlags::MSF_RMB_DOWN)) {
		auto view = (widIdAtCursor == -1) ? nullptr : mActiveWidgets[widIdAtCursor].view;
		if (view != QGuiApplication::focusWindow()) {
			view->requestActivate();
		}
	}

	if (mouseMsg.flags & MouseStateFlags::MSF_LMB_CLICK)
	{
		auto widIdAtCursor2 = GetWidgetAt(mouseMsg.x, mouseMsg.y); // probably redundant to do again, but just to be safe...
		if (widIdAtCursor2 != -1)
		{
			auto button = GetButton(widIdAtCursor2);
			if (button)
			{
				int buttonState = button->buttonState;
				switch (button->buttonState) {
				case LgcyButtonState::Hovered:
					button->buttonState = LgcyButtonState::Down;
					sound.MssPlaySound(button->sndDown);
					break;
				case LgcyButtonState::Disabled:
					return 0;
				}
			}
			newTigMsg.widgetEventType = TigMsgWidgetEvent::Clicked;
			newTigMsg.widgetId = widIdAtCursor2;
			mMouseButtonId = widIdAtCursor2;
			messageQueue->Enqueue(newTigMsg);
		}
	}

	if ((mouseMsg.flags & MouseStateFlags::MSF_LMB_RELEASED) && mMouseButtonId != -1)
	{
		auto button = GetButton(mMouseButtonId);
		if (button)
		{
			switch (button->buttonState)
			{
			case LgcyButtonState::Down:
				button->buttonState = LgcyButtonState::Hovered;
				sound.MssPlaySound(button->sndClick);
				break;
			case LgcyButtonState::Released:
				button->buttonState = LgcyButtonState::Normal;
				sound.MssPlaySound(button->sndClick);
				break;
			case LgcyButtonState::Disabled:
				return false;
			}
		}
		auto widIdAtCursor2 = GetWidgetAt(mouseMsg.x, mouseMsg.y); // probably redundant to do again, but just to be safe...
		newTigMsg.widgetId = mMouseButtonId;
		newTigMsg.widgetEventType = (widIdAtCursor2 != mMouseButtonId) ? TigMsgWidgetEvent::MouseReleasedAtDifferentButton : TigMsgWidgetEvent::MouseReleased;
		messageQueue->Enqueue(newTigMsg);
		mMouseButtonId = -1;
	}
	
	return false;
}

static inline QString messageKeyText(const int wParam)
{
	const QChar ch = QChar(ushort(wParam));
	return ch.isNull() ? QString() : QString(ch);
}

enum WindowsNativeModifiers {
	ShiftLeft = 0x00000001,
	ControlLeft = 0x00000002,
	AltLeft = 0x00000004,
	MetaLeft = 0x00000008,
	ShiftRight = 0x00000010,
	ControlRight = 0x00000020,
	AltRight = 0x00000040,
	MetaRight = 0x00000080,
	CapsLock = 0x00000100,
	NumLock = 0x00000200,
	ScrollLock = 0x00000400,
	ExtendedKey = 0x01000000,

	// Convenience mappings
	ShiftAny = 0x00000011,
	ControlAny = 0x00000022,
	AltAny = 0x00000044,
	MetaAny = 0x00000088,
	LockAny = 0x00000700
};

/*
	A Windows KeyboardLayoutItem has 8 possible states:
	1. Unmodified
	2. Shift
	3. Control
	4. Control + Shift
	5. Alt
	6. Alt + Shift
	7. Alt + Control
	8. Alt + Control + Shift
*/
struct KeyboardLayoutItem {
	uint dirty : 1;
	uint exists : 1; // whether this item has been initialized (by updatePossibleKeyCodes)
	quint8 deadkeys;
	static const size_t NumQtKeys = 9;
	quint32 qtKey[NumQtKeys]; // Can by any Qt::Key_<foo>, or unicode character
};


// Key translation ---------------------------------------------------------------------[ start ] --
// Meaning of values:
//             0 = Character output key, needs keyboard driver mapping
//   Key_unknown = Unknown Virtual Key, no translation possible, ignore
static const uint KeyTbl[] = { // Keyboard mapping table
							   // Dec |  Hex | Windows Virtual key
	Qt::Key_unknown,    //   0   0x00
	Qt::Key_unknown,    //   1   0x01   VK_LBUTTON          | Left mouse button
	Qt::Key_unknown,    //   2   0x02   VK_RBUTTON          | Right mouse button
	Qt::Key_Cancel,     //   3   0x03   VK_CANCEL           | Control-Break processing
	Qt::Key_unknown,    //   4   0x04   VK_MBUTTON          | Middle mouse button
	Qt::Key_unknown,    //   5   0x05   VK_XBUTTON1         | X1 mouse button
	Qt::Key_unknown,    //   6   0x06   VK_XBUTTON2         | X2 mouse button
	Qt::Key_unknown,    //   7   0x07   -- unassigned --
	Qt::Key_Backspace,  //   8   0x08   VK_BACK             | BackSpace key
	Qt::Key_Tab,        //   9   0x09   VK_TAB              | Tab key
	Qt::Key_unknown,    //  10   0x0A   -- reserved --
	Qt::Key_unknown,    //  11   0x0B   -- reserved --
	Qt::Key_Clear,      //  12   0x0C   VK_CLEAR            | Clear key
	Qt::Key_Return,     //  13   0x0D   VK_RETURN           | Enter key
	Qt::Key_unknown,    //  14   0x0E   -- unassigned --
	Qt::Key_unknown,    //  15   0x0F   -- unassigned --
	Qt::Key_Shift,      //  16   0x10   VK_SHIFT            | Shift key
	Qt::Key_Control,    //  17   0x11   VK_CONTROL          | Ctrl key
	Qt::Key_Alt,        //  18   0x12   VK_MENU             | Alt key
	Qt::Key_Pause,      //  19   0x13   VK_PAUSE            | Pause key
	Qt::Key_CapsLock,   //  20   0x14   VK_CAPITAL          | Caps-Lock
	Qt::Key_unknown,    //  21   0x15   VK_KANA / VK_HANGUL | IME Kana or Hangul mode
	Qt::Key_unknown,    //  22   0x16   -- unassigned --
	Qt::Key_unknown,    //  23   0x17   VK_JUNJA            | IME Junja mode
	Qt::Key_unknown,    //  24   0x18   VK_FINAL            | IME final mode
	Qt::Key_unknown,    //  25   0x19   VK_HANJA / VK_KANJI | IME Hanja or Kanji mode
	Qt::Key_unknown,    //  26   0x1A   -- unassigned --
	Qt::Key_Escape,     //  27   0x1B   VK_ESCAPE           | Esc key
	Qt::Key_unknown,    //  28   0x1C   VK_CONVERT          | IME convert
	Qt::Key_unknown,    //  29   0x1D   VK_NONCONVERT       | IME non-convert
	Qt::Key_unknown,    //  30   0x1E   VK_ACCEPT           | IME accept
	Qt::Key_Mode_switch,//  31   0x1F   VK_MODECHANGE       | IME mode change request
	Qt::Key_Space,      //  32   0x20   VK_SPACE            | Spacebar
	Qt::Key_PageUp,     //  33   0x21   VK_PRIOR            | Page Up key
	Qt::Key_PageDown,   //  34   0x22   VK_NEXT             | Page Down key
	Qt::Key_End,        //  35   0x23   VK_END              | End key
	Qt::Key_Home,       //  36   0x24   VK_HOME             | Home key
	Qt::Key_Left,       //  37   0x25   VK_LEFT             | Left arrow key
	Qt::Key_Up,         //  38   0x26   VK_UP               | Up arrow key
	Qt::Key_Right,      //  39   0x27   VK_RIGHT            | Right arrow key
	Qt::Key_Down,       //  40   0x28   VK_DOWN             | Down arrow key
	Qt::Key_Select,     //  41   0x29   VK_SELECT           | Select key
	Qt::Key_Printer,    //  42   0x2A   VK_PRINT            | Print key
	Qt::Key_Execute,    //  43   0x2B   VK_EXECUTE          | Execute key
	Qt::Key_Print,      //  44   0x2C   VK_SNAPSHOT         | Print Screen key
	Qt::Key_Insert,     //  45   0x2D   VK_INSERT           | Ins key
	Qt::Key_Delete,     //  46   0x2E   VK_DELETE           | Del key
	Qt::Key_Help,       //  47   0x2F   VK_HELP             | Help key
	0,                  //  48   0x30   (VK_0)              | 0 key
	0,                  //  49   0x31   (VK_1)              | 1 key
	0,                  //  50   0x32   (VK_2)              | 2 key
	0,                  //  51   0x33   (VK_3)              | 3 key
	0,                  //  52   0x34   (VK_4)              | 4 key
	0,                  //  53   0x35   (VK_5)              | 5 key
	0,                  //  54   0x36   (VK_6)              | 6 key
	0,                  //  55   0x37   (VK_7)              | 7 key
	0,                  //  56   0x38   (VK_8)              | 8 key
	0,                  //  57   0x39   (VK_9)              | 9 key
	Qt::Key_unknown,    //  58   0x3A   -- unassigned --
	Qt::Key_unknown,    //  59   0x3B   -- unassigned --
	Qt::Key_unknown,    //  60   0x3C   -- unassigned --
	Qt::Key_unknown,    //  61   0x3D   -- unassigned --
	Qt::Key_unknown,    //  62   0x3E   -- unassigned --
	Qt::Key_unknown,    //  63   0x3F   -- unassigned --
	Qt::Key_unknown,    //  64   0x40   -- unassigned --
	0,                  //  65   0x41   (VK_A)              | A key
	0,                  //  66   0x42   (VK_B)              | B key
	0,                  //  67   0x43   (VK_C)              | C key
	0,                  //  68   0x44   (VK_D)              | D key
	0,                  //  69   0x45   (VK_E)              | E key
	0,                  //  70   0x46   (VK_F)              | F key
	0,                  //  71   0x47   (VK_G)              | G key
	0,                  //  72   0x48   (VK_H)              | H key
	0,                  //  73   0x49   (VK_I)              | I key
	0,                  //  74   0x4A   (VK_J)              | J key
	0,                  //  75   0x4B   (VK_K)              | K key
	0,                  //  76   0x4C   (VK_L)              | L key
	0,                  //  77   0x4D   (VK_M)              | M key
	0,                  //  78   0x4E   (VK_N)              | N key
	0,                  //  79   0x4F   (VK_O)              | O key
	0,                  //  80   0x50   (VK_P)              | P key
	0,                  //  81   0x51   (VK_Q)              | Q key
	0,                  //  82   0x52   (VK_R)              | R key
	0,                  //  83   0x53   (VK_S)              | S key
	0,                  //  84   0x54   (VK_T)              | T key
	0,                  //  85   0x55   (VK_U)              | U key
	0,                  //  86   0x56   (VK_V)              | V key
	0,                  //  87   0x57   (VK_W)              | W key
	0,                  //  88   0x58   (VK_X)              | X key
	0,                  //  89   0x59   (VK_Y)              | Y key
	0,                  //  90   0x5A   (VK_Z)              | Z key
	Qt::Key_Meta,       //  91   0x5B   VK_LWIN             | Left Windows  - MS Natural kbd
	Qt::Key_Meta,       //  92   0x5C   VK_RWIN             | Right Windows - MS Natural kbd
	Qt::Key_Menu,       //  93   0x5D   VK_APPS             | Application key-MS Natural kbd
	Qt::Key_unknown,    //  94   0x5E   -- reserved --
	Qt::Key_Sleep,      //  95   0x5F   VK_SLEEP
	Qt::Key_0,          //  96   0x60   VK_NUMPAD0          | Numeric keypad 0 key
	Qt::Key_1,          //  97   0x61   VK_NUMPAD1          | Numeric keypad 1 key
	Qt::Key_2,          //  98   0x62   VK_NUMPAD2          | Numeric keypad 2 key
	Qt::Key_3,          //  99   0x63   VK_NUMPAD3          | Numeric keypad 3 key
	Qt::Key_4,          // 100   0x64   VK_NUMPAD4          | Numeric keypad 4 key
	Qt::Key_5,          // 101   0x65   VK_NUMPAD5          | Numeric keypad 5 key
	Qt::Key_6,          // 102   0x66   VK_NUMPAD6          | Numeric keypad 6 key
	Qt::Key_7,          // 103   0x67   VK_NUMPAD7          | Numeric keypad 7 key
	Qt::Key_8,          // 104   0x68   VK_NUMPAD8          | Numeric keypad 8 key
	Qt::Key_9,          // 105   0x69   VK_NUMPAD9          | Numeric keypad 9 key
	Qt::Key_Asterisk,   // 106   0x6A   VK_MULTIPLY         | Multiply key
	Qt::Key_Plus,       // 107   0x6B   VK_ADD              | Add key
	Qt::Key_Comma,      // 108   0x6C   VK_SEPARATOR        | Separator key
	Qt::Key_Minus,      // 109   0x6D   VK_SUBTRACT         | Subtract key
	Qt::Key_Period,     // 110   0x6E   VK_DECIMAL          | Decimal key
	Qt::Key_Slash,      // 111   0x6F   VK_DIVIDE           | Divide key
	Qt::Key_F1,         // 112   0x70   VK_F1               | F1 key
	Qt::Key_F2,         // 113   0x71   VK_F2               | F2 key
	Qt::Key_F3,         // 114   0x72   VK_F3               | F3 key
	Qt::Key_F4,         // 115   0x73   VK_F4               | F4 key
	Qt::Key_F5,         // 116   0x74   VK_F5               | F5 key
	Qt::Key_F6,         // 117   0x75   VK_F6               | F6 key
	Qt::Key_F7,         // 118   0x76   VK_F7               | F7 key
	Qt::Key_F8,         // 119   0x77   VK_F8               | F8 key
	Qt::Key_F9,         // 120   0x78   VK_F9               | F9 key
	Qt::Key_F10,        // 121   0x79   VK_F10              | F10 key
	Qt::Key_F11,        // 122   0x7A   VK_F11              | F11 key
	Qt::Key_F12,        // 123   0x7B   VK_F12              | F12 key
	Qt::Key_F13,        // 124   0x7C   VK_F13              | F13 key
	Qt::Key_F14,        // 125   0x7D   VK_F14              | F14 key
	Qt::Key_F15,        // 126   0x7E   VK_F15              | F15 key
	Qt::Key_F16,        // 127   0x7F   VK_F16              | F16 key
	Qt::Key_F17,        // 128   0x80   VK_F17              | F17 key
	Qt::Key_F18,        // 129   0x81   VK_F18              | F18 key
	Qt::Key_F19,        // 130   0x82   VK_F19              | F19 key
	Qt::Key_F20,        // 131   0x83   VK_F20              | F20 key
	Qt::Key_F21,        // 132   0x84   VK_F21              | F21 key
	Qt::Key_F22,        // 133   0x85   VK_F22              | F22 key
	Qt::Key_F23,        // 134   0x86   VK_F23              | F23 key
	Qt::Key_F24,        // 135   0x87   VK_F24              | F24 key
	Qt::Key_unknown,    // 136   0x88   -- unassigned --
	Qt::Key_unknown,    // 137   0x89   -- unassigned --
	Qt::Key_unknown,    // 138   0x8A   -- unassigned --
	Qt::Key_unknown,    // 139   0x8B   -- unassigned --
	Qt::Key_unknown,    // 140   0x8C   -- unassigned --
	Qt::Key_unknown,    // 141   0x8D   -- unassigned --
	Qt::Key_unknown,    // 142   0x8E   -- unassigned --
	Qt::Key_unknown,    // 143   0x8F   -- unassigned --
	Qt::Key_NumLock,    // 144   0x90   VK_NUMLOCK          | Num Lock key
	Qt::Key_ScrollLock, // 145   0x91   VK_SCROLL           | Scroll Lock key
	// Fujitsu/OASYS kbd --------------------
	0, //Qt::Key_Jisho, // 146   0x92   VK_OEM_FJ_JISHO     | 'Dictionary' key /
	//              VK_OEM_NEC_EQUAL  = key on numpad on NEC PC-9800 kbd
	Qt::Key_Massyo,     // 147   0x93   VK_OEM_FJ_MASSHOU   | 'Unregister word' key
	Qt::Key_Touroku,    // 148   0x94   VK_OEM_FJ_TOUROKU   | 'Register word' key
	0, //Qt::Key_Oyayubi_Left,//149   0x95  VK_OEM_FJ_LOYA  | 'Left OYAYUBI' key
	0, //Qt::Key_Oyayubi_Right,//150  0x96  VK_OEM_FJ_ROYA  | 'Right OYAYUBI' key
	Qt::Key_unknown,    // 151   0x97   -- unassigned --
	Qt::Key_unknown,    // 152   0x98   -- unassigned --
	Qt::Key_unknown,    // 153   0x99   -- unassigned --
	Qt::Key_unknown,    // 154   0x9A   -- unassigned --
	Qt::Key_unknown,    // 155   0x9B   -- unassigned --
	Qt::Key_unknown,    // 156   0x9C   -- unassigned --
	Qt::Key_unknown,    // 157   0x9D   -- unassigned --
	Qt::Key_unknown,    // 158   0x9E   -- unassigned --
	Qt::Key_unknown,    // 159   0x9F   -- unassigned --
	Qt::Key_Shift,      // 160   0xA0   VK_LSHIFT           | Left Shift key
	Qt::Key_Shift,      // 161   0xA1   VK_RSHIFT           | Right Shift key
	Qt::Key_Control,    // 162   0xA2   VK_LCONTROL         | Left Ctrl key
	Qt::Key_Control,    // 163   0xA3   VK_RCONTROL         | Right Ctrl key
	Qt::Key_Alt,        // 164   0xA4   VK_LMENU            | Left Menu key
	Qt::Key_Alt,        // 165   0xA5   VK_RMENU            | Right Menu key
	Qt::Key_Back,       // 166   0xA6   VK_BROWSER_BACK     | Browser Back key
	Qt::Key_Forward,    // 167   0xA7   VK_BROWSER_FORWARD  | Browser Forward key
	Qt::Key_Refresh,    // 168   0xA8   VK_BROWSER_REFRESH  | Browser Refresh key
	Qt::Key_Stop,       // 169   0xA9   VK_BROWSER_STOP     | Browser Stop key
	Qt::Key_Search,     // 170   0xAA   VK_BROWSER_SEARCH   | Browser Search key
	Qt::Key_Favorites,  // 171   0xAB   VK_BROWSER_FAVORITES| Browser Favorites key
	Qt::Key_HomePage,   // 172   0xAC   VK_BROWSER_HOME     | Browser Start and Home key
	Qt::Key_VolumeMute, // 173   0xAD   VK_VOLUME_MUTE      | Volume Mute key
	Qt::Key_VolumeDown, // 174   0xAE   VK_VOLUME_DOWN      | Volume Down key
	Qt::Key_VolumeUp,   // 175   0xAF   VK_VOLUME_UP        | Volume Up key
	Qt::Key_MediaNext,  // 176   0xB0   VK_MEDIA_NEXT_TRACK | Next Track key
	Qt::Key_MediaPrevious, //177 0xB1   VK_MEDIA_PREV_TRACK | Previous Track key
	Qt::Key_MediaStop,  // 178   0xB2   VK_MEDIA_STOP       | Stop Media key
	Qt::Key_MediaPlay,  // 179   0xB3   VK_MEDIA_PLAY_PAUSE | Play/Pause Media key
	Qt::Key_LaunchMail, // 180   0xB4   VK_LAUNCH_MAIL      | Start Mail key
	Qt::Key_LaunchMedia,// 181   0xB5   VK_LAUNCH_MEDIA_SELECT Select Media key
	Qt::Key_Launch0,    // 182   0xB6   VK_LAUNCH_APP1      | Start Application 1 key
	Qt::Key_Launch1,    // 183   0xB7   VK_LAUNCH_APP2      | Start Application 2 key
	Qt::Key_unknown,    // 184   0xB8   -- reserved --
	Qt::Key_unknown,    // 185   0xB9   -- reserved --
	0,                  // 186   0xBA   VK_OEM_1            | ';:' for US
	0,                  // 187   0xBB   VK_OEM_PLUS         | '+' any country
	0,                  // 188   0xBC   VK_OEM_COMMA        | ',' any country
	0,                  // 189   0xBD   VK_OEM_MINUS        | '-' any country
	0,                  // 190   0xBE   VK_OEM_PERIOD       | '.' any country
	0,                  // 191   0xBF   VK_OEM_2            | '/?' for US
	0,                  // 192   0xC0   VK_OEM_3            | '`~' for US
	Qt::Key_unknown,    // 193   0xC1   -- reserved --
	Qt::Key_unknown,    // 194   0xC2   -- reserved --
	Qt::Key_unknown,    // 195   0xC3   -- reserved --
	Qt::Key_unknown,    // 196   0xC4   -- reserved --
	Qt::Key_unknown,    // 197   0xC5   -- reserved --
	Qt::Key_unknown,    // 198   0xC6   -- reserved --
	Qt::Key_unknown,    // 199   0xC7   -- reserved --
	Qt::Key_unknown,    // 200   0xC8   -- reserved --
	Qt::Key_unknown,    // 201   0xC9   -- reserved --
	Qt::Key_unknown,    // 202   0xCA   -- reserved --
	Qt::Key_unknown,    // 203   0xCB   -- reserved --
	Qt::Key_unknown,    // 204   0xCC   -- reserved --
	Qt::Key_unknown,    // 205   0xCD   -- reserved --
	Qt::Key_unknown,    // 206   0xCE   -- reserved --
	Qt::Key_unknown,    // 207   0xCF   -- reserved --
	Qt::Key_unknown,    // 208   0xD0   -- reserved --
	Qt::Key_unknown,    // 209   0xD1   -- reserved --
	Qt::Key_unknown,    // 210   0xD2   -- reserved --
	Qt::Key_unknown,    // 211   0xD3   -- reserved --
	Qt::Key_unknown,    // 212   0xD4   -- reserved --
	Qt::Key_unknown,    // 213   0xD5   -- reserved --
	Qt::Key_unknown,    // 214   0xD6   -- reserved --
	Qt::Key_unknown,    // 215   0xD7   -- reserved --
	Qt::Key_unknown,    // 216   0xD8   -- unassigned --
	Qt::Key_unknown,    // 217   0xD9   -- unassigned --
	Qt::Key_unknown,    // 218   0xDA   -- unassigned --
	0,                  // 219   0xDB   VK_OEM_4            | '[{' for US
	0,                  // 220   0xDC   VK_OEM_5            | '\|' for US
	0,                  // 221   0xDD   VK_OEM_6            | ']}' for US
	0,                  // 222   0xDE   VK_OEM_7            | ''"' for US
	0,                  // 223   0xDF   VK_OEM_8
	Qt::Key_unknown,    // 224   0xE0   -- reserved --
	Qt::Key_unknown,    // 225   0xE1   VK_OEM_AX           | 'AX' key on Japanese AX kbd
	Qt::Key_unknown,    // 226   0xE2   VK_OEM_102          | "<>" or "\|" on RT 102-key kbd
	Qt::Key_unknown,    // 227   0xE3   VK_ICO_HELP         | Help key on ICO
	Qt::Key_unknown,    // 228   0xE4   VK_ICO_00           | 00 key on ICO
	Qt::Key_unknown,    // 229   0xE5   VK_PROCESSKEY       | IME Process key
	Qt::Key_unknown,    // 230   0xE6   VK_ICO_CLEAR        |
	Qt::Key_unknown,    // 231   0xE7   VK_PACKET           | Unicode char as keystrokes
	Qt::Key_unknown,    // 232   0xE8   -- unassigned --
	// Nokia/Ericsson definitions ---------------
	Qt::Key_unknown,    // 233   0xE9   VK_OEM_RESET
	Qt::Key_unknown,    // 234   0xEA   VK_OEM_JUMP
	Qt::Key_unknown,    // 235   0xEB   VK_OEM_PA1
	Qt::Key_unknown,    // 236   0xEC   VK_OEM_PA2
	Qt::Key_unknown,    // 237   0xED   VK_OEM_PA3
	Qt::Key_unknown,    // 238   0xEE   VK_OEM_WSCTRL
	Qt::Key_unknown,    // 239   0xEF   VK_OEM_CUSEL
	Qt::Key_unknown,    // 240   0xF0   VK_OEM_ATTN
	Qt::Key_unknown,    // 241   0xF1   VK_OEM_FINISH
	Qt::Key_unknown,    // 242   0xF2   VK_OEM_COPY
	Qt::Key_unknown,    // 243   0xF3   VK_OEM_AUTO
	Qt::Key_unknown,    // 244   0xF4   VK_OEM_ENLW
	Qt::Key_unknown,    // 245   0xF5   VK_OEM_BACKTAB
	Qt::Key_unknown,    // 246   0xF6   VK_ATTN             | Attn key
	Qt::Key_unknown,    // 247   0xF7   VK_CRSEL            | CrSel key
	Qt::Key_unknown,    // 248   0xF8   VK_EXSEL            | ExSel key
	Qt::Key_unknown,    // 249   0xF9   VK_EREOF            | Erase EOF key
	Qt::Key_Play,       // 250   0xFA   VK_PLAY             | Play key
	Qt::Key_Zoom,       // 251   0xFB   VK_ZOOM             | Zoom key
	Qt::Key_unknown,    // 252   0xFC   VK_NONAME           | Reserved
	Qt::Key_unknown,    // 253   0xFD   VK_PA1              | PA1 key
	Qt::Key_Clear,      // 254   0xFE   VK_OEM_CLEAR        | Clear key
	0
};

class KeyMapper {
public:
	static const size_t NumKeyboardLayoutItems = 256;
	KeyboardLayoutItem keyLayout[NumKeyboardLayoutItems];

	// We not only need the scancode itself but also the extended bit of key messages. Thus we need
	// the additional bit when masking the scancode.
	enum { scancodeBitmask = 0x1ff };

	KeyMapper() {
		memset(keyLayout, 0, sizeof(keyLayout));
	}

	bool handleTigMsg(QQuickWindow *view, const TigMsg &msg) {
		if (msg.type != TigMsgType::CHAR && msg.type != TigMsgType::KEYSTATECHANGE) {
			return false;
		}

		auto wParam = WPARAM(msg.arg3);
		auto lParam = LPARAM(msg.arg4);

		// WM_(IME_)CHAR messages already contain the character in question so there is
		// no need to fiddle with our key map. In any other case add this key to the
		// keymap if it is not present yet.
		if (msg.type != TigMsgType::CHAR)
			updateKeyMap(wParam, lParam);

		const quint32 scancode = (lParam >> 16) & scancodeBitmask;
		quint32 vk_key = quint32(wParam);

		// Map native modifiers to some bit representation
		quint32 nModifiers = 0;
		nModifiers |= (GetKeyState(VK_LSHIFT) & 0x80 ? ShiftLeft : 0);
		nModifiers |= (GetKeyState(VK_RSHIFT) & 0x80 ? ShiftRight : 0);
		nModifiers |= (GetKeyState(VK_LCONTROL) & 0x80 ? ControlLeft : 0);
		nModifiers |= (GetKeyState(VK_RCONTROL) & 0x80 ? ControlRight : 0);
		nModifiers |= (GetKeyState(VK_LMENU) & 0x80 ? AltLeft : 0);
		nModifiers |= (GetKeyState(VK_RMENU) & 0x80 ? AltRight : 0);
		nModifiers |= (GetKeyState(VK_LWIN) & 0x80 ? MetaLeft : 0);
		nModifiers |= (GetKeyState(VK_RWIN) & 0x80 ? MetaRight : 0);
		// Add Lock keys to the same bits
		nModifiers |= (GetKeyState(VK_CAPITAL) & 0x01 ? CapsLock : 0);
		nModifiers |= (GetKeyState(VK_NUMLOCK) & 0x01 ? NumLock : 0);
		nModifiers |= (GetKeyState(VK_SCROLL) & 0x01 ? ScrollLock : 0);

		if (lParam & ExtendedKey) {
			nModifiers |= lParam & ExtendedKey;
		}

		// Get the modifier states (may be altered later, depending on key code)
		int state = 0;
		state |= (nModifiers & ShiftAny ? int(Qt::ShiftModifier) : 0);
		state |= (nModifiers & ControlAny ? int(Qt::ControlModifier) : 0);
		state |= (nModifiers & AltAny ? int(Qt::AltModifier) : 0);
		state |= (nModifiers & MetaAny ? int(Qt::MetaModifier) : 0);

		// Use different translation logic for key events
		std::unique_ptr<QEvent> qtEvent;
		if (msg.type == TigMsgType::KEYSTATECHANGE) {

			// This gets complicated. We have a virtual key code from Windows, but Qt wants a Qt key code from us
			// so we need to translate VK -> Qt key
			int modifiersIndex = 0;
			modifiersIndex |= (nModifiers & ShiftAny ? 0x1 : 0);
			modifiersIndex |= (nModifiers & ControlAny ? 0x2 : 0);
			modifiersIndex |= (nModifiers & AltAny ? 0x4 : 0);

			int code = keyLayout[vk_key].qtKey[modifiersIndex];


			// Invert state logic:
			// If the key actually pressed is a modifier key, then we remove its modifier key from the
			// state, since a modifier-key can't have itself as a modifier
			if (code == Qt::Key_Control)
				state = state ^ Qt::ControlModifier;
			else if (code == Qt::Key_Shift)
				state = state ^ Qt::ShiftModifier;
			else if (code == Qt::Key_Alt)
				state = state ^ Qt::AltModifier;

			// If the bit 24 of lParm is set you received a enter,
			// otherwise a Return. (This is the extended key bit)
			if ((code == Qt::Key_Return) && (lParam & 0x1000000))
				code = Qt::Key_Enter;

			// All cursor keys without extended bit
			if (!(lParam & 0x1000000)) {
				switch (code) {
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Up:
				case Qt::Key_Down:
				case Qt::Key_PageUp:
				case Qt::Key_PageDown:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_Insert:
				case Qt::Key_Delete:
				case Qt::Key_Asterisk:
				case Qt::Key_Plus:
				case Qt::Key_Minus:
				case Qt::Key_Period:
				case Qt::Key_Comma:
				case Qt::Key_0:
				case Qt::Key_1:
				case Qt::Key_2:
				case Qt::Key_3:
				case Qt::Key_4:
				case Qt::Key_5:
				case Qt::Key_6:
				case Qt::Key_7:
				case Qt::Key_8:
				case Qt::Key_9:
					state |= ((wParam >= '0' && wParam <= '9')
						|| (wParam >= VK_OEM_PLUS && wParam <= VK_OEM_3))
						? 0 : int(Qt::KeypadModifier);
				default:
					if (uint(lParam) == 0x004c0001 || uint(lParam) == 0xc04c0001)
						state |= Qt::KeypadModifier;
					break;
				}
			}
			// Other keys with with extended bit
			else {
				switch (code) {
				case Qt::Key_Enter:
				case Qt::Key_Slash:
				case Qt::Key_NumLock:
					state |= Qt::KeypadModifier;
					break;
				default:
					break;
				}
			}

			QEvent::Type type = msg.arg2 ? QEvent::KeyPress : QEvent::KeyRelease;
			QKeyEvent evt(type, code, Qt::KeyboardModifiers(state), scancode, vk_key, nModifiers, QString(), false);

			if (QGuiApplication::sendEvent(view, &evt)) {
				return true;
			}
		} else if (msg.type == TigMsgType::CHAR) {
			// We have to virtually send a press+release message
			QString text = messageKeyText(wParam);
			QKeyEvent press(QEvent::KeyPress, 0, Qt::KeyboardModifiers(state), scancode, vk_key, nModifiers, text, false);
			QKeyEvent release(QEvent::KeyRelease, 0, Qt::KeyboardModifiers(state), scancode, vk_key, nModifiers, text, false);

			if (QGuiApplication::sendEvent(view, &press) && QGuiApplication::sendEvent(view, &release)) {
				return true;
			}
		}

		return false;
	}

	void deleteLayouts()
	{
		for (size_t i = 0; i < NumKeyboardLayoutItems; ++i)
			keyLayout[i].exists = false;
	}

	void changeKeyboard()
	{
		deleteLayouts();
	}

	// Adds the msg's key to keyLayout if it is not yet present there
	void updateKeyMap(WPARAM wParam, LPARAM lParam)
	{
		unsigned char kbdBuffer[256]; // Will hold the complete keyboard state
		GetKeyboardState(kbdBuffer);
		const quint32 scancode = (lParam >> 16) & scancodeBitmask;
		updatePossibleKeyCodes(kbdBuffer, scancode, quint32(wParam));
	}

	// Helper function that is used when obtaining the list of characters that can be produced by one key and
	// every possible combination of modifiers
	inline void setKbdState(unsigned char *kbd, bool shift, bool ctrl, bool alt)
	{
		kbd[VK_LSHIFT] = (shift ? 0x80 : 0);
		kbd[VK_SHIFT] = (shift ? 0x80 : 0);
		kbd[VK_LCONTROL] = (ctrl ? 0x80 : 0);
		kbd[VK_CONTROL] = (ctrl ? 0x80 : 0);
		kbd[VK_RMENU] = (alt ? 0x80 : 0);
		kbd[VK_MENU] = (alt ? 0x80 : 0);
	}

	/**
	Remap return or action key to select key for windows mobile.
	*/
	inline quint32 winceKeyBend(quint32 keyCode)
	{
		return KeyTbl[keyCode];
	}

	// Translate a VK into a Qt key code, or unicode character
	inline quint32 toKeyOrUnicode(quint32 vk, quint32 scancode, unsigned char *kbdBuffer, bool *isDeadkey = 0)
	{
		Q_ASSERT(vk > 0 && vk < 256);
		quint32 code = 0;
		QChar unicodeBuffer[5];
		int res = ToUnicode(vk, scancode, kbdBuffer, reinterpret_cast<LPWSTR>(unicodeBuffer), 5, 0);
		// When Ctrl modifier is used ToUnicode does not return correct values. In order to assign the
		// right key the control modifier is removed for just that function if the previous call failed.
		if (res == 0 && kbdBuffer[VK_CONTROL]) {
			const unsigned char controlState = kbdBuffer[VK_CONTROL];
			kbdBuffer[VK_CONTROL] = 0;
			res = ToUnicode(vk, scancode, kbdBuffer, reinterpret_cast<LPWSTR>(unicodeBuffer), 5, 0);
			kbdBuffer[VK_CONTROL] = controlState;
		}
		if (res)
			code = unicodeBuffer[0].toUpper().unicode();

		// Qt::Key_*'s are not encoded below 0x20, so try again, and DEL keys (0x7f) is encoded with a
		// proper Qt::Key_ code
		if (code < 0x20 || code == 0x7f) // Handles res==0 too
			code = winceKeyBend(vk);

		if (isDeadkey)
			*isDeadkey = (res == -1);

		return code == Qt::Key_unknown ? 0 : code;
	}

	// Fills keyLayout for that vk_key. Values are all characters one can type using that key
	// (in connection with every combination of modifiers) and whether these "characters" are
	// dead keys.
	void updatePossibleKeyCodes(unsigned char *kbdBuffer, quint32 scancode, quint32 vk_key)
	{
		if (!vk_key || (keyLayout[vk_key].exists && !keyLayout[vk_key].dirty))
			return;

		// Copy keyboard state, so we can modify and query output for each possible permutation
		unsigned char buffer[256];
		memcpy(buffer, kbdBuffer, sizeof(buffer));
		// Always 0, as Windows doesn't treat these as modifiers;
		buffer[VK_LWIN] = 0;
		buffer[VK_RWIN] = 0;
		buffer[VK_CAPITAL] = 0;
		buffer[VK_NUMLOCK] = 0;
		buffer[VK_SCROLL] = 0;
		// Always 0, since we'll only change the other versions
		buffer[VK_RSHIFT] = 0;
		buffer[VK_RCONTROL] = 0;
		buffer[VK_LMENU] = 0; // Use right Alt, since left Ctrl + right Alt is considered AltGraph

							  // keyLayout contains the actual characters which can be written using the vk_key together with the
							  // different modifiers. '2' together with shift will for example cause the character
							  // to be @ for a US key layout (thus keyLayout[vk_key].qtKey[1] will be @). In addition to that
							  // it stores whether the resulting key is a dead key as these keys have to be handled later.
		bool isDeadKey = false;
		keyLayout[vk_key].deadkeys = 0;
		keyLayout[vk_key].dirty = false;
		keyLayout[vk_key].exists = true;
		setKbdState(buffer, false, false, false);
		keyLayout[vk_key].qtKey[0] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x01 : 0;
		setKbdState(buffer, true, false, false);
		keyLayout[vk_key].qtKey[1] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x02 : 0;
		setKbdState(buffer, false, true, false);
		keyLayout[vk_key].qtKey[2] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x04 : 0;
		setKbdState(buffer, true, true, false);
		keyLayout[vk_key].qtKey[3] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x08 : 0;
		setKbdState(buffer, false, false, true);
		keyLayout[vk_key].qtKey[4] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x10 : 0;
		setKbdState(buffer, true, false, true);
		keyLayout[vk_key].qtKey[5] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x20 : 0;
		setKbdState(buffer, false, true, true);
		keyLayout[vk_key].qtKey[6] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x40 : 0;
		setKbdState(buffer, true, true, true);
		keyLayout[vk_key].qtKey[7] = toKeyOrUnicode(vk_key, scancode, buffer, &isDeadKey);
		keyLayout[vk_key].deadkeys |= isDeadKey ? 0x80 : 0;
		// Add a fall back key for layouts which don't do composition and show non-latin1 characters
		quint32 fallbackKey = winceKeyBend(vk_key);
		if (!fallbackKey || fallbackKey == Qt::Key_unknown) {
			fallbackKey = 0;
			if (vk_key != keyLayout[vk_key].qtKey[0] && vk_key < 0x5B && vk_key > 0x2F)
				fallbackKey = vk_key;
		}
		keyLayout[vk_key].qtKey[8] = fallbackKey;

		// If one of the values inserted into the keyLayout above, can be considered a dead key, we have
		// to run the workaround below.
		if (keyLayout[vk_key].deadkeys) {
			// Push a Space, then the original key through the low-level ToAscii functions.
			// We do this because these functions (ToAscii / ToUnicode) will alter the internal state of
			// the keyboard driver By doing the following, we set the keyboard driver state back to what
			// it was before we wrecked it with the code above.
			// We need to push the space with an empty keystate map, since the driver checks the map for
			// transitions in modifiers, so this helps us capture all possible deadkeys.
			unsigned char emptyBuffer[256];
			memset(emptyBuffer, 0, sizeof(emptyBuffer));
			::ToAscii(VK_SPACE, 0, emptyBuffer, reinterpret_cast<LPWORD>(&buffer), 0);
			::ToAscii(vk_key, scancode, kbdBuffer, reinterpret_cast<LPWORD>(&buffer), 0);
		}
	}
};

bool UiManager::ProcessMessage(TigMsg &msg)
{
	// Dispatch time update messages continuously to all advanced widgets
	if (msg.type == TigMsgType::UPDATE_TIME) {
		for (auto &entry : mActiveWidgets) {
			if (entry.second.advancedWidget) {
				entry.second.advancedWidget->OnUpdateTime(msg.createdMs);
			}
		}
	}

	static KeyMapper mapper;

	switch (msg.type) {
	case TigMsgType::MOUSE:
		return ProcessMouseMessage(msg);
	case TigMsgType::WIDGET:
		return ProcessWidgetMessage(msg);
	default:
		// In order from top to bottom (back is top)
		for (auto it = mActiveWindows.rbegin(); it != mActiveWindows.rend(); it++) {
			auto window = GetWidget(*it);
			
			auto view = mActiveWidgets[*it].view;
			if (view && mapper.handleTigMsg(view, msg)) {
				return true;
			}

			if (!window->IsHidden() && window->CanHandleMessage()) {
				if (window->HandleMessage(msg)) {
					return true;
				}
			}
		}
		return false;
	}

}

LgcyWidgetId UiManager::AddQmlWindow(int x, int y, int w, int h, const std::string & path)
{

	LgcyWindow window;
	window.x = x;
	window.y = y;
	window.width = w;
	window.height = h;

	auto id = AddWindow(window);

	auto view = new QQuickView(newImpl->engine.get(), nullptr);
	view->setBaseSize(QSize(w, h));
	view->setPosition(x, y);
	view->setSource(QUrl::fromLocalFile(QString::fromStdString(path)));

	mActiveWidgets[id].view = view;

	view->show();

	return id;
}

bool UiManager::ProcessWidgetMessage(TigMsg & msg)
{
	auto widgetId = msg.arg1;
	LgcyWidget *dispatchTo;
	while (widgetId != -1 && (dispatchTo = GetWidget(widgetId)) != nullptr)
	{
		LgcyWidget *parent = (dispatchTo->parentId != -1) ? GetWindow(dispatchTo->parentId) : nullptr;
		if ((!parent || !parent->IsHidden()) && !dispatchTo->IsHidden())
		{
			auto view = mActiveWidgets[widgetId].view;
			if (view) {
				TigMsgWidget &widgetMsg = (TigMsgWidget&)msg;
				QPointF localPos(widgetMsg.x - view->x(), widgetMsg.y - view->y());
				QPointF screenPos(widgetMsg.x, widgetMsg.y);

				if (widgetMsg.widgetEventType == TigMsgWidgetEvent::Entered) {
					QEnterEvent evt(localPos, localPos, screenPos);
					if (QGuiApplication::sendEvent(view, &evt)) {
						return true;
					}
				} else if (widgetMsg.widgetEventType == TigMsgWidgetEvent::Exited) {
					QEvent evt(QEvent::Leave);
					if (QGuiApplication::sendEvent(view, &evt)) {
						return true;
					}
				}
			}

			if (dispatchTo->CanHandleMessage())
			{
				if (dispatchTo->HandleMessage(msg)) {
					return true;
				}
			}
		}
		// Bubble up the msg if the widget didn't handle it
		widgetId = dispatchTo->parentId;
	}
	return false;
}

bool UiManager::ProcessMouseMessage(TigMsg & msg)
{
	// Handle if a widget requested mouse capture
	if (mMouseCaptureWidgetId != -1)
	{
		auto advWidget = GetAdvancedWidget(mMouseCaptureWidgetId);
		if (advWidget) {			
			advWidget->HandleMessage(msg);
			return true;
		} else {
			auto widget = GetWidget(mMouseCaptureWidgetId);
			if (widget && widget->CanHandleMessage()) {
				widget->HandleMessage(msg);
				return true;
			}
		}
		return false;
	}

	static Qt::MouseButtons sLastButtons = Qt::NoButton;

	for (auto it = mActiveWindows.rbegin(); it != mActiveWindows.rend(); it++) {
		auto windowId = *it;
		auto window = GetWindow(windowId);

		if (!window || !window->IsWindow() || window->IsHidden() || !DoesWidgetContain(windowId, msg.arg1, msg.arg2)) {
			continue;
		}

		auto view = mActiveWidgets[windowId].view;
		if (view) {

			auto &mouseMsg = (TigMsgMouse&)msg;

			QPointF localPos(mouseMsg.x - view->x(), mouseMsg.y - view->y());
			QPointF screenPos(mouseMsg.x, mouseMsg.y);

			Qt::MouseButtons buttons = Qt::NoButton;
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_DOWN && !(mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED)) {
				buttons |= Qt::LeftButton;
			}
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_MMB_DOWN && !(mouseMsg.buttonStateFlags & MouseStateFlags::MSF_MMB_RELEASED)) {
				buttons |= Qt::MiddleButton;
			}
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_DOWN && !(mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED)) {
				buttons |= Qt::RightButton;
			}
			
			Qt::MouseButton button = Qt::NoButton;
			QEvent::Type type;
			if (mouseMsg.buttonStateFlags & (MouseStateFlags::MSF_POS_CHANGE)) {
				type = QEvent::MouseMove;
			} else if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_DOWN) {
				button = Qt::LeftButton;
				type = QEvent::MouseButtonPress;
			} else if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
				button = Qt::LeftButton;
				type = QEvent::MouseButtonRelease;
			} else {
				// Swallow the synthetic click events
				return true;
			}

			// Swallow events that dont change anything
			if (sLastButtons == buttons && type != QEvent::MouseMove) {
				return true;
			}
			sLastButtons = buttons;

			QMouseEvent evt(type, localPos, screenPos, button, buttons, Qt::NoModifier);

			if (QGuiApplication::sendEvent(view, &evt)) {
				return true;
			}

		}

		// Try dispatching the msg to all children of the window that are also under the mouse cursor, in reverse order of their
		// own insertion into the children list
		for (auto j = 0u; j < window->childrenCount; j++) {
			auto idx = window->childrenCount - 1 - j;
			auto childId = window->children[idx];

			if (DoesWidgetContain(childId, msg.arg1, msg.arg2))
			{
				auto child = GetWidget(childId);
				if (child && child->CanHandleMessage() && !child->IsHidden()) {
					if (child->HandleMessage(msg)) {
						return true;
					}
				}
			}
		}

		// After checking with all children, dispatch the msg to the window itself
		if (window->CanHandleMessage() && !window->IsHidden() && window->HandleMessage(msg)) {
			return true;
		}

	}

	return false;
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
