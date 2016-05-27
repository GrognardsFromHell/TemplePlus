
#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "spell.h"
#include "obj.h"
#include "tig/tig_mes.h"
#include "temple_functions.h"
#include "ui\ui_picker.h"
#include "temple_functions.h"
#include "radialmenu.h"
#include "critter.h"
#include "gamesystems/objects/objsystem.h"
#include <infrastructure/elfhash.h>
#include "gamesystems/gamesystems.h"
#include "gamesystems/particlesystems.h"
#include <particles/instances.h>
#include "particles.h"
#include "python/python_integration_spells.h"
#include "util/streams.h"
#include "combat.h"
#include "sound.h"
#include "bonus.h"
#include "float_line.h"
#include "history.h"
#include "damage.h"

static_assert(sizeof(SpellStoreData) == (32U), "SpellStoreData structure has the wrong size!");

#define NORMAL_SPELL_RANGE 600
#define SPELL_LIKE_ABILITY_RANGE 699

struct SpellCondListEntry {
	CondStruct *condition;
	int unknown;
};


static struct SpellAddresses : temple::AddressTable {
	SpellCondListEntry *spellConds;


	void (__cdecl *UpdateSpellPacket)(const SpellPacketBody &spellPktBody);
	uint32_t(__cdecl*GetMaxSpellSlotLevel)(objHndl objHnd, Stat classCode, int casterLvl);


	uint32_t(__cdecl * ConfigSpellTargetting)(PickerArgs* pickerArgs, SpellPacketBody* spellPacketBody);
	int(__cdecl*ParseSpellSpecString)(SpellStoreData* spellStoreData, char* spellString);

	SpellAddresses() {
		rebase(UpdateSpellPacket, 0x10075730);
		rebase(GetMaxSpellSlotLevel, 0x100765B0);
		rebase(ParseSpellSpecString, 0x10078F20);

		rebase(ConfigSpellTargetting, 0x100B9690);

		rebase(spellConds, 0x102E2600);
		
	}

	
} addresses;

IdxTableWrapper<SpellEntry> spellEntryRegistry(0x10AAF428);
IdxTableWrapper<SpellPacket> spellsCastRegistry(0x10AAF218);

// Expand the range of usable spellEnums. Currently walled off at 802.
class SpellFuncReplacements : public TempleFix {
public:
	static void hookedPrint(const char * fmt, const char* spellName)
	{
		logger->debug("SpellLoad: Spell {}", spellName);
	};

	void apply() override {
		// writeHex(0x100779DE + 2, "A0 0F"); // this prevents the crash from casting from scroll, but it fucks up normal spell casting... (can't go to radial menu to cast!)
		replaceFunction(0x100FDEA0, _getWizSchool);
		replaceFunction(0x100779A0, _getSpellEnum);
		replaceFunction(0x100762D0, _spellKnownQueryGetData);
		replaceFunction(0x10076190, _spellMemorizedQueryGetData);
		replaceFunction(0x1007A140, _spellCanCast);
		replaceFunction(0x100754B0, _spellRegistryCopy);
		replaceFunction(0x10075660, _GetSpellEnumFromSpellId);
		replaceFunction(0x100756E0, _GetSpellPacketBody);
		replaceFunction(0x100F1010, _SetSpontaneousCastingAltNode);

		replaceFunction<void(__cdecl)()>(0x10079390, []() {
			spellSys.SpellSave();
		});

		replaceFunction<void(__cdecl)()>(0x100793F0, []() {
			spellSys.GetSpellsFromTransferInfo();
		});

		replaceFunction<int(__cdecl)(int, int)>(0x10079980, [](int id, int endDespiteTargetList){
			return spellSys.SpellEnd(id, endDespiteTargetList);
		});

		static bool(__cdecl*orgSpellCastSerializeToFile)(SpellPacket*, int, TioFile*) = replaceFunction<bool(__cdecl)(SpellPacket*, int, TioFile*)>(0x100781B0, [](SpellPacket* pkt, int key, TioFile* file){
			auto result = orgSpellCastSerializeToFile(pkt, key, file);
			if (result)
			{
				logger->debug("Serialized spell {} id {} to file.", spellSys.GetSpellName(pkt->spellPktBody.spellEnum), key);
			} else
			{
				logger->debug("Failed to serialize spell to file! Spell was {} id {}.", spellSys.GetSpellName(pkt->spellPktBody.spellEnum), key);
			}
			return result;
		});

		redirectCall(0x1007870F, hookedPrint);

		

		replaceFunction<const char*(__cdecl)(uint32_t)>(0x10077970, [](uint32_t spellEnum){
			return spellSys.GetSpellEnumTAG(spellEnum);
		});

		// SpellPacketSetCasterLevel
		replaceFunction<void(__cdecl)(SpellPacketBody*)>(0x10079B70, [](SpellPacketBody* spellPkt){
			spellSys.spellPacketSetCasterLevel(spellPkt);
		});
		
	}
} spellFuncReplacements;


SpontCastSpellLists spontCastSpellLists;

//temple::GlobalPrimitive<uint16_t>
//1028D09C

// Spell Hostility bug: fix mass cure spells triggering hostile reaction. Can be expanded to other spells.
class SpellHostilityFlagFix : public TempleFix {
public:
	void apply() override {
		writeHex(0x10076EF4, "02"); // Cure Light Wounds, Mass
		writeHex(0x10076F42, "02"); // Mass Heal
		writeHex(0x10077058, "02 02 02"); // Cure Moderate + Serious + Critical Wounds, Mass
	}
} spellHostilityFlagFix;


#pragma region Spell System Implementation

LegacySpellSystem spellSys;

//UiPickerType SpellEntry::GetModeTarget() const
//{
//	return (UiPickerType)(modeTargetSemiBitmask & 0xFF);
//}

SpellEntry::SpellEntry()
{
	memset(this, 0, sizeof(SpellEntry));
}

SpellEntry::SpellEntry(uint32_t spellEnumIn)
{
	spellSys.spellRegistryCopy(spellEnumIn, this);
}

bool SpellEntry::IsBaseModeTarget(UiPickerType type){
	auto _type = (uint64_t)type;
	return (modeTargetSemiBitmask & 0xFF) == _type;
}

SpellPacketBody::SpellPacketBody()
{
	spellSys.spellPacketBodyReset(this);
}

SpellPacketBody::SpellPacketBody(uint32_t spellId){
	spellSys.GetSpellPacketBody(spellId, this);
}

bool SpellPacketBody::UpdateSpellsCastRegistry() const
{
	if (!spellId)
		return true;

	SpellPacket pkt;
	if (spellsCastRegistry.copy(spellId, &pkt) && pkt.isActive){
		pkt.spellPktBody = *this;
		spellsCastRegistry.put(spellId, pkt);
		return true;
	}

	return false;
}

bool SpellPacketBody::FindObj(objHndl obj, int* idx) const
{
	for (auto i = 0u; i < targetCount; i++) {
		if (targetListHandles[i]== obj)	{
			*idx = i;
			return true;
		}
	}
	*idx = -1;
	return false;
}

bool SpellPacketBody::InsertToPartsysList(uint32_t idx, int partsysId)
{
	Expects(idx < 32 && idx >= 0);
	if (idx >= targetCount && idx < 32){
		targetListPartsysIds[idx] = partsysId;
		return true;
	}

	memcpy(&targetListPartsysIds[idx + 1], &targetListPartsysIds[idx], sizeof(int)*(targetCount - idx));
	targetListPartsysIds[idx] = partsysId;
	return true;
}

bool SpellPacketBody::InsertToTargetList(uint32_t idx, objHndl tgt){
	Expects(idx >= 0 && idx < 32 && idx <= targetCount);
	for (int i = targetCount; i > (int) idx; i--){
		targetListHandles[i] = targetListHandles[i-1];
	}
	targetListHandles[idx] = tgt;
	targetCount++;
	return true;
}

