
#include "stdafx.h"

#include <infrastructure/exception.h>
#include <temple/dll.h>
#include "ui_legacysystems.h"


//*****************************************************************************
//* MainMenu-UI
//*****************************************************************************

UiMainMenu::UiMainMenu(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10112810);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system MainMenu-UI");
    }
}
UiMainMenu::~UiMainMenu() {
    auto shutdown = temple::GetPointer<void()>(0x101128e0);
    shutdown();
}
const std::string &UiMainMenu::GetName() const {
    static std::string name("MainMenu-UI");
    return name;
}

//*****************************************************************************
//* MM-UI
//*****************************************************************************

UiMM::UiMM(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10117370);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system MM-UI");
    }
}
UiMM::~UiMM() {
    auto shutdown = temple::GetPointer<void()>(0x101164c0);
    shutdown();
}
void UiMM::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101172c0);
    resize(&resizeArg);
}
const std::string &UiMM::GetName() const {
    static std::string name("MM-UI");
    return name;
}

//*****************************************************************************
//* LoadGame
//*****************************************************************************

UiLoadGame::UiLoadGame(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10177ed0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system LoadGame");
    }
}
UiLoadGame::~UiLoadGame() {
    auto shutdown = temple::GetPointer<void()>(0x101772b0);
    shutdown();
}
void UiLoadGame::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10178130);
    resize(&resizeArg);
}
const std::string &UiLoadGame::GetName() const {
    static std::string name("LoadGame");
    return name;
}

//*****************************************************************************
//* SaveGame
//*****************************************************************************

UiSaveGame::UiSaveGame(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101767f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system SaveGame");
    }
}
UiSaveGame::~UiSaveGame() {
    auto shutdown = temple::GetPointer<void()>(0x10175950);
    shutdown();
}
void UiSaveGame::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10176ae0);
    resize(&resizeArg);
}
const std::string &UiSaveGame::GetName() const {
    static std::string name("SaveGame");
    return name;
}

//*****************************************************************************
//* Intgame
//*****************************************************************************

UiInGame::UiInGame(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10112e70);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Intgame");
    }
}
UiInGame::~UiInGame() {
    auto shutdown = temple::GetPointer<void()>(0x10112eb0);
    shutdown();
}
void UiInGame::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10112ec0);
    resize(&resizeArg);
}
void UiInGame::Reset() {
    auto reset = temple::GetPointer<void()>(0x101140b0);
    reset();
}
bool UiInGame::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101140c0);
        return load(&save) == 1;
}
const std::string &UiInGame::GetName() const {
    static std::string name("Intgame");
    return name;
}

//*****************************************************************************
//* IntgameSelect
//*****************************************************************************

UiInGameSelect::UiInGameSelect(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10138a40);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system IntgameSelect");
    }
}
UiInGameSelect::~UiInGameSelect() {
    auto shutdown = temple::GetPointer<void()>(0x10137560);
    shutdown();
}
void UiInGameSelect::Reset() {
    auto reset = temple::GetPointer<void()>(0x10137640);
    reset();
}
const std::string &UiInGameSelect::GetName() const {
    static std::string name("IntgameSelect");
    return name;
}

//*****************************************************************************
//* RadialMenu
//*****************************************************************************

UiRadialMenu::UiRadialMenu(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1013d230);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system RadialMenu");
    }
}
UiRadialMenu::~UiRadialMenu() {
    auto shutdown = temple::GetPointer<void()>(0x1013c1f0);
    shutdown();
}
void UiRadialMenu::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10139d60);
    resize(&resizeArg);
}
const std::string &UiRadialMenu::GetName() const {
    static std::string name("RadialMenu");
    return name;
}

//*****************************************************************************
//* TurnBased
//*****************************************************************************

