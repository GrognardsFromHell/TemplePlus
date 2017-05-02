
#pragma once

#include <string>
#include <cstdint>

#include "ui_system.h"
#include "ui_ingame.h"
#include "ui_item_creation.h"
#include "config/config.h"
#include "party.h"

struct UiSystemConf {
	BOOL editor = FALSE;
	int width;
	int height;
	int viewportId = -1;
	void *field_10;
	void *renderfunc;
};

class UiMainMenu : public UiSystem {
public:
    static constexpr auto Name = "MainMenu-UI";
    UiMainMenu(const UiSystemConf &config);
    ~UiMainMenu();
    const std::string &GetName() const override;
};

class UiLoadGame : public UiSystem {
public:
    static constexpr auto Name = "LoadGame";
    UiLoadGame(const UiSystemConf &config);
    ~UiLoadGame();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	void Show(bool fromMainMenu);
	void Hide();
	void Hide2(); // This seems to be an internal one
};

class UiSaveGame : public UiSystem {
public:
    static constexpr auto Name = "SaveGame";
    UiSaveGame(const UiSystemConf &config);
    ~UiSaveGame();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	void Show(bool fromMainMenu);
	void Hide();
	void Hide2(); // Pops back to main menu sometimes (?)
};

class UiInGameSelect : public UiSystem {
public:
    static constexpr auto Name = "IntgameSelect";
    UiInGameSelect(const UiSystemConf &config);
    ~UiInGameSelect();
    void Reset() override;
    const std::string &GetName() const override;
};

class UiRadialMenu : public UiSystem {
public:
    static constexpr auto Name = "RadialMenu";
    UiRadialMenu(const UiSystemConf &config);
    ~UiRadialMenu();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiAnim : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Anim-UI";
    UiAnim(const UiSystemConf &config);
    void Reset() override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    const std::string &GetName() const override;
};

class UiTB : public UiSystem {
public:
    static constexpr auto Name = "TB-UI";
    UiTB(const UiSystemConf &config);
    ~UiTB();
    const std::string &GetName() const override;
};

class UiWMapRnd : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "WMap-Rnd";
    void LoadModule() override;
    void UnloadModule() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    const std::string &GetName() const override;

	void StartRandomEncounterTimer();
};

class UiCombat : public UiSystem {
public:
    static constexpr auto Name = "Combat-UI";
    UiCombat(const UiSystemConf &config);
    ~UiCombat();
    void Reset() override;
    const std::string &GetName() const override;

	void Update();
};

class UiSlide : public UiSystem {
public:
    static constexpr auto Name = "Slide-UI";
    void LoadModule() override;
    void UnloadModule() override;
    const std::string &GetName() const override;
};

class UiDlg : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Dlg-UI";
    UiDlg(const UiSystemConf &config);
    ~UiDlg();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	bool IsActive() const;

private:
	uint32_t& mFlags = temple::GetRef<uint32_t>(0x10BEA5F4);
	LgcyWidgetId& mWindowId = temple::GetRef<LgcyWidgetId>(0x10BEA2E4);
};

class UiPcCreation : public UiSystem {
public:
    static constexpr auto Name = "pc_creation";
    UiPcCreation(const UiSystemConf &config);
    ~UiPcCreation();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	/**
	 * Will immediately start a new game if the default party was set.
	 */
	void Start();
};

/**
 * For which purpose has the inventory screen been opened?
 */
enum class UiCharDisplayType : uint32_t {
	Hidden = 0,
	Looting = 1,
	Bartering = 2,
	LevelUp = 3,
	UsingMagicOnItem = 4,
	PartyPool = 5,
	Unk6 = 6
};

class UiChar : public UiSystem {
public:
    static constexpr auto Name = "Char-UI";
    UiChar(const UiSystemConf &config);
    ~UiChar();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	// Was @ 101F97D0 (GetInventoryObjState)
	bool GetInventoryObjectState() const {
		return mInventoryObjState != 0;
	}

	// Was @ 101F97E0 (UiCharInventoryObjSetState)
	void SetInventoryObjectState(bool state) {
		mInventoryObjState = state ? 1 : 0;
	}

	// Was @ 10155160 (GetCharInventoryObj)
	objHndl GetInventoryObject() const {
		return mInventoryObj;
	}

