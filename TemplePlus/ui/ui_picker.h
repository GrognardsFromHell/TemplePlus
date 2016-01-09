#pragma once
#include "../obj.h"
#include "../objlist.h"

enum class UiPickerType : uint64_t {
	None = 0,
	Single,
	Multi,
	Cone,
	Area,
	Location,
	Personal,
	InventoryItem,
	Ray = 8,
	BecomeTouch = 0x100,
	AreaOrObj = 0x200,
	OnceMulti = 0x400,
	Any30Feet = 0x800,
	Primary30Feet = 0x1000,
	EndEarlyMulti = 0x2000,
	LocIsClear = 0x4000
};

enum class UiPickerIncFlags : uint64_t
{
	None = 0,
	Self = 0x1,
	Other = 0x2,
	NonCritter = 0x4,
	Dead = 0x8,
	Undead = 0x10,
	Unconscious = 0x20,
	Hostile = 0x40,
	Friendly = 0x80,
	Potion = 0x100,
	Scroll = 0x200
};

enum UiPickerFlagsTarget : uint64_t
{
	None = 0,
	Min = 0x1,
	Max = 0x2,
	Radius = 0x4,
	Range = 0x8,
	Exclude1st = 0x10,
	Degrees = 0x20,
	FixedRadius = 0x40,
	Unknown80h = 0x80, // these are not supported by the spell rules importer, but apparently used in at least one place (the py cast_spell function)
	Unknown100h = 0x100
};

enum PickerResultFlags {
	// User pressed escape
	PRF_CANCELLED = 0x10
};

struct PickerResult {
	int flags;
	int field4;
	objHndl handle;
	ObjListResult objList;
	LocAndOffsets location;
	float offsetz;
	int fieldbc;
};

const auto TestSizeOfPickerResult = sizeof(PickerResult);

typedef void(__cdecl *PickerCallback)(const PickerResult &result, void *callbackData);

struct PickerArgs {
	UiPickerFlagsTarget flagsTarget;
	UiPickerType modeTarget;
	UiPickerIncFlags incFlags;
	UiPickerIncFlags excFlags;
	int minTargets;
	int maxTargets;
	int radiusTarget;
	int range;
	float degreesTarget;
	int spellEnum;
	objHndl caster;
	PickerCallback callback;
	int field44;
	PickerResult result;
	int field108;
	int field10c;
};

const auto TestSizeOfPickerArgs = sizeof(PickerArgs);

class UiPicker {
public:
	BOOL PickerActiveCheck();
	
	int ShowPicker(const PickerArgs &args, void *callbackArgs);
	uint32_t sub_100BA030(objHndl objHnd, PickerArgs * pickerArgs);

	void FreeCurrentPicker();
	uint32_t sub_100BA480(objHndl objHndl, PickerArgs* pickerArgs);
	void sub_100BA6A0(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t sub_100BA540(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
};

extern UiPicker uiPicker;