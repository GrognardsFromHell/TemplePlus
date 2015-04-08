
#include "stdafx.h"
#include "fixes.h"
#include "feat.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20.h"
#include "tig_mes.h"
#include "common.h"
#include "weapon.h"


FeatSystem feats;

class FeatFixes : public TempleFix {
public:
	const char* name() override {
		return "Re-implementation of SpellSlinger's fix for Rogues feats above level 10 in a way that doesn't involve undeclared memory segments";
	}

	void apply() override {
		//writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
		
		replaceFunction(0x1007C8F0, _FeatPrereqsCheck);
		replaceFunction(0x1007B990, _FeatExistsInArray);
		replaceFunction(0x1007BF10, _RogueSpecialFeat);
		replaceFunction(0x1007B930, _HasFeatCount);
		replaceFunction(0x1007C080, _HasFeatCountByClass);
		replaceFunction(0x1007C370, _FeatListGet);
		replaceFunction(0x1007C3F0, _FeatListElective);
		replaceFunction(0x1007C8D0, _WeaponFeatCheckSimpleWrapper);
		//replaceFunction(0x1007C4F0, _WeaponFeatCheck); // usercall bullshit; replaced the functions that used it anyway
	}
};
FeatFixes featFixes;

# pragma region FeatSystem Implementations
FeatSystem::FeatSystem()
{
	rebase(featPropertiesTable, 0x102BFD78); 		// TODO: export this to a mesfile
	rebase(classFeatTable, 0x102CAAF8); 		// TODO: export this to a mesfile
	rebase(featPreReqTable, 0x102C07A0); 		// TODO: export this to a mesfile
	rebase(charEditorObjHnd, 0x11E741A0);		// TODO: move this to the appropriate system
	rebase(charEditorClassCode, 0x11E72FC0);	// TODO: move this to the appropriate system
	rebase(ToEE_WeaponFeatCheck, 0x1007C4F0);

	uint32_t _racialFeatsTable[NUM_RACES * 10] = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		FEAT_SIMPLE_WEAPON_PROFICIENCY_ELF, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // table presupposes 10 items on each row, terminator character -1
	uint32_t _rangerArcheryFeats[2 * 2] = { FEAT_RANGER_RAPID_SHOT, 2, FEAT_RANGER_MANYSHOT, 6 };
	uint32_t _rangerTwoWeaponFeats[2 * 2] = { FEAT_TWO_WEAPON_FIGHTING_RANGER, 2, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, 6 };

	memcpy(racialFeats, _racialFeatsTable, sizeof(racialFeats));
	memcpy(rangerArcheryFeats, _rangerArcheryFeats, sizeof(rangerArcheryFeats));
	memcpy(rangerTwoWeaponFeats, _rangerTwoWeaponFeats, sizeof(rangerTwoWeaponFeats));
};

uint32_t FeatSystem::HasFeatCount(objHndl objHnd, feat_enums featEnum)
{
	uint32_t featCount = 0;
	uint32_t numFeats = templeFuncs.Obj_Get_IdxField_NumItems(objHnd, obj_f_critter_feat_idx);
	for (uint32_t i = 0; i < numFeats; i++)
	{
		if (templeFuncs.Obj_Get_IdxField_32bit(objHnd, obj_f_critter_feat_idx, i) == featEnum)
		{
			featCount++;
		}
	}
	return featCount;
}


uint32_t FeatSystem::HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classEnum, ::uint32_t rangerSpecializationFeat)
{
	return _HasFeatCountByClass(objHnd, featEnum, classEnum, rangerSpecializationFeat);
};

uint32_t FeatSystem::FeatListGet(objHndl objHnd, feat_enums* listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat)
{
	uint32_t featCount = 0;
	int32_t hasFeatTimes = 0;
	uint32_t i = 0;
	void * ptrOut = listOut;
	while (i < NUM_FEATS)
	{
		hasFeatTimes = feats.HasFeatCountByClass(objHnd, (feat_enums)i, classBeingLevelled, rangerSpecFeat);

		if (hasFeatTimes && listOut && hasFeatTimes > 0)
		{
			for (auto j = 0; j < hasFeatTimes; j++)
			{
				memcpy(&(listOut[featCount+j]), &i, sizeof(uint32_t));

			}
			featCount += hasFeatTimes;
			
		}
		i++;
	}
	return featCount;
}


