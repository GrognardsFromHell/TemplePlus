#pragma once
#include "../obj.h"
#include "../objlist.h"


struct TigMsg;

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
	LosNotRequired = 0x100
};

enum PickerResultFlags : int32_t {
	PRF_HAS_SINGLE_OBJ = 0x1,
	PRF_HAS_MULTI_OBJ = 0x2,
	PRF_CANCELLED = 0x10 // User pressed escape or RMB
};

enum PickerStatusFlags : int32_t
{
	PSF_OutOfRange = 0x1,
	PSF_Invalid = 0x2
};

struct PickerResult {
	int flags; // see PickerResultFlags
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


	bool IsBaseModeTarget(UiPickerType type);
	bool IsModeTargetFlagSet(UiPickerType type);
};

const auto TestSizeOfPickerArgs = sizeof(PickerArgs);


struct PickerMsgHandlers
{
	int(__cdecl *lmbClick)(TigMsg *);
	int(__cdecl *lmbReleased)(TigMsg *);
	int(__cdecl *rmbClick)(TigMsg *);
	int(__cdecl *rmbReleased)(TigMsg *);
	int(__cdecl *mmbClick)(TigMsg *);
	int(__cdecl *mmbReleased)(TigMsg *);
	int(__cdecl *posChange)(TigMsg *);
	int(__cdecl *posChange2)(TigMsg *);
	int(__cdecl *scrollwheel)(TigMsg *);
	int(__cdecl *keystateChange)(TigMsg *);
	int(__cdecl *charFunc)(TigMsg *);
};

struct PickerSpec{
	int idx;
	const char *name;
	PickerMsgHandlers *msg;
	void(__cdecl *cursorTextDraw)(int x, int y, void *data);
	void(__cdecl *init)();
};

class UiPicker {
public:
	BOOL PickerActiveCheck();
	
	int ShowPicker(const PickerArgs &args, void *callbackArgs);
	uint32_t sub_100BA030(objHndl objHnd, PickerArgs * pickerArgs);

	void FreeCurrentPicker();
	uint32_t SetSingleTarget(objHndl objHndl, PickerArgs* pickerArgs);
	void SetConeTargets(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t GetListRange(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
};

extern UiPicker uiPicker;