
#include "stdafx.h"
#include <infrastructure/meshes.h>
#include "secret_door.h"
#include "obj.h"
#include "util/fixes.h"
#include "gamesystems/objects/objsystem.h"
#include "objlist.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/particlesystems.h"
#include "float_line.h"
#include "python/python_integration_obj.h"
#include "history.h"
#include "combat.h"
#include "critter.h"
#include "party.h"
#include "location.h"
#include "animgoals\anim.h"
#include <ui/ui_dialog.h>
#include <config/config.h>
#include <rng.h>

constexpr int TAGGED_SCENERY_START = 1800;
constexpr int TAGGED_SCENERY_END = 1979;
constexpr int NAMES_SEEN_CAP = 100;

//*****************************************************************************
//* Secretdoor
//*****************************************************************************

SecretdoorSystem::SecretdoorSystem(const GameSystemConf& config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10046370);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Secretdoor");
	}
}
void SecretdoorSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10046390);
	reset();

	// Temple+: adds reset to namesSeen; fixes SecretDoor bug at the root
	memset(mNamesSeen, 0, sizeof(mNamesSeen[0]) * NAMES_SEEN_CAP);
}
bool SecretdoorSystem::SaveGame(TioFile* file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x100463b0);
	return save(file) == 1;
}
bool SecretdoorSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x10046400);
	return load(saveFile) == 1;
}
const std::string& SecretdoorSystem::GetName() const {
	static std::string name("Secretdoor");
	return name;
}


SecretDoorSys secretdoorSys;

class SecretDoorReplacements : public TempleFix
{
public:
	void apply() override{
		replaceFunction(0x10046470, _isSecretDoor); 
		replaceFunction(0x10046510, _getSecretDoorDC); 
		replaceFunction(0x100464A0, _secretDoorIsRevealed); 

		// Automatic detection from SearchTimeEventExpires
		static int(__cdecl * orgSearchEventSecretDoorDetect)(objHndl, objHndl) = replaceFunction<int(objHndl, objHndl)>(0x10046CE0, [](objHndl sd, objHndl seeker)
		{
			return secretdoorSys.SearchEventSecretDoorDetect(sd, seeker);
		});

		// To check if suitable for auto detection
		static int(__cdecl * orgSecretDoorGetDC)(objHndl) = replaceFunction<int(objHndl)>(0x100465B0, [](objHndl secdoor)
		{
			return secretdoorSys.SecretDoorGetDCAutoDetect(secdoor) - 1;
		});

		// Secret Door detected
		static BOOL (__cdecl*orgSecretDoorDetect)(objHndl , objHndl) = replaceFunction<BOOL(objHndl,objHndl)>(0x10046920,[](objHndl sd, objHndl seeker)
		{
			return secretdoorSys.SecretDoorDetect(sd, seeker);
		});

		// Manual search
		static int(__cdecl * orgSearchConcentratedPerformFunc)(D20Actn*) = replaceFunction<int(D20Actn*)>(0x10090F50, [](D20Actn* d20a)
		{
			return secretdoorSys.SearchConcentratedPerformFunc(d20a);
		});

		// Cooperated search for traps and secret doors
		static int(__cdecl * orgSecretDoorSkillCheck)(objHndl, objHndl*) = replaceFunction<int(objHndl, objHndl*)>(0x1007DBD0, [](objHndl seeker, objHndl* foundObject)
		{
			return secretdoorSys.SecretDoorSkillCheck(seeker, foundObject);
		});

		// Cooperated roll and check for secret door
		static int(__cdecl * orgSecretDoorRollAndReveal)(objHndl, objHndl, BonusList*) = replaceFunction<int(objHndl, objHndl, BonusList*)>(0x10046DB0, [](objHndl secdoor, objHndl seeker, BonusList *bonList)
		{
			return secretdoorSys.SecretDoorRollAndReveal(secdoor, seeker, bonList);
		});

		replaceFunction<BOOL(objHndl)>(0x10046650, [](objHndl handle) {
			return secretdoorSys.TaggedSceneryWasSeen(handle) ? TRUE:FALSE;
			});

		replaceFunction<void(__cdecl)(objHndl)>(0x100466B0, [](objHndl handle) {
			return secretdoorSys.MarkTaggedScenerySeenAndPlayVoice(handle);
			});
			
	}
} secDoorReplacements;



bool SecretDoorSys::isSecretDoor(objHndl obj)
{
	return (objects.getInt32(obj, obj_f_secretdoor_flags) & OSDF_SECRET_DOOR) != 0;
}