bool SpellPacketBody::AddTarget(objHndl tgt, int partsysId, int replaceExisting)
{
	int idx = -1;
	if (FindObj(tgt, &idx) || idx > -1)	{
		if (replaceExisting == 1){
			targetListPartsysIds[idx] = partsysId;
			return 0;
		} 
		
		logger->debug("SpellPacketAddTarget: Object {} already in list!", description.getDisplayName(tgt));
		return 0;	
	}

	InsertToPartsysList(targetCount, partsysId);
	if (InsertToTargetList(targetCount, tgt))
	{
		if (UpdateSpellsCastRegistry())	{
			pySpellIntegration.UpdateSpell(spellId);
			return true;
		}
		logger->warn("SpellPacketAddTarget: Unable to save SpellPacket!");
		return false;
	}

	logger->debug("SpellPacketAddTarget: Unable to add obj {} to target list!", description.getDisplayName(tgt));
	return false;
}

bool SpellPacketBody::SavingThrow(objHndl target, D20SavingThrowFlag flags) {
	SpellEntry spEntry(spellEnum);
	return damage.SavingThrowSpell(target, caster, dc, (SavingThrowType)spEntry.savingThrowType, flags, spellId );
}

const char* SpellPacketBody::GetName(){
	return spellSys.GetSpellName(spellEnum);
}

bool SpellPacketBody::IsVancian(){
	if (spellSys.isDomainSpell(spellClass))
		return true;
	
	if (d20ClassSys.isVancianCastingClass(spellSys.GetCastingClass(spellClass)))
		return true;

	return false;
}

void SpellPacketBody::Debit(){
	// preamble
	if (!caster){
		logger->warn("SpellPacketBody::Debit() Null caster!");
		return;
	}

	if (invIdx != INV_IDX_INVALID) // this is handled separately
		return;

	auto casterObj = gameSystems->GetObj().GetObject(caster);
	
	auto spellEnumDebited = spellEnumOriginal;

	// Spontaneous vs. Normal logging
	bool isSpont = (spellEnum != spellEnumOriginal) && spellEnumOriginal != 0;
	auto spellName = spellSys.GetSpellName(spellEnumOriginal);
	if (isSpont){
		logger->debug("Debiting Spontaneous casted spell. Original spell: {}", spellName);
	} else	{
		logger->debug("Debiting casted spell {}", spellName);
	}
	
	// Vancian spell handling - debit from the spells_memorized list
	if (IsVancian()){
		
		auto numMem = casterObj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize();
		auto spellFound = false;
		for (auto i = 0u; i < numMem; i++){
			auto spellMem = casterObj->GetSpell(obj_f_critter_spells_memorized_idx, i);
			spellMem.pad0 &= 0x7F; // clear out metamagic indictor
			
			if (!spellSys.isDomainSpell(spellMem.classCode)){
				if (spellMem.spellEnum != spellEnumDebited)
					continue;
			} 
			else if (spellMem.spellEnum != spellEnum){
				continue;
			}

			if (spellMem.spellLevel == spellKnownSlotLevel // todo: check if the spell level should be adjusted for MetaMagic
				&& spellMem.classCode == spellClass
				&& spellMem.spellStoreState.spellStoreType == SpellStoreType::spellStoreMemorized
				&& spellMem.spellStoreState.usedUp == 0
				&& spellMem.metaMagicData == metaMagicData)	{
				casterObj->SetSpell(obj_f_critter_spells_memorized_idx, i, spellMem);
				break;
			}
		}

		if (!spellFound){
			logger->warn("Spell debit: Spell not found!");
		}
		
	} 

	// add to casted list (so it shows up as used / gets counted up for spells per day)
	SpellStoreData sd(spellEnum, spellKnownSlotLevel, spellClass, metaMagicData);
	sd.spellStoreState.spellStoreType = SpellStoreType::spellStoreCast;
	casterObj->SetSpell(obj_f_critter_spells_cast_idx, casterObj->GetSpellArray(obj_f_critter_spells_cast_idx).GetSize(), sd);

}

void SpellPacketBody::MemorizedUseUp(SpellStoreData &spellData){
	if (!caster) {
		logger->warn("SpellPacketBody::Debit() Null caster!");
		return;
	}

	if (invIdx != INV_IDX_INVALID) // this is handled separately
		return;

	auto casterObj = gameSystems->GetObj().GetObject(caster);

	auto numMem = casterObj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize();
	if (!numMem)
		return;

	static auto spellComparer = [](SpellStoreData &sd1, SpellStoreData& sd2){
		return sd1.spellEnum == sd2.spellEnum
			&& sd1.spellLevel == sd2.spellLevel
			&& sd1.classCode == sd2.classCode
			&& sd1.spellStoreState.spellStoreType == sd2.spellStoreState.spellStoreType
			&& sd1.spellStoreState.usedUp == sd2.spellStoreState.usedUp
			&& sd1.metaMagicData == sd2.metaMagicData	;
	};

	for (auto i = 0u; i < numMem; i++){
		auto sd = casterObj->GetSpell(obj_f_critter_spells_memorized_idx, i);
		
		if (spellComparer(sd, spellData)) {
			sd.spellStoreState.usedUp = 1;	
			casterObj->SetSpell(obj_f_critter_spells_memorized_idx, i, sd);
			break;
		}
	}

}

void SpellPacketBody::Reset(){
	spellSys.spellPacketBodyReset(this);
}

SpellMapTransferInfo::SpellMapTransferInfo()
{
	//memset(this, 0, sizeof(SpellMapTransferInfo));
	spellId  = -1;

	// vanilla init:
	
	casterObjId.subtype = ObjectIdKind::Null;
	aoeObjId.subtype = ObjectIdKind::Null;
	for (int i = 0; i < 128; i++)
	{
		spellObjs[i].subtype = ObjectIdKind::Null;
	}
	for (int i = 0; i < 32; i++)
	{
		targets[i].subtype = ObjectIdKind::Null;
	}
	for (int i = 0; i < 5; i++)
	{
		projectiles[i].subtype = ObjectIdKind::Null;
	}

}

uint32_t LegacySpellSystem::spellRegistryCopy(uint32_t spellEnum, SpellEntry* spellEntry)
{
	return spellEntryRegistry.copy(spellEnum, spellEntry);
}

uint32_t LegacySpellSystem::ConfigSpellTargetting(PickerArgs* pickerArgs, SpellPacketBody* spellPktBody)
{
	return addresses.ConfigSpellTargetting(pickerArgs, spellPktBody);
}

uint32_t LegacySpellSystem::GetMaxSpellSlotLevel(objHndl objHnd, Stat classCode, int casterLvl)
{
	return addresses.GetMaxSpellSlotLevel(objHnd, classCode, casterLvl);
}

int LegacySpellSystem::ParseSpellSpecString(SpellStoreData* spell, char* spellString)
{
	return addresses.ParseSpellSpecString(spell, spellString);
}

const char* LegacySpellSystem::GetSpellMesline(uint32_t lineNumber) const
{
	MesLine mesLine;
	mesLine.key = lineNumber;
	mesFuncs.GetLine_Safe(*spellMes, &mesLine);
	return mesLine.value;
}

bool LegacySpellSystem::CheckAbilityScoreReqForSpell(objHndl handle, uint32_t spellEnum, int statBeingRaised) const
{
	return temple::GetRef<BOOL(__cdecl)(objHndl, uint32_t, int)>(0x10075C60)(handle, spellEnum, statBeingRaised) != 0;
}

const char* LegacySpellSystem::GetSpellEnumTAG(uint32_t spellEnum){

	MesLine mesline;
	mesline.key = spellEnum + 20000;

	if (mesFuncs.GetLine(spellSys.spellEnumsExt, &mesline)){
		return mesline.value;
	} 
	
	mesFuncs.GetLine_Safe(*spellSys.spellEnumMesHandle, &mesline);
	return mesline.value;
}

const char* LegacySpellSystem::GetSpellName(uint32_t spellEnum) const
{
	if (spellEnum > SPELL_ENUM_MAX || static_cast<int>(spellEnum) <=0){
		logger->warn("Spell Enum outside expected range: {}", spellEnum);
	}
	return GetSpellMesline(spellEnum);
}

