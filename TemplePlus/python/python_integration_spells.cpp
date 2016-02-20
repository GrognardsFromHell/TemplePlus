#include "stdafx.h"
#include "python_integration_spells.h"
#include "python_integration_obj.h"
#include "python_spell.h"
#include "python_object.h"
#include <util/fixes.h>

PythonSpellIntegration pythonSpellIntegration;

static struct PythonIntegrationSpellsAddresses : temple::AddressTable {

	//100BF0C0
	char** spellEventNames;
	PyObject* (__cdecl * PySpellFromSpellId)(uint32_t spellId);
	void (__cdecl * SpellPacketUpdateFromPySpell)(); //usercall  PySpell * @<eax>
	LocFull* pySpellLoc;
	LocFull* pySpellLoc2;
	uint32_t(__cdecl *SpellSoundPlay)(SpellPacketBody* spellPacketBody, SpellEvent pySpellEventCode);
	uint32_t(__cdecl *SpellProjectileSoundPlay)(SpellPacketBody* spellPacketBody, SpellEvent pySpellEventCode, objHndl projectile);

	PythonIntegrationSpellsAddresses() {
		rebase(spellEventNames, 0x102CFE24);
		rebase(PySpellFromSpellId, 0x100BE880);
		rebase(pySpellLoc, 0x10BCABD8);
		rebase(pySpellLoc2, 0x10BCABF0);
		rebase(SpellPacketUpdateFromPySpell, 0x100BE2C0);
		rebase(SpellSoundPlay, 0x100BF770);
		rebase(SpellProjectileSoundPlay, 0x100BFBB0);
	}		
} addresses;

/*
Calls the actual python spell script.
*/
static void SpellTrigger(int spellId, SpellEvent evt) {
	pythonSpellIntegration.SpellTrigger(spellId, evt);
}

/*
Calls the python spell with more args?
*/
static void SpellTriggerProjectile(int spellId, SpellEvent evt, objHndl projectile, int targetIdx) {
	pythonSpellIntegration.SpellTriggerProjectile(spellId, evt, projectile, targetIdx);
}

/*
	This is called when a spell packet is being updated in the native code.
	It will update any Python spell packets that are still actively being used
	with the new data from the spell packet.
*/
static void UpdatePythonSpell(int spellId) {
	pythonSpellIntegration.UpdateSpell(spellId);
}

/*
	This is called when a spell packet is being removed, for example
	if the spell ends. This will remove the local references to the
	corresponding Python spell object.
*/
static void RemovePythonSpell(int spellId) {
	pythonSpellIntegration.RemoveSpell(spellId);
}

static class PythonSpellIntegrationFix : public TempleFix {
public:
	const char* name() override {
		return "Python Script Integration Extensions (Spells)";
	}

	void apply() override {
		replaceFunction(0x100C0180, SpellTrigger);
		replaceFunction(0x100C0390, SpellTriggerProjectile);
		replaceFunction(0x100BEB80, UpdatePythonSpell);
		replaceFunction(0x100BEAF0, RemovePythonSpell);
	}

} fix;

PythonSpellIntegration::PythonSpellIntegration()
	: PythonIntegration("scr\\spell*.py", "(spell(\\d{3}).*)\\.py") {
}

void PythonSpellIntegration::SpellTrigger(int spellId, SpellEvent evt) {
	bool cancelled = 0;

	SpellPacketBody spellPktBody;
	if (!spellSys.GetSpellPacketBody(spellId, &spellPktBody)) {
		logger->warn("Trying to trigger {} for spell id {}, which is invalid.", GetFunctionName((EventId)evt), spellId);
		return;
	}

	if (!spellSys.IsSpellActive(spellId)) {
		logger->warn("Trying to trigger {} for spell id {}, which is inactive.", GetFunctionName((EventId)evt), spellId);
		return;
	}

	if (evt == SpellEvent::SpellEffect && spellPktBody.targetCount > 0) {
		for (uint32_t i = 0; i < spellPktBody.targetCount; i++) {
			auto tgtObj = spellPktBody.targetListHandles[i];
			// TODO: Verify attachee vs. target here
			if (!pythonObjIntegration.ExecuteObjectScript(spellPktBody.caster, tgtObj, spellId, ObjScriptEvent::SpellCast)) {
				cancelled = 1;
			}
		}
		if (cancelled) {
			return;
		}
	}


	auto pySpell = PySpell_Create(spellId);
	auto args = Py_BuildValue("(O)", pySpell);

	auto spellEnum = spellSys.GetSpellEnumFromSpellId(spellId);
	auto result = RunScript(spellEnum, (EventId)evt, args);
	Py_DECREF(args);

	// Result of RunScript is a bit different here
	if (result == 0 || result == -1) {
		PySpell_UpdatePacket(pySpell);
		addresses.SpellSoundPlay(&spellPktBody, evt);
	}
	Py_DECREF(pySpell);
}

void PythonSpellIntegration::SpellTriggerProjectile(int spellId, SpellEvent evt, objHndl projectile, int targetIdx) {

	SpellPacketBody spellPktBody;
	if (!spellSys.GetSpellPacketBody(spellId, &spellPktBody)) {
		logger->warn("Trying to trigger {} for spell id {}, which is invalid.", GetFunctionName((EventId)evt), spellId);
		return;
	}

	if (!spellSys.IsSpellActive(spellId)) {
		logger->warn("Trying to trigger {} for spell id {}, which is inactive.", GetFunctionName((EventId)evt), spellId);
		return;
	}

	auto pySpell = PySpell_Create(spellId);
	auto projectileObj = PyObjHndl_Create(projectile);
	auto args = Py_BuildValue("(OOi)", pySpell, projectileObj, targetIdx);
	Py_DECREF(projectileObj);

	auto spellEnum = spellSys.GetSpellEnumFromSpellId(spellId);
	auto result = RunScript(spellEnum, (EventId)evt, args);
	Py_DECREF(args);

	// The meaning of the results is different from obj scripts
	if (result == -1 || result == 0) {
		PySpell_UpdatePacket(pySpell);

		spellSys.GetSpellPacketBody(spellId, &spellPktBody);
		addresses.SpellProjectileSoundPlay(&spellPktBody, evt, projectile);
	}
	
	Py_DECREF(pySpell);
}

void PythonSpellIntegration::UpdateSpell(int spellId) {
	PySpell_Update(spellId);
}

void PythonSpellIntegration::RemoveSpell(int /*spellId*/) {
	/*
		As soon as the last ref to the PySpell object goes away, the
		spell will automatically be removed from the active spell list.
	*/
}

static const char* spellEventNames[] = {
	"OnSpellEffect",
	"OnBeginSpellCast",
	"OnEndSpellCast",
	"OnBeginRound",
	"OnEndRound",
	"OnBeginProjectile",
	"OnEndProjectile",
	"OnBeginRoundD20Ping",
	"OnEndRoundD20Ping",
	"OnAreaOfEffectHit",
	"OnSpellStruck"
};

const char* PythonSpellIntegration::GetFunctionName(EventId eventId) {
	return spellEventNames[eventId];
}
