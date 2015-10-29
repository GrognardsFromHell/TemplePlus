#include "stdafx.h"
#include "obj.h"
#include <temple/dll.h>
#include "d20.h"
#include "common.h"
#include "temple_functions.h"
#include "condition.h"
#include "obj_fieldnames.h"
#include "location.h"
#include "pathfinding.h"
#include "float_line.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "util/fixes.h"

#include "python/python_integration_obj.h"
#include <set>

Objects objects;

static_assert(temple::validate_size<CondNode, 44>::value, "Condition node structure has incorrect size.");

struct ObjectSystemAddresses : temple::AddressTable
{
	uint32_t(__cdecl *GetProtoNum)(objHndl);

	uint32_t(__cdecl *GetArrayFieldNumItems)(objHndl obj, obj_f fieldIdx);
	void(__cdecl *ClearArrayField)(objHndl, obj_f);
	void(__cdecl * PropCollectionRemoveField)(objHndl objHnd, obj_f fieldIdx);
	void(__cdecl* SetFieldObjHnd)(objHndl obj, obj_f field, objHndl value);
	ObjectSystemAddresses()
	{
		rebase(GetProtoNum, 0x10039320);

		rebase(GetArrayFieldNumItems, 0x1009E7E0);
		rebase(ClearArrayField, 0x1009E860);
		rebase(PropCollectionRemoveField, 0x1009E9C0);

		rebase(SetFieldObjHnd, 0x100A0280);
		/*
		
	rebase(Obj_Set_Field_32bit, 0x100A0190);
	rebase(Obj_Set_Field_64bit, 0x100A0200);
	rebase(Obj_Set_IdxField_byValue, 0x100A1310);
	rebase(Obj_Set_IdxField_byPtr, 0x100A1540);
	rebase(Obj_Set_IdxField_ObjHnd, 0x100A14A0);
		*/
	}
} addresses;

#pragma region Object Internals
static_assert(temple::validate_size<ObjectId, 24>::value, "Object ID structure has incorrect size.");



// Root hashtable for all objects
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

static struct ObjInternal : temple::AddressTable {
	ObjFieldDef** fieldDefs;

	ObjInternal() {
		rebase(fieldDefs, 0x10B3D7D8);
	}
} objInternal;
#pragma endregion

#pragma region Objects implementation

uint32_t Objects::getInt32(objHndl obj, obj_f fieldIdx)
{
	if (!obj) {
		logger->warn("Called getInt32 on a null handle!");
		return 0;
	}

	auto objBody = _GetMemoryAddress(obj);
	Expects(!!objBody);
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

uint64_t Objects::getInt64(objHndl obj, obj_f fieldIdx)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	uint64_t dataOut[4] = { 0, 0, 0, 0 }; // so the game doesn't crash when the naive modder tries to read a non-32bit field at least ;)
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "getInt64");
		return 0;
	}
	if (fieldIdx == obj_f_type){ return objType; }
	PropFetcher(objBody, fieldIdx, dataOut);
	return dataOut[0];
}

void Objects::SetFieldObjHnd(objHndl obj, obj_f field, objHndl value)
{
	addresses.SetFieldObjHnd(obj, field, value);
}

objHndl Objects::getObjHnd(objHndl obj, obj_f fieldIdx)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;	
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "getObjHnd");
		return 0;
	}
	if (fieldIdx == obj_f_prototype_handle)
	{
		if (!objBody->protoHandle)
		{
			ObjectId objIdtemp;
			memcpy(&objIdtemp, &objBody->protoId, sizeof(ObjectId));
			objBody->protoHandle = GetHandle(objIdtemp);
		}
		return objBody->protoHandle;
	} 

	ObjectId dataOut;
	PropFetcher(objBody, fieldIdx, &dataOut);
	if (dataOut.IsHandle()) {
		return dataOut.GetHandle();
	} else {
		return 0;
	}
	
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

