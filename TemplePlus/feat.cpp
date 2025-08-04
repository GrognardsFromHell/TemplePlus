
#include "stdafx.h"
#include "config/config.h"
#include "feat.h"
#include "obj.h"
#include "temple_functions.h"
#include "d20.h"
#include "tig/tig_mes.h"
#include "common.h"
#include "weapon.h"
#include "critter.h"
#include "tab_file.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "ui/ui.h"
#include <infrastructure/elfhash.h>
#include "python/python_integration_feat.h"
#include "d20_race.h"

TabFileStatus featPropertiesTabFile;
uint32_t featPropertiesTable[NUM_FEATS + 1000];
FeatPrereqRow featPreReqTable[NUM_FEATS + 1000];


LegacyFeatSystem feats;

// Re-implementation of SpellSlinger's fix for Rogues feats above level 10 in
// a way that doesn't involve undeclared memory segments
class FeatFixes : public TempleFix {
public:
	void apply() override {
		//writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");

		auto classFeatTable = feats.classFeatTable;
		classFeatTable->classEntries[0].entries[9].feat = FEAT_GREATER_RAGE;
		classFeatTable->classEntries[0].entries[9].minLvl = 11;
		classFeatTable->classEntries[0].entries[10].feat = FEAT_INDOMITABLE_WILL;
		classFeatTable->classEntries[0].entries[10].minLvl = 14;
		classFeatTable->classEntries[0].entries[11].feat = FEAT_TIRELESS_RAGE;
		classFeatTable->classEntries[0].entries[11].minLvl = 17;
		classFeatTable->classEntries[0].entries[12].feat = FEAT_MIGHTY_RAGE;
		classFeatTable->classEntries[0].entries[12].minLvl = 20;
		*(int*)&classFeatTable->classEntries[0].entries[13].feat = -1;
		
		replaceFunction(0x1007B990, _FeatExistsInArray);
		replaceFunction(0x1007B9C0, _GetFeatName);
		replaceFunction(0x1007B9D0, _GetFeatDescription);
		replaceFunction(0x1007BA10, _GetFeatPrereqDescription);
		replaceFunction<const char*(__cdecl)(feat_enums)>(0x1007BAD0, [](feat_enums feat)->const char* {
			return feats.GetFeatHelpTopic(feat);
			});

		
		
		replaceFunction<BOOL(__cdecl)()>(0x1007BFA0, []() {return feats.FeatSystemInit(); });
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
		
		int writeNumFeats = NUM_FEATS;

		// overwrite the iteration limits for a bunch of ui loops
		
		write(0x10182C73 + 2, &writeNumFeats, sizeof(int)); // UiPcCreationFeatsActivate


		write(0x101A811D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A815D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A819D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant

		write(0x101A81DD + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A821D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A825D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A829D + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		write(0x101A82DD + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsMultiselectActivate - now redundant
		

		write(0x101A8BE1 + 2, &writeNumFeats, sizeof(int)); // CharEditorFeatsActive - now redundant
		write(0x101BBDF4 + 2, &writeNumFeats, sizeof(int)); // charUiFeatList iteration limit
	//	writeHex(0x101A940E, "90 90 90 90 90");
		

		// overwrite the "already taken" limit (jge command)
		writeHex(0x101A87B2, "90 90 90 90     90 90");
	}
};
FeatFixes featFixes;

# pragma region LegacyFeatSystem Implementations

// Originally 0x1007CF30; added checkPrereq option
void LegacyFeatSystem::FeatAdd(objHndl handle, feat_enums feat, bool checkPrereq){
	if (!handle)
		return;
	auto obj = objSystem->GetObject(handle);
	if (!obj) 
		return;
	if (checkPrereq && !FeatPrereqsCheck(handle, feat, nullptr, 0, (Stat)0 ,(Stat)0) ){
		std::string featName (GetFeatName(feat));
		logger->warn("Warning: {} does not meet prereqs for feat {} (adding anyway)", handle, featName);
	}
	obj->AppendInt32(obj_f_critter_feat_idx, (int)feat);
}

LegacyFeatSystem::LegacyFeatSystem()
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

	int _racialFeatsTable[VANILLA_NUM_RACES * 10] = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		FEAT_SIMPLE_WEAPON_PROFICIENCY_ELF, -1, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		-1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // table presupposes 10 items on each row, terminator character -1
	int _rangerArcheryFeats[4 * 2] = { FEAT_RANGER_RAPID_SHOT, 2, FEAT_RANGER_MANYSHOT, 6, FEAT_IMPROVED_PRECISE_SHOT_RANGER, 11, -1, -1 };
	int _rangerTwoWeaponFeats[4 * 2] = { FEAT_TWO_WEAPON_FIGHTING_RANGER, 2, FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER, 6, FEAT_GREATER_TWO_WEAPON_FIGHTING_RANGER, 11, - 1, -1 };

	memcpy(racialFeats, _racialFeatsTable, sizeof(_racialFeatsTable));
	memcpy(rangerArcheryFeats, _rangerArcheryFeats, sizeof(_rangerArcheryFeats));
	memcpy(rangerTwoWeaponFeats, _rangerTwoWeaponFeats, sizeof(_rangerTwoWeaponFeats));
	featPropertiesTable[FEAT_GREATER_TWO_WEAPON_FIGHTING] = 0x10;
	featPreReqTable[FEAT_GREATER_TWO_WEAPON_FIGHTING].featPrereqs[2].featPrereqCode = 266;
	featPreReqTable[FEAT_GREATER_TWO_WEAPON_FIGHTING].featPrereqs[2].featPrereqCodeArg = 11;

	memset(emptyString, 0, 1);

	//Add standard metamagic feats to the list
	metamagicFeats.insert(FEAT_EMPOWER_SPELL);
	metamagicFeats.insert(FEAT_ENLARGE_SPELL);
	metamagicFeats.insert(FEAT_EXTEND_SPELL);
	metamagicFeats.insert(FEAT_HEIGHTEN_SPELL);
	metamagicFeats.insert(FEAT_WIDEN_SPELL);
	metamagicFeats.insert(FEAT_MAXIMIZE_SPELL);
	metamagicFeats.insert(FEAT_QUICKEN_SPELL);
	metamagicFeats.insert(FEAT_STILL_SPELL);
	metamagicFeats.insert(FEAT_SILENT_SPELL);

}

/* 0x1007bfa0 */
BOOL LegacyFeatSystem::FeatSystemInit()
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

		// New file-based Feats
		_GetNewFeatsFromFile();

		_CompileParents();
		return 1;
	}

	
	return 0;

}

