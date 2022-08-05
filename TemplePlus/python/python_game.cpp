#include "stdafx.h"
#include "python_game.h"
#include <temple/dll.h>
#include <tio/tio.h>
#include "obj.h"
#include "python_object.h"
#include "python_globalvars.h"
#include "python_globalflags.h"
#include "python_time.h"
#include "python_quests.h"
#include "python_counters.h"
#include "python_areas.h"
#include "python_spell.h"
#include "messages/messagequeue.h"
#include "tig/tig_mouse.h"
#include "maps.h"
#include "../gameview.h"
#include "../gamesystems/gamesystems.h"
#include "../gamesystems/particlesystems.h"
#include "../gamesystems/legacy.h"
#include "../gamesystems/objects/objsystem.h"
#include "fade.h"
#include "combat.h"
#include "objlist.h"
#include "d20.h"
#include "movies.h"
#include "textbubbles.h"
#include "party.h"
#include "raycast.h"
#include "sound.h"
#include "ui/ui.h"
#include "ui/ui_dialog.h"
#include "ui/ui_tutorial.h"
#include "ui/ui_picker.h"
#include "particles.h"
#include "python_support.h"
#include "python_integration_obj.h"
#include "../gamesystems/timeevents.h"
#include "tig/tig_startup.h"
#include <graphics/device.h>
#include <particles/instances.h>
#include <gamesystems/legacymapsystems.h>
#include <history.h>
#include <config/config.h>
#include <gamesystems/deity/legacydeitysystem.h>
#include <damage.h>
#include <weapon.h>
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "ui/ui_worldmap.h"
#include "ui/ui_char.h"
#include "ui/ui_mainmenu.h"
#include "infrastructure/elfhash.h"
#include "ui/ui_alert.h"
#include <fmt/format.h>

#include <pybind11/embed.h>
#include <gamesystems/mapsystem.h>
#include <tig/tig_keyboard.h>
#include <infrastructure/keyboard.h>
namespace py = pybind11;

std::map<std::string, MesHandle> mesMap;

static PyObject *encounterQueue = nullptr;

struct PyGameObject {
	PyObject_HEAD;
	PyObject *globalVars;
	PyObject *globalFlags;
	PyObject *quests;
	PyObject *areas;
};

static struct PyGameAddresses : temple::AddressTable {

	int* partyAlignment;
	int* sid;

	void (__cdecl *SetPartyAlignment)(int alignment);
	int (__cdecl *GetPartyAlignment)();

	void (__cdecl *SetStoryState)(int alignment);
	int (__cdecl *GetStoryState)();

	int (__cdecl *PartyGetSelectedCount)();
	objHndl (__cdecl *PartyGetSelectedByIdx)(int idx);

	int (__cdecl *PartyGetCount)();

	// Returns the first conscious party member (which is then the leader) or 0
	objHndl (__cdecl *PartyGetLeader)();

	// Shakes the screen
	void(__cdecl *ShakeScreen)(float amount, float duration);

	PyGameAddresses() {
		rebase(partyAlignment, 0x1080ABA4);
		rebase(sid, 0x10BD2DA4);

		rebase(SetPartyAlignment, 0x1002B720);
		rebase(GetPartyAlignment, 0x1002B730);

		rebase(SetStoryState, 0x10006A30);
		rebase(GetStoryState, 0x10006A20);

		rebase(PartyGetSelectedCount, 0x1002B5C0);
		rebase(PartyGetSelectedByIdx, 0x1002B5D0);

		rebase(PartyGetCount, 0x1002B2B0);
		rebase(PartyGetLeader, 0x1002BE60);

		rebase(ShakeScreen, 0x10005840);
	}
} pyGameAddresses;

/*
	NYI (because unused):
	floaters
	console
*/

// Generic function to build a tuple of handles from a count and a getter
static PyObject* BuildHandleList(int count, function<objHndl(int)> getCallback) {
	auto result = PyTuple_New(count);

	for (auto i = 0; i < count; ++i) {
		auto handle = getCallback(i);
		auto handleObj = PyObjHndl_Create(handle);
		PyTuple_SET_ITEM(result, i, handleObj); // Steals handleObj
	}

	return result;
}

/*
	Returns a tuple that contains the obj handles of all selected party members.
*/
static PyObject* PyGame_GetPartySelected(PyObject*, void*) {
	auto count = pyGameAddresses.PartyGetSelectedCount();
	return BuildHandleList(count, pyGameAddresses.PartyGetSelectedByIdx);
}

/*
	Returns a tuple that contains the obj handles of all selected party members.
*/
static PyObject* PyGame_GetParty(PyObject*, void*) {
	auto count = pyGameAddresses.PartyGetCount();
	return BuildHandleList(count, party.GroupListGetMemberN);
}

/*
	Returns the object the mouse is currently hovering over in the game world.
*/
static PyObject* PyGame_GetHovered(PyObject*, void*) {
	auto pos = mouseFuncs.GetPos();
	objHndl handle;

	if (PickObjectOnScreen(pos.x, pos.y, &handle, GRF_HITTEST_3D)) {
		return PyObjHndl_Create(handle);
	} else {
		return PyObjHndl_CreateNull();
	}
}


static PyObject* PyGame_GetMapsVisited(PyObject*, void*) {
	auto visitedMapIds = maps.GetVisited();

	auto result = PyTuple_New(visitedMapIds.size());
	for (size_t i = 0; i < visitedMapIds.size(); ++i) {
		auto mapId = PyInt_FromLong(visitedMapIds[i]);
		PyTuple_SET_ITEM(result, i, mapId);
	}
	return result;

}

static PyObject* PyGame_GetTime(PyObject*, void*) {
	return PyTimeStamp_Create();
}


static PyObject* PyGame_GetTimePlayed(PyObject*, void*) {
	auto result = PyTimeStamp_Create(gameSystems->GetTimeEvent().GetRealTime());
	return result;
}
static PyObject* PyGame_GetTimeSession(PyObject*, void*) {
	auto result = PyTimeStamp_Create(gameSystems->GetTimeEvent().GetTimeSession());
	return result;
}

static PyObject* PyGame_GetLeader(PyObject*, void*) {
	return PyObjHndl_Create(pyGameAddresses.PartyGetLeader());
}

static PyObject* PyGame_GetGlobalVars(PyObject *obj, void*) {
	auto self = (PyGameObject*)obj;
	Py_INCREF(self->globalVars);
	return self->globalVars;
}

static PyObject* PyGame_GetGlobalFlags(PyObject *obj, void*) {
	auto self = (PyGameObject*)obj;
	Py_INCREF(self->globalFlags);
	return self->globalFlags;
}

static PyObject* PyGame_GetQuests(PyObject *obj, void*) {
	auto self = (PyGameObject*)obj;
	Py_INCREF(self->quests);
	return self->quests;
}

static PyObject* PyGame_GetAreas(PyObject *obj, void*) {
	auto self = (PyGameObject*)obj;
	Py_INCREF(self->areas);
	return self->areas;
}

static PyObject* PyGame_GetCounters(PyObject *obj, void*) {
	return PyCounters_Create();
}

static PyObject* PyGame_GetEncounterQueue(PyObject *obj, void*) {
	if (!encounterQueue) {
		encounterQueue = PyList_New(0);
		Py_INCREF(encounterQueue); // store an additional ref to it in the global
	}
	Py_INCREF(encounterQueue);
	return encounterQueue;
}

static PyObject* PyGame_GetPartyAlignment(PyObject *obj, void*) {
	return PyInt_FromLong(pyGameAddresses.GetPartyAlignment());
}

static PyObject* PyGame_GetStoryState(PyObject *obj, void*) {
	return PyInt_FromLong(pyGameAddresses.GetStoryState());
}

static int PyGame_SetStoryState(PyObject *obj, PyObject *value, void*) {
	pyGameAddresses.SetStoryState(PyInt_AsLong(value));
	return 0;
}

static PyObject* PyGame_GetSid(PyObject *obj, void*) {
	return PyInt_FromLong(*pyGameAddresses.sid);
}

