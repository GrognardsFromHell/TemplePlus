
#include "stdafx.h"

#include "common.h"
#include <temple/dll.h>
#include "description.h"

DescriptionSystem description;

const char* DescriptionSystem::getDisplayName(objHndl obj)
{
	return _getDisplayName(obj, obj);
}

const char* DescriptionSystem::getDisplayName(objHndl obj, objHndl observer)
{
	return _getDisplayName(obj, observer);
}