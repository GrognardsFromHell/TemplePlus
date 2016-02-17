#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include <temple/dll.h>

class UiExitCrashFix : TempleFix
{
public: 
	const char* name() override { return "UI crash at exit fix";} 
	
	void apply() override 
	{
		int asdf;
		read(0x1015B3E8 + 2, &asdf, sizeof(int));
		
		write(0x1015B3F5 + 2, &asdf, sizeof(int));// need to free the "centered on party" button; Spellslinger forgot to change this
		//read(0x1015B3F5 + 2, &asdf, sizeof(int));

	}
} uiExitCrashFix;