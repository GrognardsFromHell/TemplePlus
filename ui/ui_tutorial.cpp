
#include "stdafx.h"
#include "ui_tutorial.h"
#include <util/addresses.h>

UiTutorial uiTutorial;

static struct UiTutorialAddresses : AddressTable {

	int (__cdecl *ShowTopic)(int topicId);
	bool *active;

	UiTutorialAddresses() {
		rebase(ShowTopic, 0x10124BE0);
		rebase(active, 0x10BDE3DC);
	}
} addresses;

bool UiTutorial::Toggle() {
	auto& active = *addresses.active;
	active = !active;
	return active;
}

bool UiTutorial::IsActive() {
	return *addresses.active;
}

int UiTutorial::ShowTopic(int topicId) {
	return addresses.ShowTopic(topicId);
}
