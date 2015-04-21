
#include "stdafx.h"

#include "common.h"
#include "util/addresses.h"
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