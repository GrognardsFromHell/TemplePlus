
#include "stdafx.h"
#include "util/config.h"
#include "feat.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20.h"
#include "tig/tig_mes.h"
#include "common.h"
#include "weapon.h"
#include "critter.h"
#include "tab_file.h"


TabFileStatus featPropertiesTabFile;
uint32_t featPropertiesTable[NUM_FEATS + 1000];
FeatPrereqRow featPreReqTable[NUM_FEATS + 1000];


FeatSystem feats;




class FeatFixes : public TempleFix {
public:
	const char* name() override {
		return "Re-implementation of SpellSlinger's fix for Rogues feats above level 10 in a way that doesn't involve undeclared memory segments";
	}

	void apply() override {
		//writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
		
		replaceFunction(0x1007B990, _FeatExistsInArray);
		replaceFunction(0x1007B9C0, _GetFeatName);
		replaceFunction(0x1007B9D0, _GetFeatDescription);
		replaceFunction(0x1007BA10, _GetFeatPrereqDescription);

		
		
		replaceFunction(0x1007BFA0, FeatInit);
		replaceFunction(0x1007B930, _HasFeatCount);
		
		
		replaceFunction(0x1007BBD0, _IsFeatEnabled);
		replaceFunction(0x1007BBF0, _IsMagicFeat);
		replaceFunction(0x1007BC80, _IsFeatPartOfMultiselect);
		replaceFunction(0x1007BCA0, _IsFeatRacialOrClassAutomatic);
		replaceFunction(0x1007BCC0, _IsExoticWeaponProfFeat);
		replaceFunction(0x1007BCE0, _IsImprovedCriticalFeat);
		replaceFunction(0x1007BD00, _IsMartialWeaponFeat);
		replaceFunction(0x1007BD20, _IsSkillFocusFeat);
		replaceFunction(0x1007BD40, _IsWeaponFinesseFeat);
		replaceFunction(0x1007BD60, _IsWeaponFocusFeat);
		replaceFunction(0x1007BD80, _IsGreaterWeaponFocusFeat);
		replaceFunction(0x1007BDA0, _IsWeaponSpecializationFeat);

		replaceFunction(0x1007BE70, _IsClassFeat);

		replaceFunction(0x1007BF10, _RogueSpecialFeat);

		
		replaceFunction(0x1007BE90, _IsFighterFeat); 
		
		replaceFunction(0x1007C080, _HasFeatCountByClass);
		replaceFunction(0x1007C370, _FeatListGet);
		replaceFunction(0x1007C3F0, _FeatListElective);
		replaceFunction(0x1007C8D0, _WeaponFeatCheckSimpleWrapper);

		replaceFunction(0x1007C8F0, _FeatPrereqsCheck);
		
		//replaceFunction(0x1007C4F0, _WeaponFeatCheck); // usercall bullshit; replaced the functions that used it anyway

		writeHex(0x102C9720, "0F 00 00 00 13 00 00 00");
		writeHex(0x102C9E20, "0F 00 00 00 13 00 00 00"); // (old:BF FF FF FF 0A 00 00 00 <-don't know why, should be rog only, mnk feat is separate)
		writeHex(0x102C9F20, "0F 00 00 00 13 00 00 00");
		writeHex(0x102C9F60, "0F 00 00 00 13 00 00 00");
		writeHex(0x102C9FA0, "0F 00 00 00 13 00 00 00");
		writeHex(0x102C9FE0, "0F 00 00 00 13 00 00 00");
		if (config.newFeatureTestMode)
		{
			char * testbuf[8];
			read(0x102C9E20, testbuf, 8);
			logger->info("New Feature Test: Re-implementing SpellSlinger's Rogue Feat Fix");
		}
		int writeNumFeats = NUM_FEATS;

		// overwrite the iteration limits for a bunch of ui loops
		
		write(0x10182C73 + 2, &writeNumFeats, sizeof(int));


		write(0x101A811D + 2, &writeNumFeats, sizeof(int));
		write(0x101A815D + 2, &writeNumFeats, sizeof(int));
		write(0x101A819D + 2, &writeNumFeats, sizeof(int));

		write(0x101A81DD + 2, &writeNumFeats, sizeof(int));
		write(0x101A821D + 2, &writeNumFeats, sizeof(int));
		write(0x101A825D + 2, &writeNumFeats, sizeof(int));
		write(0x101A829D + 2, &writeNumFeats, sizeof(int));
		write(0x101A82DD + 2, &writeNumFeats, sizeof(int));
		

		write(0x101A8BE1 + 2, &writeNumFeats, sizeof(int));
		write(0x101BBDF4 + 2, &writeNumFeats, sizeof(int)); // charUiFeatList iteration limit
	//	writeHex(0x101A940E, "90 90 90 90 90");
		

		// overwrite the "already taken" limit (jge command)
		writeHex(0x101A87B2, "90 90 90 90     90 90");
	}
};
FeatFixes featFixes;

