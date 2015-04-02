
#include "stdafx.h"
#include "fixes.h"
#include "feat.h"
#include "obj.h"
#include "temple_functions.h"

GlobalPrimitive<uint32_t, 0x102BFD78> featPropertiesTable;
GlobalPrimitive<uint32_t, 0x102C07A0> featPreReqTable;
GlobalPrimitive<objHndl, 0x11E741A0> charEditorObjHnd;
GlobalPrimitive<Stat, 0x11E72FC0> charEditorClassCode;

uint32_t FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen);
uint32_t FeatSthg_sub_1007C4F0(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat eax_objStat, uint32_t ebx_n1);
uint32_t ObjCheckFeatPrereqs(objHndl, feat_enums featIdx, feat_enums*, uint32_t, Stat, Stat);
uint32_t RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel);


class FeatFixes : public TempleFix {
public:
	const char* name() override {
		return "Re-implementation of SpellSlinger's fix for Rogues feats above level 10 in a way that doesn't involve undeclared memory segments";
	}

	void apply() override {
		//writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
		replaceFunction(0x1007C8F0, ObjCheckFeatPrereqs);
		replaceFunction(0x1007B990, FeatExistsInArray);
		replaceFunction(0x1007BF10, RogueSpecialFeat);
		
	}
} featPrereqFix;

struct FeatFuncs : AddressTable
{
	void (__cdecl *_FeatSthg_sub_1007C4F0)();
	FeatFuncs()
	{
		rebase(_FeatSthg_sub_1007C4F0, 0x1007C4F0);
	}
} featFuncs;

uint32_t FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen)
{
	for (uint32_t i = 0; i < featArrayLen; i++)
	{
		if (featArray[i] == featCode){ return 1; }
	}
	return 0;
};