UiTurnBased::UiTurnBased(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10174d70);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system TurnBased");
    }
}
UiTurnBased::~UiTurnBased() {
    auto shutdown = temple::GetPointer<void()>(0x10173ab0);
    shutdown();
}
void UiTurnBased::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10173a50);
    resize(&resizeArg);
}
void UiTurnBased::Reset() {
    auto reset = temple::GetPointer<void()>(0x10173ac0);
    reset();
}
const std::string &UiTurnBased::GetName() const {
    static std::string name("TurnBased");
    return name;
}

//*****************************************************************************
//* Anim-UI
//*****************************************************************************

UiAnim::UiAnim(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101739f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Anim-UI");
    }
}
void UiAnim::Reset() {
    auto reset = temple::GetPointer<void()>(0x10173780);
    reset();
}
bool UiAnim::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101737b0);
        return load(&save) == 1;
}
const std::string &UiAnim::GetName() const {
    static std::string name("Anim-UI");
    return name;
}

//*****************************************************************************
//* TB-UI
//*****************************************************************************

UiTB::UiTB(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014e1f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system TB-UI");
    }
}
UiTB::~UiTB() {
    auto shutdown = temple::GetPointer<void()>(0x1014de70);
    shutdown();
}
const std::string &UiTB::GetName() const {
    static std::string name("TB-UI");
    return name;
}

//*****************************************************************************
//* WMap-Rnd
//*****************************************************************************

void UiWMapRnd::LoadModule() {
    auto loadModule = temple::GetPointer<int()>(0x10121300);
    if (!loadModule()) {
        throw TempleException("Unable to load module data for game system WMap-Rnd");
    }
}
void UiWMapRnd::UnloadModule() {
    auto unloadModule = temple::GetPointer<void()>(0x10121180);
    unloadModule();
}
bool UiWMapRnd::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x10120bf0);
        return save(file) == 1;
}
bool UiWMapRnd::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x10120d40);
        return load(&save) == 1;
}
const std::string &UiWMapRnd::GetName() const {
    static std::string name("WMap-Rnd");
    return name;
}

//*****************************************************************************
//* Combat-UI
//*****************************************************************************

UiCombat::UiCombat(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10173690);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Combat-UI");
    }
}
UiCombat::~UiCombat() {
    auto shutdown = temple::GetPointer<void()>(0x10172df0);
    shutdown();
}
void UiCombat::Reset() {
    auto reset = temple::GetPointer<void()>(0x10172e70);
    reset();
}
const std::string &UiCombat::GetName() const {
    static std::string name("Combat-UI");
    return name;
}

//*****************************************************************************
//* Slide-UI
//*****************************************************************************

void UiSlide::LoadModule() {
    auto loadModule = temple::GetPointer<int()>(0x10138bb0);
    if (!loadModule()) {
        throw TempleException("Unable to load module data for game system Slide-UI");
    }
}
void UiSlide::UnloadModule() {
    auto unloadModule = temple::GetPointer<void()>(0x10138bd0);
    unloadModule();
}
const std::string &UiSlide::GetName() const {
    static std::string name("Slide-UI");
    return name;
}

//*****************************************************************************
//* Dlg-UI
//*****************************************************************************

UiDlg::UiDlg(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014dd40);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Dlg-UI");
    }
}
UiDlg::~UiDlg() {
    auto shutdown = temple::GetPointer<void()>(0x1014ccc0);
    shutdown();
}
void UiDlg::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1014de30);
    resize(&resizeArg);
}
void UiDlg::Reset() {
    auto reset = temple::GetPointer<void()>(0x1014ccf0);
    reset();
}
bool UiDlg::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x1014c830);
        return save(file) == 1;
}
bool UiDlg::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1014cd50);
        return load(&save) == 1;
}
const std::string &UiDlg::GetName() const {
    static std::string name("Dlg-UI");
    return name;
}

//*****************************************************************************
//* pc_creation
//*****************************************************************************