/* 0x1007b900 */
void LegacyFeatSystem::FeatSystemShutdown(){
	tabSys.tabFileStatusDealloc(feats.featTabFile);
	mesFuncs.Close(*feats.featEnumsMes);
	mesFuncs.Close(*feats.featMes);
	mesFuncs.Close(feats.featMesNew);
}

void LegacyFeatSystem::_GeneratePowerCriticalChildFeats(const NewFeatSpec &parentFeat)
{
	NewFeatSpec childFeat(parentFeat);
	
	//Change common members for the child feat as appropriate
	childFeat.flags = static_cast<FeatPropertyFlag>(FPF_POWER_CRIT_ITEM | FPF_MULTI_SELECT_ITEM | FPF_NON_CORE);
	childFeat.parentId = static_cast<feat_enums>(ElfHash::Hash(parentFeat.name));
	childFeat.prereqs.emplace_back();  //Already have the BAB req from copying the parent feat
	childFeat.prereqs[1].featPrereqCodeArg = 1;

	//Generate each child feat
	for (int type = static_cast<int>(wt_gauntlet); type <= static_cast<int>(wt_ray); type++) {

		//No crits for grapple or net
		if ((type != wt_grapple) && (type != wt_net)) {
			childFeat.weapType = static_cast<WeaponTypes>(type);
			childFeat.name = parentFeat.name;
			childFeat.name += " - ";
			childFeat.name += weapons.GetName(childFeat.weapType);
			// Weapon focus in the same weapon is the other prereq
			childFeat.prereqs[1].featPrereqCode = type + static_cast<int>(FEAT_WEAPON_FOCUS_GAUNTLET) + 1000;
			_AddFeat(childFeat);  //Add the child feat
		}
	}
}

void LegacyFeatSystem::_AddFeat(const NewFeatSpec &featSpec)
{
	if (featSpec.name.size()) {
		auto featId = static_cast<feat_enums>(ElfHash::Hash(featSpec.name));
		Expects(static_cast<uint32_t>(featId) > NUM_FEATS && static_cast<uint32_t>(featId) > 1000); // ensure no collision with the normal ToEE feats, and also > 1000 so that it meshes with the feat prerequisites check
		mNewFeats[featId] = featSpec;
		newFeats.push_back(featId);
	}
}

void LegacyFeatSystem::_GetNewFeatsFromFile()
{
	TioFileList flist;
	tio_filelist_create(&flist, "rules\\feats\\*.txt");

	for (auto i=0; i<flist.count; i++){
		
		auto &f = flist.files[i];
		auto featFile = tio_fopen(fmt::format("rules\\feats\\{}",f.name).c_str(), "rt");

		char lineContent[2000]={0,};
		NewFeatSpec featSpec;
		while (tio_fgets(lineContent, 1000, featFile)){
			auto len = strlen(lineContent);
			if (!len)
				continue;

			if (lineContent[len - 1] == *"\n"){
				lineContent[len - 1] = 0;
			}

			if (!_strnicmp(lineContent, "name:", 5)){
				for (auto ch = lineContent + 5; *ch != 0 ; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					featSpec.name = fmt::format("{}", ch);
					break;
				}
				
			}

			else if (!_strnicmp(lineContent, "flags:", 6)){
				
				for (auto ch = lineContent + 6; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					*(int*)&featSpec.flags |= (FeatPropertyFlag)atol(fmt::format("{}", ch).c_str());
					break;
				}
			}
			else if (!_strnicmp(lineContent, "prereqs:", 8))
			{
				auto prereqCount = 0;
				auto prereqArgCount = 0;
				for (auto ch = lineContent + 8; *ch != 0; ch++)
				{
					if ( *ch == ':' || *ch == ' ' || *ch == '\t')
						continue;
					
					if (prereqArgCount < prereqCount)
					{
						if (featSpec.prereqs.size() <= static_cast<size_t>(prereqArgCount))
							featSpec.prereqs.resize(featSpec.prereqs.size() + 1);
						featSpec.prereqs[prereqArgCount++].featPrereqCodeArg = atol(ch);
					}
					else
					{
						if (featSpec.prereqs.size() <= static_cast<size_t>(prereqCount))
							featSpec.prereqs.resize(featSpec.prereqs.size() + 1);
						featSpec.prereqs[prereqCount++].featPrereqCode = atol(ch);
					}

					// advance till next piece of content
					while (*ch && *ch !=  ' ' && *ch != '\t')
						ch++;
				}
			}
			else if (!_strnicmp(lineContent, "description:", 12))
			{
				for (auto ch = lineContent + 12; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					for (auto chNew = ch; *chNew; chNew++)
					{
						if (*chNew == '\v')
							*chNew = '\n';
					}
					featSpec.description = fmt::format("{}", ch);
					break;
				}
			}

			else if (!_strnicmp(lineContent, "prereq descr:", 13))
			{
				for (auto ch = lineContent + 13; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					featSpec.prereqDescr = fmt::format("{}", ch);
					break;
				}
			}

			else if (!_strnicmp(lineContent, "parent:", 7)){
				for (auto ch = lineContent + 7; *ch != 0; ch++){
					if (*ch == ' ' || *ch == ':')
						continue;
					featSpec.parentId = (feat_enums) ElfHash::Hash(ch);
					break;
				}
			}
		}

		_AddFeat(featSpec);

		//Add the power critical child feats based on the flag
		if (featSpec.flags & FPF_POWER_CRIT_ITEM) {
		    _GeneratePowerCriticalChildFeats(featSpec);
		}

		tio_fclose(featFile);

	}

	tio_filelist_destroy(&flist);
}

void LegacyFeatSystem::_CompileParents(){
	for (auto it: mNewFeats){
		if (it.second.parentId){
			auto findIt = mNewFeats.find(it.second.parentId);

			if (findIt == mNewFeats.end()){
				logger->warn("Missing parent feat for {}", it.second.name);
			} 
			else{
				findIt->second.children.push_back(it.first);
			}
		}
	}
}




uint32_t LegacyFeatSystem::HasFeatCount(objHndl objHnd, feat_enums featEnum)
{
	uint32_t featCount = 0;
	auto obj = objSystem->GetObject(objHnd);
	auto feats = obj->GetInt32Array(obj_f_critter_feat_idx);
	for (size_t i = 0; i < feats.GetSize(); i++) {
		if (feats[i] == featEnum) {
			featCount++;
		}
	}
	return featCount;
}