# pragma region FeatSystem Implementations


FeatSystem::FeatSystem()
{
	//rebase(featPropertiesTable, 0x102BFD78); 	
	//rebase(featPreReqTable, 0x102C07A0); 			// now imported from file :)
	m_featPreReqTable = (FeatPrereqRow *)&featPreReqTable;
	m_featPropertiesTable = (uint32_t*)&featPropertiesTable;
	rebase(featTabLineParser, 0x1007BF80);

	//rebase(featNames, 0x10AB6268);
	rebase(featEnumsMes, 0x10AB6CC8);
	rebase(featMes,   0x10AB7090);
	rebase(featTabFile, 0x10AB7094);
	
	
	rebase(classFeatTable, 0x102CAAF8); 		// TODO: export this to a mesfile
	rebase(charEditorObjHnd, 0x11E741A0);		// TODO: move this to the appropriate system
	rebase(charEditorClassCode, 0x11E72FC0);	// TODO: move this to the appropriate system
	rebase(ToEE_WeaponFeatCheck, 0x1007C4F0);
	rebase(FeatAdd, 0x1007CF30);

	uint32_t _racialFeatsTable[NUM_RACES * 10] = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		FEAT_SIMPLE_WEAPON_PROFICIENCY_ELF, -1, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // table presupposes 10 items on each row, terminator character -1
	uint32_t _rangerArcheryFeats[4 * 2] = { FEAT_RANGER_RAPID_SHOT, 2, FEAT_RANGER_MANYSHOT, 6, FEAT_IMPROVED_PRECISE_SHOT_RANGER, 11, -1, -1 };
	uint32_t _rangerTwoWeaponFeats[4 * 2] = { FEAT_TWO_WEAPON_FIGHTING_RANGER, 2, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, 6, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, 11, - 1, -1 };

	memcpy(racialFeats, _racialFeatsTable, sizeof(racialFeats));
	memcpy(rangerArcheryFeats, _rangerArcheryFeats, sizeof(rangerArcheryFeats));
	memcpy(rangerTwoWeaponFeats, _rangerTwoWeaponFeats, sizeof(rangerTwoWeaponFeats));
	featPropertiesTable[FEAT_GREATER_TWO_WEAPON_FIGHTING] = 0x10;
	featPreReqTable[FEAT_GREATER_TWO_WEAPON_FIGHTING].featPrereqs[2].featPrereqCode = 266;
	featPreReqTable[FEAT_GREATER_TWO_WEAPON_FIGHTING].featPrereqs[2].featPrereqCodeArg = 11;
	classFeatTable->classEntries[0].entries[9].feat = FEAT_GREATER_RAGE;
	classFeatTable->classEntries[0].entries[9].minLvl = 11;
	*(int*)&classFeatTable->classEntries[0].entries[10].feat = -1;
	memset(emptyString, 0, 1);

};

int FeatInit()
{
	if (mesFuncs.Open("mes\\feat.mes", feats.featMes) && mesFuncs.Open("rules\\feat_enum.mes", feats.featEnumsMes) && mesFuncs.Open("tpmes\\feat.mes", &feats.featMesNew))
	{
		memset(feats.featNames, 0, sizeof(feats.featNames));
		tabSys.tabFileStatusInit(feats.featTabFile, feats.featTabLineParser);
		if (tabSys.tabFileStatusBasicFormatter(feats.featTabFile, "rules\\feat.tab"))
		{
			tabSys.tabFileStatusDealloc(feats.featTabFile);
			return 0;
		}

		tabSys.tabFileParseLines(feats.featTabFile);
		MesLine mesLine;
		mesLine.key = 0;
		do
		{
			auto lineFetched = mesFuncs.GetLine(*feats.featMes, &mesLine);
			feats.featNames[mesLine.key] = (char *)(lineFetched != 0 ? mesLine.value : 0);
			if (!lineFetched)
			{
				lineFetched = mesFuncs.GetLine(feats.featMesNew, &mesLine);
				feats.featNames[mesLine.key] = (char *)(lineFetched != 0 ? mesLine.value : 0);
			}
			mesLine.key++;
		} while (mesLine.key < NUM_FEATS);

		tabSys.tabFileStatusInit(&featPropertiesTabFile, featPropertiesTabLineParser);
		if (tabSys.tabFileStatusBasicFormatter(&featPropertiesTabFile, "tprules//feat_properties.tab"))
		{
			tabSys.tabFileStatusDealloc(&featPropertiesTabFile);
		}
		else
		{
			tabSys.tabFileParseLines(&featPropertiesTabFile);
		}


		return 1;
	}
	return 0;
}

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
}