static PyObject* PyGame_GetNewSid(PyObject *obj, void*) {
	return PyInt_FromLong(pythonObjIntegration.GetNewSid());
}

static int PyGame_SetNewSid(PyObject *obj, PyObject *value, void*) {
	pythonObjIntegration.SetNewSid(PyInt_AsLong(value));
	return 0;
}

static PyObject* PyGame_GetCombatTurn(PyObject* obj, void*) {
	auto result = combatSys.GetCombatRoundCount();
	return PyInt_FromLong(result);
}

static PyObject* PyGame_CharUIDisplayType(PyObject* obj, void*) {
	auto result = uiSystems->GetChar().GetDisplayType();
	return PyInt_FromLong((uint32_t)result);
}

static PyObject* PyGame_CharUIPage(PyObject* obj, void*) {
	auto result = uiSystems->GetChar().GetPageType();
	return PyInt_FromLong((uint32_t)result);
}



static PyGetSetDef PyGameGettersSetters[] = {
	{"party_alignment", PyGame_GetPartyAlignment, NULL, NULL },
	{"story_state", PyGame_GetStoryState, PyGame_SetStoryState, NULL },
	{"sid", PyGame_GetSid, NULL, NULL },
	{"new_sid", PyGame_GetNewSid, PyGame_SetNewSid, NULL },
	{"selected", PyGame_GetPartySelected, NULL, NULL},
	{"party", PyGame_GetParty, NULL, NULL},
	{"hovered", PyGame_GetHovered, NULL, NULL},
	{"maps_visited", PyGame_GetMapsVisited, NULL, NULL},
	{"leader", PyGame_GetLeader, NULL, NULL},
	{"elader", PyGame_GetLeader, NULL, NULL }, // commonly made typo so might as well support it :D

	{"time", PyGame_GetTime, NULL, NULL},
	{"time_played", PyGame_GetTimePlayed, NULL, NULL},
	{"time_session", PyGame_GetTimeSession, NULL, NULL},

	{"global_vars", PyGame_GetGlobalVars, NULL, NULL},
	{"ggv", PyGame_GetGlobalVars, NULL, NULL },
	{"global_flags", PyGame_GetGlobalFlags, NULL, NULL},
	{"ggf", PyGame_GetGlobalFlags, NULL, NULL },
	{"quests", PyGame_GetQuests, NULL, NULL},
	{"areas", PyGame_GetAreas, NULL, NULL},
	{"counters", PyGame_GetCounters, NULL, NULL},
	{"encounter_queue", PyGame_GetEncounterQueue, NULL, NULL},
	{"combat_turn", PyGame_GetCombatTurn, NULL, NULL},
	{"char_ui_display_type", PyGame_CharUIDisplayType, NULL, NULL},
	{"char_ui_page", PyGame_CharUIPage, NULL, NULL},
	{NULL, NULL, NULL, NULL}
};

PyObject* PyGame_FadeAndTeleport(PyObject*, PyObject* args) {

	FadeAndTeleportArgs fadeArgs;

	if (!PyArg_ParseTuple(args, "iiiiii", &fadeArgs.timeToAdvance, &fadeArgs.soundId, &fadeArgs.movieId, &fadeArgs.destMap, &fadeArgs.destLoc.locx, &fadeArgs.destLoc.locy)) {
		return 0;
	}

	// Temple+: added option to auto-set start location by map props when coords are both < 0
	if ( (int)fadeArgs.destLoc.locx < 0 && (int)fadeArgs.destLoc.locy < 0) {
		fadeArgs.destLoc = gameSystems->GetMap().GetStartPos(fadeArgs.destMap);
	}

	fadeArgs.field34 = 48;
	fadeArgs.field50 = 48;
	fadeArgs.field2c = 0;
	fadeArgs.field3c = 0;
	fadeArgs.color = XMCOLOR(0, 0, 0, 1);
	fadeArgs.somefloat = 2.0f;
	fadeArgs.somefloat2 = 2.0f;
	fadeArgs.flags = 0;

	auto leader = party.GroupListGetMemberN(0);
	fadeArgs.somehandle = leader;
	if (fadeArgs.timeToAdvance > 0)
		fadeArgs.flags |= ftf_advance_time;
	if (fadeArgs.soundId)
		fadeArgs.flags |= ftf_play_sound;
	if (fadeArgs.movieId > 0)
	{
		fadeArgs.flags |= ftf_play_movie;
		fadeArgs.field20 = 0;
	}
	
	auto result = fade.FadeAndTeleport(fadeArgs);
	return PyInt_FromLong(result);
}

PyObject* PyGame_Fade(PyObject*, PyObject* args) {

	// TODO: This seems to be way too much logic for the glue code layer, move this to fade.cpp maybe?

	int timeToAdvance;
	int soundId;
	int movieId;
	int fadeOutTime;

	if (!PyArg_ParseTuple(args, "iiii", &timeToAdvance, &soundId, &movieId, &fadeOutTime)) {
		return 0;
	}

	// Trigger the fade out immediately
	FadeArgs fadeArgs;
	fadeArgs.flags = 0;
	fadeArgs.field10 = 0;
	fadeArgs.color = XMCOLOR(0, 0, 0, 1);
	fadeArgs.transitionTime = 2.0f;
	fadeArgs.countSthgUsually48 = 48;
	fade.PerformFade(fadeArgs);

	textBubbles.HideAll();

	if (timeToAdvance > 0) {
		auto time = GameTime::FromSeconds(timeToAdvance);
		gameSystems->GetTimeEvent().GameTimeAdd(time);
	}

	if (movieId) {
		// Originally the soundId was passed here
		// But since no BinkW movie actually uses soundtrack ids,
		// just skip it.
		if (soundId > 0) {
			movieFuncs.PlayMovieId(movieId, 0, soundId);
		} else {
			movieFuncs.PlayMovieId(movieId, 0, 0);
		}
	} else if (soundId > 0) {
		sound.PlaySound(soundId);
	}

	if (fadeOutTime) {
		TimeEvent evt;
		fadeOutTime *= 1000;
		evt.system = TimeEventType::Fade;
		evt.params[0].int32 = 1;
		evt.params[1].int32 = 0xFF000000;
		evt.params[2].float32 = 2.0f;
		evt.params[3].int32 = 48;
		gameSystems->GetTimeEvent().Schedule(evt, fadeOutTime);
	} else {
		fadeArgs.flags = 1;
		fade.PerformFade(fadeArgs);
	}

	Py_RETURN_NONE;
}

PyObject* PyGame_PartySize(PyObject*, PyObject* args) {
	return PyInt_FromLong(party.GroupListGetLen());
}

PyObject* PyGame_PartyPcSize(PyObject*, PyObject* args) {
	return PyInt_FromLong(party.GroupPCsLen());
}

PyObject* PyGame_PartyNpcSize(PyObject*, PyObject* args) {
	return PyInt_FromLong(party.GroupNPCFollowersLen());
}

static PyObject *ObjListToTuple(ObjList &objList) {
	auto result = PyTuple_New(objList.size());
	for (auto i = 0; i < objList.size(); ++i) {
		auto handle = PyObjHndl_Create(objList[i]);
		PyTuple_SET_ITEM(result, i, handle);
	}
	return result;
}

PyObject* PyGame_ObjList(PyObject*, PyObject* args) {
	locXY loc;
	int flags;
	if (!PyArg_ParseTuple(args, "Li:game.obj_list", &loc, &flags)) {
		return 0;
	}

	ObjList objList;
	objList.ListTile(loc, flags);
	return ObjListToTuple(objList);
}

PyObject* PyGame_ObjListVicinity(PyObject*, PyObject* args) {
	locXY loc;
	int flags;
	if (!PyArg_ParseTuple(args, "Li:game.obj_list_vicinity", &loc, &flags)) {
		return 0;
	}

	ObjList objList;
	objList.ListVicinity(loc, flags);
	return ObjListToTuple(objList);
}