void LegacySpellSystem::SetSpontaneousCastingAltNode(objHndl obj, int nodeIdx, SpellStoreData* spellData)
{
	auto spellClassCode = spellData->classCode;
	if (isDomainSpell(spellClassCode))
		return;
	auto castingClassCode = GetCastingClass(spellClassCode);
	if (castingClassCode == stat_level_cleric)
	{
		RadialMenuEntry radEntry;
		d20Sys.D20ActnSetSpellData(&radEntry.d20SpellData, spellData->spellEnum, spellData->classCode, spellData->spellLevel, 0xFF, spellData->metaMagicData);

		radEntry.type = RadialMenuEntryType::Action;
		radEntry.minArg = 0;
		radEntry.maxArg = 0;
		radEntry.actualArg = 0;
		radEntry.d20ActionType = D20A_CAST_SPELL;
		radEntry.d20ActionData1 = 0;
		auto alignmentChoice = objects.getInt32(obj, obj_f_critter_alignment_choice);
		auto spellLevel = spellData->spellLevel;
		if (alignmentChoice == 1) // good aligned
		{
			if (spellLevel <= 9)
				radEntry.text = (char*)GetSpellMesline(spontCastSpellLists.spontCastSpellsGoodCleric[spellLevel]);
			d20Sys.D20ActnSetSetSpontCast(&radEntry.d20SpellData, SpontCastType::spontCastGoodCleric);
		} else
		{
			if (spellLevel <= 9)
				radEntry.text = (char*)GetSpellMesline(spontCastSpellLists.spontCastSpellsEvilCleric[spellLevel]);
			d20Sys.D20ActnSetSetSpontCast(&radEntry.d20SpellData, SpontCastType::spontCastEvilCleric);
		}
		radEntry.helpId = ElfHash::Hash("TAG_CLASS_FEATURES_CLERIC_SPONTANEOUS_CASTING");
		
		radialMenus.SetMorphsTo(obj, nodeIdx, radialMenus.AddRootNode(obj, &radEntry));
	} 
	else if (castingClassCode == stat_level_druid && spellData->spellLevel > 0)
	{
		RadialMenuEntry radEntry;
		auto spellLevel = spellData->spellLevel;
		if (spellLevel > 9)
			spellLevel = 9;
		auto radOptionsMesLine = spontCastSpellLists.spontCastSpellsDruidSummons[spellLevel]; // mes line inside spells_radial_menu_options
		auto druidSpontSpell = spontCastSpellLists.spontCastSpellsDruid[spellLevel];
		radEntry.text = (char*)GetSpellMesline(druidSpontSpell);
		int parentIdx = radialMenus.AddRootParentNode(obj, &radEntry );
		radialMenus.SetMorphsTo(obj, nodeIdx, parentIdx);
		MesLine mesLine;
		mesLine.key = radOptionsMesLine;
		mesFuncs.GetLine_Safe(*spellsRadialMenuOptionsMes, &mesLine);
		auto numSummons = atol(mesLine.value);
		for (int i = 1; i <= numSummons; i++ )
		{
			mesLine.key = i + radOptionsMesLine;
			mesFuncs.GetLine_Safe(*spellsRadialMenuOptionsMes, &mesLine);
			auto protoNum = atol(mesLine.value);
			auto protoHandle = objects.GetProtoHandle(protoNum);
			mesLine.key = objects.getInt32(protoHandle, obj_f_description);
			mesFuncs.GetLine_Safe(*description.descriptionMes, &mesLine);
			
			radEntry.SetDefaults();
			radEntry.text = (char*)mesLine.value;
			radEntry.d20ActionType = D20A_CAST_SPELL;
			radEntry.d20ActionData1 = 0;
			radialMenus.SetCallbackCopyEntryToSelected(&radEntry);
			radEntry.minArg = protoNum;
			d20Sys.D20ActnSetSpellData(&radEntry.d20SpellData, spellData->spellEnum, spellData->classCode, spellData->spellLevel, 0xFF, spellData->metaMagicData);
			d20Sys.D20ActnSetSetSpontCast(&radEntry.d20SpellData, SpontCastType::spontCastDruid);
			radEntry.helpId = ElfHash::Hash(GetSpellEnumTAG(spellData->spellEnum));
			radialMenus.AddChildNode(obj, &radEntry, parentIdx);
		}
	}
}

uint32_t LegacySpellSystem::getBaseSpellCountByClassLvl(uint32_t classCode, uint32_t classLvl, uint32_t slotLvl, uint32_t unknown1)
{
	__asm{
		// ecx - classLvl
		// eax - slotLvl
		// edx - unknown?
		push esi;
		push ecx;
		mov ecx, this;
		mov esi, [ecx]._getSpellCountByClassLvl;
		mov eax, classCode;
		push eax;
		mov eax, slotLvl;
		mov ecx, classLvl;
		mov edx , unknown1
		call esi;
		add esp, 4;
		pop ecx;
		pop esi;
	}
}

uint32_t LegacySpellSystem::getWizSchool(objHndl objHnd)
{
	return ( objects.getInt32(objHnd, obj_f_critter_school_specialization) & 0x000000FF );
}

uint32_t LegacySpellSystem::getStatModBonusSpellCount(objHndl objHnd, uint32_t classCode, uint32_t slotLvl)
{
	uint32_t objHndLSB = objHnd.GetHandleLower();
	uint32_t objHndMSB = objHnd.GetHandleUpper();
	uint32_t result = 0;
	__asm{
		// esi - slotLvl
		// eax - classCode
		push edi;
		push edx;
		push esi;
		push ecx;
		mov ecx, this;
		mov edi, [ecx]._getStatModBonusSpellCount;
		mov eax, objHndMSB;
		mov edx, objHndLSB;
		push eax;
		push edx;
		mov eax, classCode;
		mov esi, slotLvl;
		call edi;
		add esp, 8;
		mov result, eax;
		pop edi;
		pop edx;
		pop esi;
		pop ecx;
	}
	return result;
}

void LegacySpellSystem::spellPacketBodyReset(SpellPacketBody* spellPktBody)
{
	_spellPacketBodyReset(spellPktBody);
}

void LegacySpellSystem::spellPacketSetCasterLevel(SpellPacketBody* spellPkt) const
{
	auto spellClassCode = spellPkt->spellClass;
	auto caster = spellPkt->caster;
	auto spellEnum = spellPkt->spellEnum;
	auto spellName = spellSys.GetSpellName(spellEnum);
	auto casterName = description.getDisplayName(caster);
	auto casterClass = spellSys.GetCastingClass(spellClassCode);

	auto casterObj = gameSystems->GetObj().GetObject(caster);

	// normal spells
	if (!spellSys.isDomainSpell(spellClassCode)){

		// casting class
		if (casterClass){
			auto casterLvl = objects.StatLevelGet(caster, casterClass);
			spellPkt->baseCasterLevel = casterLvl;
			if (d20ClassSys.IsLateCastingClass(casterClass))
				spellPkt->baseCasterLevel = casterLvl / 2;
			logger->info("Critter {} is casting spell {} at base caster_level {}.", casterName, spellName , casterLvl);
		} 

		// item spell
		else if (spellPkt->invIdx != 255 && (spellPkt->spellEnum < NORMAL_SPELL_RANGE || spellPkt->spellEnum > SPELL_LIKE_ABILITY_RANGE)){
			spellPkt->baseCasterLevel = 0;
			logger->info("Critter {} is casting item spell {} at base caster_level {}.", casterName, spellName, 0);
		}

		// monster
		else if (casterObj->IsCritter())
		{
		    if (objects.IsNPC(caster)) {
				spellPkt->baseCasterLevel = casterObj->GetInt32Array(obj_f_npc_hitdice_idx)[0];
			} else
			{
				spellPkt->baseCasterLevel = casterObj->GetInt32Array(obj_f_critter_level_idx).GetSize();
			}
			logger->info("Monster {} is casting spell {} at base caster_level {}.", casterName, spellName, spellPkt->baseCasterLevel);
		} else
		{
			spellPkt->baseCasterLevel = 0;
		}
	} 
	
	// domain spell
	else if ( objects.StatLevelGet(caster, stat_level_cleric) > 0){
		spellPkt->baseCasterLevel = objects.StatLevelGet(caster, stat_level_cleric);
		logger->info("Critter {} is casting Domain spell {} at base caster_level {}.", casterName, spellName, spellPkt->baseCasterLevel);
	
	// domain special (usually used for monsters)
	} else if (spellPkt->spellClass == Domain_Special)
	{
		if (spellPkt->invIdx != 255 && (spellPkt->spellEnum < NORMAL_SPELL_RANGE || spellPkt->spellEnum > SPELL_LIKE_ABILITY_RANGE)) {
			spellPkt->baseCasterLevel = 0;
			logger->info("Critter {} is casting item spell {} at base caster_level {}.", casterName, spellName, 0);
		} 
		
		else {
			spellPkt->baseCasterLevel = objects.GetHitDiceNum(caster);
			logger->info("Monster {} is casting spell {} at base caster_level {}.", casterName, spellName, spellPkt->baseCasterLevel);
		}
	}

	auto orgCasterLvl = spellPkt->baseCasterLevel;
	dispatch.Dispatch35BaseCasterLevelModify(caster, spellPkt);
	
	// if changed
	if (spellPkt->baseCasterLevel != orgCasterLvl){
		logger->info("Spell level modified to {}", spellPkt->baseCasterLevel);
	}
	//_spellPacketSetCasterLevel(spellPkt);
}

