
#pragma once

#include "addresses.h"

struct GameSystemConf;

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
	Examples: mm_ui.c:1057
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
	Examples: charmap_ui.c:203, options_ui.c:1342
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


typedef int(__cdecl *UiSystemInit)(const GameSystemConf *conf);
typedef void(__cdecl *UiSystemReset)();
typedef bool(__cdecl *UiSystemLoadModule)();
typedef void(__cdecl *UiSystemUnloadModule)();
typedef void(__cdecl *UiSystemShutdown)();
typedef bool(__cdecl *UiSystemSaveGame)(void *tioFile);
typedef bool(__cdecl *UiSystemLoadGame)(void *sth);

struct UiSystemSpec {
	const char *name;
	UiSystemInit init;
	UiSystemReset reset;
	UiSystemLoadModule loadModule;
	UiSystemUnloadModule unloadModule;
	UiSystemShutdown shutdown;
	UiSystemSaveGame savegame;
	UiSystemLoadGame loadgame;
	uint32_t field_20;
};

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

struct UiFuncs : AddressTable {

	bool (__cdecl *Init)(const GameSystemConf *conf);
	bool (__cdecl *LoadModule)();
	void (__cdecl *UnloadModule)();
	void (__cdecl *Shutdown)();

	ImgFile *(__cdecl *LoadImg)(const char *filename);

	bool GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int *textureIdOut) {
		return _GetAsset(assetType, (uint32_t)assetIndex, textureIdOut, 0) == 0;
	}

	void rebase(Rebaser rebase) override {
		rebase(Init, 0x101156F0);
		rebase(LoadModule, 0x10115790);
		rebase(LoadImg, 0x101E8320);
		rebase(UnloadModule, 0x101152C0);
		rebase(Shutdown, 0x10115230);
		rebase(_GetAsset, 0x1004A360);
	}

private:
	int(__cdecl *_GetAsset)(UiAssetType assetType, uint32_t assetIndex, int *textureIdOut, int offset);

};

extern UiFuncs uiFuncs;