PyObject* PyGame_FindNpcNear(PyObject*, PyObject* args) {
	int flags = OLC_NPC;
	int npcName = -1;
	char * npcNameStr = 0;
	int livingOnly = 1;
	int multiple = 0; // unimplemented yet

	if (PyTuple_GET_SIZE(args) >= 1)
	{
		auto arg1 = PyTuple_GET_ITEM(args, 0);
		if (PyString_Check(arg1))
		{
			npcNameStr = PyString_AsString(arg1);
		} 
		else if (PyInt_Check(arg1))
		{
			npcName = PyInt_AsLong(arg1);
		}
	} 

	LocAndOffsets loc;
	objHndl gameLeader = pyGameAddresses.PartyGetLeader();
	locSys.getLocAndOff(gameLeader, &loc);

	ObjList objList;
	objList.ListVicinity(loc.location, flags);

	int numFound = 0;
	objHndl handlesFound[100];
	for (int i = 0; i < objList.size() && i < 100; i++)
	{
		objHndl dude = objList.get(i);
		if (npcName != -1)
		{
			if (dude && objects.GetNameId(dude) == npcName)
				handlesFound[numFound++] = dude;
		} 
		else if(npcNameStr != 0)
		{
			auto dudePyHndl = PyObjHndl_Create(dude);
			char * dudeName = PyString_AsString( dudePyHndl->ob_type->tp_repr(dudePyHndl) );
			
			if (strstr(dudeName, npcNameStr))
			{
				handlesFound[numFound++] = dude;
			}
		}
		
	}
	if (!numFound)
		return PyInt_FromLong(0);
	auto result = PyTuple_New(numFound);
	for (int i = 0; i < numFound; i++)
	{
		PyTuple_SetItem(result, i, PyObjHndl_Create(handlesFound[i]));
	}

	if (multiple)
		return result;
	return PyObjHndl_Create(handlesFound[0]);
}

PyObject* PyGame_Vlist(PyObject*, PyObject* args) {
	int objName;
	int flags = OLC_NPC;
	if (!PyArg_ParseTuple(args, "|ii:game.vlist", &objName, &flags)) {
		return 0;
	}
	if (!flags)
		flags = OLC_NPC;

	LocAndOffsets loc;
	objHndl gameLeader = pyGameAddresses.PartyGetLeader();
	locSys.getLocAndOff(gameLeader, &loc);
	
	
	ObjList objList;
	objList.ListVicinity(loc.location, flags);
	return ObjListToTuple(objList);
}


PyObject* PyGame_GetBabForClass(PyObject*, PyObject* args){
	Stat classCode;
	int classLvl;
	if (!PyArg_ParseTuple(args, "ii:game.get_bab_for_class", &classCode, &classLvl)) {
		return nullptr;
	}

	return PyInt_FromLong(d20ClassSys.GetBaseAttackBonus(classCode, classLvl));
}

PyObject* PyGame_GetWpnTypeForFeat(PyObject*, PyObject* args) {
	feat_enums featCode;
	if (!PyArg_ParseTuple(args, "i:game.get_weapon_type_for_feat", &featCode)) {
		return nullptr;
	}

	return PyInt_FromLong(feats.GetWeaponType(featCode));
}

PyObject* PyGame_GetFeatName(PyObject*, PyObject* args) {
	feat_enums featCode;
	if (!PyArg_ParseTuple(args, "i:game.get_feat_name", &featCode)) {
		return nullptr;
	}

	return PyString_FromString(feats.GetFeatName(featCode));
}

PyObject* PyGame_MakeCustomName(PyObject*, PyObject* args) {
	char *name;

	if (!PyArg_ParseTuple(args, "s:game.make_custom_name", &name)) { 
		return 0;
	}

	auto nameId = objects.description.CustomNameNew(name);
	return PyInt_FromLong(nameId);
}

PyObject* PyGame_IsRangedWeapon(PyObject*, PyObject* args) {
	WeaponTypes wt;

	if (PyTuple_GET_SIZE(args) < 1) {
		return nullptr;
	}

	PyObject* arg1 = PyTuple_GET_ITEM(args, 0);
	wt = static_cast<WeaponTypes>(PyLong_AsLong(arg1));

	return PyInt_FromLong(weapons.IsRangedWeapon(wt));
}

PyObject* PyGame_IsMeleeWeapon(PyObject*, PyObject* args) {
	WeaponTypes wt;

	if (PyTuple_GET_SIZE(args) < 1) {
		return nullptr;
	}

	PyObject* arg1 = PyTuple_GET_ITEM(args, 0);
	wt = static_cast<WeaponTypes>(PyLong_AsLong(arg1));

	return PyInt_FromLong(weapons.IsMeleeWeapon(wt));
}

PyObject* PyGame_GetFeatForWpnType(PyObject*, PyObject* args) {
	WeaponTypes wt;
	feat_enums baseFeat = FEAT_NONE;

	auto numArgs = PyTuple_GET_SIZE(args);
	if (numArgs < 1u) {
		return nullptr;
	}

	PyObject* arg1 = PyTuple_GET_ITEM(args, 0);
	wt = static_cast<WeaponTypes>(PyLong_AsLong(arg1));

	if (numArgs >= 2u){
		PyObject* arg2 = PyTuple_GET_ITEM(args, 1);
		if (PyString_Check(arg2)) {
			auto argString = fmt::format("{}", PyString_AsString(arg2));
			baseFeat = static_cast<feat_enums>(ElfHash::Hash(argString));
		}
		else {
			baseFeat = static_cast<feat_enums>(PyLong_AsLong(arg2));
		}
	}
	

	return PyInt_FromLong(feats.GetFeatForWeaponType(wt, baseFeat));
}



PyObject* PyGame_DamageTypeMatch(PyObject*, PyObject* args) {
	DamageType dt1, dt2;
	if (!PyArg_ParseTuple(args, "ii:game.damage_type_match", &dt1, &dt2)) {
		return nullptr;
	}
	return PyInt_FromLong(damage.DamageTypeMatch(dt1, dt2) + damage.DamageTypeMatch(dt2, dt1));
}

PyObject* PyGame_GetWpnDamType(PyObject*, PyObject* args) {
	WeaponTypes wt;
	if (!PyArg_ParseTuple(args, "i:game.get_weapon_damage_type", &wt)) {
		return nullptr;
	}
	auto damType = weapons.wpnProps[wt].damType;
	return PyInt_FromLong((int)damType);
}



PyObject* PyGame_IsSaveFavoerdForClass(PyObject*, PyObject* args) {
	Stat classCode;
	int saveType;
	if (!PyArg_ParseTuple(args, "ii:game.is_save_favored_for_class", &classCode, &saveType)) {
		return nullptr;
	}

	return PyInt_FromLong(d20ClassSys.IsSaveFavoredForClass(classCode, saveType));
}

PyObject* PyGame_IsLaxRules(PyObject*, PyObject* args) {
	return PyInt_FromLong(config.laxRules);
}


PyObject* PyGame_GetProto(PyObject*, PyObject* args) {
	int32_t protoId;
	if (!PyArg_ParseTuple(args, "i:game.getproto", &protoId)) {
		return nullptr;
	}

	auto handle = objSystem->GetProtoHandle(protoId);
	if (!handle) {
		return PyErr_Format(PyExc_RuntimeError, 
			"Unknown prototype id: %d", protoId);
	}
	
	return PyObjHndl_Create(handle);
}

PyObject* PyGame_ObjListRange(PyObject*, PyObject* args) {
	LocAndOffsets loc;
	int radius;
	int flags;
	if (!PyArg_ParseTuple(args, "Lii:game.obj_list_range", &loc.location, &radius, &flags)) {
		return 0;
	}
	loc.off_x = 0; loc.off_y = 0;
	float radiusArg = radius * 12.0f; // Apparently a shoddy conversion to coordinate space

	ObjList objList;
	objList.ListRadius(loc, radiusArg, flags);
	return ObjListToTuple(objList);
}

