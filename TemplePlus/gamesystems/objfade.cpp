#include "stdafx.h"
#include <temple/dll.h>
#include <gamesystems/objfade.h>
#include "idxtables.h"
#include "timeevents.h"
#include "obj.h"
#include "objects/objsystem.h"
#include "gamesystems.h"

//*****************************************************************************
//* ObjFade
//*****************************************************************************


IdxTableWrapper<ObjFadeArgs> objFadeTable(0x10AA3230);

ObjFadeSystem::ObjFadeSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1004c130);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system ObjFade");
	}
}
ObjFadeSystem::~ObjFadeSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1004c170);
	shutdown();
}
void ObjFadeSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1004c190);
	reset();
}
bool ObjFadeSystem::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x1004c1c0);
	return save(file) == 1;
}
bool ObjFadeSystem::LoadGame(GameSystemSaveFile* saveFile) {
	auto load = temple::GetPointer<int(GameSystemSaveFile*)>(0x1004c220);
	return load(saveFile) == 1;
}
const std::string &ObjFadeSystem::GetName() const {
	static std::string name("ObjFade");
	return name;
}

void ObjFadeSystem::SetValidationObj(objHndl handle) {
	temple::GetRef<objHndl>(0x10AA3250) = handle;
}

objHndl ObjFadeSystem::GetValidationObj()
{
	return temple::GetRef<objHndl>(0x10AA3250);
}

int ObjFadeSystem::AppendToTable(int quantum, int initialOpacity, int goalOpacity, int tickTimeMs, int flags){

	auto &objFadeSerial = temple::GetRef<int>(0x10AA3240);

	auto result = objFadeSerial;

	ObjFadeArgs objFadeArgs;
	objFadeArgs.id = objFadeSerial;
	objFadeArgs.initialOpacity = initialOpacity;
	objFadeArgs.goalOpacity = goalOpacity;
	objFadeArgs.tickQuantum = quantum;
	objFadeArgs.flags = flags;
	objFadeArgs.tickMs = tickTimeMs;

	objFadeTable.put(objFadeArgs.id, objFadeArgs);

	objFadeSerial++;

	return result;
}

void ObjFadeSystem::RemoveFromTable(int id){
	objFadeTable.remove(id);
}

int ObjFadeSystem::TimeEventExpired(const TimeEvent * evt){

	auto id = evt->params[0].int32;
	auto handle = evt->params[1].handle;
	auto fadeArgs = objFadeTable.get(id);
	if (!fadeArgs){
		logger->error("ObjFadeSystem::TimeEventExpired: Unknown fade id {}", id);
		return TRUE;
	}

	auto curOpacity = objSystem->GetObject(handle)->GetInt32(obj_f_transparency);
	auto goalOpacity = fadeArgs->goalOpacity;

	int newOpacity;
	if (fadeArgs->initialOpacity <= goalOpacity){
		newOpacity = curOpacity + fadeArgs->tickQuantum;
		if (newOpacity >= goalOpacity){
			objects.SetTransparency(handle, goalOpacity);
			return TRUE;
		}
	}
	else{
		newOpacity = curOpacity - fadeArgs->tickQuantum;
		if (newOpacity <= goalOpacity) {
			objects.SetTransparency(handle, goalOpacity);
			return TRUE;
		}
	}

	TimeEvent newEvt;
	newEvt.system = TimeEventType::ObjFade;
	newEvt.params[0].int32 = id;
	newEvt.params[1].handle = handle;
	
	
	gameSystems->GetTimeEvent().Schedule(newEvt, fadeArgs->tickMs);
	objects.SetTransparency(handle, newOpacity);
	return TRUE;

}