uint32_t FeatSystem::FeatListElective(objHndl objHnd, feat_enums* listOut)
{
	return FeatListGet(objHnd, listOut, (Stat)0, (feat_enums)0);
}

uint32_t FeatSystem::FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen)
{
	for (uint32_t i = 0; i < featArrayLen; i++)
	{
		if (featArray[i] == featCode){ return 1; }
	}
	return 0;
};

uint32_t FeatSystem::WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType)
{
	return _WeaponFeatCheck( objHnd,  featArray,  featArrayLen,  classBeingLeveled,  wpnType);
};

#pragma endregion


uint32_t __declspec(naked) ToEEWeaponFeatCheckUsercallWrapper(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat eax_objStat, uint32_t ebx_n1)
{ 
	// Note: NO LONGER NECESSARY - function has been re-implemented in TemplePlus!
	// objStat goes into eax,  n1 goes into ebx
	__asm {// stack frame
		//arg4	  // esp0 - 0x20
		//arg8      // esp0 - 0x1C
		//argC      // esp0 - 0x18
		//		   arg10	  // esp0 - 0x14
		//ebx       // esp0 - 0x10
		//ecx
		//esi
		//edi
		//retaddr    esp0
		//arg4	objHnd
		//arg8
		// argC	featArray
		// arg10	featArrayLen
		// arg14   objStat  esp0+0x14
		// arg18   n1       esp0+0x18
		//
		push edi;
		push esi;
		push ecx;
		push ebx; // esp0 - 0x10
		sub esp, 0x10			// esp0 - 0x20
			mov edi, esp;
		lea esi, [esp + 0x24];	// esi = esp0 + 4	
		mov ecx, 4;
		rep movsd;
		mov eax, [esp + 0x34]; // objStat   esp0+0x14
		mov ebx, [esp + 0x38]; // n1		 esp0+0x18
		mov esi, feats.ToEE_WeaponFeatCheck;
		call esi; // esp0 - 0x20
		add esp, 0x10;
		pop ebx;
		pop ecx;
		pop esi;
		pop edi;
		ret;

	}
};


uint32_t _WeaponFeatCheckSimpleWrapper(objHndl objHnd, WeaponTypes wpnType)
{
	return feats.WeaponFeatCheck(objHnd, 0, 0, (Stat)0, wpnType);
}

