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

void FloatLineSystem::FloatCombatLineWithExtraString(const objHndl& obj, int combatMesLine, const std::string& cs, const std::string& cs2){
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

void FloatLineSystem::FloatSpellLine(objHndl target, int lineId, FloatLineColor color, const char* prefix,
	const char* suffix){
	char text[1024] = {0,};
	auto line = spellSys.GetSpellMesline(lineId);
	if (prefix){
		if (suffix){
			sprintf(text, "%s%s%s", prefix, line, suffix);
		}
		else{
			sprintf(text, "%s%s", prefix, line);
		}
	}
	else if (suffix){
		sprintf(text, "%s%s", line, suffix);
	}
	else{
		sprintf(text, "%s", line);
	}
	floatMesLine(target, 1, color, text);
}

FloatLineSystem::FloatLineSystem()
{
	
	rebase(_FloatSpellLine, 0x10076820);
	rebase(floatMesLine, 0x100A2200);
	rebase(_FloatCombatLine, 0x100B4B60);
}
