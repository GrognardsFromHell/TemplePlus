#include "stdafx.h"
#include "common.h"
#include "util/config.h"

class CharUiSystem : TempleFix
{
public: 
	const char* name() override { return "CharUi - disabling stat text draw calls";} 
	void apply() override 
	{
		if (config.usingCo8)
		{
			writeHex(0x1011DD4D, "90 90 90 90 90");
		}
	}
} charUiSys;