void Objects::setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, int data)
{
	FieldDataMax fdm;
	fdm.data[0] = data;
	setArrayFieldByValue(obj, fieldIdx, subIdx, fdm);
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

void Objects::getArrayField(objHndl obj, obj_f fieldIdx, uint32_t subIdx, void* dataOut)
{
	GameObjectBody * objBody = _GetMemoryAddress(obj);
	uint32_t objType = objBody->type;
	if (!DoesTypeSupportField(objBody->type, fieldIdx))
	{
		fieldNonexistantDebug(obj, objBody, fieldIdx, objType, "getArrayField");
		return ;
	}
	getArrayFieldInternal(objBody, dataOut, fieldIdx, subIdx);
}

uint32_t Objects::getArrayFieldNumItems(objHndl obj, obj_f fieldIdx)
{
	return addresses.GetArrayFieldNumItems(obj, fieldIdx);
}

void Objects::ClearArrayField(objHndl objHnd, obj_f objF)
{
	addresses.ClearArrayField(objHnd, objF);
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

int Objects::ObjectIdPrint(char* printOut, ObjectId objId)
{
	return _ObjectIdPrint(printOut, objId);
}

Objects::Objects()
{
	pathfinding = &pathfindingSys;
	loc = &locSys;
	floats = &floatSys;
	rebase(_GetDisplayName, 0x1001F970);


	rebase(_SetFlag,	0x10020F50);
	rebase(_ClearFlag,	0x10021020);
	rebase(_GetRadius,	0x10021C40);

	rebase(_Destroy,	0x100257A0);
	rebase(_Move,		0x10025950);
	rebase(_Create,		0x10028D20);
	
	rebase(_IsPlayerControlled, 0x1002B390);

	rebase(_SecretdoorDetect,	0x10046920);
	rebase(_FadeTo,				0x1004C390);
	rebase(_GetSize,			0x1004D690);

	rebase(_AdjustReaction,		0x10053F20);
	rebase(_GetReaction,		0x10054180);

	rebase(_AiForceSpreadOut,	0x1005A640);
	
	rebase(_StatLevelGet,			0x10074800);
	rebase(_StatLevelGetBase,		0x10074CF0);
	rebase(_StatLevelSetBase,		0x10074E10);

	rebase(_HasSpellEffects,		0x10076370);

	rebase(_IsObjDeadNullDestroyed, 0x1007E650);

	rebase(_GetId, 0x1009CA40);

	rebase(_DoesObjectFieldExist,	0x1009C190);
	rebase(_ObjectPropFetcher,		0x1009CD40);
	rebase(_getArrayFieldInternal, 0x1009CEB0);

	rebase(_GetInternalFieldInt32,		0x1009E1D0);
	rebase(_GetInternalFieldInt64,		0x1009E2E0);
	rebase(_GetInternalFieldFloat,		0x1009E260);
	rebase(_GetInternalFieldInt32Array, 0x1009E5C0);
	rebase(_InsetDataIntoInternalStack, 0x1009EA80);



	rebase(_SetInternalFieldInt32,	0x100A0190);
	rebase(_setArrayFieldLowLevel,	0x100A0500); 
	rebase(_SetInternalFieldFloat,	0x100A0190); // This is actually the same function as 32-bit heh


	rebase(_PortalToggleOpen,	0x100B4700);

	rebase(_TargetRandomTileNear, 0x100B99A0);
	rebase(_FindFreeSpot,		  0x100BDB50);

	rebase(_ObjectIdPrint,		0x100C2460);
	rebase(_GetMemoryAddress,	0x100C2A70);
	rebase(_VerifyHandle,		0x100C2D00);
	rebase(_GetHandle,			0x100C3050);
	rebase(_lookupInHandlesList,0x100C3050);

	rebase(_ContainerToggleOpen, 0x1010EA00);

	rebase(_DLLFieldNames,		0x102CD840);
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
	logger->info("{} Error: Accessing non-existant field [{}: {}] in object type [{}].",accessType, _DLLFieldNames[fieldIdx], fieldIdx, objType); // TODO: replace this with the corrected FieldNames. Fucking std::string, how does it work???. Not a big deal actually since the early fields exist for all objects.
	uint32_t protonum = 0;
	if (objBody->id.IsPrototype())
		protonum = objBody->id.GetPrototypeId();
	else if (objBody->id.IsPermanent())
		protonum = GetProtoNum(obj);
	logger->info("Sonavabitch proto is {}", protonum);
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

void Objects::SetRotation(objHndl handle, float rotation) {
	// Normalizes the rotation parameter to valid radians range
	static auto PI_2 = (float)(2.0 * M_PI);
	
	while (rotation >= PI_2) {
		rotation -= PI_2;
	}
	while (rotation < 0) {
		rotation += PI_2;
	}
	_SetInternalFieldFloat(handle, obj_f_rotation, rotation);
}

int Objects::GetScript(objHndl handle, int index) {
	ObjectScript scriptIdx;
	templeFuncs.Obj_Get_ArrayElem_Generic(handle, obj_f_scripts_idx, index, &scriptIdx);
	return scriptIdx.scriptId;
}

void Objects::SetScript(objHndl handle, int index, int scriptId) {
	ObjectScript scriptIdx;
	templeFuncs.Obj_Get_ArrayElem_Generic(handle, obj_f_scripts_idx, index, &scriptIdx);
	scriptIdx.scriptId = scriptId;
	templeFuncs.Obj_Set_IdxField_byPtr(handle, obj_f_scripts_idx, index, &scriptIdx);
}

ObjectScript Objects::GetScriptAttachment(objHndl handle, int index) {
	ObjectScript scriptIdx;
	templeFuncs.Obj_Get_ArrayElem_Generic(handle, obj_f_scripts_idx, index, &scriptIdx);
	return scriptIdx;
}

void Objects::SetScriptAttachment(objHndl handle, int index, const ObjectScript& script) {
	templeFuncs.Obj_Set_IdxField_byPtr(handle, obj_f_scripts_idx, index, const_cast<ObjectScript*>(&script));
}

Dice Objects::GetHitDice(objHndl handle) {
	auto count = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 0);
	auto sides = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 1);
	auto modifier = _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 2);
	return Dice(count, sides, modifier);
}