uint32_t LegacyFeatSystem::HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat, uint32_t newDomain1, 
	uint32_t newDomain2, uint32_t alignmentChoiceNew)
{
	if (!feats.IsFeatEnabled(featEnum))
		return FALSE;


	// Race feats
	auto objRace = critterSys.GetRace(objHnd, false);
	if (d20RaceSys.HasFeat(objRace, featEnum)){
		return TRUE;
	}

	// Class automatic feats
	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;

		auto classLvl = objects.StatLevelGet(objHnd, classEnum);
		if (classLevelBeingRaised == it) {
			classLvl++;
		}

		if (classLvl <= 0)
			continue;

		if (d20ClassSys.HasFeat(featEnum, classEnum, classLvl))
			return TRUE;
	}

	for (uint32_t i = 0; i < VANILLA_NUM_CLASSES; i++) {
		if (d20ClassSys.vanillaClassEnums[i] == stat_level_monk)
			continue; // so it doesn't add the improved trip feat

		uint32_t classLevel = objects.StatLevelGet(objHnd, d20ClassSys.vanillaClassEnums[i]);
		if (classLevelBeingRaised == d20ClassSys.vanillaClassEnums[i]) {
			classLevel++;
		}

		feat_enums * classFeat = &feats.classFeatTable->classEntries[i].entries[0].feat;
		feat_enums * classFeatStart = classFeat;

		if (classLevel == 0) {
			continue;
		}

		while (featEnum != classFeat[0]
			&& classFeat[0] != -1) {
			classFeat += 2;
		}
		if (classFeat[0] != -1 && (int)classLevel >= classFeat[1])
		{
			return 1;
		}

	}

	auto nBarbarianLevel = objects.StatLevelGet(objHnd, stat_level_barbarian);
	auto nRogueLevel = objects.StatLevelGet(objHnd, stat_level_rogue);

	if (classLevelBeingRaised == stat_level_barbarian) {
		nBarbarianLevel++;
	}

	if (classLevelBeingRaised == stat_level_rogue) {
		nRogueLevel++;
	}

	// special casing for uncanny dodge for Brb 2 / Rog 4 combo
	if (featEnum == FEAT_IMPROVED_UNCANNY_DODGE) {
		if (nBarbarianLevel >= 2 && nRogueLevel >= 4)
		{
			return 1;
		}
	}
	
	// ranger styles
	auto rangerLvl = objects.StatLevelGet(objHnd, stat_level_ranger);
	if (classLevelBeingRaised == stat_level_ranger) {
		rangerLvl++;
	}

	if (rangerSpecializationFeat) { rangerLvl++; }
	if (rangerLvl >= 2)
	{
		if (_HasFeatCount(objHnd, FEAT_RANGER_ARCHERY_STYLE) || rangerSpecializationFeat == FEAT_RANGER_ARCHERY_STYLE)
		{
			auto rangerArcheryFeat = feats.rangerArcheryFeats;
			while (*rangerArcheryFeat != -1)
			{
				if (rangerArcheryFeat[0] == featEnum && rangerLvl >= (int)rangerArcheryFeat[1]) { return 1; }
				rangerArcheryFeat += 2;
			}
		}
		else if (_HasFeatCount(objHnd, FEAT_RANGER_TWO_WEAPON_STYLE) || rangerSpecializationFeat == FEAT_RANGER_TWO_WEAPON_STYLE)
		{
			auto rangerTWFeat = feats.rangerTwoWeaponFeats;
			while (rangerTWFeat[0] != -1)
			{
				if (rangerTWFeat[0] == featEnum && rangerLvl >= (int)rangerTWFeat[1]) { return 1; }
				rangerTWFeat += 2;
			}
		}
	}

	// war domain
	auto objDeity = objects.getInt32(objHnd, obj_f_critter_deity);
	auto domain_1 = objects.getInt32(objHnd, obj_f_critter_domain_1);
	auto domain_2 = objects.getInt32(objHnd, obj_f_critter_domain_2);
	if (domain_1 == 0) {
		domain_1 = newDomain1;
	}
	if (domain_2 == 0) {
		domain_2 = newDomain2;
	}
	if (domain_1 == Domain_War || domain_2 == Domain_War){
		switch (objDeity){
		case DEITY_CORELLON_LARETHIAN:
			if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSWORD || featEnum == FEAT_WEAPON_FOCUS_LONGSWORD) { return 1; }
			break;
		case DEITY_ERYTHNUL:
			if (featEnum == FEAT_WEAPON_FOCUS_MORNINGSTAR) { return 1; }
			break;
		case DEITY_GRUUMSH:
			if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSPEAR || featEnum == FEAT_WEAPON_FOCUS_LONGSPEAR) { return 1; }
			break;
		case DEITY_HEIRONEOUS:
			if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_LONGSWORD || featEnum == FEAT_WEAPON_FOCUS_LONGSWORD) { return 1; }
			break;
		case DEITY_HEXTOR:
			if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_HEAVY_FLAIL || featEnum == FEAT_WEAPON_FOCUS_HEAVY_FLAIL) { return 1; }
		}
	}

	auto currentAlignmentChoice = objects.getInt32(objHnd, obj_f_critter_alignment_choice);
	if (currentAlignmentChoice == 0) {
		currentAlignmentChoice = alignmentChoiceNew;
	}

	auto nPaladinLevel = objects.StatLevelGet(objHnd, stat_level_paladin);
	auto nClericLevel = objects.StatLevelGet(objHnd, stat_level_cleric);
	if (classLevelBeingRaised == stat_level_paladin) {
		nPaladinLevel++;
	}
	else if (classLevelBeingRaised == stat_level_cleric) {
		nClericLevel++;
	}

	// simple weapon prof
	if (featEnum == FEAT_SIMPLE_WEAPON_PROFICIENCY) {
		auto obj = gameSystems->GetObj().GetObject(objHnd);
		if (obj->type == obj_t_npc) {
			auto monCat = critterSys.GetCategory(objHnd);
			if (monCat == mc_type_outsider || monCat == mc_type_monstrous_humanoid
				|| monCat == mc_type_humanoid || monCat == mc_type_giant || monCat == mc_type_fey) {
				return 1;
			}
		}

	}
	else if (featEnum == FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL)
	{
		if ((uint32_t)critterSys.IsCategoryType(objHnd, mc_type_outsider)
			&& objects.StatLevelGet(objHnd, stat_strength) >= 6)
		{
			return 1;
		}
	}
	else if (featEnum == FEAT_TURN_UNDEAD && (nClericLevel >= 1 || nPaladinLevel >= 4) && currentAlignmentChoice == 1)
	{
		return 1;
	}
	else if (featEnum == FEAT_REBUKE_UNDEAD && nClericLevel >= 1 && currentAlignmentChoice == 2)
	{
		return 1;
	}


	auto clrLvl = objects.StatLevelGet(objHnd, stat_level_cleric);
	auto palLvl = objects.StatLevelGet(objHnd, stat_level_paladin);
	auto align = objects.getInt32(objHnd, obj_f_critter_alignment_choice);

	return _HasFeatCount(objHnd, featEnum);

}