void LegacySpellSystem::SpellSavePruneInactive() const
{
	int numPruned = 0;
	struct SpellDebugInfo
	{
		uint32_t spellEnum; uint32_t targetCount;
		SpellDebugInfo(int spellEn, int tgtCount)
		{
			spellEnum = spellEn;
			targetCount = tgtCount;
		}
	};
	std::vector < SpellDebugInfo > prunedSpells; // for debug
	std::vector<SpellDebugInfo> preservedSpells;

	for (auto it = spellsCastRegistry.begin(); it != spellsCastRegistry.end(); )
	{
		auto& node = *it;

		bool shouldPrune = false;
		auto& spellPacketBody = node.data->spellPktBody;

		if (node.data->isActive == 0)	{
			shouldPrune = true;
		}  
		else if (!spellPacketBody.caster) {
			const char * spellName = GetSpellName(spellPacketBody.spellEnum);
			logger->warn("Spell id {} ({}) has been pruned because the spell has a null caster.", spellPacketBody.spellId, spellName);
			shouldPrune = true;
		} 
		else if (spellPacketBody.targetCount &&  !spellPacketBody.targetListHandles[0]) {
			const char * spellName = GetSpellName(spellPacketBody.spellEnum);
			logger->warn("Spell id {} ({}) has been pruned because the spell has num_targets > 0 but there are no targets!", spellPacketBody.spellId, spellName);
			shouldPrune = true;
		}
		
		if (shouldPrune) {
			numPruned++;
			prunedSpells.push_back(SpellDebugInfo( spellPacketBody.spellEnum, spellPacketBody.targetCount ));
			it = spellsCastRegistry.erase(it); // erase advances the iterator too so there shouldn't be one in the loop execution section
		} else{
			++it;
			preservedSpells.push_back(SpellDebugInfo( spellPacketBody.spellEnum, spellPacketBody.targetCount ));
		}
	}

	logger->info("Pruned {} spells from active list.", numPruned);
	for (auto it: prunedSpells){
		auto spellName = GetSpellName(it.spellEnum);
		logger->debug("{}, targetCount {}", spellName, it.targetCount);
	}
	logger->info("Preserved {} spells from active list.", preservedSpells.size());
	for (auto it : preservedSpells) {
		auto spellName = GetSpellName(it.spellEnum);
		logger->debug("{}, targetCount {}", spellName, it.targetCount);
	}
}

SpellMapTransferInfo LegacySpellSystem::SaveSpellForTeleport(const SpellPacket& data)
{
	SpellMapTransferInfo result;

	auto spellEnum = data.spellPktBody.spellEnum;
	Expects(spellEnum > 0 && spellEnum < 10000); // keeping a margin for now because co8 has messed with this a bit
	if (spellEnum > SPELL_ENUM_MAX)	{
		logger->warn("Spell enum beyond expected range encountered: {}", spellEnum);
	}
	
	auto spellPkt = &data.spellPktBody;
	auto spellName = GetSpellName(spellPkt->spellEnum);
	int id = data.key;
	
	auto getPartSysNameHash = [=](int handle) -> std::string {
		if (!handle) {
			return "";
		}
		auto sys = gameSystems->GetParticleSys().GetByHandle(handle);
		if (!sys) {
			logger->debug("PrepareActiveSpellForTeleport: \t Trying to get name hash for invalid particle system handle: {}. Spell is {} id {}", handle, spellName, id);
			return "";
		} else {
			logger->debug("PrepareActiveSpellForTeleport: \t Hashing name for partsys: {}. Spell is {} id {}", handle, spellName, id);
			return sys->GetSpec()->GetName();
		}
	};
	
	result.spellId = id;
	result.casterObjId = objects.GetId(spellPkt->caster);

	if (result.casterObjId.IsNull()) {
		logger->warn("PrepareActiveSpellForTeleport: Spell caster has invalid handle. Spell {} id {}", spellName, id);
	}

	auto partHandle = spellPkt->casterPartsysId;
	result.casterPartsys = getPartSysNameHash(partHandle);

	if (spellPkt->aoeObj) {
		result.aoeObjId = objects.GetId(spellPkt->aoeObj);
		if (result.aoeObjId.IsNull()) {
			logger->warn("PrepareActiveSpellForTeleport: AoeObj has invalid handle. Spell {} id {}", spellName, id);
		}
	} else {
		result.aoeObjId.subtype = ObjectIdKind::Null;
	}

	result.spellObjPartsys->clear();
	memset(result.spellObjs, 0, sizeof result.spellObjs);
	result.targetlistPartsys->clear();
	memset(result.targets, 0, sizeof result.targets);
	//memset(result.projectiles, 0, sizeof result.projectiles);

	
	for (auto i = 0; i < spellPkt->numSpellObjs;i++){
		if (spellPkt->spellObjs[i].obj) {
			result.spellObjs[i] = objects.GetId(spellPkt->spellObjs[i].obj);
		} else {
			logger->debug("PrepareActiveSpellForTeleport: Spell obj #{} has invalid handle. Spell {} id {}", i, spellName, id);
		}
		auto handle = spellPkt->spellObjs[i].partySysId;
		if (handle) {
			result.spellObjPartsys[i] = getPartSysNameHash(handle);
		}
	}
	
	for (uint32_t i = 0; i < spellPkt->targetCount; i++) {
		if (spellPkt->targetListHandles[i]) {
			result.targets[i] = objects.GetId(spellPkt->targetListHandles[i]);
		} else {
			logger->debug("PrepareActiveSpellForTeleport: Target obj #{} has invalid handle. Spell {} id {}", i, spellName, id);
		}

		auto handle = spellPkt->targetListPartsysIds[i];
		if (handle) {
			result.targetlistPartsys[i] = getPartSysNameHash(handle);
		}
	}

	for (uint32_t i = 0; i < spellPkt->projectileCount; i++) {
		if (spellPkt->projectiles[i]) {
			result.projectiles[i] = objects.GetId(spellPkt->projectiles[i]);
		} else {
			logger->debug("PrepareActiveSpellForTeleport: Projectile #{} has invalid handle. Spell {} id {}", i, spellName, id);
		}
	}
	
	return result;
}

void LegacySpellSystem::SpellSave()
{
	SpellSavePruneInactive();

	// Clean up previous transfer info
	spellMapTransInfo.clear();
	spellMapTransInfo.reserve(spellsCastRegistry.itemCount());
	for (auto it : spellsCastRegistry) {
		spellMapTransInfo.emplace_back(SaveSpellForTeleport(*it.data));
	}

}

