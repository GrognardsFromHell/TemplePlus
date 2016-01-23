#include "stdafx.h"
#include "objfind.h"

#include <temple/dll.h>

void ObjFindSupport::Remove(objHndl handle, GameObjectBody * obj)
{
	static auto obj_find_remove = temple::GetPointer<void(objHndl)>(0x100c11f0);
	obj_find_remove(handle);
}
