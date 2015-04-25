#include "stdafx.h"
#include "common.h"

enum class FloatLineColor : uint32_t {
	White = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Yellow = 4,
	LightBlue = 5
};

struct FloatLineSystem : AddressTable
{
	void(__cdecl * floatMesLine)(objHndl, int categoryBit, FloatLineColor color, const char *text);
	
	/*
		Float a text line from mes/spells.mes with an optional prefix and suffix text.
	*/
	void FloatSpellLine(objHndl target, int spellMesId, FloatLineColor color, const char *prefix = 0, const char *suffix = 0) {
		_FloatSpellLine(target, spellMesId, color, prefix, suffix);
	}

	FloatLineSystem();
private:

	void(__cdecl *_FloatSpellLine)(objHndl, int mesId, FloatLineColor colorId, const char *prefix, const char *suffix);
};

extern  FloatLineSystem floatSys;