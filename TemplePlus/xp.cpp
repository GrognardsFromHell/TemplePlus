#include "stdafx.h"
#include "util/fixes.h"
#include <temple/dll.h>
#include "temple_functions.h"
#include "config/config.h"
#include "xp.h"
#include "obj.h"
#include "critter.h"
#include "party.h"
#include "d20_level.h"
#include "gamesystems/objects/objsystem.h"
#include "sound.h"
#include "combat.h"
#include "history.h"
#include "float_line.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/particlesystems.h"
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"

temple::GlobalPrimitive<float, 0x102CF708> experienceMultiplier;
temple::GlobalPrimitive<int, 0x10BCA850> numCrittersSlainByCR;
temple::GlobalPrimitive<int, 0x10BCA8BC> xpPile; // sum of all the XP given to party members before divinding by party size

static_assert(CRMIN == -2, "CRMIN for XP award definition should be at most -2!");
static_assert(CRMAX >= 20, "CRMAX for XP award definition should be at least 20!");
static_assert(XPTABLE_MAXLEVEL >= 20, "XPTABLE_MAXLEVEL for XP award definition should be at least 20!");


// static int XPAwardTable[XPTABLE_MAXLEVEL][CRMAX - CRMIN + 1] = {};


BOOL XPAward::XpGainProcess(objHndl handle, int xpGainRaw){


	if (!xpGainRaw || !handle)
		return FALSE;

	auto obj = objSystem->GetObject(handle);
	auto couldAlreadyLevelup = d20LevelSys.CanLevelup(handle);
	
	auto xpReduction = GetMulticlassXpReductionPercent(handle);
	auto xpGain = (int)(1.0 - xpReduction / 100.0)*xpGainRaw;

	std::string text(fmt::format("{} {} {} {}", description.getDisplayName(handle), combatSys.GetCombatMesLine(145), xpGain, combatSys.GetCombatMesLine(146) )); // [obj] gains [xpGain] experience points
	if (xpReduction){
		text.append(fmt::format(" {}", combatSys.GetCombatMesLine(147)));
	}
	histSys.CreateFromFreeText( (text + "\n").c_str());

	auto xpNew = obj->GetInt32(obj_f_critter_experience) + xpGain;
	auto curLvl = objects.StatLevelGet(handle, stat_level);
	
	auto xpCap = d20LevelSys.GetXpRequireForLevel(curLvl + 2) - 1;
	if (curLvl >= config.maxLevel)
		xpCap = d20LevelSys.GetXpRequireForLevel(config.maxLevel);

	if (!config.allowXpOverflow && xpNew > xpCap)
		xpNew = xpCap;

	d20Sys.d20SendSignal(handle, DK_SIG_Experience_Awarded, xpNew, 0);
	obj->SetInt32(obj_f_critter_experience, xpNew);

	if (couldAlreadyLevelup || !d20LevelSys.CanLevelup(handle))
		return FALSE;

	floatSys.FloatCombatLine(handle, 148); // gains a level!
	histSys.CreateFromFreeText(fmt::format("{} {}\n", description.getDisplayName(handle), combatSys.GetCombatMesLine(148)).c_str()); // [obj] gains a level!
	gameSystems->GetParticleSys().CreateAtObj("LEVEL UP", handle);

	return TRUE;
}

int XPAward::GetMulticlassXpReductionPercent(objHndl handle){
	auto highestLvl = -1;
	auto reductionPct = 0;

	if (config.laxRules && config.disableMulticlassXpPenalty)
		return reductionPct;

	for (auto it: d20ClassSys.baseClassEnums){
		auto classEnum = (Stat)it;
		auto classLvl = objects.StatLevelGet(handle, classEnum);
		if (classLvl > highestLvl && !d20Sys.d20QueryWithData(handle, DK_QUE_FavoredClass, classEnum,0)){
			highestLvl = classLvl;
		}
	}

	for (auto it : d20ClassSys.baseClassEnums) {
		auto classEnum = (Stat)it;
		auto classLvl = objects.StatLevelGet(handle, classEnum);
		if (classLvl > 0 && classLvl < highestLvl - 1 && !d20Sys.d20QueryWithData(handle, DK_QUE_FavoredClass, classEnum,0)){
			reductionPct += 20;
		}
	}

	if (reductionPct >= 80)
		reductionPct = 80;

	return reductionPct;
}