uint32_t LegacyFeatSystem::HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat)
{
	return HasFeatCountByClass(objHnd, featEnum, classLevelBeingRaised, rangerSpecializationFeat, 0, 0, 0);
}

uint32_t LegacyFeatSystem::HasFeatCountByClass(objHndl objHnd, feat_enums featEnum)
{
	return HasFeatCountByClass(objHnd, featEnum, (Stat)0, 0,0,0,0);
}
bool LegacyFeatSystem::HasMetamagicFeat(objHndl handle)
{
	auto mmFeatList = temple::GetRef<feat_enums[9]>(0x102BFC30);
	for (auto i=0; i< 9; i++){
		if (HasFeatCountByClass(handle, mmFeatList[i])){
			return true;
		}
			
	}
	return false;
}
;

uint32_t LegacyFeatSystem::FeatListGet(objHndl objHnd, feat_enums* listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat)
{
	uint32_t featCount = 0;
	int32_t hasFeatTimes = 0;
	uint32_t i = 0;
	void * ptrOut = listOut;
	while (i < NUM_FEATS){
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
	for (auto feat: newFeats)
	{
		hasFeatTimes = HasFeatCountByClass(objHnd, feat, classBeingLevelled, rangerSpecFeat);
		if (hasFeatTimes && listOut && hasFeatTimes > 0)
		{
			for (auto j = 0; j < hasFeatTimes; j++)
			{
				listOut[featCount++] = feat;

			}
		}
	}
	return featCount;
}


uint32_t LegacyFeatSystem::FeatListElective(objHndl objHnd, feat_enums* listOut)
{
	return FeatListGet(objHnd, listOut, (Stat)0, (feat_enums)0);
}

uint32_t LegacyFeatSystem::FeatExistsInArray(feat_enums featCode, feat_enums * featArray, uint32_t featArrayLen)
{
	for (uint32_t i = 0; i < featArrayLen; i++)
	{
		if (featArray[i] == featCode){ return 1; }
	}
	return 0;
};

WeaponTypes LegacyFeatSystem::GetWeaponType(feat_enums feat){
	if (feat > NUM_FEATS){
		return mNewFeats[feat].weapType;
	}

	if (IsFeatPropertySet(feat, FPF_WEAP_FOCUS_ITEM)){
		return (WeaponTypes)(wt_gauntlet + (feat-FEAT_WEAPON_FOCUS_GAUNTLET));
	}
	else if (IsFeatPropertySet(feat, FPF_GREATER_WEAP_FOCUS_ITEM)){
		return (WeaponTypes)(wt_gauntlet + (feat - FEAT_GREATER_WEAPON_FOCUS_GAUNTLET));
	}
	else if (IsFeatPropertySet(feat, FPF_WEAP_SPEC_ITEM)){
		return (WeaponTypes)(wt_gauntlet + (feat - FEAT_WEAPON_SPECIALIZATION_GAUNTLET));
	}
	else if (IsFeatPropertySet(feat, FPF_GREAT_WEAP_SPEC_ITEM)) {
		return (WeaponTypes)(wt_gauntlet + (feat - FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET));
	}
	else if (IsFeatPropertySet(feat, FPF_IMPR_CRIT_ITEM)) {
		return (WeaponTypes)(wt_gauntlet + (feat - FEAT_IMPROVED_CRITICAL_GAUNTLET));
	}
	else if (IsFeatPropertySet(feat, FPF_MARTIAL_WEAP_ITEM)){
		return (WeaponTypes(wt_throwing_axe + (feat - FEAT_MARTIAL_WEAPON_PROFICIENCY_THROWING_AXE)));
	}
	else if (IsFeatPropertySet(feat, FPF_EXOTIC_WEAP_ITEM)){
		return (WeaponTypes)(wt_halfling_kama + (feat - FEAT_EXOTIC_WEAPON_PROFICIENCY_HALFLING_KAMA));
	}

	return wt_unarmed_strike_medium_sized_being;
}

uint32_t LegacyFeatSystem::WeaponFeatCheck(objHndl objHnd, feat_enums * featArray, uint32_t featArrayLen, Stat classBeingLeveled, WeaponTypes wpnType){
	return _WeaponFeatCheck( objHnd,  featArray,  featArrayLen,  classBeingLeveled,  wpnType);
}

feat_enums LegacyFeatSystem::GetFeatForWeaponType(WeaponTypes wt, feat_enums baseFeat){

	if (baseFeat > NUM_FEATS){
		
		auto ft = mNewFeats[baseFeat];
		for (auto it:ft.children){
			if (mNewFeats[it].weapType == wt){
				return it;
			}
		}
		return FEAT_NONE;
	}

	if (baseFeat == FEAT_NONE){ // assume weapon proficiency
		if (weapons.IsExotic(wt))
			baseFeat = FEAT_EXOTIC_WEAPON_PROFICIENCY;
		else if (weapons.IsMartial(wt))
			baseFeat = FEAT_MARTIAL_WEAPON_PROFICIENCY;
		else
			baseFeat = FEAT_SIMPLE_WEAPON_PROFICIENCY;
	}

	if (baseFeat == FEAT_WEAPON_FOCUS) {
		if (wt >= wt_gauntlet && wt <= wt_ray)
			return (feat_enums)(FEAT_WEAPON_FOCUS_GAUNTLET + (wt - wt_gauntlet));
	}
	else if (baseFeat == FEAT_GREATER_WEAPON_FOCUS){
		if (wt >= wt_gauntlet && wt <= wt_ray)
			return (feat_enums)(FEAT_GREATER_WEAPON_FOCUS_GAUNTLET + (wt - wt_gauntlet));
	}
	else if (baseFeat == FEAT_WEAPON_SPECIALIZATION) {
		if (wt >= wt_gauntlet && wt <= wt_grapple)
			return (feat_enums)(FEAT_WEAPON_SPECIALIZATION_GAUNTLET + (wt - wt_gauntlet));
	}
	else if (baseFeat == FEAT_GREATER_WEAPON_SPECIALIZATION){
		if (wt >= wt_gauntlet && wt <= wt_grapple)
			return (feat_enums)(FEAT_GREATER_WEAPON_SPECIALIZATION_GAUNTLET + (wt - wt_gauntlet));
	}
	else if (baseFeat == FEAT_IMPROVED_CRITICAL){
		if (wt >= wt_gauntlet && wt <= wt_net)
			return (feat_enums)(FEAT_IMPROVED_CRITICAL_GAUNTLET + (wt - wt_gauntlet));
	}
	else if (baseFeat == FEAT_EXOTIC_WEAPON_PROFICIENCY){
		if (wt >= wt_halfling_kama && wt <= wt_net) {
			return (feat_enums)(FEAT_EXOTIC_WEAPON_PROFICIENCY_HALFLING_KAMA + (wt - wt_halfling_kama));
		}
	}
	else if (baseFeat == FEAT_MARTIAL_WEAPON_PROFICIENCY){
		if (wt >= wt_throwing_axe && wt <= wt_composite_longbow){
			return (feat_enums)(FEAT_MARTIAL_WEAPON_PROFICIENCY_THROWING_AXE + (wt - wt_throwing_axe));
		}
	}
	else if (baseFeat == FEAT_SIMPLE_WEAPON_PROFICIENCY){
		return FEAT_SIMPLE_WEAPON_PROFICIENCY;
	}

	return FEAT_NONE;
}

uint32_t LegacyFeatSystem::FeatPrereqsCheck(objHndl objHnd, feat_enums featIdx, feat_enums* featArray, uint32_t featArrayLen, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased){

	FeatPrereqRow * featPrereqs = feats.m_featPreReqTable;

	if (!feats.IsFeatEnabled(featIdx) && !feats.IsFeatMultiSelectMaster(featIdx)) {
		return 0;
	}


	auto isNewFeat = featIdx > NUM_FEATS;

	// checking feats in the character editor - SpellSlinger hack for special Rogue feats for level > 10
	if (classCodeBeingLevelledUp == stat_level_rogue && objHnd && feats.IsFeatPropertySet(featIdx, FPF_ROGUE_BONUS)) {
		auto newClassLvl = objects.StatLevelGet(objHnd, stat_level_rogue) + 1;
		if (newClassLvl == 10 || newClassLvl == 13 || newClassLvl == 16 || newClassLvl == 19) {
			return 1;
		}
	}


	auto i = 0u;
	auto endOfReqs = false;
	int featReqCode, featReqCodeArg;
	

	std::vector<FeatPrereq> prereqs;
	if (isNewFeat){
		prereqs = feats.mNewFeats[featIdx].prereqs;
	} else
	{
		while (featPrereqs[featIdx].featPrereqs[i].featPrereqCode != featReqCodeTerminator){
			prereqs.push_back(featPrereqs[featIdx].featPrereqs[i++]);
		}
	}

	for (auto it: prereqs){

		featReqCode = it.featPrereqCode;
		featReqCodeArg = it.featPrereqCodeArg;
		
		if (featReqCode == featReqCodeTerminator)
			return TRUE;

		if (featReqCode == featReqCodeMinCasterLevel){
			// temporarily add the level to calculate the new caster level
			auto obj = objSystem->GetObject(objHnd);
			auto critterLvls = obj->GetInt32Array(obj_f_critter_level_idx).GetSize();
			obj->SetInt32(obj_f_critter_level_idx, critterLvls, classCodeBeingLevelledUp);
			auto casterLevel = critterSys.GetCasterLevel(objHnd);
			obj->RemoveInt32(obj_f_critter_level_idx, critterLvls); 
			
			

			if (d20ClassSys.IsCastingClass(classCodeBeingLevelledUp, true)){

				auto classLeveledNew = critterSys.GetCasterLevelForClass(objHnd, classCodeBeingLevelledUp) + 1;
				if (classLeveledNew > casterLevel)
					casterLevel = classLeveledNew;
			}
				

			if (casterLevel < featReqCodeArg)
				return FALSE;
		}

		else if (featReqCode == featReqCodeTurnUndeadRelated){
			auto critterAlignment = objects.StatLevelGet(objHnd, stat_alignment);
			auto turnReqLvl = featReqCodeArg;


			auto palLvl = objects.StatLevelGet(objHnd, stat_level_paladin);
			auto paladinTurnLvl = max(0, palLvl + (classCodeBeingLevelledUp == stat_level_paladin) - 3);
			
			auto clericLvl = objects.StatLevelGet(objHnd, stat_level_cleric);
			auto clericTurnLvl = clericLvl + (classCodeBeingLevelledUp == stat_level_cleric) - 0;

			auto otherTurnLvl = max(0, d20Sys.D20QueryPython(objHnd, "Turn Undead Level", 0, classCodeBeingLevelledUp));
			otherTurnLvl += max(0, d20Sys.D20QueryPython(objHnd, "Turn Undead Level", 1, classCodeBeingLevelledUp));
			
			auto highestTurnLvl = clericTurnLvl + paladinTurnLvl + otherTurnLvl;

			if (featIdx == FEAT_TURN_UNDEAD){
				if (!(critterAlignment & ALIGNMENT_EVIL || critterAlignment == (ALIGNMENT_CHAOTIC | ALIGNMENT_LAWFUL)))// TODO: is this a bug??? might be harmless mistake, might be deliberate
				{
					if (highestTurnLvl  < turnReqLvl ) {
						return 0;
					}
				}

			}
			else if (featIdx == FEAT_REBUKE_UNDEAD){
				if (!(critterAlignment > 10 || critterAlignment & ALIGNMENT_GOOD || critterAlignment == 3))	{
					if (highestTurnLvl  < turnReqLvl ) {
						return 0;
					}
				};
			}

			if (highestTurnLvl < turnReqLvl) {
				return 0;
			}
		}

		else if (featReqCode == featReqCodeEvasionRelated){
			auto rogueLevel = objects.StatLevelGet(objHnd, stat_level_rogue);
			auto monkLevel = objects.StatLevelGet(objHnd, stat_level_monk);
			if (featIdx == FEAT_EVASION)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 2
					&& monkLevel + (classCodeBeingLevelledUp == stat_level_monk) < 1
					) {
					return 0;
				}
			}
			else if (featIdx == FEAT_IMPROVED_EVASION)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 10
					&& monkLevel + (classCodeBeingLevelledUp == stat_level_monk) < 9
					) {
					return 0;
				}
			}
		}
		
		else if (featReqCode == featReqCodeFastMovement){
			auto monkLevel = objects.StatLevelGet(objHnd, stat_level_monk);
			auto barbarianLevel = objects.StatLevelGet(objHnd, stat_level_barbarian);
			if (featIdx == FEAT_FAST_MOVEMENT)
			{
				if (barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 1
					&& monkLevel + (classCodeBeingLevelledUp == stat_level_monk) < 3
					) {
					return 0;
				}
			}
		}

		else if (featReqCode == featReqCodeMinArcaneCasterLevel){ // so far only Call Familiar has this
			auto foundOne = false;

			for (auto it: d20ClassSys.classEnumsWithSpellLists){
				if (!d20ClassSys.IsArcaneCastingClass(Stat(it)))
					continue;
				
				auto arcaneLvl = critterSys.GetCasterLevelForClass(objHnd, it);
				if (classCodeBeingLevelledUp == it)  // todo: extend for PRCs that extend caster level. Not critical since only Call Familiar has this, and it's a level 1 affair.
					arcaneLvl++; 

				if (arcaneLvl >= featReqCodeArg){
					foundOne = true;	break;
				}
			}

			if (!foundOne) 
				return 0;
			
		}

		else if (featReqCode == featReqCodeUncannyDodgeRelated){
#pragma region Uncanny Dodge Related
			auto rogueLevel = objects.StatLevelGet(objHnd, stat_level_rogue);
			auto barbarianLevel = objects.StatLevelGet(objHnd, stat_level_barbarian);
			if (featIdx == FEAT_UNCANNY_DODGE)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 3
					&& barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 2
					) {
					return 0;
				}
			}
			else if (featIdx == FEAT_IMPROVED_UNCANNY_DODGE)
			{
				if (rogueLevel + (classCodeBeingLevelledUp == stat_level_rogue) < 8
					&& barbarianLevel + (classCodeBeingLevelledUp == stat_level_barbarian) < 5
					) {
					return 0;
				}
			}