// Reimplements 100801D0
int Objects::GetHitDiceNum(objHndl handle) {
	int result = getArrayFieldNumItems(handle, obj_f_critter_level_idx);
	if (GetType(handle) == obj_t_npc) {
		result += _GetInternalFieldInt32Array(handle, obj_f_npc_hitdice_idx, 0);
	}
	return result;
}

int Objects::GetSize(objHndl handle) {
	// This function uses the dispatcher internally and should probably be rewritten
	//return _GetSize(handle);
	return dispatch.DispatchGetSizeCategory(handle);
}

objHndl Objects::Create(objHndl proto, locXY tile) {
	objHndl handle;
	if (_Create(proto, tile, &handle)) {
		return handle;
	} else {
		return 0;
	}
}

bool Objects::FindFreeSpot(LocAndOffsets location, float radius, LocAndOffsets& freeSpotOut) {
	return _FindFreeSpot(location, radius, freeSpotOut);
}

objHndl Objects::GetProtoHandle(int protoNumber) {
	return templeFuncs.GetProtoHandle(protoNumber);
}

bool Objects::AiForceSpreadOut(objHndl handle) {
	return _AiForceSpreadOut(handle, nullptr);
}

bool Objects::AiForceSpreadOut(objHndl handle, LocAndOffsets &location) {
	return _AiForceSpreadOut(handle, &location);
}

locXY Objects::TargetRandomTileNear(objHndl handle, int distance) {
	locXY result;
	_TargetRandomTileNear(handle, distance, &result);
	return result;
}

float Objects::GetRotationTowards(objHndl from, objHndl to) {
	auto locFrom = GetLocationFull(from);
	auto locTo = GetLocationFull(to);

	return AngleBetweenPoints(locFrom, locTo);
}

void Objects::FadeTo(objHndl obj, int targetOpacity, int fadeTimeInMs, int unk1, int unk2) {
	_FadeTo(obj, targetOpacity, fadeTimeInMs, unk1, unk2);
}

void Objects::Move(objHndl handle, LocAndOffsets toLocation) {
	_Move(handle, toLocation);
}

#include "combat.h"
#include "turn_based.h"