uint32_t _WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType)
{
	if (templeFuncs.sub_100664B0(objHnd, wpnType) == 3){ return 0; }

	if (weapons.IsSimple(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsMartial(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL, classBeingLeveled, 0))
		{
			return 1;
		}
		if (feats.FeatExistsInArray(FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL, featArray, featArrayLen))
		{
			return 1;
		}

		if (feats.HasFeatCountByClass(objHnd, (feat_enums) ((uint32_t)wpnType + (uint32_t)FEAT_IMPROVED_CRITICAL_REPEATING_CROSSBOW), classBeingLeveled, 0))
		{
			return 1;
		}

		if (feats.FeatExistsInArray((feat_enums)((uint32_t)wpnType + (uint32_t)FEAT_IMPROVED_CRITICAL_REPEATING_CROSSBOW), featArray, featArrayLen))
		{
			return 1;
		}
	}

	if (weapons.IsExotic(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, (feat_enums)((uint32_t)wpnType - 21), classBeingLeveled, 0))
		{
			return 1;
		}
		if (feats.FeatExistsInArray((feat_enums)((uint32_t)wpnType - 21), featArray, featArrayLen))
		{
			return 1;
		}
	}

	if (wpnType == wt_bastard_sword || wpnType == wt_dwarven_waraxe)
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsDruidWeapon(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_DRUID, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsMonkWeapon(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_DRUID, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsRogueWeapon(objects.StatLevelGet(objHnd, stat_size), wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_ROGUE, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsWizardWeapon(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_WIZARD, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (weapons.IsElvenWeapon(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_ELF, classBeingLeveled, 0))
		{
			return 1;
		}
	}


	if (weapons.IsBardWeapon(wpnType))
	{
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_BARD, classBeingLeveled, 0))
		{
			return 1;
		}
	}

	if (wpnType == wt_orc_double_axe)
	{
		if (objects.GetRace(objHnd) == race_halforc)
		{
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_gnome_hooked_hammer)
	{
		if (objects.GetRace(objHnd) == race_gnome)
		{
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_dwarven_waraxe)
	{
		if (objects.GetRace(objHnd) == race_dwarf)
		{
			return 1;
		}
		return 0;
	} else if (wpnType == wt_grenade)
	{
		return 1;
	}

	return 0;
}

uint32_t _FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen)
{
	for (uint32_t i = 0; i < featArrayLen; i++)
	{
		if (featArray[i] == featCode){ return 1; }
	}
	return 0;
};

uint32_t _FeatPrereqsCheck(objHndl objHnd, feat_enums featIdx, feat_enums * featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased)
{
	uint32_t featProps = feats.featPropertiesTable[featIdx];
	uint32_t * featPrereqs = feats.featPreReqTable;
	const uint8_t numCasterClasses = 7;
	uint32_t casterClassCodes[numCasterClasses] = { stat_level_bard, stat_level_cleric, stat_level_druid, stat_level_paladin, stat_level_ranger, stat_level_sorcerer, stat_level_wizard };

	if (featProps & 2)
	{
		return 0;
	}

	//return 1; // h4x :)

#pragma region	checking feats in the character editor - SpellSlinger hack for special Rogue feats for level > 10
	if ( *feats.charEditorClassCode != 0 && *feats.charEditorObjHnd != 0)
	{
		auto newClassLvl = templeFuncs.ObjStatLevelGet(*feats.charEditorObjHnd, *feats.charEditorClassCode) + 1;

		if (classCodeBeingLevelledUp == stat_level_rogue)
		{
			if (newClassLvl == 10 || newClassLvl == 13 || newClassLvl == 16 || newClassLvl == 19)
			{
				if (featProps & featPropRogueFeat)
				{
					return 1;
				}
			}
		}
	}
#pragma endregion


	uint32_t initOffset = featIdx * 16;

	if ( featPrereqs[initOffset]  == featReqCodeTerminator){ return 1; };


	for (uint32_t i = 0; featPrereqs[initOffset + i] != featReqCodeTerminator; i+=2)
	{
		//disassm notes:
		// eax is classCodeBeingLevelledUp
		// esi is featReqCode
		// ecx is featIdx

		int32_t featReqCode = featPrereqs[initOffset + i];
		auto featReqCodeArg = featPrereqs[initOffset + i + 1];
		uint32_t var_2C = 0;

		

		if (featReqCode == featReqCodeMinCasterLevel)
		{
# pragma region Minimum Caster Level
			for (uint8_t j = 0; j < numCasterClasses; j++)
			{
				
				auto casterLevel = templeFuncs.ObjStatLevelGet(objHnd, casterClassCodes[j]);
				if (classCodeBeingLevelledUp == casterClassCodes[j]){ casterLevel++; };
				if (casterClassCodes[j] == stat_level_paladin || casterClassCodes[j] == stat_level_ranger){ casterLevel /= 2; };
				if ((uint32_t)casterLevel >= (uint32_t)featReqCodeArg){ var_2C = 1; };
			};

			if (var_2C == 0){ return 0; }
#pragma endregion 
		} 
		else if (featReqCode == featReqCodeTurnUndeadRelated)
		{
#pragma region Turn / Rebuke Undead Stuff
			uint32_t critterAlignment = templeFuncs.ObjStatLevelGet(objHnd, stat_alignment);
			uint32_t paladinLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_paladin);
			uint32_t clericLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_cleric);
			uint32_t paladinReqLvl = featReqCodeArg;
			uint32_t clericReqLvl = featReqCodeArg;
			if (featIdx == FEAT_TURN_UNDEAD)
			{
				if ( !(critterAlignment & ALIGNMENT_EVIL || critterAlignment == (ALIGNMENT_CHAOTIC | ALIGNMENT_LAWFUL) ) )// TODO: is this a bug??? might be harmless mistake, might be deliberate
				{
					if (paladinLevel + (classCodeBeingLevelledUp == stat_level_paladin) < 4
						&& clericLevel + (classCodeBeingLevelledUp == stat_level_cleric) < 1
						){ return 0;}
				} 
				
			} 
			else if (featIdx == FEAT_REBUKE_UNDEAD)
			{
				if  (  ! ( critterAlignment > 10 || critterAlignment & ALIGNMENT_GOOD  	 
							|| critterAlignment == 3))
				{
					if (paladinLevel + (classCodeBeingLevelledUp == stat_level_paladin) < 4
						&& clericLevel + (classCodeBeingLevelledUp == stat_level_cleric) < 1
						){	return 0;	}
				};
			}
			if (paladinLevel + (classCodeBeingLevelledUp == stat_level_paladin) < paladinReqLvl
				&& clericLevel + (classCodeBeingLevelledUp == stat_level_cleric) < clericReqLvl
				){	return 0;	}
#pragma endregion 

		} else if (featReqCode == featReqCodeEvasionRelated)
		{
# pragma region Evasion Related
			auto rogueLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_rogue);
			auto monkLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_monk);
			if (featIdx == FEAT_EVASION)
			{
				if (rogueLevel+ (classCodeBeingLevelledUp == stat_level_rogue) < 2
					&& monkLevel+ (classCodeBeingLevelledUp == stat_level_monk) < 1
					){
					return 0;
				}
			} else if (featIdx == FEAT_IMPROVED_EVASION)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 10
					&& monkLevel + (classCodeBeingLevelledUp == stat_level_monk) < 9
					){
					return 0;
				}
			}
