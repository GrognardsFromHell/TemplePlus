#include "stdafx.h"
#include "obj.h"
#include "util/addresses.h"
#include "d20.h"
#include "common.h"
#include "temple_functions.h"
#include "condition.h"
#include "obj_fieldnames.h"

Objects objects;

static_assert(validate_size<CondNode, 36>::value, "Condition node structure has incorrect size.");


class ObjectReplacements : public TempleFix {
public:
	const char* name() override {
		return "Replacements for Object functions (mainly for debugging purposes now)";
	}

	void apply() override {
		replaceFunction(0x1009E1D0, _obj_get_int);
		replaceFunction(0x100A0190, _obj_set_int);
		replaceFunction(0x1004E7F0, _abilityScoreLevelGet);
	}
} objReplacements;


#pragma region Object Internals
const size_t objHeaderSize = 4; // Constant
const size_t objBodySize = 168; // Passed in to Object_Tables_Init
const size_t objSize = objHeaderSize + objBodySize;
static_assert(validate_size<GameObject, objSize>::value, "Object structure has incorrect size.");



// Root hashtable for all objects
struct ObjectMasterTableRow {
	GameObject objects[0x2000];
};

struct ObjectMasterTable {
	ObjectMasterTableRow* rows[256];
};

const uint32_t ObjFieldDefCount = 430;

struct ObjFieldDef {
	int protoPropIndex;
	int field4;
	int bitmapIndex1;
	uint32_t bitmapMask;
	int bitmapIndex2;
	uint32_t IsStoredInPropCollection;
	uint32_t FieldTypeCode;
};

static struct ObjInternal : AddressTable {
	ObjFieldDef** fieldDefs;

	ObjectMasterTable** masterTable;

	// Starts at 1, Max is 256
	int* masterTableSize;

	int *poolSize;

	ObjInternal() {
		rebase(fieldDefs, 0x10B3D7D8);
		rebase(masterTable, 0x10BCAC50);
		rebase(masterTableSize, 0x10BCAC4C);
		rebase(poolSize, 0x10BCAC30);
	}
} objInternal;
#pragma endregion

#pragma region Objects implementation

uint32_t Objects::GetInt32(objHndl obj, obj_f fieldIdx)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint32_t dataOut[8] = {0,0,0,0, 0,0,0,0}; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		hooked_print_debug_message("GetInt32 Error: Accessing non-existant field [%s: %d] in object type [%d].", _DLLFieldNames[fieldIdx] , fieldIdx, objType); // TODO: replace this with the corrected FieldNames. Fucking std::string, how does it work???
		uint32_t subtype = objBody->id.subtype;
		if (subtype == 1)
		{
			hooked_print_debug_message("Sonavabitch proto is %d", objBody->id.guid.Data1);
		}
		else if (subtype == 2){
			hooked_print_debug_message("Sonavabitch proto is %d", ObjGetProtoNum(obj));
		}
		return 0;
	}
	if (fieldIdx == obj_f_type){ return objType; }
	PropFetcher(objBody, fieldIdx, dataOut);
	return dataOut[0];
	
}


void Objects::SetInt32(objHndl obj, obj_f fieldIdx, uint32_t dataIn)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint32_t dataOut[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		hooked_print_debug_message("SetInt32 Error: Accessing non-existant field [%s: %d] in object type [%d].", _DLLFieldNames[fieldIdx], fieldIdx, objType); // TODO: replace this with the corrected FieldNames. Fucking std::string, how does it work???. Not a big deal actually since the early fields exist for all objects.
		uint32_t subtype = objBody->id.subtype;
		uint32_t protonum = 0;
		if (subtype == 1)
		{
			protonum = objBody->id.guid.Data1;
			
		}
		else if (subtype == 2){

			protonum = ObjGetProtoNum(obj);
		}
		hooked_print_debug_message("Sonavabitch proto is %d", protonum);
		if (protonum < 12624 || protonum > 12680)
		{
			uint32_t dummy = 0;
		}
		
		return ;
	}
	InsertDataIntoInternalStack(objBody, fieldIdx, &dataIn);
	return;
}

void Objects::InsertDataIntoInternalStack(GameObjectBody * objBody, obj_f fieldIdx, void * dataIn)
{
	__asm{
		push esi;
		push ecx;
		push edx;
		mov ecx, this;
		mov esi, [ecx]._InsetDataIntoInternalStack;
		mov eax, dataIn;
		push eax;
		mov eax, fieldIdx;
		push eax;
		mov eax, objBody;
		call esi;
		add esp, 8;
		pop edx;
		pop ecx;
		pop esi;
	}
}

void Objects::PropFetcher(GameObjectBody* objBody, obj_f fieldIdx, void * dataOut) {
	__asm {
		push esi;
		push ecx;
		push edx;
		mov ecx, this;
		mov esi, [ecx]._ObjectPropFetcher;
		mov edx, objBody;
		mov ecx, dataOut;
		mov eax, fieldIdx;
		call esi;
		pop edx;
		pop ecx;
		pop esi;
	}
}

uint32_t Objects::DoesTypeSupportField(uint32_t objType, _fieldIdx objField) {
	uint32_t result;
	assert(objField >= 0);
	__asm {
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, [ecx]._DoesObjectFieldExist;
		mov ecx, objType;
		mov eax, objField;
		call esi;
		pop ecx;
		pop esi;
		mov result, eax
	}
	return result != 0;
}

uint32_t Objects::abilityScoreLevelGet(objHndl objHnd, Stat stat, DispIO* dispIO)
{
	return objects.dispatch.dispatcherForCritters(objHnd, dispIO, dispTypeAbilityScoreLevel, stat + 1);
}


#pragma endregion


#pragma region Hooks

uint32_t _obj_get_int(objHndl obj, obj_f fieldIdx)
{
	return objects.GetInt32(obj, fieldIdx);
}

void _obj_set_int(objHndl obj, obj_f fieldIdx, uint32_t dataIn)
{
	objects.SetInt32(obj, fieldIdx, dataIn);
}

uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO)
{
	return objects.abilityScoreLevelGet(obj, abScore, dispIO);
}

#pragma endregion