void Objects::Destroy(objHndl ObjHnd) {
	static set<objHndl> destroyed;
	std::string name = this->GetDisplayName(ObjHnd, ObjHnd);
	logger->info("Destroying {}", name);
	if (destroyed.find(ObjHnd) != destroyed.end()) {
		logger->error("Double destroying object {:x}", ObjHnd);
	}
	destroyed.insert(ObjHnd);

	auto flags = _GetInternalFieldInt32(ObjHnd, obj_f_flags);

	if (flags & OF_DESTROYED) {
		return; // Already destroyed
	}
	
	if (!pythonObjIntegration.ExecuteObjectScript(ObjHnd, ObjHnd, ObjScriptEvent::Destroy)) {
		return; // Scripts tells us to skip it
	}

	auto moveContentToLoc = temple::GetPointer<void(objHndl, BOOL)>(0x1006DB80);

	auto type = _GetInternalFieldInt32(ObjHnd, obj_f_type);
	if (type != obj_t_pc && type != obj_t_npc)
	{
		if (type >= obj_t_weapon && type <= obj_t_generic || type == obj_t_bag)
		{
			auto parentObj = inventory.GetParent(ObjHnd);
			if (parentObj)
			{
				auto loc = GetLocation(parentObj);
				inventory.ItemRemove(ObjHnd);
				auto moveObj = temple::GetPointer<void(objHndl, locXY)>(0x100252D0);
				moveObj(ObjHnd, loc);
			}
		}
		if (type == obj_t_container)
		{
			moveContentToLoc(ObjHnd, 1);
		}
	}
	else
	{
		auto removeFromGroups = temple::GetPointer<int(objHndl)>(0x10080DA0);
		removeFromGroups(ObjHnd);

		auto removeAiTimer = temple::GetPointer<int(objHndl)>(0x100588D0);
		removeAiTimer(ObjHnd);

		if (type == obj_t_npc)
		{
			auto getDlgTarget = temple::GetPointer<objHndl(objHndl)>(0x10053CA0);
			auto cancelDialog = temple::GetPointer<void(objHndl, int)>(0x1009A5D0);

			auto v3 = getDlgTarget(ObjHnd);
			if (v3)
				cancelDialog(v3, 0);
		}
		moveContentToLoc(ObjHnd, 1);
	}
	
	auto cancelAnims = temple::GetPointer<void(objHndl)>(0x1000C760);
	cancelAnims(ObjHnd);
	
	if (combatSys.isCombatActive())
	{
		if (tbSys.turnBasedGetCurrentActor() == ObjHnd) {
			templeFuncs.TurnProcessing(ObjHnd);
		}
	}
	combatSys.RemoveFromInitiative(ObjHnd);
	
	
	auto removeDispatcher = temple::GetPointer<int(objHndl)>(0x1004FEE0);
	removeDispatcher(ObjHnd);

	auto updateTbUi = temple::GetPointer<void(objHndl)>(0x1014DE90);
	updateTbUi(ObjHnd);
	
	auto killRendering = temple::GetPointer<void(objHndl)>(0x10021290);
	
	auto v6 = getInt32(ObjHnd, obj_f_animation_handle);
	if (v6) {
		auto freeAasModel = temple::GetPointer<void(int)>(0x10264510);
		freeAasModel(v6);
		setInt32(ObjHnd, obj_f_animation_handle, 0);
	}
	
	auto v7 = GetFlags(ObjHnd);
	setInt32(ObjHnd, obj_f_flags, v7 | OF_DESTROYED);
}

ObjectId Objects::GetId(objHndl handle) {
	ObjectId result;
	_GetId(&result, handle);
	return result;
}

objHndl Objects::GetHandle(const ObjectId &id) {
	return _GetHandle(id);
}

ObjectType Objects::GetType(objHndl obj)
{
	return static_cast<ObjectType>(getInt32(obj, obj_f_type));
}

uint32_t Objects::IsDeadNullDestroyed(objHndl obj)
{
	return _IsObjDeadNullDestroyed(obj);
}

uint32_t Objects::IsUnconscious(objHndl obj)
{
	if (obj == 0){ return 1; }
	if (GetFlags(obj) & OF_DESTROYED){ return 1; }
	if (GetHPCur(obj) <= -10){ return 1; }
	return d20.d20Query(obj, DK_QUE_Unconscious) != 0;
}

int32_t Objects::GetHPCur(objHndl obj)
{
	return _StatLevelGet(obj, stat_hp_current);
}