int SecretDoorSys::getSecretDoorDC(objHndl obj)
{
	uint32_t result = 0;
	if (obj && isSecretDoor(obj)) {
		result = objects.getInt32(obj, obj_f_secretdoor_dc) & 0x7F;
	}
	return result;
}

uint32_t SecretDoorSys::secretDoorIsRevealed(objHndl secDoor)
{	// this function replacement should fix the motherfucking secretdoor bug
	// it worked on Aquinas' save!
	// The problem was that there's some sort of "revealed scenery" list which for some reason the secret door appears there
	// so I just removed that - there's the secret_door_found flag anyway
	// come to think of it, it may have supposed to prevent the player from searching again and again, but fuckit
	// they take 20 anyway (well maybe not in KotB >_>)
	if (!secDoor) return 0;
	uint32_t secDoorFlags = objects.getInt32(secDoor, obj_f_secretdoor_flags);
	if ( (secDoorFlags & OSDF_SECRET_DOOR) == 0 )  return 0;
	return (secDoorFlags & OSDF_SECRET_DOOR_FOUND) != 0;
	// there was also a look up in some name table, but that seems to be bugged (the door would already appear there in the secret door bug case, thus preventing its revelation)
}

int SecretDoorSys::SearchConcentratedPerformFunc(D20Actn* d20a)
{
	objHndl foundObject = objHndl::null;
	int found = secretdoorSys.SecretDoorSkillCheck(d20a->d20APerformer, &foundObject);
	int lineId = 1300;
	if (found)
		lineId = 1301;

	MesHandle mesHnd;
	MesLine line(lineId);
	mesFuncs.Open("mes\\skill_ui.mes", &mesHnd);
	mesFuncs.GetLine_Safe(mesHnd, &line);
	if (found)
		floatSys.floatMesLine(foundObject, 1, FloatLineColor::White, line.value);
	else 
		floatSys.floatMesLine(party.GetConsciousPartyLeader(), 1, FloatLineColor::White, line.value);
	mesFuncs.Close(mesHnd);
	return 0;
}

// Originally @ 0x1007DBD0

