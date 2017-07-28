
#include "stdafx.h"
#include "secret_door.h"
#include "obj.h"
#include "util/fixes.h"
#include "gamesystems/objects/objsystem.h"
#include "objlist.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/particlesystems.h"
#include "float_line.h"


SecretDoorSys secretdoorSys;

class SecretDoorReplacements : public TempleFix
{
public:
	void apply() override{
		replaceFunction(0x10046470, _isSecretDoor); 
		replaceFunction(0x10046510, _getSecretDoorDC); 
		replaceFunction(0x100464A0, _secretDoorIsRevealed); 
		static BOOL (__cdecl*orgSecretDoorDetect)(objHndl , objHndl) = replaceFunction<BOOL(objHndl,objHndl)>(0x10046920,[](objHndl sd, objHndl seeker)
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
			auto sdCtrl = objHndl::null;
			ObjList objList;
			objList.ListVicinity(sd, OLC_ALL);
			for (auto i=0; i<objList.size(); i++)
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
			
			auto aasHndl = objects.GetAasHandle(sd);
			if (!aasHndl){
				logger->error("Secret door: cannot start particle efgect, noanimhandlefor {}", sd);
				return FALSE;
			}

			for (auto i=0; i<29;i++)
			{
				auto boneName = fmt::format("effect-secretdoor-%02d", i);
				auto hasBone = temple::GetRef<BOOL(__cdecl)(int, const char*)>(0x10263A10);
				if (!hasBone(aasHndl, boneName.c_str() ))
				{
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
		});
	}
} secDoorReplacements;



bool SecretDoorSys::isSecretDoor(objHndl obj)
{
	return (objects.getInt32(obj, obj_f_secretdoor_flags) & OSDF_SECRET_DOOR) != 0;
}

int SecretDoorSys::getSecretDoorDC(objHndl obj)
{
	if (obj && isSecretDoor(obj)) {
		return objects.getInt32(obj, obj_f_secretdoor_dc) & 0x7F;
	}
	return 0;
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