#pragma endregion 
		}
		
		else if (featReqCode == featReqCodeAnimalCompanion){
			auto druidLevel = objects.StatLevelGet(objHnd, stat_level_druid);
			auto rangerLevel = objects.StatLevelGet(objHnd, stat_level_ranger);


			if (featIdx == FEAT_ANIMAL_COMPANION){
				if (druidLevel + (classCodeBeingLevelledUp == stat_level_druid) < 1
					&& rangerLevel + (classCodeBeingLevelledUp == stat_level_ranger) < 4
					) {
					return 0;
				}
			}
		}
		
		else if (featReqCode == featReqCodeCrossbowFeat){
			
			if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_light_crossbow)
		     && !feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_heavy_crossbow)
			 && !feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, wt_hand_crossbow)){
				return 0;
			}	
		}

		else if (featReqCode == featReqCodeWeaponFeat){ // Weapon related feats general
			if (!feats.WeaponFeatCheck(objHnd, featArray, featArrayLen, classCodeBeingLevelledUp, (WeaponTypes)featReqCodeArg))	{
				return 0;
			}
		}

		else if (featReqCode >= 1000 && featReqCode <= 1999){
# pragma region Custom Feat Stat Requirement (?)
			feat_enums featIdxFromReqCode = (feat_enums)(featReqCode - 1000);
			if (!_FeatExistsInArray(featIdxFromReqCode, featArray, featArrayLen))
			{
				if (objects.StatLevelGetBase(objHnd, (Stat)featReqCode) < featReqCodeArg)
				{
					uint32_t stat = 0;
					switch (featIdxFromReqCode){
					case FEAT_TWO_WEAPON_FIGHTING:
						stat = 1000 + FEAT_TWO_WEAPON_FIGHTING_RANGER;
						break;
					case FEAT_RAPID_SHOT:
						stat = 1000 + FEAT_RANGER_RAPID_SHOT;
						break;
					case FEAT_MANYSHOT:
						stat = 1000 + FEAT_RANGER_MANYSHOT;
					case FEAT_IMPROVED_TWO_WEAPON_FIGHTING:
						stat = 1000 + FEAT_IMPROVED_TWO_WEAPON_FIGHTING_RANGER;
					default:
						return FALSE;
					}
					if (objects.StatLevelGetBase(objHnd, (Stat)stat) < featReqCodeArg) { return 0; }
				}
			}
#pragma endregion 
		}

	#pragma region Stats
		// Class Level
		else if (featReqCode >= stat_level_barbarian && featReqCode <= stat_level_shadow_sun_ninja)	{
			if (classCodeBeingLevelledUp == featReqCode) { featReqCodeArg--; }
			if (objects.StatLevelGet(objHnd, (Stat)featReqCode) < featReqCodeArg) { return 0; }
		}
		// BAB
		else if (featReqCode == stat_attack_bonus){
			auto babAfterLvl = critterSys.GetBaseAttackBonus(objHnd, classCodeBeingLevelledUp);
			if (babAfterLvl < featReqCodeArg)
				return 0;
		}
		// Ability Score
		else if (featReqCode >= stat_strength && featReqCode <= stat_charisma){
			if (abilityScoreBeingIncreased == featReqCode) { featReqCodeArg--; }
			Stat abilityStat = (Stat)featReqCode;
			if ( objects.StatLevelGetBaseWithModifiers(objHnd, abilityStat, nullptr) < featReqCodeArg){
				return 0;
			}
		}
		// Default: Stat requirement
		else if (featReqCode != stat_attack_bonus){
			if (objects.StatLevelGetBase(objHnd, (Stat)featReqCode) < featReqCodeArg){
				return 0;
			}
		}
	#pragma endregion


	}

	if (feats.IsFeatPropertySet(featIdx, FPF_CUSTOM_REQ)){
		if (!pyFeatIntegration.CheckPrereq(featIdx, objHnd, classCodeBeingLevelledUp, abilityScoreBeingIncreased))
			return FALSE;
	}

	return TRUE;
};

