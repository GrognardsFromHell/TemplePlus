#include "stdafx.h"
#include "tutorial.h"
#include <temple/dll.h>
#include "maps.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/scripting.h"


Tutorial tutorial;

bool Tutorial::IsTutorialActive() const
{
	auto isTutorialActive = temple::GetRef<int(__cdecl)()>(0x1009AB80);
	return isTutorialActive() != 0;
}

void Tutorial::Toggle() const
{
	auto toggleTut = temple::GetRef<void(__cdecl)()>(0x1009AB70);
	toggleTut();
}

int Tutorial::ShowTopic(int topicId) const
{
	auto showTopic = temple::GetRef<void(__cdecl)(int)>(0x1009AB90);
	showTopic(topicId);
	return 0;
}

void Tutorial::CastingSpells(int spellEnum){

	if (maps.GetCurrentMapId() == 5118) { // tutorial map
		if (spellEnum == 288 && gameSystems->GetScript().GetGlobalFlag(6)) { // Magic Missile

			if (!tutorial.IsTutorialActive()) {
				tutorial.Toggle();
			}
			tutorial.ShowTopic(30);
			gameSystems->GetScript().SetGlobalFlag(6, 0);
			gameSystems->GetScript().SetGlobalFlag(7, 1);

		}

		else if (spellEnum == 171 && gameSystems->GetScript().GetGlobalFlag(9)) { // Fireball
			if (!tutorial.IsTutorialActive()) {
				tutorial.Toggle();
			}
			tutorial.ShowTopic(36);
			gameSystems->GetScript().SetGlobalFlag(9, 0);
		}
	}
}