UiPcCreation::UiPcCreation(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10120420);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system pc_creation");
    }
}
UiPcCreation::~UiPcCreation() {
    auto shutdown = temple::GetPointer<void()>(0x1011ebc0);
    shutdown();
}
void UiPcCreation::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10120b30);
    resize(&resizeArg);
}
const std::string &UiPcCreation::GetName() const {
    static std::string name("pc_creation");
    return name;
}

//*****************************************************************************
//* Char-UI
//*****************************************************************************

UiChar::UiChar(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014b900);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Char-UI");
    }
}
UiChar::~UiChar() {
    auto shutdown = temple::GetPointer<void()>(0x10149820);
    shutdown();
}
void UiChar::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1014ba20);
    resize(&resizeArg);
}
void UiChar::Reset() {
    auto reset = temple::GetPointer<void()>(0x10143f80);
    reset();
}
const std::string &UiChar::GetName() const {
    static std::string name("Char-UI");
    return name;
}

//*****************************************************************************
//* ToolTip-UI
//*****************************************************************************

UiToolTip::UiToolTip(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10124380);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system ToolTip-UI");
    }
}
UiToolTip::~UiToolTip() {
    auto shutdown = temple::GetPointer<void()>(0x10122d00);
    shutdown();
}
const std::string &UiToolTip::GetName() const {
    static std::string name("ToolTip-UI");
    return name;
}

//*****************************************************************************
//* Logbook-UI
//*****************************************************************************

UiLogbook::UiLogbook(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101281a0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Logbook-UI");
    }
}
UiLogbook::~UiLogbook() {
    auto shutdown = temple::GetPointer<void()>(0x10128270);
    shutdown();
}
void UiLogbook::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10125ce0);
    resize(&resizeArg);
}
void UiLogbook::Reset() {
    auto reset = temple::GetPointer<void()>(0x10125dc0);
    reset();
}
bool UiLogbook::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x10125de0);
        return save(file) == 1;
}
bool UiLogbook::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x10125e40);
        return load(&save) == 1;
}
const std::string &UiLogbook::GetName() const {
    static std::string name("Logbook-UI");
    return name;
}

//*****************************************************************************
//* Scrollpane-UI
//*****************************************************************************

UiScrollpane::UiScrollpane(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10172330);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Scrollpane-UI");
    }
}
UiScrollpane::~UiScrollpane() {
    auto shutdown = temple::GetPointer<void()>(0x10171ea0);
    shutdown();
}
const std::string &UiScrollpane::GetName() const {
    static std::string name("Scrollpane-UI");
    return name;
}

//*****************************************************************************
//* Townmap-UI
//*****************************************************************************

UiTownmap::UiTownmap(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1012e1c0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Townmap-UI");
    }
}
UiTownmap::~UiTownmap() {
    auto shutdown = temple::GetPointer<void()>(0x1012bbe0);
    shutdown();
}
void UiTownmap::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10128450);
    resize(&resizeArg);
}
void UiTownmap::Reset() {
    auto reset = temple::GetPointer<void()>(0x1012bb40);
    reset();
}
bool UiTownmap::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x10128650);
        return save(file) == 1;
}
bool UiTownmap::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101288f0);
        return load(&save) == 1;
}
const std::string &UiTownmap::GetName() const {
    static std::string name("Townmap-UI");
    return name;
}

//*****************************************************************************
//* Popup-UI
//*****************************************************************************

UiPopup::UiPopup(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10171df0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Popup-UI");
    }
}
UiPopup::~UiPopup() {
    auto shutdown = temple::GetPointer<void()>(0x10171510);
    shutdown();
}
void UiPopup::Reset() {
    auto reset = temple::GetPointer<void()>(0x10171e70);
    reset();
}
const std::string &UiPopup::GetName() const {
    static std::string name("Popup-UI");
    return name;
}

//*****************************************************************************
//* TextDialog-UI
//*****************************************************************************

