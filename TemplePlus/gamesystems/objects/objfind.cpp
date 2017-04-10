#include "stdafx.h"
#include "objfind.h"

#include <temple/dll.h>

void ObjFindSupport::Remove(objHndl handle, GameObjectBody * obj)
{
	static auto obj_find_remove = temple::GetPointer<void(objHndl)>(0x100c11f0);

	auto findNode = obj->GetInt32(obj_f_find_node);
	if (!findNode)
		return;
	obj_find_remove(handle);
}
