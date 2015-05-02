
#include "stdafx.h"
#include "traps.h"

Traps traps;

static struct TrapsAddresses : AddressTable {

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