vector<feat_enums> LegacyFeatSystem::GetFeats(objHndl handle) {

	auto obj = objSystem->GetObject(handle);
	auto feats = obj->GetInt32Array(obj_f_critter_feat_idx);

	auto featCount = feats.GetSize();
	vector<feat_enums> result(featCount);

	for (size_t i = 0; i < featCount; ++i) {
		result[i] = (feat_enums) feats[i];
	}

	return result;
}

char* LegacyFeatSystem::GetFeatName(feat_enums feat)
{
	if (feat > NUM_FEATS) {
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (char*)featFind->second.name.c_str();
	}

	return featNames[feat];
}

char* LegacyFeatSystem::GetFeatDescription(feat_enums feat)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (char*)featFind->second.description.c_str();
	}

	char getLineResult; 
	const char *result; 
	MesLine mesLine(5000 + feat); 

	if (feat >= FEAT_NONE
		|| feat == FEAT_IMPROVED_SHIELD_BASH
		|| feat == FEAT_IMPROVED_DISARM
		|| feat ==  FEAT_GREATER_WEAPON_SPECIALIZATION
		|| feat == FEAT_IMPROVED_SUNDER
		|| feat == FEAT_GREATER_TWO_WEAPON_FIGHTING)
		getLineResult = mesFuncs.GetLine(feats.featMesNew, &mesLine) == 0;
	else
		getLineResult = mesFuncs.GetLine(*feats.featMes, &mesLine) == 0;
	result = mesLine.value;
	if (getLineResult)
		result = emptyString;
	return (char*)result;
}

