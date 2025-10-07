#pragma once
#include "../obj.h"
#include "../objlist.h"
#include "raycast.h"


struct TigMsg;
#define MAX_PICKER_COUNT 32

enum UiPickerIncFlags : uint64_t
{
	UIPI_None = 0,
	UIPI_Self = 0x1,
	UIPI_Other = 0x2,
	UIPI_NonCritter = 0x4,
	UIPI_Dead = 0x8,
	UIPI_Undead = 0x10,
	UIPI_Unconscious = 0x20,
	UIPI_Hostile = 0x40,
	UIPI_Friendly = 0x80,
	UIPI_Potion = 0x100,
	UIPI_Scroll = 0x200

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
	PRF_HAS_LOCATION = 0x4,
	PRF_UNK8 = 0x8,
	PRF_CANCELLED = 0x10, // User pressed escape or RMB
	PRF_HAS_SELECTED_OBJECT = 0x20,
};

enum PickerStatusFlags : int32_t
{
	PSF_OutOfRange = 0x1,
	PSF_Invalid = 0x2
};

constexpr PickerStatusFlags operator|(PickerStatusFlags l, PickerStatusFlags r)
{
	return static_cast<PickerStatusFlags>(static_cast<int32_t>(l) | static_cast<int32_t>(r));
}

inline PickerStatusFlags & operator |=(PickerStatusFlags & l, PickerStatusFlags r)
{
	l = l | r;
	return l;
}

enum WallPickerState : int32_t {
	// wall targeting consists of two stages: start point stage and end point stage
	WallPicker_StartPoint = 0,
	WallPicker_EndPoint = 1,

	// circle targeting
	WallPicker_CenterPoint = 10,
	WallPicker_Radius = 11
};



const auto TestSizeOfPickerResult = sizeof(PickerResult);

typedef void(__cdecl *PickerCallback)(const PickerResult &result, void *callbackData);

struct PickerArgs {
	friend class PickerCacheEntry;
	UiPickerFlagsTarget flagsTarget;
	UiPickerType modeTarget;
	UiPickerIncFlags incFlags;
	UiPickerIncFlags excFlags;
	int minTargets;
	int maxTargets;
	int radiusTarget;
	int range; // in feet
	float degreesTarget;
	int spellEnum;
	objHndl caster;
	PickerCallback callback;
	int field44;
	PickerResult result;
	float trimmedRangeInches; // after processing for collision with walls
	int field10c;

	PickerArgs() { memset(this, 0, sizeof(PickerArgs)); };
	bool IsBaseModeTarget(UiPickerType type);
	bool IsModeTargetFlagSet(UiPickerType type);
	void SetModeTargetFlag(UiPickerType type);
	UiPickerType GetBaseModeTarget();

	void GetTrimmedRange(LocAndOffsets &originLoc, LocAndOffsets &tgtLoc, float radiusInch, float maxRangeInch, float incrementInches = 0);
	void GetTargetsInPath(LocAndOffsets &originLoc, LocAndOffsets &tgtLoc, float radiusInch); // must have valid trimmedRangeInches value; must also free preexisting ObjectList!

	void DoExclusions();
	bool CheckTargetVsIncFlags(objHndl tgt);
	bool TargetValid( objHndl objHndl); // check exclusions from flags, and range
	bool LosBlocked(objHndl objHndl);
	bool SetSingleTgt(objHndl tgt);
	bool ContainsHandle(objHndl tgt);
	void FreeObjlist();

protected:
	void ExcludeTargets();
	void FilterResults();
};

const auto TestSizeOfPickerArgs = sizeof(PickerArgs); // should be 272 (0x110)

class PickerCacheEntry
{
public:
	PickerArgs args;
	void *callbackArgs;
	int field114;
	int cursorStackCount_Maybe;
	int x;
	int y;
	int field124;
	objHndl tgt;
	int tgtIdx;
	int field134;

	BOOL Finalize();
	GameRaycastFlags GetFlagsFromExclusions();
};

const auto TestSizeOfPickerCacheEntry = sizeof(PickerCacheEntry); // should be 312 (0x138)

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

	PickerMsgHandlers();
};

