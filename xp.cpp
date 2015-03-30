#include "stdafx.h"
#include "fixes.h"
#include "addresses.h"
#include "temple_functions.h"
#include "config.h"
#include "xp.h"

GlobalPrimitive<float, 0x102CF708> experienceMultiplier;
GlobalPrimitive<int, 0x10BCA850> numCrittersSlainByCR;
GlobalPrimitive<int, 0x10BCA8BC> xpPile; // sum of all the XP given to party members before divinding by party size


static_assert(CRMIN == -2, "CRMIN for XP award definition should be at most -2!");
static_assert(CRMAX >= 20, "CRMAX for XP award definition should be at least 20!");
static_assert(MAXLEVEL >= 20, "MAXLEVEL for XP award definition should be at least 20!");


// static int XPAwardTable[MAXLEVEL][CRMAX - CRMIN + 1] = {};



XPAward::XPAward(){
	int table[MAXLEVEL][CRMAX - CRMIN + 1];
	memset(table, 0, sizeof(table));

	// First set the table's "spine" - when CR = Level  then   XP = 300*level 
	for (int level = 1; level <= MAXLEVEL; level++) {
		assert(level >= 1);
		assert(level - CRMIN < CRCOUNT);
		table[level - 1][level - CRMIN] = level * 300;
	}

	// Fill out the bottom left portion
	for (int level = 1; level <= MAXLEVEL; level++){
		for (int j = level - CRMIN - 1; j >= 2; j--){
			int i = level - 1;
			int cr = j + CRMIN;

			assert(i >= 0 && i < MAXLEVEL);
			assert(j >= 0 && j < CRCOUNT);

			if (cr <= level - 8){
				table[i][j] = 0;
			}
			else if (cr == 0){
				table[i][2] = table[i][3] / 2;
				table[i][1] = table[i][3] / 3;
				table[i][0] = table[i][3] / 4;
			}
			else if (cr == level - 1) {
				assert(i >= 1);
				table[i][j] = min(table[i - 1][j], level * 200);
			}
			else {
				assert(i >= 1);
				assert(j + 2 < CRCOUNT);
				table[i][j] = min(table[i - 1][j], table[i][j + 2] / 2);
			}
		}
	}

	// Fill out the top right portion
	for (int cr_off = 1; cr_off < CRMAX; cr_off++){

		for (int level = 1; level <= MAXLEVEL && level + cr_off <= CRMAX; level++){
			int i = level - 1;
			int j = level - CRMIN + cr_off;

			assert(i >= 0 && i < MAXLEVEL);
			assert(j >= 0 && j < CRCOUNT);

			if (cr_off >= 10){
				table[i][j] = table[level - 1][j - 1]; // repeat the last value
			}
			else if (cr_off == 1){
				assert(j >= 1);
				assert(i + 1 < MAXLEVEL);
				table[i][j] = max((table[i][j - 1] * 3) / 2, table[i + 1][j]);
			}
			else {
				assert(i + 1 < MAXLEVEL);
				assert(j >= 2);
				table[i][j] = max(table[i][j - 2] * 2, table[i + 1][j]);
			}
		}
	}

	memcpy(XPAwardTable, table, sizeof(table));
};


XPAward xpawarddd;

void GiveXPAwards(){
	float fNumLivingPartyMembers = 0.0;

	//int XPAwardTable[MAXLEVEL][CRMAX - CRMIN + 1] = {};

	for (uint32_t i = 0; i < templeFuncs.GroupPCsLen(); i++){
		objHndl objHndPC = templeFuncs.GroupPCsGetMemberN(i);
		if (!templeFuncs.IsObjDeadNullDestroyed(objHndPC)){
			fNumLivingPartyMembers += 1.0;
		}
	};
	for (uint32_t i = 0; i < templeFuncs.GroupNPCFollowersLen(); i++){
		objHndl objHndNPCFollower = templeFuncs.GroupNPCFollowersGetMemberN(i);
		if (!templeFuncs.IsObjDeadNullDestroyed(objHndNPCFollower)
			&& !templeFuncs.DispatcherD20Query(objHndNPCFollower, DK_QUE_ExperienceExempt)){
			fNumLivingPartyMembers += 1.0;
		}
	};
	if (fNumLivingPartyMembers < 0.99){
		return;
	};

	auto killCountByCR = numCrittersSlainByCR.ptr();
	bool bShouldUpdatePartyUI = false;
	int xpForxpPile = 0;

	for (uint32_t i = 0; i < templeFuncs.GroupListGetLen(); i++){
		objHndl objHnd = templeFuncs.GroupListGetMemberN(i);
		if (templeFuncs.IsObjDeadNullDestroyed(objHnd)){ continue; };
		if (templeFuncs.DispatcherD20Query(objHnd, DK_QUE_ExperienceExempt)) { continue; };
		if (templeFuncs.ObjIsAIFollower(objHnd)) { continue; };

		int level = templeFuncs.ObjStatGet(objHnd, stat_level);
		if (level <= 0) { continue; };

		int xpGainRaw = 0; // raw means it's prior to applying multiclass penalties, which  is Someone Else's Problem :P

		for (int n = 0; n < CRMAX - CRMIN + 1; n++){
			float nkill = (float)killCountByCR[n];
			float xp = (float)xpawarddd.XPAwardTable[level - 1][n];
			if (nkill){
				xpGainRaw += (int)(
					experienceMultiplier *
					nkill *	xp
					);
			}
			
		}
		xpForxpPile += xpGainRaw;
		xpGainRaw = int((float)(xpGainRaw) / fNumLivingPartyMembers);
		if (templeFuncs.ObjXPGainProcess(objHnd, xpGainRaw)){
			if (templeFuncs.Obj_Get_Field_32bit(objHnd, obj_f_type) == obj_t_pc || config.NPCsLevelLikePCs){
				bShouldUpdatePartyUI = true;
			}
		}
	}

	for (int n = 0; n < CRMAX - CRMIN + 1; n++){
		killCountByCR[n] = 0;
	};
	*(xpPile.ptr()) = xpForxpPile;

	if (bShouldUpdatePartyUI){
		templeFuncs.UpdatePartyUI();
	}

	return;
}

class XPTableForHighLevels : public TempleFix {
public:
	const char* name() override {
		return "XP Table Fix for higher levels";
	}
	void apply() override;
} xpTableFix;

void XPTableForHighLevels::apply() {
	logger->info("Applying XP Table Extension upto Level 20");
	replaceFunction(0x100B5700, GiveXPAwards);

}