char* LegacyFeatSystem::GetFeatPrereqDescription(feat_enums feat)
{
	char * result;
	const char *v2;
	char v3; 
	MesLine mesLinePrerequisites(9999);
	MesLine mesLineFeatDesc(10000 + feat); 
	MesLine mesLineNone(9998);
	MesHandle * mesHnd = feats.featMes;

	if (feat >= FEAT_NONE
		|| feat == FEAT_IMPROVED_SHIELD_BASH
		|| feat == FEAT_IMPROVED_DISARM
		|| feat == FEAT_GREATER_WEAPON_SPECIALIZATION
		|| feat == FEAT_IMPROVED_SUNDER
		|| feat == FEAT_STUNNING_FIST
		|| feat == FEAT_GREATER_TWO_WEAPON_FIGHTING)
		mesHnd = &feats.featMesNew;

	
	mesFuncs.GetLine_Safe(*feats.featMes, &mesLineNone);
	mesFuncs.GetLine_Safe(*feats.featMes, &mesLinePrerequisites);

	if (feat > NUM_FEATS) {
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		auto &preDesc = featFind->second.prereqDescr;
		if (!preDesc.size()){
			sprintf(featPrereqDescrBuffer, "%s%s", mesLinePrerequisites.value, mesLineNone.value);
			result = featPrereqDescrBuffer;
		} else
		{
			sprintf(featPrereqDescrBuffer, "%s%s", mesLinePrerequisites.value, preDesc.c_str());
			result = featPrereqDescrBuffer;
		}
		
		return result;
	}

	
	if ( mesFuncs.GetLine(*mesHnd, &mesLineFeatDesc))
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

/* 0x1007BAD0 */
const char * LegacyFeatSystem::GetFeatHelpTopic(feat_enums feat){

	MesLine line(feat+10000);
	if (mesFuncs.GetLine(*featEnumsMes, &line))
		return line.value;
	return "";
}

int LegacyFeatSystem::IsFeatEnabled(feat_enums feat)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & FPF_DISABLED ) == 0;
	}
	return (m_featPropertiesTable[feat] & FPF_DISABLED ) == 0;
}

int LegacyFeatSystem::IsMagicFeat(feat_enums feat)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & FPF_WIZARD_BONUS) != 0;
	}
	return (m_featPropertiesTable[feat] & FPF_WIZARD_BONUS) != 0;
}

void LegacyFeatSystem::AddMetamagicFeat(feat_enums feat)
{
	//Add Metamagic feat to the list
	metamagicFeats.insert(feat);
}

bool LegacyFeatSystem::IsMetamagicFeat(feat_enums feat)
{
	//Search for the feat in the matamgic list
	return metamagicFeats.find(feat) != metamagicFeats.end();
}

std::vector<feat_enums> LegacyFeatSystem::GetMetamagicFeats()
{
	return std::vector(metamagicFeats.begin(), metamagicFeats.end());
}

int LegacyFeatSystem::IsFeatPartOfMultiselect(feat_enums feat)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		if ((featFind->second.flags & FPF_MULTI_SELECT_ITEM) != 0)
			return TRUE;
		return featFind->second.parentId != 0;

	}
	return ( m_featPropertiesTable[feat] & FPF_MULTI_SELECT_ITEM ) != 0;
}

int LegacyFeatSystem::IsFeatRacialOrClassAutomatic(feat_enums feat)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & (FPF_RACE_AUTOMATIC | FPF_CLASS_AUTMATIC)) != 0;
	}
	return (m_featPropertiesTable[feat] & (FPF_RACE_AUTOMATIC | FPF_CLASS_AUTMATIC) ) != 0;
}

int LegacyFeatSystem::IsClassFeat(feat_enums feat){

	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & FPF_CLASS_AUTMATIC) != 0;
	}

	if (feat > FEAT_NONE && feat < 664)
		return 0;
	return ( m_featPropertiesTable[feat] & FPF_CLASS_AUTMATIC ) != 0;
}

int LegacyFeatSystem::IsFighterFeat(feat_enums feat){

	if (feat>NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & FPF_FIGHTER_BONUS) == FPF_FIGHTER_BONUS;
	}

	if (feat > 649 && feat < 664)
	{
		if (feat > 657)
			return 0;
		if (feat != 653)
			return 1;
		return 0;
	}
	return (m_featPropertiesTable[feat] & FPF_FIGHTER_BONUS ) != 0;
}

int LegacyFeatSystem::IsFeatPropertySet(feat_enums feat, int featProp)
{
	if (feat > NUM_FEATS){
		auto featFind = mNewFeats.find(feat);
		if (featFind == mNewFeats.end())
			return FALSE;
		return (featFind->second.flags & featProp) == featProp;
	}
	return (m_featPropertiesTable[feat] & featProp) == featProp;
}

