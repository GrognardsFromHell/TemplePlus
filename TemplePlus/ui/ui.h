
#pragma once

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

struct Widget {
	uint32_t type; // 1 - window 2 - button 3 - scrollbar
	uint32_t size;
	int parentId;
	int widgetId;
	char name[64];
	uint32_t widgetFlags; // 1 - hidden?
	int x;
	int y;
	int xrelated;
	int yrelated;
	uint32_t width;
	uint32_t height;
	uint32_t field_6c;
	int(__cdecl*renderTooltip)(int x, int y, int* widId) ;
	void(__cdecl*render)(int widId); // Function pointer
	bool(__cdecl*handleMessage)(int widId, TigMsg* msg); // Function pointer
};

/*
	Type: 1
	Size: 660
	Examples: mm_ui->c:1057
*/
struct WidgetType1 : public Widget {
	int children[128];
	uint32_t field_27c;
	uint32_t childrenCount;
	int windowId;
	uint32_t field_288;
	uint32_t field_28c;
	uint32_t field_290;


	WidgetType1()
	{
		widgetId = -1;
		parentId = -1;
		type = 1;
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
		field_288 = 0;
		field_28c = 0;
		windowId = 0;
		widgetFlags = 0;
	}

	void WidgetType1Init(int xin, int yin, int widthin, int heightin)
	{
		widgetId = -1;
		parentId = -1;
		type = 1;
		x = xin;
		y = yin;
		xrelated = xin;
		yrelated = yin;
		height = heightin;
		width = widthin;
		render = 0;
		handleMessage = 0;
		renderTooltip = 0;
		childrenCount = 0;
		field_288 = 0;
		field_28c = 0;
		windowId = 0;
		widgetFlags = 0;
	}
	bool Add(int * widIdOut);
};

/*
	Type: 2
	Size: 188
	Examples: charmap_ui->c:203, options_ui->c:1342
*/
enum UiButtonState : uint32_t
{
	UBS_NORMAL = 0,
	UBS_HOVERED,
	UBS_DOWN,
	UBS_RELEASED,
	UBS_DISABLED
};

struct WidgetType2 : public Widget {
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
	WidgetType2();
	WidgetType2(char* ButtonName, int ParentId, int X, int Y, int Width, int Height);
	WidgetType2(char* ButtonName, int ParentId, TigRect& rect);
	bool Add(int* widIdOut);
};

/*
	Type: 3
	Size: 176
	Only Example: wft\wft_scrollbar.c:138
*/
struct WidgetType3 : public Widget {
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
	bool Add(int * widIdOut);
};

struct ActiveWidgetListEntry {
	Widget *widget;
	const char *sourceFilename;
	uint32_t sourceLine;
	ActiveWidgetListEntry *next;
};




struct ImgFile {
	int tilesX;
	int tilesY;
	int width;
	int height;
	int textureIds[16];
	int field_50;
};

enum class UiAssetType : uint32_t {
	Portraits = 0,
	Inventory,
	Generic, // Textures
	GenericLarge // IMG files
};

enum class UiGenericAsset : uint32_t {
	AcceptHover = 0,
	AcceptNormal,
	AcceptPressed,
	DeclineHover,
	DeclineNormal,
	DeclinePressed,
	DisabledNormal,
	GenericDialogueCheck
};

typedef int(*UiMsgFunc)(int, TigMsg*);


struct TooltipStyle{
	const char* styleName;
	const char* fontName;
	int fontSize;
	int fontColorAlpha;
	int fontColorRed;
	int fontColorGreen;
	int fontColorBlue;
};
#pragma endregion

/*
Tracks all active widgets with information about where they come frome.
*/
extern temple::GlobalPrimitive<ActiveWidgetListEntry*, 0x10EF68DC> activeWidgetAllocList;


extern temple::GlobalPrimitive<int, 0x10EF68D8> activeWidgetCount;


