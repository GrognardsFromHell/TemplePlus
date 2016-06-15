
#pragma once

#include <string>
#include <cstdint>

#include "ui_system.h"

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

class UiMM : public UiSystem {
public:
    static constexpr auto Name = "MM-UI";
    UiMM(const UiSystemConf &config);
    ~UiMM();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiLoadGame : public UiSystem {
public:
    static constexpr auto Name = "LoadGame";
    UiLoadGame(const UiSystemConf &config);
    ~UiLoadGame();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiSaveGame : public UiSystem {
public:
    static constexpr auto Name = "SaveGame";
    UiSaveGame(const UiSystemConf &config);
    ~UiSaveGame();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiInGame : public UiSystem, public SaveGameAwareUiSystem {
public:
    static constexpr auto Name = "Intgame";
    UiInGame(const UiSystemConf &config);
    ~UiInGame();
    void Reset() override;
    bool LoadGame(const UiSaveFile &saveGame) override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
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

class UiTurnBased : public UiSystem {
public:
    static constexpr auto Name = "TurnBased";
    UiTurnBased(const UiSystemConf &config);
    ~UiTurnBased();
    void Reset() override;
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
};

class UiCombat : public UiSystem {
public:
    static constexpr auto Name = "Combat-UI";
    UiCombat(const UiSystemConf &config);
    ~UiCombat();
    void Reset() override;
    const std::string &GetName() const override;
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
};

class UiPcCreation : public UiSystem {
public:
    static constexpr auto Name = "pc_creation";
    UiPcCreation(const UiSystemConf &config);
    ~UiPcCreation();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiChar : public UiSystem {
public:
    static constexpr auto Name = "Char-UI";
    UiChar(const UiSystemConf &config);
    ~UiChar();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
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

class UiItemCreation : public UiSystem {
public:
    static constexpr auto Name = "ItemCreation-UI";
    UiItemCreation(const UiSystemConf &config);
    ~UiItemCreation();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
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
};

class UiPccPortrait : public UiSystem {
public:
    static constexpr auto Name = "pcc_portrait";
    UiPccPortrait(const UiSystemConf &config);
    ~UiPccPortrait();
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
};

class UiParty : public UiSystem {
public:
    static constexpr auto Name = "Party-UI";
    UiParty(const UiSystemConf &config);
    ~UiParty();
    void Reset() override;
    void ResizeViewport(const UiResizeArgs &resizeArgs) override;
    const std::string &GetName() const override;
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

class UiManager : public UiSystem {
public:
    static constexpr auto Name = "UI-Manager";
    UiManager(const UiSystemConf &config);
    ~UiManager();
    void Reset() override;
    const std::string &GetName() const override;
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
};

class UiCharmap : public UiSystem {
public:
    static constexpr auto Name = "charmap_ui";
    UiCharmap(const UiSystemConf &config);
    ~UiCharmap();
    const std::string &GetName() const override;
};