PyObject* PyGame_ObjListCone(PyObject*, PyObject* args) {
	objHndl originHndl;
	int flags, coneLeft, coneArc;
	float radius;
	if (!PyArg_ParseTuple(args, "O&ifii:game.obj_list_cone", &ConvertObjHndl, &originHndl, &flags, &radius, &coneLeft, &coneArc)) {
		return 0;
	}

	radius *= 12.0f; // Apparently a shoddy conversion to coordinate space

	// Get location and rotation of obj
	auto origin = objects.GetLocationFull(originHndl);
	float rotation = objects.GetRotation(originHndl);

	// Modify rotation by coneLeft to get the start angle of the cone
	float coneLeftFloat = 0.0;
	__asm{
		fild coneLeft;
		fstp coneLeftFloat;
	}
	rotation += deg2rad(coneLeftFloat);
	
	float coneArcFloat = 0.0;
	__asm{
		fild coneArc;
		fstp coneArcFloat;
	}
	float coneArcRad = deg2rad(coneArcFloat);

	ObjList objList;
	objList.ListCone(origin, radius, rotation, coneArcRad, flags);
	return ObjListToTuple(objList);
}

PyObject* PyGame_Sound(PyObject*, PyObject* args) {
	int soundId;
	int loopCount = 1;
	if (!PyArg_ParseTuple(args, "i|i:game.sound", &soundId, &loopCount)) {
		return 0;
	}

	// Seems to be -1 on failure
	auto streamId = sound.PlaySound(soundId, loopCount);
	return PyInt_FromLong(streamId);
}

PyObject* PyGame_SoundLocalObj(PyObject*, PyObject* args) {
	objHndl handle;
	int soundId;
	int loopCount = 1;
	if (!PyArg_ParseTuple(args, "iO&|i:game.sound_local_obj", &soundId, &ConvertObjHndl, &handle, &loopCount)) {
		return 0;
	}

	// Seems to be -1 on failure
	auto streamId = sound.PlaySoundAtObj(soundId, handle, loopCount);
	return PyInt_FromLong(streamId);
}

PyObject* PyGame_SoundLocalLoc(PyObject*, PyObject* args) {
	locXY tileLoc;
	int soundId;
	int loopCount = 1;
	if (!PyArg_ParseTuple(args, "iL|i:game.sound_local_loc", &soundId, &tileLoc, &loopCount)) {
		return 0;
	}

	// Seems to be -1 on failure
	auto streamId = sound.PlaySoundAtLoc(soundId, tileLoc, loopCount);
	return PyInt_FromLong(streamId);
}

PyObject* PyGame_Particles(PyObject*, PyObject* args) {

	char *name;
	PyObject *locOrObj;

	if (!PyArg_ParseTuple(args, "sO:game.particles", &name, &locOrObj)) {
		return 0;
	}

	auto& particles = gameSystems->GetParticleSys();

	if (PyObjHndl_Check(locOrObj)) {
		auto objHandle = PyObjHndl_AsObjHndl(locOrObj);
		if (!objHandle){
			PyErr_SetString(PyExc_TypeError, "PyGame_Particles called with null object handle!");
			return 0;
		}
		auto partHandle = particles.CreateAtObj(name, objHandle);
		return PyInt_FromLong(partHandle);
	}
	else if (PyLong_Check(locOrObj)) {
		auto loc = locXY::fromField(PyLong_AsUnsignedLongLong(locOrObj));
		auto pos = loc.ToInches3D();
		auto partHandle = particles.CreateAtPos(name, pos);
		return PyInt_FromLong(partHandle);
	} else {
		PyErr_SetString(PyExc_TypeError, "Location of particle system must be either a tile location (long) or an object handle.");
		return 0;
	}

}

PyObject* PyGame_ObjCreate(PyObject*, PyObject* args) {
	int protoId;
	locXY loc = { 0, 0 };
	float offsetX = 0, offsetY = 0;

	if (!PyArg_ParseTuple(args, "i|Lff:game.obj_create", &protoId, &loc, &offsetX, &offsetY)) {
		return 0;
	}

	// Use mouse position if no location was given
	if (loc.locx == 0 && loc.locy == 0) {
		auto mousePos = mouseFuncs.GetPos();
		auto& device = tig->GetRenderingDevice();
		auto worldPos = gameView->ScreenToWorld((float) mousePos.x, (float) mousePos.y);
		loc.locx = (int)(worldPos.x / INCH_PER_TILE);
		loc.locy = (int)(worldPos.x / INCH_PER_TILE);
	}

	// resolve the proto handle for the prototype number
	auto protoHandle = objects.GetProtoHandle(protoId);

	if (!protoHandle) {
		PyErr_SetString(PyExc_ValueError, "Cannot create object for unknown prototype id.");
		return 0;
	}

	auto handle = objects.Create(protoHandle, loc);

	if (handle) {
		auto radius = objects.GetRadius(handle);
		LocAndOffsets locOff = { loc, offsetX, offsetY };
		LocAndOffsets freeSpot;
		if (objects.FindFreeSpot(locOff, radius, freeSpot)) {
			objects.Move(handle, freeSpot);
		}
		if (objects.IsCritter(handle)) {
			objects.AiForceSpreadOut(handle);
		}
	}

	return PyObjHndl_Create(handle);
}

PyObject* PyGame_TimeEventAdd(PyObject*, PyObject* args) {

	TimeEvent evt;
	int realtime = 0;
	int timeInMsec;

	if (!PyArg_ParseTuple(args, "OOi|i:game.timeevent_add", &evt.params[0].pyobj, &evt.params[1].pyobj, &timeInMsec, &realtime)) {
		return 0;
	}

	evt.system = TimeEventType::PythonScript;
	if (realtime) {
		evt.system = TimeEventType::PythonRealtime;
	}
	Py_INCREF(evt.params[0].pyobj);
	Py_INCREF(evt.params[1].pyobj);
	gameSystems->GetTimeEvent().Schedule(evt, timeInMsec);
	Py_RETURN_NONE;
}

/*
	Reveals a flag on the townmap UI for a map.
	1st arg is the map id (i.e. 5001)
	2nd arg is the flag id (see rules\townmap_ui_placed_flag_locations.mes)
	3rd arg is unused, but seemed to indicate that it should be revealed (which is all this function can do)
*/
PyObject* PyGame_MapFlags(PyObject*, PyObject* args) {
	int mapId, flagId, reveal;
	if (!PyArg_ParseTuple(args, "ii|i:game.mapflags", &mapId, &flagId, &reveal)) {
		return 0;
	}

	if (reveal != 1) {
		logger->warn("Using map_flags with a reveal-argument other than 1: {}. Flags can only be revealed using this function.", reveal);
	}

	maps.RevealFlag(mapId, flagId);
	Py_RETURN_NONE;
}

PyObject* PyGame_ParticlesKill(PyObject*, PyObject* args) {
	int partSysId;
	if (!PyArg_ParseTuple(args, "i:game.particles_kill", &partSysId)) {
		return 0;
	}
	if (!partSysId) {
		PyErr_SetString(PyExc_ValueError, "Cannot kill particle system id 0. Invalid value.");
		return 0;
	}

	gameSystems->GetParticleSys().Remove(partSysId);
	Py_RETURN_NONE;
}

PyObject* PyGame_ParticlesEnd(PyObject*, PyObject* args) {
	int partSysId;
	if (!PyArg_ParseTuple(args, "i:game.particles_kill", &partSysId)) {
		return 0;
	}
	if (!partSysId) {
		PyErr_SetString(PyExc_ValueError, "Cannot kill particle system id 0. Invalid value.");
		return 0;
	}
	auto partSys = gameSystems->GetParticleSys().GetByHandle(partSysId);
	if (partSys) {
		partSys->EndPrematurely();
	}
	Py_RETURN_NONE;
}

PyObject* PyGame_SaveGame(PyObject*, PyObject* args) {
	char *filename;
	char *displayName;
	if (!PyArg_ParseTuple(args, "ss:game.savegame", &filename, &displayName)) {
		return 0;
	}

	auto result = gameSystems->SaveGame(filename, displayName);
	return PyInt_FromLong(result);
}