int LegacySpellSystem::SpellSave(TioOutputStream& file)
{
	static auto SerializeHandleToFile = [](objHndl obj, TioOutputStream& stream)
	{
		ObjectId objId;
		if (obj)
		{
			auto objBod = objSystem->GetObject(obj);
			if (objBod)
				objId = objSystem->GetObject(obj)->id;
			else
			{
				objId.subtype = ObjectIdKind::Null;
				logger->warn("SpellSave: Invalid obj handle caught while saving!");
			}
		}
		else
		{
			objId.subtype = ObjectIdKind::Null;
		}
		stream.WriteObjectId(objId);
		return true;
	};
	static auto SerializePartsysToFile = [](int partsysId, TioOutputStream&stream)
	{
		int partsysHash = 0;
		if (partsysId){
			auto partSys = gameSystems->GetParticleSys().GetByHandle(partsysId);
			if (partSys)
				partsysHash = gameSystems->GetParticleSys().GetByHandle(partsysId)->GetSpec()->GetNameHash();
			else
			{
				logger->debug("Tried to serialize an invalid partsysId");
			}
		}
		stream.WriteInt32(partsysHash);
		return true;
	};
	static auto SerializeHandlesToFile = [](objHndl objs[], uint32_t num, TioOutputStream&streama)
	{
		for (auto i = 0u; i < num; i++)
		{
			if (!SerializeHandleToFile(objs[i], streama))
				return false;
		}
		return true;
	};
	static auto SerializePartSystemsToFile = [](uint32_t partsys[], uint32_t num, TioOutputStream&stream)
	{
		for (auto i = 0u; i < num; i++)
		{
			if (!SerializePartsysToFile(partsys[i], stream))
				return false;
		}
		return true;
	};
	static auto SerializeSpellObjsToFile = [](SpellPacketBody::SpellObj spellObjs[], uint32_t num, TioOutputStream&stream)
	{
		for (auto i = 0u; i < num; i++)
		{
			if (!SerializeHandleToFile(spellObjs[i].obj, stream))
				return false;
			if (!SerializePartsysToFile(spellObjs[i].partySysId, stream))
				return false;
		}
		return true;
	};

	int numSerialized = 0;

	for (auto it: spellsCastRegistry){
		
		if (!it.data->isActive)
		{
			logger->debug("Serializing an inactive spell after pruning?! Spell {} Id {}", it.data->spellPktBody.spellEnum, it.id);
			//continue;
		}
			

		auto &pkt = it.data->spellPktBody;
		auto spellName = GetSpellName(it.data->spellPktBody.spellEnum);
		logger->info("SpellSave: Saving spell {} id {}", spellName, it.id);
		file.WriteInt32(it.id);
		file.WriteInt32(it.data->key);
		file.WriteInt32(it.data->isActive);
		file.WriteInt32(pkt.spellEnum);
		file.WriteInt32(pkt.spellEnumOriginal);
		file.WriteInt32(pkt.flagSthg);

		// caster
		if (!SerializeHandleToFile(pkt.caster, file))
		{
			return false;
		}
			
		if (!SerializePartsysToFile(pkt.casterPartsysId, file))
			return false;

		file.WriteInt32(pkt.spellClass);
		file.WriteInt32(pkt.spellKnownSlotLevel);
		file.WriteInt32(pkt.baseCasterLevel);
		file.WriteInt32(pkt.dc);

		// spell objs
		file.WriteInt32(pkt.numSpellObjs);
		

		if (!SerializeHandleToFile(pkt.aoeObj, file))
			return false;

		if (!SerializeSpellObjsToFile(pkt.spellObjs, 128, file))
		{
			return false;
		}
			

		// targets
		file.WriteInt32(pkt.orgTargetCount);
		file.WriteInt32(pkt.targetCount);
		if (!SerializeHandlesToFile(pkt.targetListHandles, 32, file))
		{
			return false;
		}
			

		if (!SerializePartSystemsToFile(pkt.targetListPartsysIds, 32, file))
			return false;

		// projectiles
		file.WriteInt32(pkt.projectileCount);
		if (!SerializeHandlesToFile(pkt.projectiles, 5, file))
			return false;

		file.WriteInt64(pkt.aoeCenter.location.location);
		file.WriteFloat(pkt.aoeCenter.location.off_x);
		file.WriteFloat(pkt.aoeCenter.location.off_y);
		file.WriteFloat(pkt.aoeCenter.off_z);
		file.WriteInt32(pkt.duration);
		file.WriteInt32(pkt.durationRemaining);
		file.WriteInt32(pkt.spellRange);
		file.WriteInt32(pkt.savingThrowResult);
		file.WriteInt32(pkt.metaMagicData);
		file.WriteInt32(pkt.spellId);
		
		numSerialized++;
	}

	return numSerialized;
}

void LegacySpellSystem::GetSpellsFromTransferInfo()
{
	for (auto it: spellMapTransInfo) {
		unsigned spellId;
		SpellPacket spellPkt;
		if (GetSpellPacketFromTransferInfo(spellId, spellPkt, it))	{
			for (size_t j = 0; j < spellPkt.spellPktBody.targetCount; j++)	{
				if (!it.targetlistPartsys[j].empty() && it.targets[j]){
					auto handle = objSystem->GetHandleById(it.targets[j]);
					if (handle){
						auto partSysId = gameSystems->GetParticleSys().CreateAtObj(it.targetlistPartsys[j], handle);
						spellPkt.spellPktBody.targetListPartsysIds[j] = partSysId;
					}
						
				}
			}
			spellsCastRegistry.put(spellId, spellPkt);

		}
	}

	spellMapTransInfo.clear();
}

bool LegacySpellSystem::GetSpellPacketFromTransferInfo(unsigned& spellId, SpellPacket& spellPkt, SpellMapTransferInfo& mtInfo){

	spellId = mtInfo.spellId;

	if (!spellsCastRegistry.copy(spellId, &spellPkt))
		return false;

	auto spellName = GetSpellName(spellPkt.spellPktBody.spellEnum);

	auto handle = objSystem->GetHandleById(mtInfo.casterObjId);
	if (handle) {
		if (!mtInfo.casterPartsys.empty()) {
			spellPkt.spellPktBody.casterPartsysId = gameSystems->GetParticleSys().CreateAtObj(mtInfo.casterPartsys, handle);
		} else {
			logger->debug("GetSpellPacketFromTransferInfo: Tried to play partsys hash 0 for spell {} id", spellName, spellId);
		}
		spellPkt.spellPktBody.caster = handle;
	} else
	{
		logger->info("Null caster detected in SpellTransferInfo, aborting. (perhaps from an NPC?)");
		return false;
	}
	

	if (mtInfo.aoeObjId.subtype != ObjectIdKind::Null) {
		auto aoeHandle = objSystem->GetHandleById(mtInfo.aoeObjId);
		spellPkt.spellPktBody.aoeObj = aoeHandle;
	} else{
		spellPkt.spellPktBody.aoeObj = 0i64;
	}
	
	for (int i = 0; i < 128; i++) {
		if (!mtInfo.spellObjs[i].IsNull()) {
			auto spellObjHandle = objSystem->GetHandleById(mtInfo.spellObjs[i]);
			spellPkt.spellPktBody.spellObjs[i].obj = spellObjHandle;

			if (!mtInfo.spellObjPartsys[i].empty()) {
				auto partSysHandle = gameSystems->GetParticleSys().CreateAtObj(mtInfo.spellObjPartsys[i], spellObjHandle);
				spellPkt.spellPktBody.spellObjs[i].partySysId = partSysHandle;
			}
		} else {
			spellPkt.spellPktBody.spellObjs[i].obj = 0i64;
		}
	}

	for (int i = 0; i < 32; i++) {
		
		if (!mtInfo.targets[i].IsNull())
			spellPkt.spellPktBody.targetListHandles[i] = objSystem->GetHandleById(mtInfo.targets[i]);
		else
			spellPkt.spellPktBody.targetListHandles[i] = 0i64;
	}

	for (int i = 0; i < 5; i++) {

		if (mtInfo.projectiles[i].subtype != ObjectIdKind::Null)
			spellPkt.spellPktBody.projectiles[i] = objSystem->GetHandleById(mtInfo.projectiles[i]);
		else
			spellPkt.spellPktBody.projectiles[i] = 0i64;
	}

	return true;
}