class Ui : public TempleFix {
public:
	bool GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int &textureIdOut);

	/*
		Loads a .img file.
	*/
	ImgFile* LoadImg(const char *filename);
	
	/*
		Notifies UI of a resized screen.
	*/
	void ResizeScreen(int bufferStuffId, int width, int height);

	/*
	
	*/
	void UpdateCombatUi();

	/*
	
	*/
	void UpdatePartyUi();

	/*
	
	*/
	void ShowWorldMap(int unk);

	void WorldMapTravelByDialog(int destination);

	/*
		Shows the party pool UI. Flag indicates whether the pool
		is being shown ingame (tavern) or from outside the game.
	*/
	void ShowPartyPool(bool ingame);

	/*
		This is actually used to *hide* the char ui if param is 0.
		Other meanings of param are currently unknown.
		1 means opening for looting; enables the Take All button
		2 means opening for bartering
		3 ??
		4 means opening for spells targeting inventory items
	*/
	void ShowCharUi(int page);

	/*
		Not quite clear what this is.
	*/
	bool ShowWrittenUi(objHndl handle);



	bool CharEditorIsActive();
	bool CharLootingIsActive();

	bool IsWidgetHidden(int widId);
	BOOL AddWindow(Widget* widget, unsigned size, int* widgetId, const char * codeFileName, int lineNumber);
	BOOL ButtonInit(WidgetType2 * widg, char* buttonName, int parentId, int x, int y, int width, int height);
	BOOL AddButton(WidgetType2* button, unsigned size, int* widgId, const char * codeFileName, int lineNumber);
	/*
		sets the button's parent, and also does a bunch of mouse handling (haven't delved too deep there yet)
	*/
	BOOL BindToParent(int parentId, int buttonId);
	BOOL SetDefaultSounds(int widId);
	BOOL ButtonSetButtonState(int widgetId, int newState);
	BOOL WidgetRemoveRegardParent(int widIdx);
	BOOL WidgetAndWindowRemove(int widId);
	BOOL WidgetSetHidden(int widId, int hiddenState);
	BOOL WidgetCopy(int widId, Widget* widgetOut);
	int WidgetSet(int widId, const Widget* widg);

	Widget* WidgetGet(int widId);
	WidgetType1* WidgetGetType1(int widId);
	WidgetType2* GetButton(int widId);
	WidgetType3* ScrollbarGet(int widId);

	int GetWindowContainingPoint(int x, int y);
	BOOL GetButtonState(int widId, int* state);
	bool GetButtonState(int widId, UiButtonState& state);
	void WidgetBringToFront(int widId);
	int WidgetlistIndexof(int widgetId, int * widgetlist, int size);
	BOOL WidgetContainsPoint(int widgetId, int x, int y);
	
	/*
			gets widget at x,y including children
			*/
	int GetAtInclChildren(int x, int y);

	void(__cdecl *__cdecl GetCursorTextDrawCallback())(int x, int y, void *data);

	void SetCursorTextDrawCallback(void(* cursorTextDrawCallback)(int, int, void*), void* data);
	int UiWidgetHandleMouseMsg(TigMouseMsg* mouseMsg);

	bool ScrollbarGetY(int widId, int * y);
	void ScrollbarSetYmax(int widId, int yMax);
	BOOL ScrollbarSetY(int widId, int value); // I think? sets field84
	const char* GetTooltipString(int line) const;
	const char* GetStatShortName(Stat stat) const;
	/*
			The list of all active widgets
			*/
	static Widget** activeWidgets;
	// = 
	void  apply  () override
	{
		activeWidgets = temple::GetPointer<Widget*>(0x10EF68E0); // [3000]
	}
};
extern Ui ui;

/*
	Utility class to load and initialize the UI system using RAII.
*/
class UiLoader {
public:
	explicit UiLoader(const GameSystemConf &conf);
	~UiLoader();
};

/*
	Utility class to load and unload the module in the UI system using RAII.
*/
class UiModuleLoader {
public:
	explicit UiModuleLoader(const UiLoader &ui);
	~UiModuleLoader();
};
