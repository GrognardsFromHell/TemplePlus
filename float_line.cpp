#include "stdafx.h"
#include "float_line.h"
#include "tig/tig_mes.h"
#include "obj.h"
#include "critter.h"
#include "party.h"
#include "combat.h"

FloatLineSystem floatSys;

void FloatLineSystem::FloatCombatLine(objHndl obj, int line)
{
	int objType; 
	FloatLineColor floatColor; 
	objHndl npcLeader; 

	objType = objects.GetType(obj);
	if (objType == obj_t_pc)
	{
		floatColor = FloatLineColor::White;
	}
	else if (objType != obj_t_npc
		|| (npcLeader = critterSys.GetLeader(obj), floatColor = FloatLineColor::Yellow, !party.IsInParty(npcLeader)))
	{
		floatColor = FloatLineColor::Red;
	}
	floatMesLine(obj, 1, floatColor, combatSys.GetCombatMesLine(line));
}

FloatLineSystem::FloatLineSystem()
{
	
	rebase(_FloatSpellLine, 0x10076820);
	rebase(floatMesLine, 0x100A2200);
	rebase(_FloatCombatLine, 0x100B4B60);
}