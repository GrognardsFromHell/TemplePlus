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

void FloatLineSystem::FloatCombatLineWithExtraString(const objHndl& obj, int combatMesLine, const string& cs, const string& cs2){
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

	auto text = fmt::format("{}{}{}", combatSys.GetCombatMesLine(combatMesLine), cs, cs2);
	floatMesLine(obj, 1, floatColor, text.c_str());
}

FloatLineSystem::FloatLineSystem()
{
	
	rebase(_FloatSpellLine, 0x10076820);
	rebase(floatMesLine, 0x100A2200);
	rebase(_FloatCombatLine, 0x100B4B60);
}