bool LegacyFeatSystem::IsFeatMultiSelectMaster(feat_enums feat){
	if (IsFeatPropertySet(feat, FPF_MULTI_MASTER) != 0)
		return true;

	if (feat > NUM_FEATS){
		if (mNewFeats[feat].children.size() > 0)
			return true;
	}

	return false;
}
feat_enums LegacyFeatSystem::MultiselectGetFirst(feat_enums feat)
{

	if (feat > NUM_FEATS) {
		if (mNewFeats[feat].children.size())
			return (feat_enums)mNewFeats[feat].children[0];
	}

	switch (feat)
	{
	case FEAT_EXOTIC_WEAPON_PROFICIENCY:
		return FEAT_EXOTIC_WEAPON_PROFICIENCY_BASTARD_SWORD;

	case FEAT_IMPROVED_CRITICAL:
		return FEAT_IMPROVED_CRITICAL_BASTARD_SWORD;

	case FEAT_MARTIAL_WEAPON_PROFICIENCY:
		return FEAT_MARTIAL_WEAPON_PROFICIENCY_BATTLEAXE;

	case FEAT_SKILL_FOCUS:
		return FEAT_SKILL_FOCUS_APPRAISE;

	case FEAT_WEAPON_FINESSE:
		return FEAT_WEAPON_FINESSE_BASTARD_SWORD;

	case FEAT_WEAPON_FOCUS:
		return FEAT_WEAPON_FOCUS_BASTARD_SWORD;

	case FEAT_GREATER_WEAPON_FOCUS:
		return FEAT_GREATER_WEAPON_FOCUS_BASTARD_SWORD;

	case FEAT_WEAPON_SPECIALIZATION:
		return FEAT_WEAPON_SPECIALIZATION_BASTARD_SWORD;

	case FEAT_GREATER_WEAPON_SPECIALIZATION:
		return FEAT_GREATER_WEAPON_SPECIALIZATION_BASTARD_SWORD;
	default:
		return feat;
	}
}
void LegacyFeatSystem::MultiselectGetChildren(feat_enums feat, std::vector<feat_enums>& out){

	out.clear();

	if (feat > NUM_FEATS){
		out = mNewFeats[feat].children;
		return;
	}

	// old shit
	auto featIt = FEAT_ACROBATIC;
	auto featProp = FPF_MULTI_SELECT_ITEM;
	switch (feat) {
		case FEAT_EXOTIC_WEAPON_PROFICIENCY:
			featProp = FPF_EXOTIC_WEAP_ITEM;
			break;
		case FEAT_IMPROVED_CRITICAL:
			featProp = FPF_IMPR_CRIT_ITEM;
			break;
		case FEAT_MARTIAL_WEAPON_PROFICIENCY:
			featProp = FPF_MARTIAL_WEAP_ITEM;
			break;
		case FEAT_SKILL_FOCUS:
			featProp = FPF_SKILL_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_FINESSE:
			featProp = FPF_WEAP_FINESSE_ITEM;
			break;
		case FEAT_WEAPON_FOCUS:
			featProp = FPF_WEAP_FOCUS_ITEM;
			break;
		case FEAT_WEAPON_SPECIALIZATION:
			featProp = FPF_WEAP_SPEC_ITEM;
			break;
		case FEAT_GREATER_WEAPON_FOCUS:
			featProp = FPF_GREATER_WEAP_FOCUS_ITEM;
			break;
		case FEAT_GREATER_WEAPON_SPECIALIZATION:
			featProp = FPF_GREAT_WEAP_SPEC_ITEM;
			break;
		default:
			break;
	}

	for (auto ft = 0; ft < NUM_FEATS; ft++) {
		featIt = (feat_enums)ft;
		if (feats.IsFeatPropertySet(featIt, featProp) && feats.IsFeatEnabled(featIt)) {
			out.push_back(featIt);
		}
	}
	
}
bool LegacyFeatSystem::IsNonCore(feat_enums feat)
{
	return IsFeatPropertySet(feat, FPF_NON_CORE) != 0;
}

void LegacyFeatSystem::DoForAllFeats(void(* cb)(int featEnum)){

	for (auto i = 0u; i < NUM_FEATS; i++) {
		auto feat = (feat_enums)i;
		if (feat == FEAT_NONE)
			continue;
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;

		cb(feat);
	}
	for (auto feat : feats.newFeats) {
		if (feat == FEAT_NONE)
			continue;
		if (!feats.IsFeatEnabled(feat) && !feats.IsFeatMultiSelectMaster(feat))
			continue;
		if (!config.nonCoreMaterials && feats.IsNonCore(feat))
			continue;

		cb(feat);
	}
}

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
		if (feats.HasFeatCountByClass(objHnd, FEAT_SIMPLE_WEAPON_PROFICIENCY_MONK, classBeingLeveled, 0))
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

	auto mprof = FEAT_MARTIAL_WEAPON_PROFICIENCY_ALL;
	if (wpnType == wt_orc_double_axe)
	{
		auto martial = feats.HasFeatCountByClass(objHnd, mprof, classBeingLeveled, 0);

		if (config.stricterRulesEnforcement) {
			// orcs treat this as a martial weapon
			if (critterSys.GetSubcategoryFlags(objHnd) & mc_subtype_orc) {
				return martial;
			}
		} else if (critterSys.GetRace(objHnd) == race_halforc) {
			// half-orcs don't actually get proficiency in this for some reason,
			// so only give it if strict rules are off
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_gnome_hooked_hammer)
	{
		// Gnomes treat hooked hammers as martial, not automatic proficiency
		if (critterSys.GetRace(objHnd) == race_gnome)
		{
			if (config.stricterRulesEnforcement)
				return feats.HasFeatCountByClass(objHnd, mprof, classBeingLeveled, 0);
			return 1;
		}
		return 0;
	}
	else if (wpnType == wt_dwarven_waraxe)
	{
		if (critterSys.GetRace(objHnd) == race_dwarf)
		{
			// Dwarves treat 1 handing waraxes as martial, not automatic proficiency
			// no need for martial check because waraxes are always martial for
			// proficiency, exotic is only for one handing.
			return !config.stricterRulesEnforcement;
		}
		return 0;
	} else if (wpnType == wt_grenade)
	{
		return 1;
	}
	else if (d20Sys.D20QueryPython(objHnd, "Proficient with Weapon", static_cast<int>(wpnType))) {  //Python "extra proficiency" support
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
	return feats.FeatPrereqsCheck(objHnd, featIdx, featArray, featArrayLen, classCodeBeingLevelledUp, abilityScoreBeingIncreased);
}

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
	return feats.HasFeatCount(objHnd, featEnum);
};


uint32_t _FeatListGet(objHndl objHnd, feat_enums * listOut, Stat classBeingLevelled, feat_enums rangerSpecFeat)
{
	return feats.FeatListGet(objHnd, listOut, classBeingLevelled, rangerSpecFeat);
};

uint32_t _FeatListElective(objHndl objHnd, feat_enums * listOut)
{
	return _FeatListGet(objHnd, listOut, (Stat)0, (feat_enums)0);
};


uint32_t _HasFeatCountByClass(objHndl objHnd, feat_enums featEnum, Stat classLevelBeingRaised, uint32_t rangerSpecializationFeat){

	return feats.HasFeatCountByClass(objHnd, featEnum, classLevelBeingRaised, rangerSpecializationFeat);
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