	// Was @ 10155170 (UiCharInventoryObjSet)
	void SetInventoryObject(objHndl handle) {
		mInventoryObj = handle;
		SetInventoryObjectState(!!handle);
	}

	objHndl GetTooltipItem() const	{
		return mTooltipItem;
	}

	// Was @ 10144030 (ui_char_has_current_critter)
	bool IsVisible() const {
		return !!mCurrentCritter;
	}

	UiCharDisplayType GetDisplayType() const {
		return mDisplayType;
	}

	bool IsLevelingUp() const {
		return GetDisplayType() == UiCharDisplayType::LevelUp;
	}

	bool IsBartering() const {
		return GetDisplayType() == UiCharDisplayType::Bartering;
	}

	bool IsLootingActive() const {
		return mLooting != FALSE;
	}

	/*
		This is actually used to *hide* the char ui if param is 0.
		Other meanings of param are currently unknown.
		1 means opening for looting; enables the Take All button
		2 means opening for bartering
		3 ??
		4 means opening for spells targeting inventory items
	*/
	void Show(UiCharDisplayType type);

	void Hide() {
		Show(UiCharDisplayType::Hidden);
	}
	
private:
	objHndl &mInventoryObj = temple::GetRef<objHndl>(0x10BEECC0);
	BOOL &mInventoryObjState = temple::GetRef<BOOL>(0x10EF97C4);
	objHndl &mCurrentCritter = temple::GetRef<objHndl>(0x10BE9940);
	UiCharDisplayType &mDisplayType = temple::GetRef<UiCharDisplayType>(0x10BE994C);
	BOOL &mLooting = temple::GetRef<BOOL>(0x10BE6EE8);
	objHndl &mTooltipItem = temple::GetRef<objHndl>(0x10BF07B8);
};

class UiToolTip : public UiSystem {
public:
    static constexpr auto Name = "ToolTip-UI";
    UiToolTip(const UiSystemConf &config);
    ~UiToolTip();
    const std::string &GetName() const override;
};

class UiLogbook : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Logbook-UI";
    UiLogbook(const UiSystemConf &config);
    ~UiLogbook();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	// Was @ 101260F0 (ui_logbook_is_visible)
	bool IsVisible() const {
		return mVisible != 0;
	}

private:
	BOOL &mVisible = temple::GetRef<BOOL>(0x10BE0C58);
};

class UiScrollpane : public UiSystem {
public:
    static constexpr auto Name = "Scrollpane-UI";
    UiScrollpane(const UiSystemConf &config);
    ~UiScrollpane();
    const std::string &GetName() const override;
};

class UiTownmap : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Townmap-UI";
    UiTownmap(const UiSystemConf &config);
    ~UiTownmap();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	// Was @ 10128B60 (ui_townmap_is_visible)
	bool IsVisible() const {
		return mVisible != 0;
	}

private:
	BOOL &mVisible = temple::GetRef<BOOL>(0x10BE1F28);
};

class UiPopup : public UiSystem {
public:
    static constexpr auto Name = "Popup-UI";
    UiPopup(const UiSystemConf &config);
    ~UiPopup();
    void Reset() override;
    const std::string &GetName() const override;
};

class UiTextDialog : public UiSystem {
public:
    static constexpr auto Name = "TextDialog-UI";
    UiTextDialog(const UiSystemConf &config);
    ~UiTextDialog();
    const std::string &GetName() const override;
};

class UiFocusManager : public UiSystem {
public:
    static constexpr auto Name = "FocusManager-UI";
    UiFocusManager(const UiSystemConf &config);
    const std::string &GetName() const override;
};

class UiWorldmap : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Worldmap-UI";
    UiWorldmap(const UiSystemConf &config);
    ~UiWorldmap();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	void Show(int mode);

	// Called by the dialog scripts to travel to an area based on a dialog choice
	void TravelToArea(int area);
};