PyObject* PyGame_ScrollTo(PyObject*, PyObject* args)
{
	PyObject *locOrObj;
	int smooth = 1;
	if (!PyArg_ParseTuple(args, "O|i:game.scroll_to",  &locOrObj, &smooth)) {
		return 0;
	}

	locXY targetLoc;

	if (PyObjHndl_Check(locOrObj)) {
		auto objHandle = PyObjHndl_AsObjHndl(locOrObj);
		targetLoc = objects.GetLocation(objHandle);
	} else if (PyLong_Check(locOrObj)) {
		targetLoc = locXY::fromField(PyLong_AsUnsignedLongLong(locOrObj));
	} else {
		PyErr_SetString(PyExc_TypeError, "scroll_to argument must be either a tile location (long) or an object handle.");
		return nullptr;
	}

	if (smooth) {
		gameSystems->GetLocation().CenterOnSmooth(targetLoc.locx, targetLoc.locy);
	}
	else {
		gameSystems->GetLocation().CenterOn(targetLoc.locx, targetLoc.locy);
	}
	
	Py_RETURN_TRUE;
}

PyObject* PyGame_Keypress(PyObject*, PyObject* args) {

	int dik = 0;
	if (!PyArg_ParseTuple(args, "i:game.keypress", &dik)) {
		return nullptr;
	}
	logger->info("game.keypress: {}", dik);

	{
		TigMsg tigMsg;
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::KEYSTATECHANGE;
		tigMsg.arg1 = dik;
		tigMsg.arg2 = 0; // Means it has changed to unpressed
		if (tigMsg.arg1 != 0) {
			messageQueue->Enqueue(tigMsg);
		}
	}
	{
		TigMsg tigMsg;
		tigMsg.createdMs = timeGetTime();
		tigMsg.type = TigMsgType::CHAR;
		tigMsg.arg1 = infrastructure::gKeyboard.ToVirtualKey(dik);
		tigMsg.arg2 = 0; // Means it has changed to unpressed
		if (tigMsg.arg1 != 0) {
			messageQueue->Enqueue(tigMsg);
		}
	}
	

	Py_RETURN_TRUE;
}


PyObject* PyGame_MouseMoveTo(PyObject*, PyObject* args)
{
	PyObject* locOrObj;

	int isScreenspace = 0;
	if (!PyArg_ParseTuple(args, "O|i:game.mouse_move_to", &locOrObj, &isScreenspace)) {
		return 0;
	}

	locXY targetLoc;

	if (PyObjHndl_Check(locOrObj)) {
		auto objHandle = PyObjHndl_AsObjHndl(locOrObj);
		targetLoc = objects.GetLocation(objHandle);
	}
	else if (PyLong_Check(locOrObj)) {
		targetLoc = locXY::fromField(PyLong_AsUnsignedLongLong(locOrObj));
	}
	else if (PyTuple_Check(locOrObj)) {
		if (!PyArg_ParseTuple(locOrObj, "ii:game.mouse_move_to", &targetLoc.locx, &targetLoc.locy)) {
			PyErr_SetString(PyExc_TypeError, "mouse_move_to tuple arg must (x,y).");
			return nullptr;
		}
	}
	else {
		PyErr_SetString(PyExc_TypeError, "mouse_move_to argument must be either a tile location (long) or an object handle.");
		return nullptr;
	}
	
	int x = 0, y = 0;
	if (isScreenspace) {
		x = targetLoc.locx;
		y = targetLoc.locy;
	}
	else {
		logger->info("mouse_move_to: location {}, {}", targetLoc.locx, targetLoc.locy);
		auto worldPos = targetLoc.ToInches3D();
		auto uiPos = gameView->WorldToScreenUi(worldPos);
		x = uiPos.x; y = uiPos.y;
	}
	
	mouseFuncs.SetPos(x, y, 0);

	Py_RETURN_TRUE;
}


PyObject* PyGame_MouseClick(PyObject*, PyObject* args)
{
	int clickType = 0;

	if (!PyArg_ParseTuple(args, "|i:game.mouse_click", &clickType)) {
		return nullptr;
	}

	if (clickType >= (int)MouseButton::LEFT && clickType <= (int)MouseButton::MIDDLE){
		mouseFuncs.SetButtonState((MouseButton)clickType, true);
		mouseFuncs.SetButtonState((MouseButton)clickType, false);
	}
	
	Py_RETURN_TRUE;
}


PyObject* PyGame_LoadGame(PyObject*, PyObject* args) {
	char *filename;
	if (!PyArg_ParseTuple(args, "s:game.loadgame", &filename)) {
		return 0;
	}
		
	gameSystems->DestroyPlayerObject();
	auto result = gameSystems->LoadGame(filename);
	return PyInt_FromLong(result);
}

PyObject* PyGame_UpdateCombatUi(PyObject*, PyObject* args) {
	uiSystems->GetCombat().Update();
	Py_RETURN_NONE;
}

PyObject* PyGame_UpdatePartyUi(PyObject*, PyObject* args) {
	uiSystems->GetParty().Update();
	Py_RETURN_NONE;
}

PyObject* PyGame_RandomRange(PyObject*, PyObject* args) {
	int from, to;
	if (!PyArg_ParseTuple(args, "ii", &from, &to)) {
		return 0;
	}
	
	auto result = RandomIntRange(from, to);
	return PyInt_FromLong(result);
}

PyObject* PyGame_TargetRandomTileNearGet(PyObject*, PyObject* args) {
	objHndl handle;
	int distance;

	if (!PyArg_ParseTuple(args, "O&i:game.target_random_tile_near_get", &ConvertObjHndl, &handle, &distance)) {
		return 0;
	}
	
	auto loc = objects.TargetRandomTileNear(handle, distance);
	return PyLong_FromLongLong(loc);
}

PyObject* PyGame_GetStatMod(PyObject*, PyObject* args) {
	int attributeValue;
	if (!PyArg_ParseTuple(args, "i:game.get_stat_mod", &attributeValue)) {
		return 0;		
	}

	return PyInt_FromLong(GetAttributeMod(attributeValue));
}


