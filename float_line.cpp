#include "stdafx.h"
#include "float_line.h"

FloatLineSystem floatSys;

FloatLineSystem::FloatLineSystem()
{
	
	rebase(_FloatSpellLine, 0x10076820);
	rebase(floatMesLine, 0x100A2200);
	rebase(_FloatCombatLine, 0x100B4B60);
}