class UiRandomEncounter : public UiSystem {
public:
    static constexpr auto Name = "RandomEncounter-UI";
    UiRandomEncounter(const UiSystemConf &config);
    ~UiRandomEncounter();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiHelp : public UiSystem {
public:
    static constexpr auto Name = "Help-UI";
    UiHelp(const UiSystemConf &config);
    ~UiHelp();
    void Reset() override;
    const std::string &GetName() const override;
};

class UiSkillMastery : public UiSystem {
public:
    static constexpr auto Name = "SkillMastery-UI";
    UiSkillMastery(const UiSystemConf &config);
    ~UiSkillMastery();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiUtilityBar : public UiSystem {
public:
    static constexpr auto Name = "UtilityBar-UI";
    UiUtilityBar(const UiSystemConf &config);
    ~UiUtilityBar();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	// Hides windows that can be opened from the utility bar
	void HideOpenedWindows(bool hideOptions);

	void Hide();
	void Show();
	bool IsRollHistoryVisible(); // is the roll history console visible
	bool IsVisible();
};

class UiTrack : public UiSystem {
public:
    static constexpr auto Name = "Track-UI";
    UiTrack(const UiSystemConf &config);
    ~UiTrack();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiPartyPool : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "party_pool";
    UiPartyPool(const UiSystemConf &config);
    ~UiPartyPool();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	/*
		Shows the party pool UI. Flag indicates whether the pool
		is being shown ingame (tavern) or from outside the game.
	*/
	void Show(bool ingame);

	void Refresh();
};

class UiPccPortrait : public UiSystem {
friend class UiPccPortraitFix;
public:
    static constexpr auto Name = "pcc_portrait";
    UiPccPortrait(const UiSystemConf &config);
    ~UiPccPortrait();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	void Hide();
	void ButtonActivateNext();
	void Refresh();
	void Disable();
	
private:
	static constexpr int MAX_PC_CREATION_PORTRAITS = 8;

	bool HandleMessage(LgcyWidgetId widgetId, TigMsg* tigMsg);

	int* pcCreationIdx = temple::GetPointer<int>(0x10BF0BC8);
	int* uiPccPortraitTexture = temple::GetPointer<int>(0x10BF0BCC);
	int* uiPccPortraitHoverTexture = temple::GetPointer<int>(0x10BF0C20);
	int* uiPccPortraitClickTexture = temple::GetPointer<int>(0x10BF1354);
	int* uiPccPortraitDisabledTexture = temple::GetPointer<int>(0x10BF1358);
	int* uiPcPortraitsFullMaybe = temple::GetPointer<int>(0x10BF0ED0);

	LgcyWindow * pcCreationPortraitsMainWidget = temple::GetPointer<LgcyWindow>(0x10BF0C28);
		
	int pcPortraitsMainId = -1;
	int pcPortraitWidgIds[MAX_PC_CREATION_PORTRAITS] = { -1, };
	TigRect pcPortraitRects[MAX_PC_CREATION_PORTRAITS];
	TigRect pcPortraitBoxRects[MAX_PC_CREATION_PORTRAITS];

	void InitWidgets(int height);

};

class UiParty : public UiSystem {
public:
    static constexpr auto Name = "Party-UI";
    UiParty(const UiSystemConf &config);
    ~UiParty();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	// Was @ 10131990
	void SetPressedObj(objHndl handle) {
		mPressedObj = handle;
	}

	// Was @ 10131970
	void SetHoveredObj(objHndl handle) {
		mHoveredObj = handle;
	}

	void Update();
	void UpdateAndShowMaybe();

private:
	/**
	 * Indicates for which party member a "mouse pressed" frame should be drawn regardless 
	 * of the actual mouse state.
	 */
	objHndl& mPressedObj = temple::GetRef<objHndl>(0x10BE3400);

	/**
	* Indicates for which party member a "mouse over" frame should be drawn regardless
	* of the actual mouse state.
	*/
	objHndl& mHoveredObj = temple::GetRef<objHndl>(0x10BE33F8);

	void(*UpdatePartyUi)() = temple::GetPointer<void()>(0x1009A740);
};

class UiFormation : public UiSystem {
public:
    static constexpr auto Name = "Formation-UI";
    UiFormation(const UiSystemConf &config);
    ~UiFormation();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiCamping : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Camping-UI";
    UiCamping(const UiSystemConf &config);
    ~UiCamping();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiHelpInventory : public UiSystem {
public:
    static constexpr auto Name = "Help-Inventory-UI";
    UiHelpInventory(const UiSystemConf &config);
    ~UiHelpInventory();
    void Reset() override;
    const std::string &GetName() const override;
};

class UiPartyQuickview : public UiSystem {
public:
    static constexpr auto Name = "Party-Quickview-UI";
    const std::string &GetName() const override;
};

class UiOptions : public UiSystem {
public:
    static constexpr auto Name = "Options-UI";
    UiOptions(const UiSystemConf &config);
    ~UiOptions();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;