uint32_t ObjCheckFeatPrereqs(objHndl objHnd, feat_enums featIdx, feat_enums * featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased)
{
	uint32_t * featPropsTable = featPropertiesTable.ptr();
	uint32_t featProps = featPropsTable[featIdx];
	uint32_t * featPrereqs = featPreReqTable.ptr();
	const uint8_t numCasterClasses = 7;
	uint32_t casterClassCodes[numCasterClasses] = { stat_level_bard, stat_level_cleric, stat_level_druid, stat_level_paladin, stat_level_ranger, stat_level_sorcerer, stat_level_wizard };

	if (featProps  & 2)
	{
		return 0;
	}

	// return 1; // h4x :)


//	checking feats in the character editor
	if (charEditorClassCode != 0 && charEditorObjHnd != 0)
	{
		auto newClassLvl = templeFuncs.ObjStatLevelGet(charEditorObjHnd, charEditorClassCode) + 1;

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

		//
		
		
	/*

	11EB6380    83EC 30         SUB ESP, 30
	11EB6383    833D C02FE711 0> CMP DWORD PTR DS : [11E72FC0], 0
	11EB638A    74 71           JE SHORT temple.11EB63FD
	11EB638C    833D A441E711 0>CMP DWORD PTR DS : [11E741A4], 0
	11EB6393    74 68           JE SHORT temple.11EB63FD
	11EB6395    833D A041E711 0>CMP DWORD PTR DS : [11E741A0], 0
	11EB639C    74 5F           JE SHORT temple.11EB63FD     / continue as usual
	11EB639E    FF35 C02FE711   PUSH DWORD PTR DS : [11E72FC0]
	11EB63A4    FF35 A441E711   PUSH DWORD PTR DS : [11E741A4]
	11EB63AA    FF35 A041E711   PUSH DWORD PTR DS : [11E741A0]
	11EB63B0    E8 4BE41BFE     CALL temple.10074800

	11EB63B5    40              INC EAX
	11EB63B6    8BC8            MOV ECX, EAX
	11EB63B8    58              POP EAX
	11EB63B9    58              POP EAX
	11EB63BA    58              POP EAX
	11EB63BB    837C24 48 0F    CMP DWORD PTR SS : [ESP + 48], 0F
	11EB63C0    75 3B           JNZ SHORT temple.11EB63FD
	11EB63C2    83F9 0A         CMP ECX, 0A
	11EB63C5    74 11           JE SHORT temple.11EB63D8
	11EB63C7    83F9 0D         CMP ECX, 0D
	11EB63CA    74 0C           JE SHORT temple.11EB63D8
	11EB63CC    83F9 10         CMP ECX, 10
	11EB63CF    74 07           JE SHORT temple.11EB63D8
	11EB63D1    83F9 13         CMP ECX, 13
	11EB63D4    74 02           JE SHORT temple.11EB63D8
	11EB63D6    EB 25           JMP SHORT temple.11EB63FD
	11EB63D8    8B4C24 3C       MOV ECX, DWORD PTR SS : [ESP + 3C]
	11EB63DC    3E : 8B048D 78FD2>MOV EAX, DWORD PTR DS : [ECX * 4 + 102BFD78]
	11EB63E4    81E0 00000400   AND EAX, 40000
	11EB63EA    81E8 00000400   SUB EAX, 40000
	11EB63F0    F7D8            NEG EAX
	11EB63F2    1BC0            SBB EAX, EAX
	11EB63F4    40              INC EAX
	11EB63F5    85C0            TEST EAX, EAX
	11EB63F7    74 08           JE SHORT temple.11EB6401
	11EB63F9    83C4 30         ADD ESP, 30
	11EB63FC    C3              RETN
	11EB63FD    8B4C24 3C       MOV ECX, DWORD PTR SS : [ESP + 3C]
	11EB6401 - E9 EF641CFE     JMP temple.1007C8F5

	*/




	uint32_t initOffset = featIdx * 16;

	if ( featPrereqs[initOffset]  == featReqCodeTerminator){ return 1; };


	for (uint32_t i = 0; featPrereqs[initOffset + i] != featReqCodeTerminator; i+=2)
	{
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
			if (!FeatSthg_sub_1007C4F0(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, FEAT_CRAFT_MAGIC_ARMS_AND_ARMOR))
			{
				if (!FeatSthg_sub_1007C4F0(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, FEAT_CRAFT_WAND))
				{
					if (!FeatSthg_sub_1007C4F0(objHnd, featArray, featArrayLen, (Stat)0, FEAT_GREATER_WEAPON_FOCUS_GAUNTLET))
					{
						return 0;
					}
				}
			}
		}
		else if (featReqCode == -4)
		{
			if (!FeatSthg_sub_1007C4F0(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, featReqCodeArg))
			{
				return 0;
			}
		} else if( featReqCode >= 1000 && featReqCode <= 1999)
		{
# pragma region Custom Feat Stat Requirement (?)
			feat_enums featIdxFromReqCode = (feat_enums)(featReqCode - 1000);
			if (!FeatExistsInArray(featIdxFromReqCode, featArray, featArrayLen))
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


uint32_t RogueSpecialFeat(feat_enums featIdx, uint32_t newLevel)
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
/*
11EB7F00    8B4424 08       MOV EAX,DWORD PTR SS:[ESP+8]
11EB7F04    3D 89020000     CMP EAX,289
11EB7F09    7F 19           JG SHORT temple.11EB7F24
11EB7F0B    3E:8B4424 0C    MOV EAX,DWORD PTR DS:[ESP+C]
11EB7F10    83F8 0A         CMP EAX,0A
11EB7F13    74 12           JE SHORT temple.11EB7F27
11EB7F15    83F8 0D         CMP EAX,0D
11EB7F18    74 0D           JE SHORT temple.11EB7F27
11EB7F1A    83F8 10         CMP EAX,10
11EB7F1D    74 08           JE SHORT temple.11EB7F27
11EB7F1F    83F8 13         CMP EAX,13
11EB7F22    74 03           JE SHORT temple.11EB7F27
11EB7F24    33C0            XOR EAX,EAX
11EB7F26    C3              RETN
11EB7F27    B8 01000000     MOV EAX,1
11EB7F2C    C3              RETN*/


uint32_t __declspec(naked) FeatSthg_sub_1007C4F0(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat eax_objStat, uint32_t ebx_n1)
{ // objStat goes into eax,  n1 goes into ebx
	__asm {/* stack frame
		   arg4	  // esp0 - 0x20
		   arg8      // esp0 - 0x1C
		   argC      // esp0 - 0x18
		   arg10	  // esp0 - 0x14
		   ebx       // esp0 - 0x10
		   ecx
		   esi
		   edi
		   retaddr    esp0
		   arg4	objHnd
		   arg8
		   argC	featArray
		   arg10	featArrayLen
		   arg14   objStat  esp0+0x14
		   arg18   n1       esp0+0x18
		   */
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
		mov esi, featFuncs._FeatSthg_sub_1007C4F0;
		call esi; // esp0 - 0x20
		add esp, 0x10;
		pop ebx;
		pop ecx;
		pop esi;
		pop edi;
		ret;

	}
};