#pragma endregion 
		}
		else if (featReqCode == featReqCodeFastMovement)
		{ 
#pragma region Fast Movement
			auto monkLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_monk);
			auto barbarianLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_barbarian);
			if (featIdx == FEAT_FAST_MOVEMENT)
			{
				if (barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 1
					&& monkLevel + (classCodeBeingLevelledUp == stat_level_monk) < 3
					){
					return 0;
				}
			}
#pragma endregion 
		}
		else if (featReqCode == featReqCodeMinArcaneCasterLevel)
		{
# pragma region Min Arcane Caster Level
			uint32_t sorcererLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_sorcerer);
			uint32_t wizardLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_wizard);
			if (sorcererLevel + (classCodeBeingLevelledUp == stat_level_sorcerer) < featReqCodeArg
				&& wizardLevel + (classCodeBeingLevelledUp == stat_level_wizard) < featReqCodeArg
				){
				return 0;
			}
#pragma endregion 
		}
		else if (featReqCode == featReqCodeUncannyDodgeRelated)
		{
#pragma region Uncanny Dodge Related
			auto rogueLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_rogue);
			auto barbarianLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_barbarian);
			if (featIdx == FEAT_UNCANNY_DODGE)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 3
					&& barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 2
					){
					return 0;
				}
			} else if (featIdx == FEAT_IMPROVED_UNCANNY_DODGE)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 8
					&& barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 5
					){
					return 0;
				}
			}
#pragma endregion 
		}
		else if (featReqCode == featReqCodeAnimalCompanion)
		{
#pragma region Animal Companion
			auto druidLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_druid);
			auto rangerLevel = templeFuncs.ObjStatLevelGet(objHnd, stat_level_ranger);
		

			if (featIdx == FEAT_ANIMAL_COMPANION)
			{
				if (druidLevel + (classCodeBeingLevelledUp == stat_level_druid) < 1
					&& rangerLevel + (classCodeBeingLevelledUp == stat_level_ranger) < 4
					){
					return 0;
				}
			}
#pragma endregion 
		}
		else if (featReqCode == -10)
		{
#pragma region Crossbow-related feats (probably rapid reload and stuff)
			if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_light_crossbow))
			{
				if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_heavy_crossbow))
				{
					if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_hand_crossbow))
					{
						return 0;
					}
				}
			}
#pragma endregion
		}
		else if (featReqCode == -4)
		{
#pragma region Weapon related feats general
			if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, (WeaponTypes)featReqCodeArg))
			{
				return 0;
			}
