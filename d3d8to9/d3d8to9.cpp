
#include "stdafx.h"
#include "d3d8to9_private.h"
#include "d3d8to9_rootobj.h"

extern "C" {
	int __vsnprintf(
		char *buffer,
		size_t count,
		const char *format,
		va_list argptr
		) {
		return _vsnprintf(buffer, count, format, argptr);
	}
}