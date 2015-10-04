#include "stdafx.h"
#include <obj.h>
#include "python_integration.h"
#include <ai.h>
#include <party.h>
#include <critter.h>
#include "python_object.h"
#include "python_integration_obj.h"
#include <combat.h>
#include <action_sequence.h>
#include <util/fixes.h>

static struct BrawlAddresses : temple::AddressTable {

	BOOL* inProgress;
	BOOL* inResultHandler;
	objHndl* player;
	objHndl* opponent;
	int* state; // Used to detect cheating, win and loss
	void (__cdecl *NoAttackUnset)(objHndl critter);

	void (__cdecl *Unk)(objHndl critter);
	void (__cdecl *ProcessDmgTaken)(objHndl critter, int a2, int a3, int a4);

	BrawlAddresses() {
		rebase(inProgress, 0x10BD01C0);
		rebase(inResultHandler, 0x10BD01D8);
		rebase(player, 0x10BD01C8);
		rebase(opponent, 0x10BD01D0);
		rebase(state, 0x102E7F38);
		rebase(NoAttackUnset, 0x10057830);
		rebase(Unk, 0x100630F0);
		rebase(ProcessDmgTaken, 0x100B8AA0);
	}
} addresses;

static void EnsureConscious(objHndl critter) {
	auto hpCur = objects.StatLevelGet(critter, stat_hp_current);
	auto subdualDamage = objects.getInt32(critter, obj_f_critter_subdual_damage);
	if (subdualDamage > hpCur)
		subdualDamage = hpCur - 1;
	critterSys.SetSubdualDamage(critter, subdualDamage);
	addresses.ProcessDmgTaken(critter, 0, 0, -1);
}

static void GetUpIfProne(objHndl critter) {
	if (!d20Sys.d20Query(critter, DK_QUE_Prone)) {
		return; // Nothing to do
	}

	if (actSeqSys.TurnBasedStatusInit(critter)) {
		actSeqSys.curSeqReset(critter);
		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_STAND_UP, 0);
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();
	}
}

/*
Calls into py00116Tolub brawl_end directly
*/
static void BrawlResult(int state) {
	if (*addresses.inResultHandler || !*addresses.inProgress) {
		return;
	}

	if (state == 0) {
		logger->debug("Brawling result: won");
	} else if (state == 1) {
		logger->debug("Brawling result: lost");
	} else {
		logger->debug("Brawling result: cheated");
	}

	if (state == 0 || state == 1) {
		aiSys.StopAttacking(*addresses.opponent);
		addresses.NoAttackUnset(*addresses.opponent);
		*addresses.inResultHandler = 1;
		addresses.Unk(party.GetLeader()); // Maybe this may recursively call this?
	}
	*addresses.inResultHandler = 0;

	d20Sys.d20SendSignal(*addresses.player, DK_SIG_DealNormalDamage, 1, 0);

	// Make it so the player is not unconscious after the brawl
	EnsureConscious(*addresses.player);

	EnsureConscious(*addresses.opponent);

	auto args = PyTuple_New(3);
	PyTuple_SET_ITEM(args, 0, PyObjHndl_Create(*addresses.opponent));
	PyTuple_SET_ITEM(args, 1, PyObjHndl_Create(*addresses.player));
	PyTuple_SET_ITEM(args, 2, PyInt_FromLong(state));
	auto result = pythonObjIntegration.ExecuteScript("py00116Tolub", "brawl_end", args);
	Py_DECREF(args);
	if (!result) {
		PyErr_Print();
	} else {
		Py_DECREF(result);
	}

	*addresses.inProgress = 0;
	*addresses.state = -1;

	if (!combatSys.isCombatActive()) {
		GetUpIfProne(*addresses.opponent);
		GetUpIfProne(*addresses.player);		
	}
}

static class PythonBrawlIntegrationFix : public TempleFix {
public:
	const char* name() override {
		return "Python Script Integration Extensions (Brawl)";
	}

	void apply() override {
		replaceFunction(0x100EBE20, BrawlResult);
	}

} fix;
