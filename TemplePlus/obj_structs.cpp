

#include "stdafx.h"
#include <iostream>

#include "obj_structs.h"
#include "description.h"
#include "gamesystems/objects/objsystem.h"

const objHndl objHndl::null{ 0 };

std::ostream &operator <<(std::ostream &out, const objHndl &handle) {
	if (!handle){
		out << "OBJ_HANDLE_NULL";
		return out;
	} 
	if (!objSystem->IsValidHandle(handle)) {
		out << "INVALID_OBJ_HANDLE [0x"
			<< std::hex << handle.handle << std::dec << "]";
		return out;
	}
	
	auto d = description.getDisplayName(handle);
	if (d != nullptr){
		out << d
			<< " [0x" << std::hex << handle.handle << std::dec << "]";
	}
	else{
		out << "DESCRIPTION MISSING"
			<< " [0x" << std::hex << handle.handle << std::dec << "]";
	}
	
	return out;
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const objHndl &handle) {
	if (!handle) {
		f.writer().write("OBJ_HANDLE_NULL");
		return;
	}
	if (!objSystem->IsValidHandle(handle)) {
		f.writer().write("INVALID_OBJ_HANDLE [0x{:x}]", handle.handle);
		return;
	} 
	
	auto d = description.getDisplayName(handle);
	if (d != nullptr) {
		f.writer().write("{} [0x{:x}]", d, handle.handle);
	}
	else {
		f.writer().write("DESCRIPTION MISSING [0x{:x}]", handle.handle);
	}
}