#pragma endregion
		} 
		else if( featReqCode >= 1000 && featReqCode <= 1999)
		{
# pragma region Custom Feat Stat Requirement (?)
			feat_enums featIdxFromReqCode = (feat_enums)(featReqCode - 1000);
			if (!_FeatExistsInArray(featIdxFromReqCode, featArray, featArrayLen))
			{
				if ((uint32_t)templeFuncs.ObjStatBaseGet(objHnd, featReqCode) < featReqCodeArg)
				{
					uint32_t stat = 0;
					if (featIdxFromReqCode == FEAT_TWO_WEAPON_FIGHTING)
					{
						stat = 1640;
					}
					else if (featIdxFromReqCode == FEAT_RAPID_SHOT)
					{
						stat = 1646;
					}
					else if (featIdxFromReqCode == FEAT_MANYSHOT)
					{
						stat = 1647;
					}
					else if (featIdxFromReqCode == FEAT_IMPROVED_TWO_WEAPON_FIGHTING)
					{
						stat = 1641;
					}
					if (stat == 0){ return 0; }
					if ((uint32_t)templeFuncs.ObjStatBaseGet(objHnd, stat) < featReqCodeArg){ return 0; }
				}
			}
#pragma endregion 
		} 
		else if (featReqCode >= 7 && featReqCode <= 17)
		{
# pragma region Class Level Requirement
			if (classCodeBeingLevelledUp == featReqCode){	featReqCodeArg--;	}
			if ((uint32_t)templeFuncs.ObjStatLevelGet(objHnd, featReqCode) < featReqCodeArg){ return 0; }
#pragma endregion 
		} 
		else if (featReqCode == 266)
		{
#pragma region BAB Requirement
			if (templeFuncs.ObjGetBABAfterLevelling(objHnd, classCodeBeingLevelledUp) < featReqCodeArg){ return 0; }
#pragma endregion 
		} 
		else if (featReqCode >= stat_strength && featReqCode <= stat_charisma)
		{
#pragma region Ability Score Requirement
			if (abilityScoreBeingIncreased == featReqCode){ featReqCodeArg--; }
			if ((uint32_t)templeFuncs.ObjStatBaseDispatch(objHnd, featReqCode, nullptr) < featReqCodeArg)
			{
				return 0;
			}

#pragma endregion 
		} 
		else if (featReqCode != 266)
		{
# pragma region Default: Stat requirement
			if ((uint32_t)templeFuncs.ObjStatBaseGet(objHnd, featReqCode) < featReqCodeArg){ return 0; }
#pragma endregion 
		}
		
		// loop
	}

	return 1;
};


uint32_t _RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel)
{
	if (featIdx > 0x289)
	{
		return 0;
	}
	if (newLevel == 10 || newLevel == 13 || newLevel == 16 || newLevel == 19)
	{
		return 1;
	}
	return 0;
};

uint32_t _HasFeatCount(objHndl objHnd, feat_enums featEnum)
{

	uint32_t featCount = 0;
	uint32_t numFeats = templeFuncs.Obj_Get_IdxField_NumItems(objHnd, obj_f_critter_feat_idx);
	for (uint32_t i = 0; i < numFeats; i++)
	{
		if (templeFuncs.Obj_Get_IdxField_32bit(objHnd, obj_f_critter_feat_idx, i) == featEnum)
		{
			featCount++;
		}
	}
	return featCount;
};


uint32_t _FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat)
{
	uint32_t featCount = 0;
	int32_t hasFeatTimes = 0;
	uint32_t i = 0;
	void * ptrOut = listOut;
	while (i < NUM_FEATS)
	{
		hasFeatTimes = _HasFeatCountByClass(objHnd, (feat_enums)i, classBeingLevelled, rangerSpecFeat);

		if (hasFeatTimes && listOut && hasFeatTimes > 0)
		{
			memset(&(listOut[featCount]), i, hasFeatTimes*sizeof(uint32_t));
			featCount += hasFeatTimes;
		}
		i++;
	}
	return featCount;
};

uint32_t _FeatListElective(objHndl objHnd, feat_enums * listOut)
{
	return _FeatListGet(objHnd, listOut, (Stat)0, (feat_enums)0);
};


// WIP:

uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat)
{

	if (feats.featPropertiesTable[(uint32_t)featEnum] & featPropDisabled){ return 0; }

	// race feats
	uint32_t objRace = objects.StatLevelGet(objHnd, stat_race);
	uint32_t * racialFeat = & (feats.racialFeats[objRace * 10]);
	while (*racialFeat != -1)
	{
		if (*racialFeat == featEnum)
		{
			return 1;
		}
		racialFeat++;
	}
	
	
	for (uint32_t i = 0; i < NUM_CLASSES; i++)
	{
		uint32_t classLevel = objects.StatLevelGet(objHnd, charClasses.charClassEnums[i]);
		if (classLevelBeingRaised == charClasses.charClassEnums[i])
		{
			classLevel++;
		}

		uint32_t * classFeat = feats.classFeatTable + 80 * i;
		uint32_t * classFeatStart = classFeat;

		if (classLevel == 0)
		{
			continue;
		}

		while (featEnum != classFeat[0] && classFeat[0] != -1)
		{
			classFeat += 2;
		}
		if (classFeat[0] != -1 && classLevel >= classFeat[1])
		{
			return 1;
		}

	}
	
	
	 if (featEnum == FEAT_IMPROVED_UNCANNY_DODGE)
	 {
		 if (objects.StatLevelGet(objHnd, stat_level_barbarian) >= 2 
			 || objects.StatLevelGet(objHnd, stat_level_rogue) >= 4)
		 {
			 return 1;
		 }
	 }

	 // ranger styles
	 auto rangerLvl = objects.StatLevelGet(objHnd, stat_level_ranger);
	 if (rangerSpecializationFeat){ rangerLvl++; }
	 if (rangerLvl >= 2)
	 {
		 if (_HasFeatCount(objHnd, FEAT_RANGER_ARCHERY_STYLE) || rangerSpecializationFeat == FEAT_RANGER_ARCHERY_STYLE)
		 {
			 auto rangerArcheryFeat = feats.rangerArcheryFeats;
			 while (*rangerArcheryFeat != -1)
			 {
				 if (rangerArcheryFeat[0] == featEnum && rangerLvl >= rangerArcheryFeat[1]){ return 1; }
				 rangerArcheryFeat += 2;
			 }
		 }
		 else if (_HasFeatCount(objHnd, FEAT_RANGER_TWO_WEAPON_STYLE) || rangerSpecializationFeat == FEAT_RANGER_TWO_WEAPON_STYLE)
		 {
			 auto rangerTWFeat = feats.rangerTwoWeaponFeats;
			 while (rangerTWFeat[0] != -1)
			 {
				 if (rangerTWFeat[0] == featEnum && rangerLvl >= rangerTWFeat[1]){ return 1; }
				 rangerTWFeat += 2;
			 }
		 }
	 }

	 // war domain
	 uint32_t objDeity= objects.GetInt32(objHnd, obj_f_critter_deity);
	 uint32_t domain_1 = objects.GetInt32(objHnd, obj_f_critter_domain_1);
	 uint32_t domain_2 = objects.GetInt32(objHnd, obj_f_critter_domain_2);
	 if (domain_1 == 21 || domain_2 == 21) // must be war domain
	 {
		 switch (objDeity)
		 {
		 case DEITY_CORELLON_LARETHIAN:
			 if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSWORD || featEnum == FEAT_WEAPON_FOCUS_LONGSWORD){ return 1; }
		 case DEITY_ERYTHNUL:
			 if (featEnum == FEAT_WEAPON_FOCUS_MORNINGSTAR){ return 1; }
		 case DEITY_GRUUMSH:
			 if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSPEAR || featEnum == FEAT_WEAPON_FOCUS_LONGSPEAR){ return 1; }
		 case DEITY_HEIRONEOUS:
			 if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSWORD || featEnum == FEAT_WEAPON_FOCUS_LONGSWORD){ return 1; }
		 case DEITY_HEXTOR:
			 if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_HEAVY_FLAIL || FEAT_WEAPON_FOCUS_HEAVY_FLAIL){ return 1; }
		 }
	 }

	 // simple weapon prof
	 if (featEnum == FEAT_SIMPLE_WEAPON_PROFICIENCY)
	 {
		 auto monCat = objects.GetCategory(objHnd);
		 if (monCat == mc_type_outsider || monCat == mc_type_monstrous_humanoid
			 || monCat == mc_type_humanoid || monCat == mc_type_giant || monCat == mc_type_fey)
		 {
			 return 1;
		 }
	 } 
	 else if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL)
	 {
		 if ((uint32_t)objects.IsCategoryType(objHnd, mc_type_outsider)
			 && objects.StatLevelGet(objHnd, stat_strength) >= 6)	
		 {	return 1;	 }
	 }
	 else if (featEnum == FEAT_TURN_UNDEAD
		 && (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1 
		     || objects.StatLevelGet(objHnd, stat_level_paladin) >= 4)
		 && objects.GetInt32(objHnd, obj_f_critter_alignment_choice) == 1)
	 {
		 return 1;
	 } 
	 else if (featEnum == FEAT_REBUKE_UNDEAD 
		 && objects.StatLevelGet(objHnd, stat_level_cleric) >= 1 
		 && objects.GetInt32(objHnd, obj_f_critter_alignment_choice) == 2)
	 {
		 return 1;
	 }
	 
	
	return _HasFeatCount(objHnd, featEnum);
}


