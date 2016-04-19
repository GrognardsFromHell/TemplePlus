#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include <temple/dll.h>

// UI crash at exit fix
class UiExitCrashFix : TempleFix
{
public: 
	void apply() override 
	{
		int asdf;
		read(0x1015B3E8 + 2, &asdf, sizeof(int));
		
		write(0x1015B3F5 + 2, &asdf, sizeof(int));// need to free the "centered on party" button; Spellslinger forgot to change this
		//read(0x1015B3F5 + 2, &asdf, sizeof(int));

	}
} uiExitCrashFix;