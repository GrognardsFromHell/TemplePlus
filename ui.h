
#pragma once

#include "addresses.h"
#include "tig.h"

struct GameSystemConf;

struct UiResizeArgs {
	int windowBufferStuffId;
	TigRect rect1;
	TigRect rect2;
};

struct Widget {
	uint32_t type;
	uint32_t size;
	int parentId;
	int widgetId;
	char name[64];
	uint32_t widgetFlags;
	int x;
	int y;
	int xrelated;
	int yrelated;
	uint32_t width;
	uint32_t height;
	uint32_t field_6c;
	uint32_t renderTooltip;
	uint32_t render; // Function pointer
	uint32_t handleMessage; // Function pointer
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
};

/*
	Type: 2
	Size: 188
	Examples: charmap_ui->c:203, options_ui->c:1342
*/
struct WidgetType2 : public Widget {
};

/*
	Type: 3
	Size: 176
	Only Example: wft\wft_scrollbar.c:138
*/
struct WidgetType3 : public Widget {
};

struct ActiveWidgetListEntry {
	Widget *widget;
	const char *sourceFilename;
	uint32_t sourceLine;
	ActiveWidgetListEntry *next;
};

/*
	Tracks all active widgets with information about where they come frome.
*/
extern GlobalPrimitive<ActiveWidgetListEntry*, 0x10EF68DC> activeWidgetAllocList;

/*
	The list of all active widgets
*/
extern GlobalPrimitive<Widget**, 0x10EF68E0> activeWidgets;
extern GlobalPrimitive<int, 0x10EF68D8> activeWidgetCount;

struct ImgFile : public TempleAlloc {
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

class Ui {
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
