#include "stdafx.h"
#include "obj.h"
#include "util/addresses.h"
#include "d20.h"
#include "common.h"
#include "temple_functions.h"
#include "condition.h"
#include "obj_fieldnames.h"
#include "location.h"
#include "pathfinding.h"
#include "float_line.h"

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
		macReplaceFun(100A1310, _setArrayFieldByValue)
		macReplaceFun(1009E5C0, _getArrayFieldInt32)
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

uint32_t Objects::getInt32(objHndl obj, obj_f fieldIdx)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint32_t dataOut[8] = {0,0,0,0, 0,0,0,0}; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "getInt32");
		return 0;
	}
	if (fieldIdx == obj_f_type){ return objType; }
	PropFetcher(objBody, fieldIdx, dataOut);
	return dataOut[0];
}


void Objects::setInt32(objHndl obj, obj_f fieldIdx, uint32_t dataIn)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint32_t dataOut[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "setInt32");
		return ;
	}
	InsertDataIntoInternalStack(objBody, fieldIdx, &dataIn);
	return;
}

void Objects::setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, FieldDataMax data)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "setArrayFieldByValue");
		return;
	}
	setArrayFieldLowLevel(objBody, &data, fieldIdx, subIdx);
	return;
}

int32_t Objects::getArrayFieldInt32(objHndl obj, obj_f fieldIdx, uint32_t subIdx)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint32_t dataOut[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "getArrayFieldInt32");
		return 0;
	}
	getArrayFieldInternal(objBody, dataOut, fieldIdx, subIdx);
	return dataOut[0];
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