struct PickerSpec{
	int idx;
	const char *name;
	PickerMsgHandlers *msg;
	void(__cdecl *cursorTextDraw)(int x, int y, void *data);
	void(__cdecl *init)(); // gets called on ShowPicker

	PickerSpec();
	static const PickerSpec null;
};

class UiPicker {
friend class UiPickerHooks;
public:




	BOOL PickerActiveCheck(); // is there an active picker?
	
	int ShowPicker(const PickerArgs &args, void *callbackArgs);
	void CancelPicker();
	BOOL FreeCurrentPicker();
	PickerCacheEntry &GetPicker(int pickerIdx);
	PickerCacheEntry &GetActivePicker();
	int &GetActivePickerIdx();



	uint32_t ObjectNodesFromPickerResult(objHndl objHnd, PickerArgs * pickerArgs);

	
	uint32_t SetSingleTarget(objHndl objHndl, PickerArgs* pickerArgs);
	void SetConeTargets(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	uint32_t GetListRange(LocAndOffsets* locAndOffsets, PickerArgs* pickerArgs);
	static bool PickerLosBlocked(objHndl, const LocAndOffsets& loc);

	// Wall mode
	void WallStateReset() { mWallState = WallPicker_StartPoint; }
	
	WallPickerState GetWallState() { return  mWallState ; };
	LocAndOffsets GetWallEndPoint();

	UiPicker();
protected:
	PickerStatusFlags& GetPickerStatusFlags();
	
	void RenderPickers(); // renders the targeting circles, area of effect overlay and "spell pointers" (the arrow pointing from the caster to AoE and vice versa)
	BOOL PickerMsgHandler(TigMsg *msg);
	BOOL PickerMsgMouse(TigMsg *msg); // Mouse message handler
	BOOL PickerMsgKeyboard(TigMsg *msg); // KB message handler
	const PickerSpec &GetPickerSpec(UiPickerType modeTarget) ;

	BOOL MultiPosChange(TigMsg*msg);

	// Wall mode
	// walls are defined by a start point and an end point
	void InitWallSpec();
	PickerSpec mWallSpec;
	PickerMsgHandlers mWallMsgHandlers;
	BOOL WallPosChange(TigMsg*msg);
	BOOL WallLmbReleased(TigMsg *msg);
	BOOL WallRmbReleased(TigMsg *msg);
	void WallCursorText(int x, int y);
	WallPickerState mWallState = WallPicker_StartPoint;
	LocAndOffsets mWallEndPt = LocAndOffsets::null;

	

	/*
		Draws the rotating spiked circle for a valid target
		Is colored depending on whether tgt is friendly to the caster
	*/
	void DrawCircleValidTarget(objHndl tgt, objHndl caster, int spellEnum);
	void DrawCircleInvalidTarget(objHndl tgt, objHndl caster, int spellEnum);

	/*
		Draws a rectangular Spell AoE between originLoc and endLoc, with specified width caps.
		I think spellEnum doesn't actually do anything (might allow for custom cursors)
	*/
	void DrawRectangleAoE(LocAndOffsets originLoc, LocAndOffsets endLoc, float width, float minLength, float maxLength, int spellEnum);

	/*
	
	*/
	void DrawConeAoE(LocAndOffsets originLoc, LocAndOffsets tgtLoc, float angularWidthDegrees, int spellEnum);

	/*
		Draws circular Spell AoE.
		unkFactor doesn't seem to do anything?
	*/
	void DrawCircleAoE(LocAndOffsets originLoc, float unkFactor, float radiusInch, int spellEnum);

	/*
		Draws Player Spell Pointer (an arrow pointing from the caster to tgtLoc)
	*/
	void DrawPlayerSpellPointer(objHndl caster, LocAndOffsets tgtLoc);

	/*
		Draws Spell Effect pointer (an arrow shaped thing pointing from an AoE circle to caster)
	*/
	void DrawSpellEffectPointer(LocAndOffsets spellAoECenter, LocAndOffsets pointedToLoc, float aoeRadiusInch); 
};

extern UiPicker uiPicker;