bool LegacySpellSystem::LoadActiveSpellElement(TioFile* file, uint32_t& spellId, SpellPacket& pkt)
{
	
	if (!tio_fread(&spellId, sizeof(int), 1, file))
		return false;
	logger->debug("Loading spellId {}", spellId);
	if (!tio_fread(&pkt.key, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.isActive, sizeof(int), 1, file))
		return false;
	///Expects(pkt.isActive);
	if (!pkt.isActive){
		logger->debug("Spell was inactive!");
	}

	if (!tio_fread(&pkt.spellPktBody.spellEnum, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.spellEnumOriginal, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.flagSthg, sizeof(int), 1, file))
		return false;

	// get the caster
	ObjectId objId;
	if (!tio_fread(&objId, sizeof(ObjectId), 1, file))
		return false;
	//Expects(objId.subtype != ObjectIdKind::Null);
	pkt.spellPktBody.caster = objSystem->GetHandleById(objId);

	// get the caster partsys
	auto partsysHash = 0;
	if (!tio_fread(&partsysHash, sizeof(int), 1, file))
		return false;
	if (pkt.spellPktBody.caster)
		pkt.spellPktBody.casterPartsysId = partsysHash ? gameSystems->GetParticleSys().CreateAtObj(partsysHash, pkt.spellPktBody.caster) : 0;
	else
	{
		logger->warn("SpellLoad: null caster handle!");
		pkt.spellPktBody.casterPartsysId = 0;
	}
		

	if (!tio_fread(&pkt.spellPktBody.spellClass, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.spellKnownSlotLevel, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.baseCasterLevel, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.dc, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.numSpellObjs, sizeof(int), 1, file))
		return false;

	// aoeObj
	if (!tio_fread(&objId, sizeof(ObjectId), 1, file))
		return false;
	pkt.spellPktBody.aoeObj = objSystem->GetHandleById(objId);


	// spellObjs
	for (int i = 0; i < 128; i++){
		
		auto& spellObj = pkt.spellPktBody.spellObjs[i];

		if (!tio_fread(&objId, sizeof(ObjectId), 1, file))
			return false;
		if (objId.subtype != ObjectIdKind::Null){
			spellObj.obj =	objSystem->GetHandleById(objId);
		}
		else
			spellObj.obj = 0i64;
		
		if (!tio_fread(&partsysHash, sizeof(int), 1, file))
			return false;
		if (partsysHash && pkt.spellPktBody.spellObjs[i].obj){
			spellObj.partySysId = gameSystems->GetParticleSys().CreateAtObj(partsysHash, spellObj.obj);
		} else	{
			spellObj.partySysId = 0;
		}
	}

	// targets
	if (!tio_fread(&pkt.spellPktBody.orgTargetCount, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.targetCount, sizeof(int), 1, file))
		return false;
	std::vector<objHndl> targets;
	for (int i = 0; i < 32; i++){
		//auto& tgt = pkt.spellPktBody.targetListHandles[i];
		if (!tio_fread(&objId, sizeof(ObjectId), 1, file))
			return false;
		if (objId.subtype != ObjectIdKind::Null) {
			auto tgt = objSystem->GetHandleById(objId);
			if (gameSystems->GetObj().IsValidHandle(tgt)){
				targets.emplace_back(tgt);
			} 
			
		}
		else{
			//tgt = 0i64;
		}	
	}
	if (targets.size()){
		memcpy(pkt.spellPktBody.targetListHandles, &targets[0], min(targets.size() * sizeof(objHndl), 32 * sizeof(objHndl)));
	}
	
	memset(&pkt.spellPktBody.targetListHandles[targets.size()], 0, (32 - targets.size()) * sizeof(objHndl));
	if (pkt.spellPktBody.targetCount != targets.size()){
		logger->debug("LoadActiveSpellElement: Culled targets - originally {} now {}", pkt.spellPktBody.targetCount , targets.size());
		pkt.spellPktBody.targetCount = targets.size();
	}

	// target particles
	for (int i = 0; i < 32; i++){
		auto& tgt = pkt.spellPktBody.targetListHandles[i];
		auto& partsysId = pkt.spellPktBody.targetListPartsysIds[i];
		if (!tio_fread(&partsysHash, sizeof(int), 1, file))
			return false;
		if (partsysHash && tgt  ) {
			partsysId = gameSystems->GetParticleSys().CreateAtObj(partsysHash, tgt);
		}
		else {
			partsysId = 0;
		}
	}

	
	// projectiles
	if (!tio_fread(&pkt.spellPktBody.projectileCount, sizeof(int), 1, file))
		return false;
	for (int i = 0; i < 5; i++) {
		auto& projectile = pkt.spellPktBody.projectiles[i];
		if (!tio_fread(&objId, sizeof(ObjectId), 1, file))
			return false;
		if (objId.subtype != ObjectIdKind::Null) {
			projectile = objSystem->GetHandleById(objId);
		}
		else
			projectile = 0i64;
	}

	if (!tio_fread(&pkt.spellPktBody.aoeCenter, sizeof(locXY), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.aoeCenter.location.off_x, sizeof(float), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.aoeCenter.location.off_y, sizeof(float), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.aoeCenter.off_z, sizeof(float), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.duration, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.durationRemaining, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.spellRange, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.savingThrowResult, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.metaMagicData, sizeof(int), 1, file))
		return false;
	if (!tio_fread(&pkt.spellPktBody.spellId, sizeof(int), 1, file))
		return false;
	Expects(pkt.spellPktBody.spellId >= 0);

	auto spellName = GetSpellName(pkt.spellPktBody.spellEnum);
	logger->info("SpellLoad: Loaded spell {} id {}", spellName, spellId);
	return true;
}

bool LegacySpellSystem::IsSpellActive(int spellid) {
	SpellPacket spellPacket; 
	if (spellsCastRegistry.copy(spellid, &spellPacket)) {
		return spellPacket.isActive == 1;
	}
	return false;
}

CondStruct* LegacySpellSystem::GetCondFromSpellIdx(int id) {
	if (id >= 3 && id < 254) {
		return addresses.spellConds[id - 1].condition;
	}
	return nullptr;
}

void LegacySpellSystem::ForgetMemorized(objHndl handle) {
	objSystem->GetObject(handle)->ClearArray(obj_f_critter_spells_memorized_idx);
}

uint32_t LegacySpellSystem::getSpellEnum(const char* spellName)
{
	MesLine mesLine;
	for (auto i = 0; i < SPELL_ENUM_MAX_EXPANDED; i++)
	{
		mesLine.key = 5000 + i;
		mesFuncs.GetLine_Safe(*spellEnumMesHandle, &mesLine);
		if (!_stricmp(spellName, mesLine.value)){
			return i;
		}
			
	}
	return 0;
}

uint32_t LegacySpellSystem::GetSpellEnumFromSpellId(uint32_t spellId)
{
	SpellPacket spellPacket;
	if (spellsCastRegistry.copy(spellId, &spellPacket))
	{
		if (spellPacket.isActive == 1)
			return spellPacket.spellPktBody.spellEnum;
	}
	return 0;
}

uint32_t LegacySpellSystem::GetSpellPacketBody(uint32_t spellId, SpellPacketBody* spellPktBodyOut)
{
	SpellPacket spellPkt;
	if (spellsCastRegistry.copy(spellId, &spellPkt))
	{
		memcpy(spellPktBodyOut, &spellPkt.spellPktBody, sizeof(SpellPacketBody));
		return 1;
	}
	return 0;
}

void LegacySpellSystem::UpdateSpellPacket(const SpellPacketBody& spellPktBody) {
	addresses.UpdateSpellPacket(spellPktBody);
}