PyObject* PyGame_GetDeityFavoredWeapon(PyObject*, PyObject* args) {
	int deityId = 0;

	if (!PyArg_ParseTuple(args, "i:game.get_deity_favored_weapon", &deityId)) {
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(deitySys.GetDeityFavoredWeapon(deityId));
}


PyObject* PyGame_UiShowWorldmap(PyObject*, PyObject* args) {
	int mode;
	if (!PyArg_ParseTuple(args, "i:game.ui_show_worldmap", &mode)) {
		return 0;
	}
	ui_worldmap().Show(mode);
	Py_RETURN_NONE;
}

PyObject* PyGame_WorldmapTravelByDialog(PyObject*, PyObject* args) {
	int area;
	if (!PyArg_ParseTuple(args, "i:game.ui_worldmap_travel_by_dialog(area)", &area)) {
		return 0;
	}
	ui_worldmap().TravelToArea(area);
	Py_RETURN_NONE;
}

PyObject* PyGame_PfxCallLightning(PyObject*, PyObject* args) {
	LocAndOffsets loc;
	float offz; // actually ignored
	if (!PyArg_ParseTuple(args, "Lfff:game.pfx_call_lightning", &loc.location, &loc.off_x, &loc.off_y, &offz)) {
		return 0;
	}

	legacyParticles.CallLightning(loc);
	Py_RETURN_NONE;
}

PyObject* PyGame_PfxChainLightning(PyObject*, PyObject* args) {

	objHndl caster;
	int targetCount;
	PyObject *spell;

	if (!PyArg_ParseTuple(args, "O&iO&", &ConvertObjHndl, &caster, &targetCount, &ConvertTargetArray, &spell)) {
		return 0;
	}

	// This is very much at the wrong location (should be in spell). but hrm.
	vector<objHndl> targets;
	for (int i = 0; i < targetCount; ++i) {
		targets.push_back(PySpell_GetTargetHandle(spell, i));
	}
	legacyParticles.ChainLightning(caster, targets);
	Py_RETURN_NONE;
}

PyObject* PyGame_PfxLightningBolt(PyObject*, PyObject* args) {
	objHndl caster;
	LocAndOffsets target;
	float offz; // Ignored
	if (!PyArg_ParseTuple(args, "O&Lfff:game.pfx_lightning_bolt", &ConvertObjHndl, &caster, &target.location, &target.off_x, &target.off_y, &offz)) {
		return 0;
	}

	legacyParticles.LightningBolt(caster, target);
	Py_RETURN_NONE;
}

PyObject* PyGame_GametimeAdd(PyObject*, PyObject* args) {
	int timeInMs;
	if (!PyArg_ParseTuple(args, "i:game.gametime_add", &timeInMs)) {
		return 0;
	}

	gameSystems->GetTimeEvent().AddTime(timeInMs);
	auto time = gameSystems->GetTimeEvent().GetTime();
	auto formattedTime = gameSystems->GetTimeEvent().FormatTime(time);
	return PyString_FromString(formattedTime.c_str());
}


PyObject* PyGame_IsIngame(PyObject*, PyObject* args) {
	return PyInt_FromLong(gameSystems->IsIngame());
}

PyObject* PyGame_IsOutdoor(PyObject*, PyObject* args) {
	return PyInt_FromLong(maps.IsCurrentMapOutdoor());
}


PyObject* PyGame_IsSpellHarmful(PyObject*, PyObject* args) {

	int spellEnum = 0;
	objHndl caster, tgt;
	if (!PyArg_ParseTuple(args, "iO&O&:game.is_spell_harmful", &spellEnum, &ConvertObjHndl, &caster, &ConvertObjHndl, &tgt)) {
		return 0;
	}
	auto result = spellSys.IsSpellHarmful(spellEnum, caster, tgt);
	return PyInt_FromLong(result);
}

PyObject* PyGame_Shake(PyObject*, PyObject* args) {
	float amount, duration;

	if (!PyArg_ParseTuple(args, "ff:game.shake", &amount, &duration)) {
		return 0;
	}

	pyGameAddresses.ShakeScreen(amount, duration);
	Py_RETURN_NONE;
}

PyObject* PyGame_MoviequeueAdd(PyObject*, PyObject* args) {
	int movieId;
	if (!PyArg_ParseTuple(args, "i:game.moviequeue_add", &movieId)) {
		return 0;
	}
	movieFuncs.MovieQueueAdd(movieId);
	Py_RETURN_NONE;
}

PyObject* PyGame_MoviequeuePlay(PyObject*, PyObject* args) {
	movieFuncs.MovieQueuePlay();
	Py_RETURN_NONE;
}

PyObject* PyGame_MoviequeuePlayEndGame(PyObject*, PyObject* args) {
	movieFuncs.MovieQueuePlay();
	gameSystems->EndGame();
	Py_RETURN_NONE;
}

static struct PyGameDialogPickerArgs {
	PyObject *isValidTarget = nullptr;
	int invalidTargetLine = 0;
	int cancelledLine = 0;
	int validTargetLine = 0;
} dialogPickerArgs;

static void __cdecl PyGame_PickerCallback(const PickerResult &result, void* callbackData);

/*
	This can only be called from within dialogs to initiate a spell picker
	that will lead in turn to a dialog line and set the picker_obj global.
*/
PyObject* PyGame_Picker(PyObject*, PyObject* args) {
	int spellId;
	objHndl caster;
	PyObject *validTargetCallback;
	PyObject *dialogLines;

	if (!PyArg_ParseTuple(args, "O&iOO!:game.picker", &ConvertObjHndl, &caster, &spellId, &validTargetCallback, &PyList_Type, &dialogLines)) {
		return 0;
	}

	if (!PyCallable_Check(validTargetCallback)) {
		PyErr_SetString(PyExc_TypeError, "Third argument must be a callable that determines whether a target is valid or not.");
		return 0;
	}

	if (PyList_Size(dialogLines) != 3) {
		PyErr_SetString(PyExc_ValueError, "Expect exactly three dialog lines in the 4th argument.");
		return 0;
	}

	for (auto i = 0; i < 3; ++i) {
		if (!PyInt_Check(PyList_GET_ITEM(dialogLines, i))) {
			PyErr_SetString(PyExc_TypeError, "Expected the list in the 4th argument to only contain integer line numers.");
			return 0;
		}
	}

	// This picker can only be used while in dialog
	if (!uiDialog->IsActive()) {
		logger->debug("Can only use game.picker from dialog");
		Py_RETURN_NONE;
	}

	// Set up the callback arguments we'll get
	Py_XDECREF(dialogPickerArgs.isValidTarget); // Clear previous callback if any
	dialogPickerArgs.isValidTarget = validTargetCallback;
	Py_INCREF(validTargetCallback);
	dialogPickerArgs.invalidTargetLine = PyInt_AsLong(PyList_GET_ITEM(dialogLines, 0));
	dialogPickerArgs.cancelledLine = PyInt_AsLong(PyList_GET_ITEM(dialogLines, 1));
	dialogPickerArgs.validTargetLine = PyInt_AsLong(PyList_GET_ITEM(dialogLines, 2));

	PickerArgs picker;
	picker.flagsTarget = LosNotRequired;
	picker.modeTarget = UiPickerType::Single;
	picker.incFlags = UiPickerIncFlags::UIPI_Other;
	picker.excFlags = UiPickerIncFlags::UIPI_None;
	picker.spellEnum = spellId;
	picker.callback = PyGame_PickerCallback;
	picker.caster = caster;
	
	uiDialog->Hide();
	uiPicker.ShowPicker(picker, &dialogPickerArgs);

	Py_RETURN_NONE;
}

// This is called for various tasks by the picker we create above
static void __cdecl PyGame_PickerCallback(const PickerResult &result, void*) {

	auto currentDlg = uiDialog->GetCurrentDialog();

	// Reset picker obj
	pythonObjIntegration.SetPickerObj(nullptr);
		
	if (result.flags & PRF_CANCELLED) {
		// The player cancelled the picker
		uiDialog->ReShowDialog(currentDlg, dialogPickerArgs.cancelledLine);
	} else {
		// Something has been picked. Is it a valid target?
		auto targetObj = PyObjHndl_Create(result.handle);
		auto isValidObj = PyObject_CallFunction(dialogPickerArgs.isValidTarget, "O", targetObj);

		if (!isValidObj) {
			// Call resulted in an exception
			logger->error("Unable to call the is_valid_target callback for game.picker");
			PyErr_Print();
		} else {
			// Target is valid
			if (PyObject_IsTrue(isValidObj)) {
				pythonObjIntegration.SetPickerObj(targetObj);
				uiDialog->ReShowDialog(currentDlg, dialogPickerArgs.validTargetLine);
			} else {
				uiDialog->ReShowDialog(currentDlg, dialogPickerArgs.invalidTargetLine);
			}
			Py_DECREF(isValidObj);
		}

		Py_DECREF(targetObj);
	}

	// Free the callback
	Py_DECREF(dialogPickerArgs.isValidTarget);
	dialogPickerArgs.isValidTarget = nullptr;
	
	// The picker would still be going if we didnt cancel it
	uiPicker.FreeCurrentPicker();
	uiDialog->Unk(); // Not clear what this does. Plays some speech samples? 
	
}

PyObject* PyGame_PartyPool(PyObject*, PyObject* args) {
	uiSystems->GetPartyPool().Show(true);
	Py_RETURN_NONE;
}

PyObject* PyGame_CharUiHide(PyObject*, PyObject* args) {
	uiSystems->GetChar().Hide();
	Py_RETURN_NONE;
}

void UpdateSleepStatus(); // Dirty stuff...
PyObject* PyGame_SleepStatusUpdate(PyObject*, PyObject* args) {
	UpdateSleepStatus();
	Py_RETURN_NONE;
}

PyObject* PyGame_Brawl(PyObject*, PyObject* args) {
	objHndl pc, npc;
	if (!PyArg_ParseTuple(args, "O&O&:game.brawl", &ConvertObjHndl, &pc, &ConvertObjHndl, &npc)) {
		return 0;
	}

	combatSys.Brawl(pc, npc);
	Py_RETURN_NONE;
}

PyObject* PyGame_TutorialToggle(PyObject*, PyObject* args) {
	return PyInt_FromLong(uiTutorial.Toggle());
}

PyObject* PyGame_TutorialIsActive(PyObject*, PyObject* args) {
	return PyInt_FromLong(uiTutorial.IsActive());
}

PyObject* PyGame_TutorialShowTopic(PyObject*, PyObject* args) {
	int topicId;
	if (!PyArg_ParseTuple(args, "i:game.tutorial_show_topic", &topicId)) {
		return 0;
	}

	return PyInt_FromLong(uiTutorial.ShowTopic(topicId));
}

PyObject* PyGame_CombatIsActive(PyObject*, PyObject* args) {
	return PyInt_FromLong(combatSys.isCombatActive());
}

PyObject* PyGame_CreateHistoryFreeform(PyObject*, PyObject* args) {

	char * histText;
	if (!PyArg_ParseTuple(args, "s:game.create_history_freeform", &histText)) {
		Py_RETURN_NONE;
	}
	histSys.CreateFromFreeText(histText);

	Py_RETURN_NONE;
}

PyObject* PyGame_CreateHistoryFromId(PyObject*, PyObject* args) {

	int histId;
	if (!PyArg_ParseTuple(args, "i:game.create_history_from_id", &histId)) {
		Py_RETURN_NONE;
	}
	histSys.CreateRollHistoryString(histId);
	Py_RETURN_NONE;
}

PyObject* PyGame_CreateHistoryFromPattern(PyObject*, PyObject* args) {

	int patternId;
	objHndl handle, handle2;
	if (!PyArg_ParseTuple(args, "iO&O&:game.create_history_from_pattern", &patternId, &ConvertObjHndl, &handle, &ConvertObjHndl, &handle2)) {
		Py_RETURN_NONE;
	}
	histSys.CreateRollHistoryLineFromMesfile(patternId, handle, handle2);
	Py_RETURN_NONE;
}

PyObject* PyGame_SpellMesline(PyObject*, PyObject* args) {
	int line;
	if (!PyArg_ParseTuple(args, "i:game.get_spell_mesline", &line)) {
		Py_RETURN_NONE;
	}
	
	auto text = spellSys.GetSpellMesline(line);
	return PyString_FromString(text);
}

PyObject* PyGame_Mesline(PyObject*, PyObject* args) {
	int line;
	char *file;
	if (!PyArg_ParseTuple(args, "si:game.get_mesline", &file, &line)) {
		Py_RETURN_NONE;
	}

	//Keep the meshandles in a map for the future if it is new
	if (mesMap.find(file) == mesMap.end()) {
		MesHandle m;
		bool bRes = mesFuncs.Open(file, &m);
		if (!bRes) {
			return PyString_FromString(fmt::format("Error! Unable to open mesfile {}", file).c_str());
		}
		else {
			mesMap.emplace(file, m);
		}
	}
	MesLine mesLine(line);
	mesFuncs.GetLine_Safe(mesMap[file], &mesLine);
	return PyString_FromString(mesLine.value);
}

PyObject* PyGame_WrittenUiShow(PyObject*, PyObject* args) {
	objHndl handle;
	if (!PyArg_ParseTuple(args, "O&:game.written_ui_show", &ConvertObjHndl, &handle)) {
		return 0;
	}

	auto result = uiSystems->GetWritten().Show(handle) ? 1 : 0;
	return PyInt_FromLong(result);
}

PyObject* PyGame_IsDaytime(PyObject*, PyObject* args) {
	return PyInt_FromLong(gameSystems->GetTimeEvent().IsDaytime());
}

PyObject* PyGame_IsAlertPopupActive(PyObject*, PyObject* args) {
	
	return PyInt_FromLong(uiSystems->GetHelp().AlertIsActive());
}


static PyObject *PySpell_SpellGetPickerEndPoint(PyObject*, PyObject *args) {

	auto wallEndPt = uiPicker.GetWallEndPoint();
	py::object blyat = py::cast(wallEndPt);
	blyat.inc_ref();
	return blyat.ptr();
}

static PyObject* PyGame_GetObjById(PyObject*, PyObject* args) {
	char* name;
	if (!PyArg_ParseTuple(args, "s:game.get_obj_by_id", &name)) {
		return 0;
	}
	if (!name) {
		return PyObjHndl_CreateNull();
	}
	auto handle = objSystem->FindObjectByIdStr(format("{}", name));
	return PyObjHndl_Create(handle);
}

PyObject* PyGame_AlertShow(PyObject*, PyObject* args) {
	char* text, *button_text;
	if (!PyArg_ParseTuple(args, "ss:game.alert_show", &text, &button_text)) {
		return 0;
	}
	if (!text || !button_text) {
		return 0;
	}
	auto result = UiAlert::ShowEx(2, 0, 0, button_text, text);
	return PyInt_FromLong(result);
}

static PyMethodDef PyGameMethods[]{
	{ "get_wall_endpt", PySpell_SpellGetPickerEndPoint, METH_VARARGS, NULL },
	{ "create_history_freeform", PyGame_CreateHistoryFreeform, METH_VARARGS, NULL },
	{ "create_history_from_id", PyGame_CreateHistoryFromId, METH_VARARGS, NULL },
	{ "create_history_from_pattern", PyGame_CreateHistoryFromPattern, METH_VARARGS, NULL },
	{ "get_spell_mesline", PyGame_SpellMesline, METH_VARARGS, NULL },
	{ "get_mesline", PyGame_Mesline, METH_VARARGS, NULL },
	{"fade_and_teleport", PyGame_FadeAndTeleport, METH_VARARGS, NULL},
	{"fade", PyGame_Fade, METH_VARARGS, NULL},
	{ "fnn", PyGame_FindNpcNear, METH_VARARGS, NULL },
	{"party_size", PyGame_PartySize, METH_VARARGS, NULL},
	{"party_pc_size", PyGame_PartyPcSize, METH_VARARGS, NULL},
	{"party_npc_size", PyGame_PartyNpcSize, METH_VARARGS, NULL},
	{"obj_list", PyGame_ObjList, METH_VARARGS, NULL},
	{"obj_list_vicinity", PyGame_ObjListVicinity, METH_VARARGS, NULL},
	{"obj_list_range", PyGame_ObjListRange, METH_VARARGS, NULL},
	{"obj_list_cone", PyGame_ObjListCone, METH_VARARGS, NULL},
	{"sound", PyGame_Sound, METH_VARARGS, NULL},
	// Not implemented, because it needs internal knowledge about the sound coordinate space
	// {"sound_local_xy", PyGame_SoundLocalXY, METH_VARARGS, NULL},
	{"sound_local_obj", PyGame_SoundLocalObj, METH_VARARGS, NULL},
	{"sound_local_loc", PyGame_SoundLocalLoc, METH_VARARGS, NULL},
	{"particles", PyGame_Particles, METH_VARARGS, NULL},
	{"obj_create", PyGame_ObjCreate, METH_VARARGS, NULL},
	{"timeevent_add", PyGame_TimeEventAdd, METH_VARARGS, NULL},
	{"timevent_add", PyGame_TimeEventAdd, METH_VARARGS, NULL},
	{"map_flags", PyGame_MapFlags, METH_VARARGS, NULL},
	{"particles_kill", PyGame_ParticlesKill, METH_VARARGS, NULL},
	{"particles_end", PyGame_ParticlesEnd, METH_VARARGS, NULL},
	{"savegame", PyGame_SaveGame, METH_VARARGS, NULL},
	{"loadgame", PyGame_LoadGame, METH_VARARGS, NULL},
	{"update_combat_ui", PyGame_UpdateCombatUi, METH_VARARGS, NULL},
	{"update_party_ui", PyGame_UpdatePartyUi, METH_VARARGS, NULL},
	{"random_range", PyGame_RandomRange, METH_VARARGS, NULL},
	{"target_random_tile_near_get", PyGame_TargetRandomTileNearGet, METH_VARARGS, NULL},
	{"get_stat_mod", PyGame_GetStatMod, METH_VARARGS, NULL},
	{"get_deity_favored_weapon", PyGame_GetDeityFavoredWeapon, METH_VARARGS, NULL},
	{"ui_show_worldmap", PyGame_UiShowWorldmap, METH_VARARGS, NULL},
	{"worldmap_travel_by_dialog", PyGame_WorldmapTravelByDialog, METH_VARARGS, NULL},
	{"pfx_call_lightning", PyGame_PfxCallLightning, METH_VARARGS, NULL},
	{"pfx_chain_lightning", PyGame_PfxChainLightning, METH_VARARGS, NULL},
	{"pfx_lightning_bolt", PyGame_PfxLightningBolt, METH_VARARGS, NULL},
	{"gametime_add", PyGame_GametimeAdd, METH_VARARGS, NULL},
	{"is_ingame", PyGame_IsIngame, METH_VARARGS, "Is in active game (i.e. not in main menu screen)"},
	{"is_outdoor", PyGame_IsOutdoor, METH_VARARGS, NULL},
	{"is_spell_harmful", PyGame_IsSpellHarmful, METH_VARARGS, NULL },
	{"keypress", PyGame_Keypress, METH_VARARGS, NULL},
	{ "scroll_to", PyGame_ScrollTo, METH_VARARGS, NULL },
	{"mouse_move_to", PyGame_MouseMoveTo, METH_VARARGS, NULL },
	{"mouse_click", PyGame_MouseClick, METH_VARARGS, NULL },
	{"shake", PyGame_Shake, METH_VARARGS, NULL},
	{"moviequeue_add", PyGame_MoviequeueAdd, METH_VARARGS, NULL},
	{"moviequeue_play", PyGame_MoviequeuePlay, METH_VARARGS, NULL},
	{"moviequeue_play_end_game", PyGame_MoviequeuePlayEndGame, METH_VARARGS, NULL},
	{"picker", PyGame_Picker, METH_VARARGS, NULL},
	{"party_pool", PyGame_PartyPool, METH_VARARGS, NULL},
	{"char_ui_hide", PyGame_CharUiHide, METH_VARARGS, NULL},
	{"sleep_status_update", PyGame_SleepStatusUpdate, METH_VARARGS, NULL},
	{"brawl", PyGame_Brawl, METH_VARARGS, NULL},
	{"tutorial_toggle", PyGame_TutorialToggle, METH_VARARGS, NULL},
	{"tutorial_is_active", PyGame_TutorialIsActive, METH_VARARGS, NULL},
	{"tutorial_show_topic", PyGame_TutorialShowTopic, METH_VARARGS, NULL},
	{"combat_is_active", PyGame_CombatIsActive, METH_VARARGS, NULL},
	{"written_ui_show", PyGame_WrittenUiShow, METH_VARARGS, NULL},
	{"is_alert_popup_active", PyGame_IsAlertPopupActive, METH_VARARGS, NULL},
	{"is_daytime", PyGame_IsDaytime, METH_VARARGS, NULL},

	{"damage_type_match", PyGame_DamageTypeMatch, METH_VARARGS, NULL },
	{"vlist", PyGame_Vlist, METH_VARARGS, NULL },
	{"getproto", PyGame_GetProto, METH_VARARGS, NULL },
	{"get_bab_for_class", PyGame_GetBabForClass,METH_VARARGS, NULL },
	{ "get_weapon_type_for_feat", PyGame_GetWpnTypeForFeat,METH_VARARGS, NULL },
	{ "get_feat_for_weapon_type", PyGame_GetFeatForWpnType,METH_VARARGS, NULL },
	{ "get_weapon_damage_type", PyGame_GetWpnDamType,METH_VARARGS, NULL },
	{"is_save_favored_for_class", PyGame_IsSaveFavoerdForClass,METH_VARARGS, NULL },
	{ "is_lax_rules", PyGame_IsLaxRules,METH_VARARGS, NULL },
	{"get_feat_name", PyGame_GetFeatName,METH_VARARGS, NULL},
	{"is_ranged_weapon", PyGame_IsRangedWeapon,METH_VARARGS, NULL},
	{"is_melee_weapon", PyGame_IsMeleeWeapon,METH_VARARGS, NULL},
	{"make_custom_name", PyGame_MakeCustomName,METH_VARARGS, NULL},
	{"get_obj_by_id", PyGame_GetObjById, METH_VARARGS, NULL},
	{"alert_show", PyGame_AlertShow, METH_VARARGS, NULL},
	// This is some unfinished UI for which the graphics are missing
	// {"charmap", PyGame_Charmap, METH_VARARGS, NULL},
	{NULL, NULL, NULL, NULL}
};

static void PyGame_Dealloc(PyObject* obj) {
	auto self = (PyGameObject*)obj;
	Py_DECREF(self->globalVars);
	Py_DECREF(self->globalFlags);
	Py_DECREF(self->quests);
	Py_DECREF(self->areas);
	PyObject_Del(self);
}

static PyTypeObject PyGameType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"PyGame", /*tp_name*/
	sizeof(PyGameObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	PyGame_Dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PyGameMethods, /* tp_methods */
	0, /* tp_members */
	PyGameGettersSetters, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	0, /* tp_init */
	0, /* tp_alloc */
	0, /* tp_new */
};

PyObject* PyGame_Create() {
	auto result = PyObject_New(PyGameObject, &PyGameType);
	result->globalVars = PyGlobalVars_Create();
	result->globalFlags = PyGlobalFlags_Create();
	result->quests = PyQuests_Create();
	result->areas = PyAreas_Create();
	return (PyObject*) result;
}

void PyGame_Exit() {
	Py_XDECREF(encounterQueue);
	encounterQueue = nullptr;
	Py_XDECREF(dialogPickerArgs.isValidTarget);
	dialogPickerArgs.isValidTarget = nullptr;

	//Cleanup any stored mes file handles
	for (const auto& mes : mesMap) {
		mesFuncs.Close(mes.second);
	}
	mesMap.clear();
}

void PyGame_Reset() {
	PyGame_Exit();
}

bool PyGame_Save(TioFile* file) {

	if (!encounterQueue) {
		int count = 0;
		if (tio_fwrite(&count, 4, 1, file) != 1) {
			logger->error("Unable to write encounter queue size.");
			return false;
		}
		return true;
	}
		
	int count = PyList_Size(encounterQueue);
	if (tio_fwrite(&count, 4, 1, file) != 1) {
		logger->error("Unable to write encounter queue size.");
		return false;
	}

	for (int i = 0; i < count; ++i) {
		auto encounterId = PyInt_AsLong(PyList_GET_ITEM(encounterQueue, i));
		if (tio_fwrite(&encounterId, 4, 1, file) != 1) {
			logger->error("Unable to write encounter id {}", encounterId);
			return false;
		}
	}

	return true;
}

bool PyGame_Load(GameSystemSaveFile *saveFile) {
	Py_XDECREF(encounterQueue);
	encounterQueue = nullptr;

	int count;
	if (tio_fread(&count, 4, 1, saveFile->file) != 1) {
		logger->error("Unable to read encounter queue size from savegame.");
		return false;
	}

	encounterQueue = PyList_New(count);
	for (int i = 0; i < count; ++i) {
		int encounterId;
		if (tio_fread(&encounterId, 4, 1, saveFile->file) != 1) {
			logger->error("Unable to read encounter id from savegame.");
			Py_DECREF(encounterQueue);
			encounterQueue = nullptr;
			return false;
		}
		PyList_SET_ITEM(encounterQueue, i, PyInt_FromLong(encounterId));
	}

	return true;
}