UiTextDialog::UiTextDialog(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1014eec0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system TextDialog-UI");
    }
}
UiTextDialog::~UiTextDialog() {
    auto shutdown = temple::GetPointer<void()>(0x1014e640);
    shutdown();
}
const std::string &UiTextDialog::GetName() const {
    static std::string name("TextDialog-UI");
    return name;
}

//*****************************************************************************
//* FocusManager-UI
//*****************************************************************************

UiFocusManager::UiFocusManager(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101709a0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system FocusManager-UI");
    }
}
const std::string &UiFocusManager::GetName() const {
    static std::string name("FocusManager-UI");
    return name;
}

//*****************************************************************************
//* Worldmap-UI
//*****************************************************************************

UiWorldmap::UiWorldmap(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10160470);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Worldmap-UI");
    }
}
UiWorldmap::~UiWorldmap() {
    auto shutdown = temple::GetPointer<void()>(0x1015e060);
    shutdown();
}
void UiWorldmap::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101599c0);
    resize(&resizeArg);
}
void UiWorldmap::Reset() {
    auto reset = temple::GetPointer<void()>(0x101597b0);
    reset();
}
bool UiWorldmap::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x101598b0);
        return save(file) == 1;
}
bool UiWorldmap::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1015e0f0);
        return load(&save) == 1;
}
const std::string &UiWorldmap::GetName() const {
    static std::string name("Worldmap-UI");
    return name;
}

//*****************************************************************************
//* RandomEncounter-UI
//*****************************************************************************

UiRandomEncounter::UiRandomEncounter(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101708f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system RandomEncounter-UI");
    }
}
UiRandomEncounter::~UiRandomEncounter() {
    auto shutdown = temple::GetPointer<void()>(0x1016fcc0);
    shutdown();
}
void UiRandomEncounter::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1016cec0);
    resize(&resizeArg);
}
const std::string &UiRandomEncounter::GetName() const {
    static std::string name("RandomEncounter-UI");
    return name;
}

//*****************************************************************************
//* Help-UI
//*****************************************************************************

UiHelp::UiHelp(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101317f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Help-UI");
    }
}
UiHelp::~UiHelp() {
    auto shutdown = temple::GetPointer<void()>(0x10130f30);
    shutdown();
}
void UiHelp::Reset() {
    auto reset = temple::GetPointer<void()>(0x10130f00);
    reset();
}
const std::string &UiHelp::GetName() const {
    static std::string name("Help-UI");
    return name;
}

//*****************************************************************************
//* ItemCreation-UI
//*****************************************************************************

UiItemCreation::UiItemCreation(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10154ba0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system ItemCreation-UI");
    }
}
UiItemCreation::~UiItemCreation() {
    auto shutdown = temple::GetPointer<void()>(0x10150eb0);
    shutdown();
}
void UiItemCreation::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10154e90);
    resize(&resizeArg);
}
void UiItemCreation::Reset() {
    auto reset = temple::GetPointer<void()>(0x1014f170);
    reset();
}
const std::string &UiItemCreation::GetName() const {
    static std::string name("ItemCreation-UI");
    return name;
}

//*****************************************************************************
//* SkillMastery-UI
//*****************************************************************************

UiSkillMastery::UiSkillMastery(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1016ce20);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system SkillMastery-UI");
    }
}
UiSkillMastery::~UiSkillMastery() {
    auto shutdown = temple::GetPointer<void()>(0x1016c270);
    shutdown();
}
void UiSkillMastery::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x1016a110);
    resize(&resizeArg);
}
const std::string &UiSkillMastery::GetName() const {
    static std::string name("SkillMastery-UI");
    return name;
}

//*****************************************************************************
//* UtilityBar-UI
//*****************************************************************************

UiUtilityBar::UiUtilityBar(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10110fe0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system UtilityBar-UI");
    }
}
UiUtilityBar::~UiUtilityBar() {
    auto shutdown = temple::GetPointer<void()>(0x10110520);
    shutdown();
}
void UiUtilityBar::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10111050);
    resize(&resizeArg);
}
void UiUtilityBar::Reset() {
    auto reset = temple::GetPointer<void()>(0x1010ee70);
    reset();
}
const std::string &UiUtilityBar::GetName() const {
    static std::string name("UtilityBar-UI");
    return name;
}