uint32_t LegacySpellSystem::spellKnownQueryGetData(objHndl objHnd, uint32_t spellEnum, uint32_t* classCodesOut, uint32_t* slotLevelsOut, uint32_t* count)
{
	uint32_t countLocal;
	uint32_t * n = count;
	if (count == nullptr) n = &countLocal;

	auto obj = objSystem->GetObject(objHnd);

	*n = 0;
	auto numSpellsKnown = obj->GetSpellArray(obj_f_critter_spells_known_idx).GetSize();
	for (size_t i = 0; i < numSpellsKnown; i++)
	{
		auto spellData = obj->GetSpell(obj_f_critter_spells_known_idx, i);
		if (spellData.spellEnum == spellEnum)
		{
			if (classCodesOut) classCodesOut[*n] = spellData.classCode;
			if (slotLevelsOut) slotLevelsOut[*n] = spellData.spellLevel;
			++*n;
		}
	}
	return *n > 0;
}

uint32_t LegacySpellSystem::spellCanCast(objHndl objHnd, uint32_t spellEnum, uint32_t spellClassCode, uint32_t spellLevel)
{
	uint32_t count = 0;
	uint32_t classCodes[10000];
	uint32_t spellLevels[10000];

	SpellEntry spellEntry;
	if (d20Sys.d20Query(objHnd, DK_QUE_CannotCast) 
		|| !spellEntryRegistry.copy(spellEnum, &spellEntry) ) 
		return 0;
	if (isDomainSpell(spellClassCode)) // domain spell
	{
		if (numSpellsMemorizedTooHigh(objHnd))	return 0;

		spellMemorizedQueryGetData(objHnd, spellEnum, classCodes, spellLevels, &count);
		for (uint32_t i = 0; i < count; i++)
		{
			if ( isDomainSpell(classCodes[i]) 
				&& (classCodes[i] & 0x7F) == ( spellClassCode &0x7F)
				&&  spellLevels[i] == spellLevel)
				return 1;
		}
		return 0;
	}

	if (d20Sys.d20Class->isNaturalCastingClass(spellClassCode & 0x7F))
	{
		if (numSpellsKnownTooHigh(objHnd)) return 0;

		spellKnownQueryGetData(objHnd, spellEnum, classCodes, spellLevels, &count);
		for (int32_t i = 0; i < (int32_t)count; i++)
		{
			if ( !isDomainSpell(classCodes[i])
				&& (classCodes[i] & 0x7F) == (spellClassCode & 0x7F)
				&& spellLevels[i] <= spellLevel)
			{
				if (spellLevels[i] < spellLevel)
					logger->info("Natural Spell Caster spellCanCast check - spell known is lower level than spellCanCast queried spell. Is this ok?? (this is vanilla code here...)");
				return 1;
			}
				
		}
		return 0;
	}

	if (numSpellsMemorizedTooHigh(objHnd)) return 0;

	spellMemorizedQueryGetData(objHnd, spellEnum, classCodes, spellLevels, &count);
	for (uint32_t i = 0; i < count; i++)
	{
		if ( !isDomainSpell(classCodes[i])
			&& (classCodes[i] & 0x7F) == (spellClassCode & 0x7F)
			&& spellLevels[i] == spellLevel)
			return 1;
	}
	return 0;
}

uint32_t LegacySpellSystem::spellMemorizedQueryGetData(objHndl objHnd, uint32_t spellEnum, uint32_t* classCodesOut, uint32_t* slotLevelsOut, uint32_t* count)
{
	uint32_t countLocal;
	uint32_t * n = count;
	if (count == nullptr) n = &countLocal;

	auto obj = objSystem->GetObject(objHnd);

	*n = 0;
	auto numSpellsMemod = obj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize();
	for (size_t i = 0; i < numSpellsMemod; i++) {
		auto spellData = obj->GetSpell(obj_f_critter_spells_memorized_idx, i);
		if (spellData.spellEnum == spellEnum && !spellData.spellStoreState.usedUp)
		{
			if (classCodesOut) classCodesOut[*n] = spellData.classCode;
			if (slotLevelsOut) slotLevelsOut[*n] = spellData.spellLevel;
			++*n;
		}
	}
	return *n > 0;
}

bool LegacySpellSystem::numSpellsKnownTooHigh(objHndl objHnd)
{
	auto obj = objSystem->GetObject(objHnd);

	if (obj->GetSpellArray(obj_f_critter_spells_known_idx).GetSize() > MAX_SPELLS_KNOWN)
	{
		logger->info("spellCanCast(): ERROR! This critter knows WAAY too many spells! Returning 0.");
		return 1;
	}
	return 0;
}

bool LegacySpellSystem::numSpellsMemorizedTooHigh(objHndl objHnd)
{
	auto obj = objSystem->GetObject(objHnd);

	if (obj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize() > MAX_SPELLS_KNOWN)
	{
		logger->info("spellCanCast(): ERROR! This critter memorized WAAY too many spells! Returning 0.");
		return 1;
	}
	return 0;
}

bool LegacySpellSystem::isDomainSpell(uint32_t spellClassCode)
{
	if (spellClassCode & 0x80) return 0;
	return 1;
}

Stat LegacySpellSystem::GetCastingClass(uint32_t spellClassCode){
	if (isDomainSpell(spellClassCode))	{
		logger->warn("GetCastingClass called with domain spell class code: {}", spellClassCode);
	}
	return (Stat)(spellClassCode & 0x7F);
}

bool LegacySpellSystem::IsArcaneSpellClass(uint32_t spellClass)
{
	// take care of domain spells first
	if (isDomainSpell(spellClass)) {
		return false;
	}

	auto casterClass = GetCastingClass(spellClass);
	if (casterClass == stat_level_bard || casterClass == stat_level_sorcerer || casterClass == stat_level_wizard)
		return true;

	return false;
}

bool LegacySpellSystem::IsSpellLike(int spellEnum){
	return (spellEnum >= NORMAL_SPELL_RANGE
		&& spellEnum <= SPELL_LIKE_ABILITY_RANGE);
}

uint32_t LegacySpellSystem::pickerArgsFromSpellEntry(SpellEntry* spellEntry, PickerArgs * pickArgs, objHndl objHnd, uint32_t casterLvl)
{
	return _pickerArgsFromSpellEntry(spellEntry, pickArgs, objHnd, casterLvl);
}

uint32_t LegacySpellSystem::GetSpellRangeExact(SpellRangeType spellRangeType, uint32_t casterLevel, objHndl caster)
{
	switch (spellRangeType)
	{
	case SpellRangeType::SRT_Personal:
		return 5;
	case SpellRangeType::SRT_Touch:
		return (int) critterSys.GetReach(caster, D20A_TOUCH_ATTACK);
	case SpellRangeType::SRT_Close:
		return (casterLevel / 2 + 5) * 5;
	case SpellRangeType::SRT_Medium:
		return 2 * (5*casterLevel + 50);
	case SpellRangeType::SRT_Long:
		return 8 * (5 * casterLevel + 50);
	case SpellRangeType::SRT_Special_Inivibility_Purge:
		return 5 * casterLevel;
	default:
		logger->warn("GetSpellRangeExact: unknown range specified for spell entry\n");
		break;
	}
	return 0;
}

uint32_t LegacySpellSystem::GetSpellRange(SpellEntry* spellEntry, uint32_t casterLevel, objHndl caster)
{
	auto spellRangeType = spellEntry->spellRangeType;
	if (spellRangeType == SpellRangeType::SRT_Specified)
		return spellEntry->spellRange;
	return GetSpellRangeExact(spellRangeType, casterLevel, caster);
}

const char* LegacySpellSystem::GetSpellEnumNameFromEnum(int spellEnum)
{
	MesLine mesline;
	mesline.key = spellEnum;
	mesFuncs.GetLine(*spellEnumMesHandle, &mesline);
	return mesline.value;
}