uint32_t FeatSystem::HasFeatCountByClass(objHndl objHnd, feat_enums featEnum)
{
	return _HasFeatCountByClass(objHnd, featEnum, (Stat)0, 0);
};

uint32_t FeatSystem::FeatListGet(objHndl objHnd, feat_enums* listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat)
{
	return _FeatListGet(objHnd, listOut, classBeingLevelled, rangerSpecFeat);
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
}

uint32_t FeatSystem::FeatPrereqsCheck(objHndl objHnd, feat_enums featIdx, feat_enums* featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased)
{
	return _FeatPrereqsCheck(objHnd, featIdx, featArray, featArrayLen, classCodeBeingLevelledUp, abilityScoreBeingIncreased);
}

vector<feat_enums> FeatSystem::GetFeats(objHndl handle) {

	auto featCount = templeFuncs.Obj_Get_IdxField_NumItems(handle, obj_f_critter_feat_idx);
	vector<feat_enums> result(featCount);

	for (int i = 0; i < featCount; ++i) {
		result[i] = (feat_enums) templeFuncs.Obj_Get_IdxField_32bit(handle, obj_f_critter_feat_idx, i);
	}

	return result;
}

char* FeatSystem::GetFeatName(feat_enums feat)
{
	return featNames[feat];
}

char* FeatSystem::GetFeatDescription(feat_enums feat)
{
	char getLineResult; 
	const char *result; 
	MesLine mesLine; 

	mesLine.key = feat + 5000;
	if (feat >= FEAT_NONE
		|| feat == FEAT_IMPROVED_DISARM
		|| feat ==  FEAT_GREATER_WEAPON_SPECIALIZATION)
		getLineResult = mesFuncs.GetLine(feats.featMesNew, &mesLine) == 0;
	else
		getLineResult = mesFuncs.GetLine(*feats.featMes, &mesLine) == 0;
	result = mesLine.value;
	if (getLineResult)
		result = emptyString;
	return (char*)result;
}

char* FeatSystem::GetFeatPrereqDescription(feat_enums feat)
{
	char * result;
	const char *v2;
	char v3; 
	MesLine mesLinePrerequisites;
	MesLine mesLineFeatDesc; 
	MesLine mesLineNone;
	MesHandle * mesHnd = feats.featMes;
	if (feat >= FEAT_NONE
		|| feat == FEAT_IMPROVED_DISARM
		|| feat == FEAT_GREATER_WEAPON_SPECIALIZATION)
		mesHnd = &feats.featMesNew;

	mesLineNone.key = 9998;
	if (mesFuncs.GetLine(*feats.featMes, &mesLineNone)
		&& (mesLinePrerequisites.key = 9999, mesFuncs.GetLine(*feats.featMes, &mesLinePrerequisites))
		&& (mesLineFeatDesc.key = feat + 10000, mesFuncs.GetLine(*mesHnd, &mesLineFeatDesc)))
	{
		v2 = mesLineFeatDesc.value;
		do
			v3 = *v2++;
		while (v3);
		if (v2 == mesLineFeatDesc.value + 1)
			sprintf(featPrereqDescrBuffer, "%s%s", mesLinePrerequisites.value, mesLineNone.value);
		else
			sprintf(featPrereqDescrBuffer, "%s%s", mesLinePrerequisites.value, mesLineFeatDesc.value);
		result = featPrereqDescrBuffer;
	}
	else
	{
		result = emptyString;
	}
	return result;
}

int FeatSystem::IsFeatEnabled(feat_enums feat)
{
	return (m_featPropertiesTable[feat] & 2 ) == 0;
}

int FeatSystem::IsMagicFeat(feat_enums feat)
{
	return (m_featPropertiesTable[feat] & 0x20000 ) != 0;
}

int FeatSystem::IsFeatPartOfMultiselect(feat_enums feat)
{
	return ( m_featPropertiesTable[feat] & 0x100 ) != 0;
}

int FeatSystem::IsFeatRacialOrClassAutomatic(feat_enums feat)
{
	return (m_featPropertiesTable[feat] & 0xC ) != 0;
}

int FeatSystem::IsClassFeat(feat_enums feat)
{
	if (feat > FEAT_NONE && feat < 664)
		return 0;
	return ( m_featPropertiesTable[feat] & 8 ) != 0;
}

