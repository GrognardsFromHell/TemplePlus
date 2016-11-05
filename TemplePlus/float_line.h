#include "common.h"
#include <temple/dll.h>

enum FloatLineColor : uint32_t {
	White = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Yellow = 4,
	LightBlue = 5
};

struct FloatLineSystem : temple::AddressTable
{
	void(__cdecl * floatMesLine)(objHndl, int categoryBit, FloatLineColor color, const char *text);


	void FloatCombatLine(objHndl obj, int line);
	void FloatCombatLineWithExtraString(const objHndl& obj, int combatMesLine, const string& cs, const string& cs2);

	/*
		Float a text line from mes/spells.mes with an optional prefix and suffix text.
	*/
	void FloatSpellLine(objHndl target, int lineId, FloatLineColor color, const char *prefix = 0, const char *suffix = 0) {
		_FloatSpellLine(target, lineId, color, prefix, suffix);
	}

	FloatLineSystem();
	
private:

	void(__cdecl *_FloatSpellLine)(objHndl, int lineId, FloatLineColor colorId, const char *prefix, const char *suffix);
	void(__cdecl * _FloatCombatLine)(objHndl obj, int line);
};

extern  FloatLineSystem floatSys;