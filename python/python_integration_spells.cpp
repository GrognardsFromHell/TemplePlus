#include "stdafx.h"
#include "python_integration_spells.h"
#include "python_integration_obj.h"
#include "python_spell.h"

PythonSpellIntegration pythonSpellIntegration;

static struct PythonIntegrationSpellsAddresses : AddressTable {

	//100BF0C0
	char** spellEventNames;
	PyObject* (__cdecl * PySpellFromSpellId)(uint32_t spellId);
	void (__cdecl * SpellPacketUpdateFromPySpell)(); //usercall  PySpell * @<eax>
	LocFull* pySpellLoc;
	LocFull* pySpellLoc2;

	PythonIntegrationSpellsAddresses() {
		rebase(spellEventNames, 0x102CFE24);
		rebase(PySpellFromSpellId, 0x100BE880);
		rebase(pySpellLoc, 0x10BCABD8);
		rebase(pySpellLoc2, 0x10BCABF0);
		rebase(SpellPacketUpdateFromPySpell, 0x100BE2C0);
		rebase(SpellSoundPlay, 0x100BF770);
	}

	uint32_t (__cdecl *SpellSoundPlay)(SpellPacketBody* spellPacketBody, SpellEvent pySpellEventCode);
} addresses;

/*
Calls the actual python spell script.
*/
static int SpellTrigger(int spellId, SpellEvent evt) {
	return pythonSpellIntegration.SpellTrigger(spellId, evt);
}

/*
Calls the python spell with more args?
*/
static int SpellTrigger2(int spellId, SpellEvent evt, objHndl handle, int someNumber) {
	return pythonSpellIntegration.SpellTrigger(spellId, evt, handle, someNumber);
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
		replaceFunction(0x100C0390, SpellTrigger2);
		replaceFunction(0x100BEB80, UpdatePythonSpell);
		replaceFunction(0x100BEAF0, RemovePythonSpell);
	}

} fix;

PythonSpellIntegration::PythonSpellIntegration()
	: PythonIntegration("scr\\spell*.py", "(spell(\\d{3}).*)\\.py") {
}

int PythonSpellIntegration::SpellTrigger(int spellId, SpellEvent evt) {
	bool cancelled = 0;

	SpellPacketBody spellPktBody;
	spellSys.GetSpellPacketBody(spellId, &spellPktBody);
	if (evt == SpellEvent::SpellEffect && spellPktBody.targetListNumItems > 0) {
		for (uint32_t i = 0; i < spellPktBody.targetListNumItems; i++) {
			auto tgtObj = spellPktBody.targetListHandles[i];
			// TODO: Verify attachee vs. target here
			if (!pythonObjIntegration.ExecuteObjectScript(spellPktBody.objHndCaster, tgtObj, spellId, ObjScriptEvent::SpellCast)) {
				cancelled = 1;
			}
		}
		if (cancelled) {
			return 1;
		}
	}


	auto pySpell = PySpell_Create(spellId);
	auto args = Py_BuildValue("(O)", pySpell);

	auto spellEnum = spellSys.GetSpellEnumFromSpellId(spellId);
	auto result = RunScript(spellEnum, (EventId)evt, args);
	Py_DECREF(args);

	if (result) {
		/*PySpellLocSet(&pySpell->target_location_full);
		PySpellLoc2Set(&pySpell->target_location_full);*/
		PySpell_UpdatePacket(pySpell);

		Py_DECREF(pySpell);

		addresses.SpellSoundPlay(&spellPktBody, evt);
	}

	return result;
}


//PyObject* PythonSpellSystem::PySpellFromSpellId(uint32_t spellId)
//{
//	return 0; // addresses.PySpellFromSpellId(spellId);
//}
//
//void PythonSpellSystem::PySpellLocSet(LocFull* locFull)
//{
//	//*addresses.pySpellLoc = *locFull;
//}
//
//void PythonSpellSystem::PySpellLoc2Set(LocFull* locFull)
//{
//	//*addresses.pySpellLoc2 = *locFull;
//}
//
//void PythonSpellSystem::SpellPacketUpdateFromPySpell(PySpell* pySpell)
//{
//	macAsmProl
//	__asm{
//		mov ecx, this;
//		mov esi, addresses.SpellPacketUpdateFromPySpell;
//		mov eax, pySpell;
//		call esi;
//	}
//	macAsmEpil
//}
//
//uint32_t PythonSpellSystem::SpellSoundPlay(SpellPacketBody* spellPktBody, PySpellEventCode pySpellEvent)
//{
//	return 0; // addresses.SpellSoundPlay(spellPktBody, pySpellEvent);
//}
//


int PythonSpellIntegration::SpellTrigger(int spellId, SpellEvent evt, objHndl handle, int someNumber) {
	// TODO
	return 0;
}

void PythonSpellIntegration::UpdateSpell(int spellId) {
	PySpell_Update(spellId);
}

void PythonSpellIntegration::RemoveSpell(int spellId) {
	// TODO
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
