#include "stdafx.h"
#include "common.h"
#include "faction.h"
#include "gamesystems/objects/objsystem.h"
#include "party.h"

FactionSystem factions;

bool FactionSystem::HasNullFaction(objHndl handle){
	
	auto obj = objSystem->GetObject(handle);
	if (!obj->IsNPC())
		return true;

	if (party.IsInParty(handle))
		return false;

	return obj->GetInt32(obj_f_npc_faction, 0) == 0;
}
