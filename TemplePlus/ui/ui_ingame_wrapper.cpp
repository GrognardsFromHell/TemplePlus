
#include "stdafx.h"

#include "messages/messagequeue.h"
#include "ui_ingame.h"
#include "util/fixes.h"
#include "obj_structs.h"
#include "ui_legacysystems.h"
#include "ui_systems.h"

static void ui_intgame_handle_msg(const Message &msg)
{
	uiSystems->GetInGame().ProcessMessage(msg);
}

/*
	Replaces functions related to the in game ui with our C++ replacements.
*/
static class UiInGameWrapper : public TempleFix {
public:

	void apply() override
	{
		replaceFunction(0x10114EF0, ui_intgame_handle_msg);

		// ObjIsUntargetable
		replaceFunction<BOOL(objHndl)>(0x1001fcb0, [](objHndl obj) {
			return uiSystems->GetInGame().IsUntargetable(obj) ? TRUE : FALSE;
		});

	}

} uiInGameWrapper;