//*****************************************************************************
//* Track-UI
//*****************************************************************************

UiTrack::UiTrack(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10169d80);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Track-UI");
    }
}
UiTrack::~UiTrack() {
    auto shutdown = temple::GetPointer<void()>(0x10169e10);
    shutdown();
}
void UiTrack::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101679e0);
    resize(&resizeArg);
}
const std::string &UiTrack::GetName() const {
    static std::string name("Track-UI");
    return name;
}

//*****************************************************************************
//* party_pool
//*****************************************************************************

UiPartyPool::UiPartyPool(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101675f0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system party_pool");
    }
}
UiPartyPool::~UiPartyPool() {
    auto shutdown = temple::GetPointer<void()>(0x10165c80);
    shutdown();
}
void UiPartyPool::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101679a0);
    resize(&resizeArg);
}
void UiPartyPool::Reset() {
    auto reset = temple::GetPointer<void()>(0x10165cd0);
    reset();
}
bool UiPartyPool::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x10165d10);
        return save(file) == 1;
}
bool UiPartyPool::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x10165da0);
        return load(&save) == 1;
}
const std::string &UiPartyPool::GetName() const {
    static std::string name("party_pool");
    return name;
}

//*****************************************************************************
//* pcc_portrait
//*****************************************************************************

UiPccPortrait::UiPccPortrait(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10163660);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system pcc_portrait");
    }
}
UiPccPortrait::~UiPccPortrait() {
    auto shutdown = temple::GetPointer<void()>(0x10163410);
    shutdown();
}
void UiPccPortrait::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101636e0);
    resize(&resizeArg);
}
const std::string &UiPccPortrait::GetName() const {
    static std::string name("pcc_portrait");
    return name;
}

//*****************************************************************************
//* Party-UI
//*****************************************************************************

UiParty::UiParty(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10134bf0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Party-UI");
    }
}
UiParty::~UiParty() {
    auto shutdown = temple::GetPointer<void()>(0x10132760);
    shutdown();
}
void UiParty::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101350b0);
    resize(&resizeArg);
}
void UiParty::Reset() {
    auto reset = temple::GetPointer<void()>(0x101350a0);
    reset();
}
const std::string &UiParty::GetName() const {
    static std::string name("Party-UI");
    return name;
}

//*****************************************************************************
//* Formation-UI
//*****************************************************************************

UiFormation::UiFormation(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10125b90);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Formation-UI");
    }
}
UiFormation::~UiFormation() {
    auto shutdown = temple::GetPointer<void()>(0x10124d70);
    shutdown();
}
void UiFormation::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10125cb0);
    resize(&resizeArg);
}
const std::string &UiFormation::GetName() const {
    static std::string name("Formation-UI");
    return name;
}

//*****************************************************************************
//* Camping-UI
//*****************************************************************************

UiCamping::UiCamping(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1012fb60);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Camping-UI");
    }
}
UiCamping::~UiCamping() {
    auto shutdown = temple::GetPointer<void()>(0x1012edd0);
    shutdown();
}
void UiCamping::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101302e0);
    resize(&resizeArg);
}
void UiCamping::Reset() {
    auto reset = temple::GetPointer<void()>(0x1012e310);
    reset();
}
bool UiCamping::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x1012e330);
        return save(file) == 1;
}
bool UiCamping::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1012e3b0);
        return load(&save) == 1;
}
const std::string &UiCamping::GetName() const {
    static std::string name("Camping-UI");
    return name;
}

//*****************************************************************************
//* Help-Inventory-UI
//*****************************************************************************

