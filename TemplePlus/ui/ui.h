
#pragma once

#include <temple/dll.h>
#include "tig/tig.h"
#include <obj.h>

#define ACTIVE_WIDGET_CAP 3000

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
	int field7c;
	int field80;
	int field84;
	int field88;
	int field8C;
	int field90;
	int mouseState;
	int field98;
	int field9C;
	int fieldA0;
	int fieldA4;
	int fieldA8;
	int sndDown;
	int sndClick;
	int hoverOn;
	int hoverOff;
};

/*
	Type: 3
	Size: 176
	Only Example: wft\wft_scrollbar.c:138
*/
struct WidgetType3 : public Widget {
	int yMin;
	int yMax;
	int field84;
	int field88;
	int field8C;
	int field90;
	int field94;
	int field98;
	int field9C;
	int fieldA0;
	int fieldA4;
	int fieldA8;
	int fieldAC;
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
extern temple::GlobalPrimitive<ActiveWidgetListEntry*, 0x10EF68DC> activeWidgetAllocList;

/*
	The list of all active widgets
*/
extern temple::GlobalPrimitive<Widget**, 0x10EF68E0> activeWidgets; // [3000]
extern temple::GlobalPrimitive<int, 0x10EF68D8> activeWidgetCount;

struct ImgFile : temple::TempleAlloc {
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
	*/
	void ShowCharUi(int page);

	/*
		Not quite clear what this is.
	*/
	bool ShowWrittenUi(objHndl handle);
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
