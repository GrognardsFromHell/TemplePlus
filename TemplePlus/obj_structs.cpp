

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

fmt::appender format(fmt::appender out, const objHndl &handle) {
    if (!handle || !objSystem->IsValidHandle(handle)) {
        return fmt::format_to(out, "OBJ_HANDLE_NULL");
    } else {
        auto d = description.getDisplayName(handle);
        if (d != nullptr) {
            return fmt::format_to(out, "{} [0x{:x}]", d, handle.handle);
        }
        else {
            return fmt::format_to(out, "DESCRIPTION MISSING [0x{:x}]", handle.handle);
        }
    }
}