bool Objects::IsCritter(objHndl obj)
{
	auto type = GetType(obj);
	return type == obj_t_npc || type == obj_t_pc;
}

bool Objects::IsNPC(objHndl obj)
{
	auto type = GetType(obj);
	return type == obj_t_npc;
}

bool Objects::IsPlayerControlled(objHndl obj)
{
	return _IsPlayerControlled(obj);
}

uint32_t Objects::GetProtoNum(objHndl obj)
{
	return addresses.GetProtoNum(obj);
}

string Objects::GetDisplayName(objHndl obj, objHndl observer) {
	char name[512];
	_GetDisplayName(obj, observer, name);
	return name;
		}

bool Objects::IsStatic(objHndl handle) {

	auto type = GetType(handle);
	if (type == obj_t_projectile 
		|| type == obj_t_container 
		|| type == obj_t_pc 
		|| type == obj_t_npc 
		|| type >= obj_t_weapon && type <= obj_t_generic 
		|| type == obj_t_bag)
		return false;
	
	return (GetFlags(handle) & OF_DYNAMIC) == 0;

}

uint32_t Objects::StatLevelGet(objHndl obj, Stat stat)
{
	return _StatLevelGet(obj, stat);
		}

int Objects::StatLevelGetBase(objHndl obj, Stat stat)
{
	return _StatLevelGetBase(obj, stat);
}

int Objects::StatLevelSetBase(objHndl obj, Stat stat, int value)
{
	return _StatLevelSetBase(obj, stat, value);
}

void Objects::PropCollectionRemoveField(objHndl objHnd, obj_f objF)
{
	addresses.PropCollectionRemoveField(objHnd, objF);
}

int Objects::GetModFromStatLevel(int statLevel)
{
	return (statLevel - 10) / 2;
}

int Objects::GetTempId(objHndl handle) {
	return _GetInternalFieldInt32(handle, obj_f_temp_id);
}

bool Objects::IsContainer(objHndl objHnd)
{
	auto type = GetType(objHnd);
	return type == obj_t_container || type == obj_t_bag;
}
#pragma endregion


#pragma region Hooks

class ObjectReplacements : public TempleFix {
public:
	const char* name() override {
		return "Replacements for Object functions (mainly for debugging purposes now)";
	}

	static uint32_t _obj_get_int(objHndl obj, obj_f fieldIdx)
	{
		return objects.getInt32(obj, fieldIdx);
	}

	static void _obj_set_int(objHndl obj, obj_f fieldIdx, uint32_t dataIn)
	{
		objects.setInt32(obj, fieldIdx, dataIn);
	}

	static uint32_t _abilityScoreLevelGet(objHndl obj, Stat abScore, DispIO * dispIO)
	{
		return objects.abilityScoreLevelGet(obj, abScore, dispIO);
	}
	
	static void _setArrayFieldByValue(objHndl obj, obj_f fieldIdx, uint32_t subIdx, FieldDataMax data)
	{
		objects.setArrayFieldByValue(obj, fieldIdx, subIdx, data);
	}

	static int32_t _getArrayFieldInt32(objHndl obj, obj_f fieldIdx, uint32_t subIdx)
	{
		return objects.getArrayFieldInt32(obj, fieldIdx, subIdx);
	}

	static uint64_t _getInt64(objHndl obj, obj_f fieldIdx)
	{
		return objects.getInt64(obj, fieldIdx);
	}

	static objHndl _getObjHnd(objHndl obj, obj_f fieldIdx)
	{
		return objects.getObjHnd(obj, fieldIdx);
	}

	static void _destroy(objHndl obj) {
		objects.Destroy(obj);
	}

	void apply() override {
		replaceFunction(0x1009E1D0, _obj_get_int);
		replaceFunction(0x100A0190, _obj_set_int);
		replaceFunction(0x1004E7F0, _abilityScoreLevelGet);
		replaceFunction(0x100A1310, _setArrayFieldByValue);
		replaceFunction(0x1009E5C0, _getArrayFieldInt32);
		replaceFunction(0x1009E2E0, _getInt64);
		replaceFunction(0x1009E360, _getObjHnd);
		replaceFunction(0x100257A0, _destroy);
	}
} objReplacements;

#pragma endregion
