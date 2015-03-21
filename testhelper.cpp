#include "stdafx.h"
#include "temple_functions.h"
#include "testhelper.h"


void MakeWizard(objHndl objHnd, int level){
	for (int n = 0; n < level; n++){
		templeFuncs.Obj_Set_IdxField_byValue(objHnd, obj_f_critter_level_idx, n, stat_level_wizard);
	}
	templeFuncs.Obj_Set_IdxField_byValue(objHnd, obj_f_critter_level_idx, level, 0);
	return;
};