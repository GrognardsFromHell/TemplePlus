
#pragma once

#include "python/python_integration_obj.h"

enum TrapFlag : uint32_t {
	TF_IN_STONE,
	TF_PC,
	TF_SPOTTED,
	TF_MAGICAL
};

#pragma pack(push, 1)
struct TrapDamage {
	DamageType type;
	uint32_t packedDice;
};

struct Trap {
	int id; // last column in trap.tab seems to be the numeric trap id, stored in "counters" field of san_trap
	char* name; // first col
	ObjScriptEvent trigger; // Which script event triggers it?
	int flags; // see TrapFlag
	char *partSysName;
	int unk1;
	int unk2;
	char *replaceWith; // If the obj is not a "real" trap, the trap script will be replaced by this trap after triggering (by name)
	BOOL unkCol12; // second to last col
	int damageCount;
	TrapDamage damage[5];
};
#pragma pack(pop)

class Traps {
public:
	const Trap *GetById(int id);
	const Trap *GetByName(const char *name);
	const Trap *GetByObj(objHndl handle);

	D20CAF Attack(objHndl target, int attackBonus, int criticalHitRangeStart, BOOL ranged);
};

extern Traps traps;
