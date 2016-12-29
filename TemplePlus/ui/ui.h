
#pragma once

#include <temple/dll.h>
#include "tig/tig.h"
#include <obj.h>
#include <util/fixes.h>
#include "ui_legacy.h"

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

struct ActiveWidgetListEntry {
	LgcyWidget *widget;
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


class Ui {
public:
	bool GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int &textureIdOut);

	/*
		Loads a .img file.
	*/
	ImgFile* LoadImg(const char *filename);
	
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
	void WidgetRemoveRegardParent(LgcyWidgetId widIdx);
	void WidgetAndWindowRemove(LgcyWidgetId widId);
	void WidgetRemove(LgcyWidgetId widId);
	void WidgetSetHidden(LgcyWidgetId widId, int hiddenState);

	LgcyWidget* GetWidget(LgcyWidgetId widId);
	LgcyWindow* GetWindow(LgcyWidgetId widId);
	LgcyButton* GetButton(LgcyWidgetId widId);
	LgcyScrollBar* GetScrollBar(LgcyWidgetId widId);

	int GetWindowContainingPoint(int x, int y);
	LgcyButtonState GetButtonState(LgcyWidgetId widId);
	void WidgetBringToFront(LgcyWidgetId widId);
	int WidgetlistIndexof(LgcyWidgetId widgetId, int * widgetlist, int size);
	bool WidgetContainsPoint(LgcyWidgetId widgetId, int x, int y);
	
	/*
			gets widget at x,y including children
			*/
	int GetAtInclChildren(int x, int y);

	/**
	 * Handles a mouse message and produces higher level mouse messages based on it.
	 */
	int TranslateMouseMessage(TigMouseMsg* mouseMsg);
	bool ProcessMessage(TigMsg& mouseMsg);

	bool ScrollbarGetY(LgcyWidgetId widId, int * y);
	void ScrollbarSetYmax(LgcyWidgetId widId, int yMax);
	void ScrollbarSetY(LgcyWidgetId widId, int value); // I think? sets field84
	const char* GetTooltipString(int line) const;
	const char* GetStatShortName(Stat stat) const;
	const char* GetStatMesLine(int line) const;

private:

	int& mMouseCaptureWidgetId = temple::GetRef<int>(0x11E74384);

	bool ProcessWidgetMessage(TigMsg &msg);
	bool ProcessMouseMessage(TigMsg &msg);

	UiLegacyManager mLegacy;
};
extern Ui ui;
