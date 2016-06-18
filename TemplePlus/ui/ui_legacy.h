
#pragma once

enum class LgcyWidgetType : uint32_t {
	Window = 1,
	Button = 2,
	Scrollbar = 3
};

typedef int LgcyWidgetId;

/*
	The base structure of all legacy widgets
*/
using LgcyWidgetRenderFn = void (*)(LgcyWidgetId widgetId);
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
	LgcyWidgetRenderTooltipFn renderTooltip;
	LgcyWidgetRenderFn render;
	LgcyWidgetHandleMsgFn handleMessage;

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

	bool Add(int * widIdOut);
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
	int field7c;
	int field80;
	int field84;
	int field88;
	int field8C;
	int field90;
	int buttonState; // 1 - hovered 2 - down  3 - released 4 - disabled
	int field98;
	int field9C;
	int fieldA0;
	int fieldA4;
	int fieldA8;
	int sndDown;
	int sndClick;
	int hoverOn;
	int hoverOff;
	LgcyButton();
	LgcyButton(char* ButtonName, int ParentId, int X, int Y, int Width, int Height);
	LgcyButton(char* ButtonName, int ParentId, TigRect& rect);
	bool Add(int* widIdOut);
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
	bool Add(int * widIdOut);
};

#pragma pack(pop)

class ActiveLegacyWidget {
public:
	const char *sourceFile;
	uint32_t sourceLine;
	unique_ptr<LgcyWidget> widget;
};

/*
	The idea of this class is to replace the original global 
	state of the legacy UI system, so it can be integrated
	more nicely into the more modern system.
*/
class UiLegacyManager {
public:
	UiLegacyManager();
	~UiLegacyManager();

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
	LgcyWidget* GetWidget(LgcyWidgetId id);

	LgcyWindow* GetWindow(LgcyWidgetId id) {
		auto widget = GetWidget(id);
		if (widget && widget->IsWindow()) {
			return (LgcyWindow*)widget;
		}
		return nullptr;
	}

	void BringToFront(LgcyWidgetId id);
	void SendToBack(LgcyWidgetId id);
	void SetHidden(LgcyWidgetId id, bool hidden);

	void RemoveWidget(LgcyWidgetId id);
	void RemoveChildWidget(LgcyWidgetId id);

	void Render();

	LgcyWidgetId GetWidgetAt(int x, int y);
	bool DoesWidgetContain(LgcyWidgetId id, int x, int y);

private:
	UiLegacyManager(UiLegacyManager&) = delete;
	UiLegacyManager& operator=(UiLegacyManager&) = delete;

	LgcyWidgetId mNextWidgetId = 0;
	unordered_map<LgcyWidgetId, ActiveLegacyWidget> mActiveWidgets;
	vector<LgcyWidgetId> mActiveWindows;
	int maxZIndex = 0;

	/*
		This will sort the windows using their z-order in the order in which
		they should be rendered.
	*/
	void SortWindows();

};

extern UiLegacyManager uiLegacyManager;
