#pragma once

#include "ui_system.h"
#include "widgets/widgets.h"

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

struct UiSystemConf;
class UiCharImpl;

class UiChar : public UiSystem {
public:
	static constexpr auto Name = "Char-UI";
	UiChar(const UiSystemConf &config);
	~UiChar();
	void Reset() override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	void TextboxSetText(const char* txt);

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

	objHndl GetTooltipItem() const {
		return mTooltipItem;
	}

	// Was @ 10144030 (ui_char_has_current_critter)
	bool IsVisible() const {
		return !!mCurrentCritter;
	}

	// Was 0x101441B0
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
	The critter or chest being looted or bartered with
	*/
	objHndl GetLootedObject() const {
		return mLootedObj;
	}
	void SetLootedObject(objHndl & handle) const {
		mLootedObj = handle;
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
	std::vector<objHndl> GetNearbyLootableCritters(const objHndl& handle);
	void ShowLooting();

	void ShowForCritter(UiCharDisplayType type, objHndl handle);
	objHndl GetCritter();
	void SetCritter(objHndl handle);

	void Hide() {
		Show(UiCharDisplayType::Hidden);
	}
	void HideLooting();

	void LootingWidgetsInit();

private:
	std::unique_ptr<UiCharImpl> mImpl;
	objHndl & mInventoryObj = temple::GetRef<objHndl>(0x10BEECC0);
	objHndl &mLootedObj = temple::GetRef<objHndl>(0x10BE6EC0);
	BOOL &mInventoryObjState = temple::GetRef<BOOL>(0x10EF97C4);
	objHndl &mCurrentCritter = temple::GetRef<objHndl>(0x10BE9940);
	UiCharDisplayType &mDisplayType = temple::GetRef<UiCharDisplayType>(0x10BE994C);
	BOOL &mLooting = temple::GetRef<BOOL>(0x10BE6EE8);
	objHndl &mTooltipItem = temple::GetRef<objHndl>(0x10BF07B8);

	int mNextLooteeBtnId = -1;
	std::unique_ptr<WidgetContainer> mLootingWnd;
	std::vector<objHndl> mCrittersLootedList; // list of adjacent critters you can loot
	int mCrittersLootedIdx = 0;
};
