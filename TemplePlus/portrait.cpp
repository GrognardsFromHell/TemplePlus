

#include "stdafx.h"
#include "common.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/legacysystems.h"

class PortraitSystemHooks : public TempleFix{

	static int GetPortraitId(objHndl handle);
	void apply() override{
		replaceFunction(0x1007DEC0, GetPortraitId);

		replaceFunction<BOOL(__cdecl)(objHndl , int* )>(0x1007DEE0, [](objHndl handle, int* idxOut){
			return gameSystems->GetPortrait().GetFirstId(handle, idxOut)?TRUE:FALSE;
		});

		replaceFunction<BOOL(__cdecl)(objHndl, int*)>(0x1007DF70, [](objHndl handle, int* idxOut) {
			return gameSystems->GetPortrait().GetNextId(handle, idxOut) ? TRUE : FALSE;
		});
	}
} portraitHooks;

int PortraitSystemHooks::GetPortraitId(objHndl handle){
	return objSystem->GetObject(handle)->GetInt32(obj_f_critter_portrait);
}
