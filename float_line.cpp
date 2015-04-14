#include "stdafx.h"
#include "float_line.h"

FloatLineSystem floatSys;

FloatLineSystem::FloatLineSystem()
{
	rebase(floatMesLine, 0x100A2200);
}