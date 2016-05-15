

#include "stdafx.h"
#include <iostream>

#include "obj_structs.h"
#include "description.h"

const objHndl objHndl::null{ 0 };

std::ostream &operator <<(std::ostream &out, const objHndl &handle) {
	if (!handle) {
		out << "OBJ_HANDLE_NULL";
	} else {
		out << description.getDisplayName(handle) 
			<< " [0x" << std::hex << handle.handle << std::dec << "]";
	}
	return out;
}