int FeatSystem::IsFighterFeat(feat_enums feat)
{
	if (feat > 649 && feat < 664)
	{
		if (feat > 657)
			return 0;
		if (feat != 653)
			return 1;
		return 0;
	}
	return (m_featPropertiesTable[feat] & 0x10 ) != 0;
}

int FeatSystem::IsFeatPropertySet(feat_enums feat, int featProp)
{
	return (m_featPropertiesTable[feat] & featProp) == featProp;
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

const char* _GetFeatName(feat_enums feat)
{
	return feats.GetFeatName(feat);
}

const char* _GetFeatDescription(feat_enums feat)
{
	return feats.GetFeatDescription(feat);
}

const char* _GetFeatPrereqDescription(feat_enums feat)
{
	return feats.GetFeatPrereqDescription(feat);
}

int _IsFeatEnabled(feat_enums feat)
{
	return feats.IsFeatEnabled(feat);
}

int _IsMagicFeat(feat_enums feat)
{
	return feats.IsMagicFeat(feat);
}

int _IsFeatPartOfMultiselect(feat_enums feat)
{
	return feats.IsFeatPartOfMultiselect(feat);
}

int _IsFeatRacialOrClassAutomatic(feat_enums feat)
{
	return feats.IsFeatRacialOrClassAutomatic(feat);
}

int _IsClassFeat(feat_enums feat)
{
	return feats.IsClassFeat(feat);
}

int _IsFighterFeat(feat_enums feat)
{
	return feats.IsFighterFeat(feat);
}

int _IsExoticWeaponProfFeat(feat_enums feat)
{
	int featProp = 0x300;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsImprovedCriticalFeat(feat_enums feat)
{
	int featProp = 0x500;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsMartialWeaponFeat(feat_enums feat)
{
	int featProp = 0x900;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsSkillFocusFeat(feat_enums feat)
{
	int featProp = 0x1100;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsWeaponFinesseFeat(feat_enums feat)
{
	int featProp = 0x2100;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsWeaponFocusFeat(feat_enums feat)
{
	int featProp = 0x4100;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsGreaterWeaponFocusFeat(feat_enums feat)
{
	int featProp = 0x10100;
	return feats.IsFeatPropertySet(feat, featProp);
}

int _IsWeaponSpecializationFeat(feat_enums feat)
{
	int featProp = 0x8100;
	return feats.IsFeatPropertySet(feat, featProp);
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
		if (critterSys.GetRace(objHnd) == race_halforc)
		{
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_gnome_hooked_hammer)
	{
		if (critterSys.GetRace(objHnd) == race_gnome)
		{
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_dwarven_waraxe)
	{
		if (critterSys.GetRace(objHnd) == race_dwarf)
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
	uint32_t featProps = (feats.m_featPropertiesTable)[featIdx];
	FeatPrereqRow * featPrereqs = feats.m_featPreReqTable;
	const uint8_t numCasterClasses = 7;
	uint32_t casterClassCodes[numCasterClasses] = { stat_level_bard, stat_level_cleric, stat_level_druid, stat_level_paladin, stat_level_ranger, stat_level_sorcerer, stat_level_wizard };

	if (featProps & 2 && !(featProps & 524290))
	{
		return 0;
	}

	//return 1; // h4x :)

#pragma region	checking feats in the character editor - SpellSlinger hack for special Rogue feats for level > 10
	if ( *feats.charEditorClassCode != 0 && *feats.charEditorObjHnd != 0)
	{
		auto newClassLvl = objects.StatLevelGet(*feats.charEditorObjHnd, *feats.charEditorClassCode) + 1;

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
	if (featIdx == FEAT_GREATER_TWO_WEAPON_FIGHTING)
	{
		int bpDummy = 1;
	}
	if (featPrereqs[featIdx].featPrereqs[0].featPrereqCode == featReqCodeTerminator){ return 1; };


	for (uint32_t i = 0; featPrereqs[featIdx].featPrereqs[i].featPrereqCode != featReqCodeTerminator; i += 1)
	{
		//disassm notes:
		// eax is classCodeBeingLevelledUp
		// esi is featReqCode
		// ecx is featIdx

		int32_t featReqCode = featPrereqs[featIdx].featPrereqs[i].featPrereqCode;
		auto featReqCodeArg = featPrereqs[featIdx].featPrereqs[i].featPrereqCodeArg;
		uint32_t var_2C = 0;

		

		if (featReqCode == featReqCodeMinCasterLevel)
		{
# pragma region Minimum Caster Level
			for (uint8_t j = 0; j < numCasterClasses; j++)
			{
				
				auto casterLevel = objects.StatLevelGet(objHnd, (Stat)casterClassCodes[j]);
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
			uint32_t critterAlignment = objects.StatLevelGet(objHnd, stat_alignment);
			uint32_t paladinLevel = objects.StatLevelGet(objHnd, stat_level_paladin);
			uint32_t clericLevel = objects.StatLevelGet(objHnd, stat_level_cleric);
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
			auto rogueLevel = objects.StatLevelGet(objHnd, stat_level_rogue);
			auto monkLevel = objects.StatLevelGet(objHnd, stat_level_monk);
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
			auto monkLevel = objects.StatLevelGet(objHnd, stat_level_monk);
			auto barbarianLevel = objects.StatLevelGet(objHnd, stat_level_barbarian);
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
			uint32_t sorcererLevel = objects.StatLevelGet(objHnd, stat_level_sorcerer);
			uint32_t wizardLevel = objects.StatLevelGet(objHnd, stat_level_wizard);
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
			auto rogueLevel = objects.StatLevelGet(objHnd, stat_level_rogue);
			auto barbarianLevel = objects.StatLevelGet(objHnd, stat_level_barbarian);
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
			auto druidLevel = objects.StatLevelGet(objHnd, stat_level_druid);
			auto rangerLevel = objects.StatLevelGet(objHnd, stat_level_ranger);
		

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
			if ((uint32_t)objects.StatLevelGet(objHnd, (Stat)featReqCode) < featReqCodeArg){ return 0; }
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
			for (auto j = 0; j < hasFeatTimes; j++)
			{
				memcpy(&(listOut[featCount + j]), &i, sizeof(uint32_t));

			}
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


uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat)
{

	if (feats.m_featPropertiesTable[(uint32_t)featEnum] & featPropDisabled){ return 0; }

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
		uint32_t classLevel = objects.StatLevelGet(objHnd, d20ClassSys.classEnums[i]);
		if (classLevelBeingRaised == d20ClassSys.classEnums[i])
		{
			classLevel++;
		}

		feat_enums * classFeat = &feats.classFeatTable->classEntries[i].entries[0].feat;
		feat_enums * classFeatStart = classFeat;

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
	 uint32_t objDeity= objects.getInt32(objHnd, obj_f_critter_deity);
	 uint32_t domain_1 = objects.getInt32(objHnd, obj_f_critter_domain_1);
	 uint32_t domain_2 = objects.getInt32(objHnd, obj_f_critter_domain_2);
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
		 auto monCat = critterSys.GetCategory(objHnd);
		 if (monCat == mc_type_outsider || monCat == mc_type_monstrous_humanoid
			 || monCat == mc_type_humanoid || monCat == mc_type_giant || monCat == mc_type_fey)
		 {
			 return 1;
		 }
	 } 
	 else if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL)
	 {
		 if ((uint32_t)critterSys.IsCategoryType(objHnd, mc_type_outsider)
			 && objects.StatLevelGet(objHnd, stat_strength) >= 6)	
		 {	return 1;	 }
	 }
	 else if (featEnum == FEAT_TURN_UNDEAD
		 && (objects.StatLevelGet(objHnd, stat_level_cleric) >= 1 
		     || objects.StatLevelGet(objHnd, stat_level_paladin) >= 4)
		 && objects.getInt32(objHnd, obj_f_critter_alignment_choice) == 1)
	 {
		 return 1;
	 } 
	 else if (featEnum == FEAT_REBUKE_UNDEAD 
		 && objects.StatLevelGet(objHnd, stat_level_cleric) >= 1 
		 && objects.getInt32(objHnd, obj_f_critter_alignment_choice) == 2)
	 {
		 return 1;
	 }
	 
	
	return _HasFeatCount(objHnd, featEnum);
}


uint32_t featPropertiesTabLineParser(TabFileStatus*, uint32_t n, const char** strings)
{
	
	feat_enums feat = (feat_enums)atoi(strings[0]);
	if (feat >= NUM_FEATS || feat < 0)
		return 0;
	uint32_t featProps = atoi(strings[2]);
	featPropertiesTable[feat] = featProps;
	for (int i = 0; i < 8; i++)
	{
		int featPrereqCode = atoi(strings[3 + i * 2]);
		int featPrereqCodeArg = atoi(strings[4 + i * 2]);
		featPreReqTable[feat].featPrereqs[i].featPrereqCode = featPrereqCode;
		featPreReqTable[feat].featPrereqs[i].featPrereqCodeArg = featPrereqCodeArg;
	}
	return 0;
}