	void Show(bool fromMainMenu);
};

enum class InGameHotKey : uint32_t {
	/*
		Toggles the corresponding party member selection
	*/
	TogglePartySelection1 = 2,
	TogglePartySelection2 = 3,
	TogglePartySelection3 = 4,
	TogglePartySelection4 = 5,
	TogglePartySelection5 = 6,
	TogglePartySelection6 = 7,
	TogglePartySelection7 = 8,
	TogglePartySelection8 = 9,
	AssignGroup1 = 10,
	AssignGroup2 = 11,
	AssignGroup3 = 12,
	AssignGroup4 = 13,
	AssignGroup5 = 14,
	AssignGroup6 = 15,
	AssignGroup7 = 16,
	AssignGroup8 = 17,
	RecallGroup1 = 18,
	RecallGroup2 = 19,
	RecallGroup3 = 20,
	RecallGroup4 = 21,
	RecallGroup5 = 22,
	RecallGroup6 = 23,
	RecallGroup7 = 24,
	RecallGroup8 = 25,
	SelectAll = 26,
	ToggleConsole = 27,
	CenterOnChar = 28,
	SelectChar1 = 29,
	SelectChar2 = 30,
	SelectChar3 = 31,
	SelectChar4 = 32,
	SelectChar5 = 33,
	SelectChar6 = 34,
	SelectChar7 = 35,
	SelectChar8 = 36,
	ToggleMainMenu = 37,
	QuickLoad = 38,
	QuickSave = 39,
	Quit = 40,
	Screenshot = 41, // Handled elsewhere, but swallowed
	ScrollUp = 42, // Handled elsewhere
	ScrollDown = 43, // Handled elsewhere
	ScrollLeft = 44, // Handled elsewhere
	ScrollRight = 45, // Handled elsewhere
	ScrollUpArrow = 46, // Handled elsewhere
	ScrollDownArrow = 47, // Handled elsewhere
	ScrollLeftArrow = 48, // Handled elsewhere
	ScrollRightArrow = 49, // Handled elsewhere
	ObjectHighlight = 50, // Handled elsewhere (but swallowed)
	ShowInventory = 51,
	ShowLogbook = 52,
	ShowMap = 53,
	ShowFormation = 54,
	Rest = 55,
	ShowHelp = 56,
	ShowOptions = 57,
	ToggleCombat = 58,
	EndTurn = 59,
	EndTurnNonParty = 60,
	None = 62
};

#pragma pack(push, 1)
struct InGameKeyEvent {
	const TigMsg &msg;
	InGameHotKey eventName = InGameHotKey::None;
	uint32_t field8;
	uint32_t field10;
	uint32_t field14;

	explicit InGameKeyEvent(const TigMsg &msg) : msg(msg) {}
};
#pragma pack(pop)

class UiKeyManager : public UiSystem {
public:
    static constexpr auto Name = "UI-Manager";
	UiKeyManager(const UiSystemConf &config);
    ~UiKeyManager();
    void Reset() override;
    const std::string &GetName() const override;

	// Was @ 101431D0 UiManStateSet
	void SetState(uint32_t state) {
		mState = state;
	}

	bool HandleKeyEvent(const InGameKeyEvent &msg);

private:
	uint32_t &mState = temple::GetRef<uint32_t>(0x10BE8CF4);
};

class UiHelpManager : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Help Manager-UI";
    UiHelpManager(const UiSystemConf &config);
    ~UiHelpManager();
    void Reset() override;
    bool SaveGame(TioFile *file) override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    const std::string &GetName() const override;
};

class UiSlider : public UiSystem {
public:
    static constexpr auto Name = "Slider-UI";
    UiSlider(const UiSystemConf &config);
    ~UiSlider();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiWritten : public UiSystem {
public:
    static constexpr auto Name = "written_ui";
    UiWritten(const UiSystemConf &config);
    ~UiWritten();
    const std::string &GetName() const override;

	bool Show(objHndl handle);

};

class UiCharmap : public UiSystem {
public:
    static constexpr auto Name = "charmap_ui";
    UiCharmap(const UiSystemConf &config);
    ~UiCharmap();
    const std::string &GetName() const override;
};
