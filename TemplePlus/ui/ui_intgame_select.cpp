#include "stdafx.h";
#include "ui_intgame_select.h";
#include <temple/dll.h>

UiIntgameSelect uiIntgameSelect;

struct UiIntgameSelectAddresses : temple::AddressTable
{
	int * id;
	UiIntgameSelectAddresses()
	{
		rebase(id, 0x10BE5B90);
	}
} addresses;

int UiIntgameSelect::GetId()
{
	return *addresses.id;
}