int SecretDoorSys::SecretDoorSkillCheck(objHndl seeker, objHndl* foundObject)
{
	BonusList bonList;
	// get Cooperation bonuses
	{
		ObjList objlist;
		objlist.ListVicinity(seeker, OLC_CRITTERS);
		for (objHndl subject : objlist) {
			if (subject.handle != seeker.handle) {
				if (party.IsInParty(subject)) {
					if (!critterSys.IsDeadOrUnconscious(subject)) {
						// check if subject rolled at least 10 in Search
						if (skillSys.SkillCheckDefaultDC(SkillEnum::skill_search, subject, 1)) {
							gameSystems->GetAnim().Interrupt(subject, AnimGoalPriority::AGP_3, true);
							gameSystems->GetAnim().PushAnimate(subject, 112);
							bonList.AddBonusWithDesc(2, 21, 144, description.getDisplayName(subject)); // {144}{Cooperation ~(Search)~[TAG_SEARCH]}
						}
					}
				}
			}
		}
	}
	gameSystems->GetAnim().Interrupt(seeker, AnimGoalPriority::AGP_3, true);
	gameSystems->GetAnim().PushAnimate(seeker, 112);
	{
		auto static HasLineOfAttack = temple::GetRef<BOOL(objHndl obj, objHndl targetObj, int* hasCover)>(0x100570C0);
		auto static trapSthgsub_10050E30 = temple::GetRef<int(objHndl obj, int a2)>(0x10050E30);
		auto static SecretDoorTrapFindCheck = temple::GetRef<int(objHndl performer, objHndl trap, BonusList *bonList)>(0x10051350);

		ObjList objlist;
		objlist.ListVicinity(seeker, OLC_ALL);
		for (objHndl handle : objlist) {
			if (HasLineOfAttack(seeker, handle, 0)) {
				auto obj = gameSystems->GetObj().GetObject(handle);
				if (trapSthgsub_10050E30(handle, 43) || obj->type == ObjectType::obj_t_trap) {
					if (SecretDoorTrapFindCheck(seeker, handle, &bonList)) {
						foundObject->handle = handle.handle;
						return TRUE;
					}
				}
				else if (secretdoorSys.isSecretDoor(handle) && secretdoorSys.SecretDoorRollAndReveal(handle, seeker, &bonList)) {
					foundObject->handle = handle.handle;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

// Originally @ 0x10046CE0

int SecretDoorSys::SearchEventSecretDoorDetect(objHndl sd, objHndl seeker)
{
	if (sd) {
		if (secretdoorSys.isSecretDoor(sd)) {
			auto static HasLineOfAttack = temple::GetRef<BOOL(objHndl obj, objHndl targetObj, int* hasCover)>(0x100570C0);
			if (HasLineOfAttack(seeker, sd, 0)) {
				auto autoDetectDC = secretdoorSys.SecretDoorGetDCAutoDetect(sd);

				auto skillRanksSearch = critterSys.SkillBaseGet(seeker, SkillEnum::skill_search);
				if (skillRanksSearch >= autoDetectDC) {
					auto dc = secretdoorSys.getSecretDoorDC(sd);
					auto result = skillSys.SkillRoll(seeker, SkillEnum::skill_search, dc, 0, 4);
					if (result) {
						return secretdoorSys.SecretDoorDetect(sd, seeker);
					}

					auto v5 = (((uint16_t)skillRanksSearch + 1) << 7) & 0x3F80;
					auto sdFlags = objects.getInt32(sd, obj_f_secretdoor_flags);
					sdFlags = sdFlags & 0xFFFFC07F | v5;
					objects.setInt32(sd, obj_f_secretdoor_flags, sdFlags);
				}
			}
		}
	}
	return FALSE;
}

// Originally @ 0x10046DB0

int SecretDoorSys::SecretDoorRollAndReveal(objHndl secdoor, objHndl seeker, BonusList* bonList)
{
	if (!secdoor || !seeker || !secretdoorSys.isSecretDoor(secdoor) || secretdoorSys.secretDoorIsRevealed(secdoor))
		return FALSE;
	if (d20Sys.d20Query(seeker, DK_QUE_CannotUseIntSkill))
		return FALSE;

	auto DC = secretdoorSys.getSecretDoorDC(secdoor);
	auto searchLevel = dispatch.dispatch1ESkillLevel(seeker, SkillEnum::skill_search, bonList, secdoor, 4);
	//Dice dice(1, 20, 0);
	auto rollResult = 20 + searchLevel; // Take 20
	if (rollResult >= DC) {
		//auto histId = histSys.RollHistoryAddType2SkillRoll(seeker, dice.ToPacked(), 20, DC, SkillEnum::skill_search-2, bonList); // see HistoryType2Parse
		//histSys.CreateRollHistoryString(histId);

		return secretdoorSys.SecretDoorDetect(secdoor, seeker);
	}
	return FALSE;
}

// Originally @ 0x10046920

BOOL SecretDoorSys::SecretDoorDetect(objHndl sd, objHndl seeker)
{
	if (!sd || !seeker)
		return FALSE;
	if (!secretdoorSys.isSecretDoor(sd))
		return FALSE;
	if (_secretDoorIsRevealed(sd))
		return FALSE;
	auto seekerObj = objSystem->GetObject(seeker);
	if (!seekerObj->IsPC())
		return FALSE;
	auto sdObj = objSystem->GetObject(sd);

	auto markSeen = temple::GetRef<void(__cdecl)(objHndl)>(0x10046620);
	markSeen(sd);

	auto sdEffectName = sdObj->GetInt32(obj_f_secretdoor_effectname);
	if (sdEffectName) {
		auto sdCtrl = objHndl::null;
		ObjList objList;
		objList.ListVicinity(sd, OLC_ALL);
		for (auto i = 0; i < objList.size(); i++)
		{
			auto res = objList[i];
			if (!res) continue;
			if (objSystem->GetObject(res)->GetInt32(obj_f_name) == sdEffectName)
			{
				sdCtrl = res;
				objects.FadeTo(sdCtrl, 0, 12, 2, 0);
				break;
			}
		}
	}

	auto aasHndl = objects.GetAnimHandle(sd);
	if (!aasHndl) {
		logger->error("Secret door: cannot start particle effect, noanimhandlefor {}", sd);
		return FALSE;
	}

	for (auto i = 0; i < 29; i++)
	{
		auto boneName = fmt::format("effect-secretdoor-{:02d}", i);
		if (aasHndl->HasBone(boneName)) {
			gameSystems->GetParticleSys().CreateAtObj(boneName, sd);
		}
	}

	if (sdObj->type != obj_t_portal)
	{
		objects.SetTransparency(sd, 0);
		objects.FadeTo(sd, 255, 10, 2);
	}
	MesHandle mesHnd;
	MesLine line(1000);
	mesFuncs.Open("mes\\skill_ui.mes", &mesHnd);
	mesFuncs.GetLine_Safe(mesHnd, &line);
	floatSys.floatMesLine(sd, 1, FloatLineColor::Blue, line.value);
	mesFuncs.Close(mesHnd);
	return TRUE;
}

// Originally @ 0x100465B0

int SecretDoorSys::SecretDoorGetDCAutoDetect(objHndl secdoor)
{
	if (!secdoor || !secretdoorSys.isSecretDoor(secdoor))
		return 0;
	auto secretdoor_dc = objects.getInt32(secdoor, obj_f_secretdoor_dc);
	return (secretdoor_dc >> 7) & 0x7F;
}

/* 0x10046650 */
bool SecretDoorSys::TaggedSceneryWasSeen(objHndl scenery)
{
	if (!scenery || !objSystem->IsValidHandle(scenery))
		return false;

	auto name = objects.getInt32(scenery, obj_f_name);
	if (name < TAGGED_SCENERY_START || name > TAGGED_SCENERY_END) {
		return false;
	}
	logger->trace("Tagged scenery {} seen, name ID {}.", scenery, name);
	auto mNamesSeen = gameSystems->GetSecretdoor().mNamesSeen;
	for (auto i = 0; i < NAMES_SEEN_CAP; ++i) {
		if (!mNamesSeen[i]) // end of list
			return false;

		if (mNamesSeen[i] == name) {
			return true;
		}
	}
	return false;
}

/* 0x100466B0 */
void SecretDoorSys::MarkTaggedScenerySeenAndPlayVoice(objHndl objSeen) {
	
	ObjList olist;
	olist.ListVicinity(objSeen, OLC_CRITTERS);
	
	
	if (!olist.size()) {
		return;
	}
	auto result = olist.GetListResult();
	for (auto it = result.begin(); it != result.end(); ) {
		auto critter = *it;
		if (!party.IsInParty(critter) || critterSys.IsDeadOrUnconscious(critter)) {
			it = result.erase(it);
			continue;
		}
		++it;	
	}
	if (!result.size())
		return;

	auto found = false;

	char text[1000]; int soundId = -1;
	static auto GetTaggedSceneryVoiceLine = temple::GetRef<void(objHndl, objHndl, objHndl, char[], int&)>(0x100381D0);

	auto talker = objHndl::null, pc = objHndl::null;
	
	std::vector<objHndl> withVoice;
	std::vector<objHndl> withoutVoice;
	for (auto& it : result) {
		auto pc = party.GetFellowPc(it);
		GetTaggedSceneryVoiceLine(it, pc, objSeen, text, soundId);
		if (soundId != -1) {
			withVoice.push_back(it);
		}
		else {
			withoutVoice.push_back(it);
		}
	}

	if (withVoice.size()) {
		talker = withVoice[rngSys.GetInt(0, withVoice.size() - 1)];
	}
	else {
		talker = withoutVoice[rngSys.GetInt(0, withoutVoice.size() - 1)];
	}
	
	pc = party.GetFellowPc(talker);
	GetTaggedSceneryVoiceLine(talker, pc, objSeen, text, soundId);
	if (!text[0])
		return;

	//auto banterText = config.GetVanillaInt("party_banter_text");
	auto banterVoice = temple::GetRef<BOOL>(0x1080ABA0) != 0;
	//if (banterText || banterVoice && soundId == -1 ) 
	if (banterVoice)
	{
		uiDialog->ShowTextBubble(talker, objHndl::null, text, soundId);
	}
	//else 
	//{
	//	critterSys.PlayCritterVoiceLine(talker, pc, text, soundId);
	//}
	
	found = true;

	// mark name seen
	if (found) {
		auto name = objects.getInt32(objSeen, obj_f_name);
		auto namesSeen = gameSystems->GetSecretdoor().mNamesSeen;
		for (auto i = 0; i < NAMES_SEEN_CAP; ++i) {
			if (!namesSeen[i]) {
				namesSeen[i] = name;
				return;
			}
		}
	}
	
}

SecretDoorSys::SecretDoorSys()
{
}

bool _isSecretDoor(objHndl obj)
{
	return secretdoorSys.isSecretDoor(obj);
}

int _getSecretDoorDC(objHndl obj)
{
	return secretdoorSys.getSecretDoorDC(obj);
}

uint32_t _secretDoorIsRevealed(objHndl secDoor)
{
	return secretdoorSys.secretDoorIsRevealed(secDoor);
}