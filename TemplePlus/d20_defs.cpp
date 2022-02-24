#include "stdafx.h"
#include "d20_defs.h"
#include "d20.h"

std::ostream & operator<<(std::ostream & out, D20ActionType d20at){

	if (d20at == D20ActionType::D20A_NONE)
	{
		out << "D20A_NONE";
		return out;
	}

	size_t i = (size_t)d20at;
	if (i <= D20A_PYTHON_ACTION){
		out << d20ActionNames[i];
	}
	else
	{
		out << fmt::format("Temple+ action {}", i);
	}
	return out;
}

void format_arg(fmt::BasicFormatter<char>& f, const char*& format_str, const D20ActionType & d20at)
{
	if (d20at == D20ActionType::D20A_NONE)
	{
		f.writer().write("D20A_NONE");
	}

	size_t i = (size_t)d20at;
	if (i <= D20A_PYTHON_ACTION) {
		f.writer().write( "{} ({})",d20ActionNames[i], i);
	}
	else
	{
		f.writer().write("Temple+ action {}", i);
	}
	return;
}