Objects::Objects()
{
	pathfinding = &pathfindingSys;
	loc = &locSys;
	floats = &floatSys;
	rebase(_GetInternalFieldInt32, 0x1009E1D0);
	rebase(_GetInternalFieldInt64, 0x1009E2E0);
	rebase(_StatLevelGet, 0x10074800);
	rebase(_SetInternalFieldInt32, 0x100A0190);
	macRebase(_setArrayFieldLowLevel, 100A0500)
	rebase(_IsPlayerControlled, 0x1002B390);
	rebase(_ObjGetProtoNum, 0x10039320);
	rebase(_IsObjDeadNullDestroyed, 0x1007E650);
	rebase(_GetMemoryAddress, 0x100C2A70);
	rebase(_DoesObjectFieldExist, 0x1009C190);
	rebase(_ObjectPropFetcher, 0x1009CD40);
	macRebase(_getArrayFieldInternal, 1009CEB0)
	rebase(_DLLFieldNames, 0x102CD840);
	rebase(_InsetDataIntoInternalStack, 0x1009EA80);
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

void Objects::setArrayFieldLowLevel(GameObjectBody* objBody, void* sourceData, obj_f fieldIdx, uint32_t subIdx)
{
	macAsmProl;
	__asm{
		mov ecx, this;
		mov esi, [ecx]._setArrayFieldLowLevel;
		mov eax, subIdx;
		push eax;
		mov ebx, fieldIdx;
		push ebx;
		mov ecx, objBody;
		mov eax, sourceData;
		call esi;
		add esp, 8;
	}
	macAsmEpil;
}

void Objects::fieldNonexistantDebug(unsigned long long obj, GameObjectBody* objBody, obj_f fieldIdx, unsigned objType, char* accessType)
{
	hooked_print_debug_message("%s Error: Accessing non-existant field [%s: %d] in object type [%d].",accessType, _DLLFieldNames[fieldIdx], fieldIdx, objType); // TODO: replace this with the corrected FieldNames. Fucking std::string, how does it work???. Not a big deal actually since the early fields exist for all objects.
	uint32_t subtype = objBody->id.subtype;
	uint32_t protonum = 0;
	if (subtype == 1)	protonum = objBody->id.guid.Data1;
	else if (subtype == 2)	protonum = ObjGetProtoNum(obj);
	hooked_print_debug_message("Sonavabitch proto is %d", protonum);
	if (protonum < 12624 || protonum > 12680)
	{
		uint32_t breakpointDummy = 0;
	}
}

void Objects::getArrayFieldInternal(GameObjectBody* objBody, void* outAddr, obj_f fieldIdx, uint32_t subIdx)
{
	macAsmProl;
	macAsmThis(getArrayFieldInternal)
	__asm{
		mov eax, fieldIdx;
		mov ecx, subIdx;
		mov ebx, outAddr;
		push ebx;
		mov edi, objBody;
		push edi;
		call esi;
		add esp, 8;
	}
	macAsmEpil
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

ObjectType Objects::GetType(objHndl obj)
{
	return static_cast<ObjectType>(_GetInternalFieldInt32(obj, obj_f_type));
}

uint32_t Objects::IsDeadNullDestroyed(objHndl obj)
{
	return _IsObjDeadNullDestroyed(obj);
}

uint32_t Objects::IsUnconscious(objHndl obj)
{
	if (obj == 0){ return 1; }
	if (GetFlags(obj) & OF_DESTROYED){ return 1; }
	if (GetHPCur(obj) < -10){ return 1; }
	return d20.d20Query(obj, DK_QUE_Unconscious) != 0;
}

int32_t Objects::GetHPCur(objHndl obj)
{
	return _StatLevelGet(obj, stat_hp_current);
}

uint32_t Objects::GetRace(objHndl obj)
{
	return _StatLevelGet(obj, stat_race);
}

bool Objects::IsCritter(objHndl obj)
{
	auto type = GetType(obj);
	return type == obj_t_npc || type == obj_t_pc;
}

bool Objects::IsPlayerControlled(objHndl obj)
{
	return _IsPlayerControlled(obj);
}

uint32_t Objects::ObjGetProtoNum(objHndl obj)
{
	return _ObjGetProtoNum(obj);
}

MonsterCategory Objects::GetCategory(objHndl objHnd)
{
	if (objHnd != 0) {
		if (IsCritter(objHnd)) {
			auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return (MonsterCategory)(monCat & 0xFFFFFFFF);
		}
	}
	return mc_type_monstrous_humanoid; // default - so they have at least a weapons proficiency
}

bool Objects::IsCategoryType(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (IsCritter(objHnd)) {
			auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return (monCat & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

bool Objects::IsCategorySubtype(objHndl objHnd, MonsterCategory categoryType)
{
	if (objHnd != 0) {
		if (IsCritter(objHnd)) {
			auto monCat = _GetInternalFieldInt64(objHnd, obj_f_critter_monster_category);
			return ((monCat >> 32) & 0xFFFFFFFF) == categoryType;
		}
	}
	return 0;
}

bool Objects::IsUndead(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_undead);
}

bool Objects::IsOoze(objHndl objHnd)
{
	return IsCategoryType(objHnd, mc_type_ooze);
}

bool Objects::IsSubtypeFire(objHndl objHnd)
{
	return IsCategorySubtype(objHnd, mc_subtye_fire);
}

uint32_t Objects::StatLevelGet(objHndl obj, Stat stat)
{
	return _StatLevelGet(obj, stat);
}
#pragma endregion


#pragma region Hooks

uint32_t _obj_get_int(objHndl obj, obj_f fieldIdx)
{
	return objects.getInt32(obj, fieldIdx);
}

void _obj_set_int(objHndl obj, obj_f fieldIdx, uint32_t dataIn)
{
	objects.setInt32(obj, fieldIdx, dataIn);
}

uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO)
{
	return objects.abilityScoreLevelGet(obj, abScore, dispIO);
}


void _setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, FieldDataMax data)
{
	objects.setArrayFieldByValue(obj, fieldIdx, subIdx, data);
}

int32_t _getArrayFieldInt32(objHndl obj, obj_f fieldIdx, uint32_t subIdx)
{
	return objects.getArrayFieldInt32(obj, fieldIdx, subIdx);
}
#pragma endregion
