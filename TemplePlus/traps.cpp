
#include "stdafx.h"
#include "traps.h"
#include <util/fixes.h>
#include <tig\tig_tabparser.h>
#include <python/python_integration_obj.h>

Traps traps;

static struct TrapsAddresses : temple::AddressTable {

	const Trap* (__cdecl *GetById)(int id);
	const Trap* (__cdecl *GetByName)(const char *name);
	const Trap* (__cdecl *GetByObj)(objHndl handle);
	D20CAF (__cdecl *Attack)(objHndl target, int attackBonus, int criticalHitRangeStart, BOOL ranged);

	TrapsAddresses() {
		rebase(GetById, 0x10050D20);
		rebase(GetByName, 0x10050D50);
		rebase(GetByObj, 0x10051030);
		rebase(Attack, 0x100B67F0);
	}
} addresses;

const Trap* Traps::GetById(int id) {
	return addresses.GetById(id);
}

const Trap* Traps::GetByName(const char* name) {
	return addresses.GetByName(name);
}

const Trap* Traps::GetByObj(objHndl handle) {
	return addresses.GetByObj(handle);
}

D20CAF Traps::Attack(objHndl target, int attackBonus, int criticalHitRangeStart, BOOL ranged) {
	return addresses.Attack(target, attackBonus, criticalHitRangeStart, ranged);
}


static class TrapHooks : public TempleFix {
public:

	static int TrapLineParser(const TigTabParser* parser, int lineIdx, char** content) {
		auto trapSpecs = temple::GetRef<Trap*>(0x10AA329C);
		auto& trapCount = temple::GetRef<int>(0x11E61524);
		auto &trap = trapSpecs[trapCount];

		if (*content[0] == '\0')
			return 17;

		trap.name = _strdup(content[0]);
		// Trigger Type
		ObjScriptEvent triggerType = ObjScriptEvent::Examine;
		for (int i = 0; i < (int) ObjScriptEvent::Count; ++i) {
			auto eventName = (ObjScriptEvent)i;
			auto scriptEventName = pythonObjIntegration.GetEventName(eventName);
			if (!_stricmp(scriptEventName.c_str(), content[1]) ){
				trap.trigger = eventName;
				break;
			}
		}

		// parse trap flags
		if (!temple::GetRef<BOOL(__cdecl)(const char*, int*)>(0x10050B50)(content[2], &trap.flags)) {
			return 17;
		}
		
		trap.partSysName = _strdup(content[3]);
		trap.searchDC = atol(content[4]);
		trap.disableDC= atol(content[5]);
		trap.replaceWith = _strdup(content[6]);

		trap.damageCount = 0;
		static auto parseTrapDamage = temple::GetRef<BOOL(__cdecl)(char*, TrapDamage*)>(0x10050AB0);
		for (int i = 0; i < 5; ++i) {
			if (!content[7 + i][0])
				break;

			if (!parseTrapDamage(content[7 + i], &trap.damage[i]))
				return 17;
		}
		trap.CR = atol(content[12]);
		trap.id = atol(content[13]);
		return 0;
	}

	void apply() override {
		
		replaceFunction<BOOL(__cdecl)(const GameSystemConf&)>(0x10050DA0, [](const GameSystemConf& conf)->BOOL {

			if (!mesFuncs.Open("mes\\trap.mes", temple::GetPointer<MesHandle>(0x10AA32A0))) {
				return FALSE;
			}

			//static auto trapsTabParser = temple::GetRef<TigTabLineParser>(0x10050C00);
			TigTabParser tabParser;
			tabParser.Init(TrapLineParser /*trapsTabParser*/);
			tabParser.Open("Rules\\traps.tab");

			auto &trapSpecs = temple::GetRef<Trap*>(0x10AA329C);
			trapSpecs = new Trap[tabParser.lineCount];
			auto& trapCount = temple::GetRef<int>(0x11E61524);
			trapCount = 0;

			tabParser.Process();
			tabParser.Close();

			return TRUE;
			});
	}

	
} trapHooks;