UiHelpInventory::UiHelpInventory(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10162f60);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Help-Inventory-UI");
    }
}
UiHelpInventory::~UiHelpInventory() {
    auto shutdown = temple::GetPointer<void()>(0x10162750);
    shutdown();
}
void UiHelpInventory::Reset() {
    auto reset = temple::GetPointer<void()>(0x10162730);
    reset();
}
const std::string &UiHelpInventory::GetName() const {
    static std::string name("Help-Inventory-UI");
    return name;
}

//*****************************************************************************
//* Party-Quickview-UI
//*****************************************************************************

const std::string &UiPartyQuickview::GetName() const {
    static std::string name("Party-Quickview-UI");
    return name;
}

//*****************************************************************************
//* Options-UI
//*****************************************************************************

UiOptions::UiOptions(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x1011b640);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Options-UI");
    }
}
UiOptions::~UiOptions() {
    auto shutdown = temple::GetPointer<void()>(0x1011b0e0);
    shutdown();
}
void UiOptions::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10117540);
    resize(&resizeArg);
}
const std::string &UiOptions::GetName() const {
    static std::string name("Options-UI");
    return name;
}

//*****************************************************************************
//* UI-Manager
//*****************************************************************************

UiManager::UiManager(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10143bd0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system UI-Manager");
    }
}
UiManager::~UiManager() {
    auto shutdown = temple::GetPointer<void()>(0x101431a0);
    shutdown();
}
void UiManager::Reset() {
    auto reset = temple::GetPointer<void()>(0x10143190);
    reset();
}
const std::string &UiManager::GetName() const {
    static std::string name("UI-Manager");
    return name;
}

//*****************************************************************************
//* Help Manager-UI
//*****************************************************************************

UiHelpManager::UiHelpManager(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10124840);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Help Manager-UI");
    }
}
UiHelpManager::~UiHelpManager() {
    auto shutdown = temple::GetPointer<void()>(0x10124870);
    shutdown();
}
void UiHelpManager::Reset() {
    auto reset = temple::GetPointer<void()>(0x10124870);
    reset();
}
bool UiHelpManager::SaveGame(TioFile *file) {
        auto save = temple::GetPointer<int(TioFile*)>(0x10124880);
        return save(file) == 1;
}
bool UiHelpManager::LoadGame(const UiSaveFile &save) {
        auto load = temple::GetPointer<int(const UiSaveFile*)>(0x101248b0);
        return load(&save) == 1;
}
const std::string &UiHelpManager::GetName() const {
    static std::string name("Help Manager-UI");
    return name;
}

//*****************************************************************************
//* Slider-UI
//*****************************************************************************

UiSlider::UiSlider(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x101626d0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system Slider-UI");
    }
}
UiSlider::~UiSlider() {
    auto shutdown = temple::GetPointer<void()>(0x10161840);
    shutdown();
}
void UiSlider::ResizeViewport(const UiResizeArgs& resizeArg) {
    auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x10160fa0);
    resize(&resizeArg);
}
const std::string &UiSlider::GetName() const {
    static std::string name("Slider-UI");
    return name;
}

//*****************************************************************************
//* written_ui
//*****************************************************************************

UiWritten::UiWritten(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10160ec0);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system written_ui");
    }
}
UiWritten::~UiWritten() {
    auto shutdown = temple::GetPointer<void()>(0x10160ee0);
    shutdown();
}
const std::string &UiWritten::GetName() const {
    static std::string name("written_ui");
    return name;
}

//*****************************************************************************
//* charmap_ui
//*****************************************************************************

UiCharmap::UiCharmap(const UiSystemConf &config) {
    auto startup = temple::GetPointer<int(const UiSystemConf*)>(0x10160d70);
    if (!startup(&config)) {
        throw TempleException("Unable to initialize game system charmap_ui");
    }
}
UiCharmap::~UiCharmap() {
    auto shutdown = temple::GetPointer<void()>(0x10160a50);
    shutdown();
}
const std::string &UiCharmap::GetName() const {
    static std::string name("charmap_ui");
    return name;
}
