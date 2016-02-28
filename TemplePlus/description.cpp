
#include "stdafx.h"

#include "common.h"
#include <temple/dll.h>
#include "description.h"

LegacyDescriptionSystem description;

const char* LegacyDescriptionSystem::getDisplayName(objHndl obj)
{
	if (obj)
		return _getDisplayName(obj, obj);
	else
		return "OBJ_HANDLE_NULL";
}

const char* LegacyDescriptionSystem::getDisplayName(objHndl obj, objHndl observer)
{
	return _getDisplayName(obj, observer);
}