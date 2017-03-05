
#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/vector.h>
#include <temple/dll.h>
#include "tig/tig.h"
#include <obj.h>
#include <util/fixes.h>

#define ACTIVE_WIDGET_CAP 3000

#pragma region UI Structs

struct TigMouseMsg;
struct TigMsg;
struct GameSystemConf;

struct UiResizeArgs {
	int windowBufferStuffId;
	TigRect rect1;
	TigRect rect2;
};

struct TooltipStyle {
	const char* styleName;
	const char* fontName;
	int fontSize;
	int fontColorAlpha;
	int fontColorRed;
	int fontColorGreen;
	int fontColorBlue;
};

enum class LgcyWidgetType : uint32_t {
	Window = 1,
	Button = 2,
	Scrollbar = 3
};

typedef int LgcyWidgetId;

struct TigMsg;
struct TigRect;
struct TigMouseMsg;

/*
The base structure of all legacy widgets
*/
using LgcyWidgetRenderFn = void(*)(LgcyWidgetId widgetId);
using LgcyWidgetRenderTooltipFn = void(*)(int x, int y, LgcyWidgetId *id);
using LgcyWidgetHandleMsgFn = BOOL(*)(LgcyWidgetId id, TigMsg *msg);

#pragma pack(push, 1)
struct LgcyWidget {
	LgcyWidgetType type;
	uint32_t size;
	LgcyWidgetId parentId;
	LgcyWidgetId widgetId;
	char name[64];
	uint32_t flags;
	int x;
	int y;
	int xrelated;
	int yrelated;
	uint32_t width;
	uint32_t height;
	uint32_t field_6c;
	LgcyWidgetRenderTooltipFn renderTooltip = nullptr;
	LgcyWidgetRenderFn render = nullptr;
	LgcyWidgetHandleMsgFn handleMessage = nullptr;

	bool IsWindow() const {
		return type == LgcyWidgetType::Window;
	}

	bool IsButton() const {
		return type == LgcyWidgetType::Button;
	}

	bool IsScrollBar() const {
		return type == LgcyWidgetType::Scrollbar;
	}

	bool IsHidden() const {
		return (flags & 1) != 0;
	}
	
	bool CanHandleMessage() const {
		return !!handleMessage;
	}

	bool HandleMessage(const TigMsg &msg) {
		return handleMessage(widgetId, const_cast<TigMsg*>(&msg)) != 0;
	}

	size_t GetSize() const;
};

enum class LgcyWindowMouseState : uint32_t {
	Outside = 0,
	Hovered = 6,
	// I have not actually found any place where this is ever set
	Pressed = 7,
	PressedOutside = 8
};

struct LgcyWindow : public LgcyWidget {
	int children[128];
	uint32_t field_27c;
	uint32_t childrenCount;
	int zIndex;
	LgcyWindowMouseState mouseState;
	uint32_t field_28c;
	uint32_t field_290;

	LgcyWindow();
	LgcyWindow(int x, int y, int w, int h);
};

/*
Type: 2
Size: 188
Examples: charmap_ui->c:203, options_ui->c:1342
*/
enum LgcyButtonState : uint32_t
{
	Normal = 0,
	Hovered,
	Down,
	Released,
	Disabled
};

struct LgcyButton : public LgcyWidget {
	int field7c = -1;
	int field80 = -1;
	int field84 = -1;
	int field88 = -1;
	int field8C = -1;
	int field90 = -1;
	LgcyButtonState buttonState = LgcyButtonState::Normal; // 1 - hovered 2 - down  3 - released 4 - disabled
	int field98;
	int field9C;
	int fieldA0;
	int fieldA4;
	int fieldA8;
	int sndDown = -1;
	int sndClick = -1;
	int sndHoverOn = -1;
	int sndHoverOff = -1;
	LgcyButton();
	LgcyButton(char* ButtonName, int ParentId, int X, int Y, int Width, int Height);
	LgcyButton(char* ButtonName, int ParentId, TigRect& rect);

	void SetDefaultSounds();
};

struct LgcyScrollBar : public LgcyWidget {
	int yMin;
	int yMax;
	int scrollbarY;
	int scrollQuantum; //the amount of change per each scrollwheel roll
	int field8C;
	int field90;
	int field94;
	int field98;
	int field9C;
	int fieldA0;
	int fieldA4;
	int fieldA8;
	int fieldAC;
	int GetY();
	bool Init(int x, int y, int height);
	bool Init(int x, int y, int height, int parentId);
};

#pragma pack(pop)

struct LgcyWidgetDeleter
{
	void operator()(LgcyWidget *p);
};

class QQuickView;

class ActiveLegacyWidget {
public:
	ActiveLegacyWidget() = default;
	EA_NON_COPYABLE(ActiveLegacyWidget)

	const char *sourceFile;
	uint32_t sourceLine;
	unique_ptr<LgcyWidget, LgcyWidgetDeleter> widget;
	QQuickView *view = nullptr;
};

#pragma endregion

inline int WidgetIdIndexOf(LgcyWidgetId widgetId, LgcyWidgetId* widgetlist, int size)
{
	for (int i = 0; i < size; i++) {
		if (widgetlist[i] == widgetId)
			return i;
	}

	return -1;
}

