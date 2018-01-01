

#include "stdafx.h"
#include <iostream>

#include "obj_structs.h"
#include "description.h"
#include "gamesystems/objects/objsystem.h"

const objHndl objHndl::null{ 0 };

std::ostream &operator <<(std::ostream &out, const objHndl &handle) {
	if (!handle || !objSystem->IsValidHandle(handle)) {
		out << "OBJ_HANDLE_NULL";
	} else {
		auto d = description.getDisplayName(handle);
		if (d != nullptr){
			out << d
				<< " [0x" << std::hex << handle.handle << std::dec << "]";
		}
		else{
			out << "DESCRIPTION MISSING"
				<< " [0x" << std::hex << handle.handle << std::dec << "]";
		}
		
	}
	return out;
}

std::ostream & operator<<(std::ostream & out, ObjectIdKind kind)
{
	switch (kind) {
	case ObjectIdKind::Null:
		out << "Null";
		break;
	case ObjectIdKind::Prototype:
		out << "Prototype";
		break;
	case ObjectIdKind::Permanent:
		out << "Permanent";
		break;
	case ObjectIdKind::Positional:
		out << "Positional";
		break;
	case ObjectIdKind::Handle:
		out << "Handle";
		break;
	case ObjectIdKind::Blocked:
		out << "Blocked";
		break;
	default:
		out << "Unknown";
		break;
	}
	
	return out;
}