XPAward::XPAward(){
	int table[XPTABLE_MAXLEVEL][CRMAX - CRMIN + 1];
	memset(table, 0, sizeof(table));
	
	// First set the table's "spine" - when CR = Level  then   XP = 300*level 
	for (int level = 1; level <= XPTABLE_MAXLEVEL; level++) {
		assert(level >= 1);
		assert(level - CRMIN < CRCOUNT);
		if (!config.slowerLevelling || level < 3)
			table[level - 1][level - CRMIN] = level * 300;
		else
			table[level - 1][level - CRMIN] = (int)(level * 300 * (1 - min(0.66f, 0.66f * powf(level - 2.0f, 0.1f) / powf(16.0f, 0.1f))));
	}

	// Fill out the bottom left portion - CRs less than level - from highest to lowest
	for (int level = 1; level <= XPTABLE_MAXLEVEL; level++){
		for (int j = level - CRMIN - 1; j >= 2; j--){
			int i = level - 1;
			int cr = j + CRMIN;

			assert(i >= 0 && i < XPTABLE_MAXLEVEL);
			assert(j >= 0 && j < CRCOUNT);

			// 8 CRs below level grant nothing
			if (cr <= level - 8){
				table[i][j] = 0;
			}
			else if (cr == 0){
				table[i][2] = table[i][3] / 2; // CR 1/2
				table[i][1] = table[i][3] / 3; // CR 1/3
				table[i][0] = table[i][3] / 4; // CR 1/4
			}
			else if (cr == level - 1) {
				assert(i >= 1);
				if (config.slowerLevelling)
					table[i][j] = min(table[i - 1][j], (table[i][j + 1] * 6) / 11);
				else
					table[i][j] = min(table[i - 1][j], (table[i][j+1] * 2) /3);
			}
			else {
				assert(i >= 1);
				assert(j + 2 < CRCOUNT);
				if (config.slowerLevelling)
					table[i][j] = min(table[i - 1][j], ( table[i][j + 2] * 3) / 10);
				else
					table[i][j] = min(table[i - 1][j], table[i][j + 2] / 2);
			}
		}
	}

	// Fill out the top right portion
	for (int cr_off = 1; cr_off < CRMAX; cr_off++){

		for (int level = 1; level <= XPTABLE_MAXLEVEL && level + cr_off <= CRMAX; level++){
			int i = level - 1;
			int j = level - CRMIN + cr_off;

			assert(i >= 0 && i < XPTABLE_MAXLEVEL);
			assert(j >= 0 && j < CRCOUNT);

			if (cr_off >= 10){
				table[i][j] = table[level - 1][j - 1]; // repeat the last value
			}
			else if (cr_off == 1){
				assert(j >= 1);
				assert(i + 1 < XPTABLE_MAXLEVEL);
				table[i][j] = max((table[i][j - 1] * 3) / 2, table[i + 1][j]);
			}
			else {
				assert(i + 1 < XPTABLE_MAXLEVEL);
				assert(j >= 2);
				table[i][j] = max(table[i][j - 2] * 2, table[i + 1][j]);
			}
		}
	}

	memcpy(XPAwardTable, table, sizeof(table));
};


// XP Table Fix for higher levels
class XPTableForHighLevels : public TempleFix {
public:

	XPAward *xpawarddd;
	void GiveXPAwards();
	void apply() override;
} xpTableFix;

void XPTableForHighLevels::GiveXPAwards(){
	float fNumLivingPartyMembers = 0.0;

	//int XPAwardTable[XPTABLE_MAXLEVEL][CRMAX - CRMIN + 1] = {};

	for (uint32_t i = 0; i < party.GroupPCsLen(); i++){
		objHndl objHndPC = party.GroupPCsGetMemberN(i);
		if (!critterSys.IsDeadNullDestroyed(objHndPC)){
			fNumLivingPartyMembers += 1.0;
		}
	};
	for (uint32_t i = 0; i < party.GroupNPCFollowersLen(); i++){
		objHndl objHndNPCFollower = party.GroupNPCFollowersGetMemberN(i);
		if (!critterSys.IsDeadNullDestroyed(objHndNPCFollower) 
			&& !d20Sys.d20Query(objHndNPCFollower, DK_QUE_ExperienceExempt)){
			fNumLivingPartyMembers += 1.0;
		}
	};
	if (fNumLivingPartyMembers < 0.99){
		return;
	};

	auto killCountByCR = numCrittersSlainByCR.ptr();
	bool bShouldUpdatePartyUI = false;
	int xpForxpPile = 0;

	for (uint32_t i = 0; i < party.GroupListGetLen(); i++){
		objHndl objHnd = party.GroupListGetMemberN(i);

		if (critterSys.IsDeadNullDestroyed(objHnd)){ continue; };
		if (d20Sys.d20Query(objHnd, DK_QUE_ExperienceExempt)) { continue; };
		if (party.ObjIsAIFollower(objHnd)) { continue; };

		int level = objects.StatLevelGet(objHnd, stat_level);
		if (level <= 0) { continue; };

		int xpGainRaw = 0; // raw means it's prior to applying multiclass penalties, which  is Someone Else's Problem :P

		for (int n = 0; n < CR_KILLED_TABLE_SIZE; n++){
			float nkill = (float)killCountByCR[n];
			float xp = (float)xpawarddd->XPAwardTable[level - 1][n];
			if (nkill){
				xpGainRaw += (int)(
					experienceMultiplier *
					nkill *	xp
					);
			}
			
		}
		xpForxpPile += xpGainRaw;
		xpGainRaw = int((float)(xpGainRaw) / fNumLivingPartyMembers);
		//if (templeFuncs.ObjXPGainProcess(objHnd, xpGainRaw)) {
		if (xpawarddd->XpGainProcess(objHnd, xpGainRaw)){
			auto obj = objSystem->GetObject(objHnd);
			if (obj->IsPC() || config.NPCsLevelLikePCs){
				bShouldUpdatePartyUI = true;
			}
		}
	}

	for (int n = 0; n < CR_KILLED_TABLE_SIZE; n++){
		killCountByCR[n] = 0;
	};
	*(xpPile.ptr()) = xpForxpPile;

	if (bShouldUpdatePartyUI){
		uiSystems->GetParty().Update();
		sound.PlaySound(100001); // LEVEL_UP.WAV
		sound.PlaySound(100001); // amp it up a bit
	}

	return;
}



void XPTableForHighLevels::apply() {
	logger->info("Applying XP Table Extension upto Level 20");
	replaceFunction<void(__cdecl)()>(0x100B5700, []() {
		xpTableFix.GiveXPAwards(); }
	);

	xpawarddd = new XPAward();

	if (config.allowXpOverflow)	{
		writeNoops(0x100B5608);
	}
}