class QQuickView;
class UiRenderControl;

class UiManager {
public:

	UiManager();
	~UiManager();

	LgcyWidgetId AddWindow(LgcyWindow& widget);
	BOOL ButtonInit(LgcyButton * widg, char* buttonName, LgcyWidgetId parentId, int x, int y, int width, int height);
	LgcyWidgetId AddButton(LgcyButton& button);
	LgcyWidgetId AddButton(LgcyButton& button, LgcyWidgetId parentId);
	LgcyWidgetId AddScrollBar(LgcyScrollBar& scrollBar);
	LgcyWidgetId AddScrollBar(LgcyScrollBar& scrollBar, LgcyWidgetId parentId);

	/*
		sets the button's parent, and also does a bunch of mouse handling (haven't delved too deep there yet)
	*/
	void SetButtonState(LgcyWidgetId widgetId, LgcyButtonState newState);
	LgcyButtonState GetButtonState(LgcyWidgetId widId);

	bool ScrollbarGetY(LgcyWidgetId widId, int * y);
	void ScrollbarSetYmax(LgcyWidgetId widId, int yMax);
	void ScrollbarSetY(LgcyWidgetId widId, int value); // I think? sets field84

	/**
		Adds a widget to the list of created widgets and returns the assigned widget id.
	*/
	LgcyWidgetId AddWidget(const LgcyWidget *widget, const char *sourceFile, uint32_t sourceLine);

	/*
	Add something to the list of active windows on top of all existing windows.
	*/
	void AddWindow(LgcyWidgetId id);
	void RemoveWindow(LgcyWidgetId id);

	/*
	Gets a pointer to the widget with the given widget ID, null if it doesn't exist.
	*/
	LgcyWidget* GetWidget(LgcyWidgetId id) const;
	LgcyWindow* GetWindow(LgcyWidgetId widId) const;
	LgcyButton* GetButton(LgcyWidgetId widId) const;
	LgcyScrollBar* GetScrollBar(LgcyWidgetId widId) const;

	void BringToFront(LgcyWidgetId id);
	void SendToBack(LgcyWidgetId id);
	void SetHidden(LgcyWidgetId id, bool hidden);
	bool IsHidden(int widId) const {
		auto widget = GetWidget(widId);
		return !widget || widget->IsHidden();
	}

	void RemoveWidget(LgcyWidgetId id);
	bool AddChild(LgcyWidgetId parentId, LgcyWidgetId childId);
	void RemoveChildWidget(LgcyWidgetId id);

	void Render();

	LgcyWindow* GetWindowAt(int x, int y);

	LgcyWidgetId GetWidgetAt(int x, int y);
	bool DoesWidgetContain(LgcyWidgetId id, int x, int y);

	using IdVector = std::vector<LgcyWidgetId>;
	using WidgetMap = eastl::hash_map<LgcyWidgetId, ActiveLegacyWidget>;

	const IdVector& GetActiveWindows() const {
		return mActiveWindows;
	}

	/**
	* Uses the current mouse position to refresh which widget is being moused over.
	* Useful if a widget is hidden, shown or added to update the mouse-over state
	* without actually moving the mouse.
	*/
	void RefreshMouseOverState();

	/**
	* Handles a mouse message and produces higher level mouse messages based on it.
	*/
	bool TranslateMouseMessage(const TigMouseMsg &mouseMsg);
	bool ProcessMessage(TigMsg& mouseMsg);

	LgcyWidgetId GetMouseCaptureWidgetId() const {
		return mMouseCaptureWidgetId;
	}

	void SetMouseCaptureWidgetId(LgcyWidgetId widgetId) {
		mMouseCaptureWidgetId = widgetId;
	}
	void UnsetMouseCaptureWidgetId(LgcyWidgetId widgetId) {
		if (mMouseCaptureWidgetId == widgetId) {
			mMouseCaptureWidgetId = -1;
		}
	}

	QQuickView* AddQmlWindow(int x, int y, int w, int h, const std::string &path);
	QQuickView* GetQmlWindow(int widgetId);

private:
	UiManager(UiManager&) = delete;
	UiManager& operator=(UiManager&) = delete;
	UiManager(UiManager&&) = delete;
	UiManager& operator=(UiManager&&) = delete;

	LgcyWidgetId mNextWidgetId = 0;
	WidgetMap mActiveWidgets;
	IdVector mActiveWindows;
	int maxZIndex = 0;

	std::unique_ptr<UiRenderControl> mRenderControl;

	/*
	This will sort the windows using their z-order in the order in which
	they should be rendered.
	*/
	void SortWindows();

	LgcyWidgetId& mMouseCaptureWidgetId = temple::GetRef<LgcyWidgetId>(0x11E74384);
	int& mWidgetMouseHandlerWidgetId = temple::GetRef<int>(0x10301324);
	LgcyWidgetId mMouseButtonId = -1; // Previously @ 0x10301328
	void(*mMouseMsgHandlerRenderTooltipCallback)(int x, int y, void* data) = temple::GetPointer<void(int x, int y, void* data)>(0x101F9870);

	bool ProcessWidgetMessage(TigMsg &msg);
	bool ProcessMouseMessage(TigMsg &msg);
};

extern UiManager *uiManager;
