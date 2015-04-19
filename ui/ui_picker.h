
#pragma
#include "../obj.h"
#include "../objlist.h"

enum class UiPickerType : uint32_t {
	None = 0,
	Single,
	Multi,
	Cone,
	Area,
	Location,
	Personal,
	InventoryItem,
	Ray
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
	locXY location;
	float offsetz;
	int fieldbc;
};

typedef void (__cdecl *PickerCallback)(const PickerResult &result, void *callbackData);

struct PickerArgs {
	int flags;
	int field4;
	UiPickerType type;
	int fieldc;
	int field10;
	int field14;
	int field18;
	int field1c;
	int field20;
	int field24;
	int field28;
	int field2c;
	int field30;
	int spellId;
	objHndl caster;
	PickerCallback callback;
	int field44;
	PickerResult result;
	int field108;
	int field10c;
};

class UiPicker {
public:
	void ShowPicker(const PickerArgs &args, void *callbackArgs);

	void FreeCurrentPicker();
};

extern UiPicker uiPicker;
