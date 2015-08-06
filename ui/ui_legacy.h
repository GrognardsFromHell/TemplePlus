
#pragma once

enum LgcyWidgetType : uint32_t {
	LWT_WINDOW = 1,
	LWT_BUTTON = 2,
	LWT_SCROLLBAR = 3
};

enum LgcyWidgetFlag : uint32_t {
	LWF_HIDDEN = 1
};

typedef int LgcyWidgetId;

/*
	The base structure of all legacy widgets
*/
typedef void(*LgcyWidgetRenderFn)(LgcyWidgetId widgetId);
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
	uint32_t renderTooltip;
	LgcyWidgetRenderFn render; // Function pointer
	uint32_t handleMessage; // Function pointer

	bool IsWindow() const {
		return type == LWT_WINDOW;
	}

	bool IsButton() const {
		return type == LWT_BUTTON;
	}

	bool IsScrollBar() const {
		return type == LWT_SCROLLBAR;
	}

	bool IsHidden() const {
		return (flags & LWF_HIDDEN) != 0;
	}

	size_t GetSize() const;
};

struct LgcyWindow : public LgcyWidget {
	int children[128];
	uint32_t field_27c;
	uint32_t childrenCount;
	int zIndex;
	uint32_t field_288;
	uint32_t field_28c;
	uint32_t field_290;	
};

struct LgcyButton : public LgcyWidget {
};

struct LgcyScrollBar : public LgcyWidget {
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