bool LegacySpellSystem::GetSpellTargets(objHndl obj, objHndl tgt, SpellPacketBody* spellPkt, unsigned spellEnum)
{
	// returns targets using the picker function
	auto getTgts = temple::GetRef<bool(__cdecl)(objHndl , objHndl , SpellPacketBody* , unsigned )>(0x10079030);
	return getTgts(obj, tgt, spellPkt, spellEnum);
}

BOOL LegacySpellSystem::SpellHasAiType(unsigned spellEnum, AiSpellType aiSpellType)
{
	SpellEntry spellEntry;
	if (spellSys.spellRegistryCopy(spellEnum, &spellEntry) && spellEntry.aiTypeBitmask)
	{
		return  ( 
					(spellEntry.aiTypeBitmask & (1 << aiSpellType)) 
						== (1 << aiSpellType));
	}
	return 0;
}

BOOL LegacySpellSystem::IsSpellHarmful(int spellEnum, const objHndl& caster, const objHndl& tgt){
	return temple::GetRef<BOOL(__cdecl)(int, objHndl, objHndl)>(0x100769F0)(spellEnum, caster, tgt);
}

int LegacySpellSystem::DispelRoll(objHndl obj, BonusList* bonlist, int rollMod, int dispelDC, char* historyText, int* rollHistId)
{
	auto dispelRoll = temple::GetRef<int(__cdecl)(objHndl, BonusList*, int, int, char*, int*)>(0x100B51E0);
	return dispelRoll(obj, bonlist, rollMod, dispelDC, historyText, rollHistId);
}

BOOL LegacySpellSystem::PlayFizzle(objHndl handle)
{
	gameSystems->GetParticleSys().CreateAtObj("Fizzle", handle);
	sound.PlaySoundAtObj(17122, handle);
	return 1;
}

int LegacySpellSystem::CheckSpellResistance(SpellPacketBody* spellPkt, objHndl handle)
{
	// check spell immunity
	DispIoImmunity dispIo;
	dispIo.flag = 1;
	dispIo.spellPkt = spellPkt;
	spellSys.spellRegistryCopy(spellPkt->spellEnum, &dispIo.spellEntry);
	
	auto obj = gameSystems->GetObj().GetObject(handle);
	auto dispatcher = obj->GetDispatcher();
	if (dispatch.Dispatch64ImmunityCheck(handle, &dispIo)){
		return 1;
	}

	// does spell allow saving?
	if (dispIo.spellEntry.spellResistanceCode != 1)
	{
		return 0;
	}
	
	// obtain bonuses
	DispIOBonusListAndSpellEntry dispIoBon;
	BonusList bonlist;

	auto casterLvlMod = dispatch.Dispatch35BaseCasterLevelModify(handle, spellPkt);
	bonlist.AddBonus(casterLvlMod, 0, 203);


	if (feats.HasFeatCountByClass(handle, FEAT_SPELL_PENETRATION)){
		auto featName=  feats.GetFeatName(FEAT_SPELL_PENETRATION);
		bonlist.AddBonusWithDesc(2,0,114, featName);
	}
	if (feats.HasFeatCountByClass(handle, FEAT_GREATER_SPELL_PENETRATION)){
		auto featName = feats.GetFeatName(FEAT_GREATER_SPELL_PENETRATION);
		bonlist.AddBonusWithDesc(2, 0, 114, featName);
	}
	dispIoBon.spellEntry = &dispIo.spellEntry;

	int srMod = dispatch.Dispatch45SpellResistanceMod(handle, &dispIoBon);

	// do the roll and log the result to the D20 window
	int rollResult = 0;
	if (srMod > 0){
		auto Spell_Resistance = combatSys.GetCombatMesLine(5048);
		int rollHistId;
		rollResult = spellSys.DispelRoll(spellPkt->caster, &bonlist, 0, srMod, Spell_Resistance, &rollHistId);
		char * outcomeText1, outcomeText2;
		if (rollResult <=0)	{
			auto spellName = GetSpellName(spellPkt->spellEnum);
			logger->info("CheckSpellResistance: Spell {} cast by {} resisted by target {}.", spellName, description.getDisplayName(spellPkt->caster), description.getDisplayName(handle));
			floatSys.FloatSpellLine(handle, 30008, FloatLineColor::White);
			PlayFizzle(handle);
			outcomeText1 = combatSys.GetCombatMesLine(119); // Spell ~fails~[ROLL_
			outcomeText1 = combatSys.GetCombatMesLine(120); // ] to overcome Spell Resistance

		} else
		{
			floatSys.FloatSpellLine(handle, 30009, FloatLineColor::Red);
			outcomeText1 = combatSys.GetCombatMesLine(121); // Spell ~overcomes~[ROLL_
			outcomeText1 = combatSys.GetCombatMesLine(122); // ] Spell Resistance
		}

		auto histText = std::string(fmt::format("{}{}{}\n\n", outcomeText1, rollHistId, outcomeText2));
		histSys.CreateFromFreeText(histText.c_str());
	}

	


	return rollResult;

}

void LegacySpellSystem::SpellsCastRegistryPut(int spellId, SpellPacket& pkt)
{
	spellsCastRegistry.put(spellId, pkt);
}

int LegacySpellSystem::SpellEnd(int spellId, int endDespiteTargetList) const
{
	SpellPacket pkt;
	if (!spellsCastRegistry.copy(spellId,&pkt)){
		logger->debug("SpellEnd: \t Couldn't find spell in registry. Spell id {}", spellId);
		return 0;
	}

	auto spellName = GetSpellName(pkt.spellPktBody.spellEnum);

	if (pkt.spellPktBody.targetCount > 0){
		logger->debug("SpellEnd: \t target count is nonzero! Spell {} id {}", spellName, spellId);
		if (!endDespiteTargetList)
			return 0;
		logger->debug("Forcing spell end anyway...");
	}

	pySpellIntegration.SpellTrigger(spellId, SpellEvent::EndSpellCast);
	pySpellIntegration.RemoveSpell(spellId); // bah :P

	// python stuff could update it so we refresh
	spellsCastRegistry.copy(spellId, &pkt);
	pkt.isActive = 0;
	spellsCastRegistry.put(spellId, pkt);
	return 1;
}


#pragma endregion



#pragma region Hooks

uint32_t __cdecl _getWizSchool(objHndl objHnd)
{
	return spellSys.getWizSchool(objHnd);
}

uint32_t __cdecl _getSpellEnum(const char* spellName)
{
	return spellSys.getSpellEnum(spellName);
}

uint32_t _spellKnownQueryGetData(objHndl objHnd, uint32_t spellEnum, uint32_t* classCodesOut, uint32_t* slotLevelsOut, uint32_t* count)
{
	return spellSys.spellKnownQueryGetData(objHnd, spellEnum, classCodesOut, slotLevelsOut, count);
}

uint32_t _spellMemorizedQueryGetData(objHndl objHnd, uint32_t spellEnum, uint32_t* classCodesOut, uint32_t* slotLevelsOut, uint32_t* count)
{
	return spellSys.spellMemorizedQueryGetData(objHnd, spellEnum, classCodesOut, slotLevelsOut, count);
}

uint32_t _spellCanCast(objHndl objHnd, uint32_t spellEnum, uint32_t spellClassCode, uint32_t spellLevel)
{
	return spellSys.spellCanCast(objHnd, spellEnum, spellClassCode, spellLevel);
}

uint32_t _spellRegistryCopy(uint32_t spellEnum, SpellEntry* spellEntry)
{
	return spellSys.spellRegistryCopy(spellEnum, spellEntry);
}

uint32_t _GetSpellEnumFromSpellId(uint32_t spellId)
{
	return spellSys.GetSpellEnumFromSpellId(spellId);
}

uint32_t _GetSpellPacketBody(uint32_t spellId, SpellPacketBody* spellPktBodyOut)
{
	return spellSys.GetSpellPacketBody(spellId, spellPktBodyOut);
}

void _SetSpontaneousCastingAltNode(objHndl obj, int nodeIdx, SpellStoreData* spellData)
{
	spellSys.SetSpontaneousCastingAltNode(obj, nodeIdx, spellData);
}
#pragma endregion 
