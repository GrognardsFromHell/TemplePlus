#include "stdafx.h"
#include "tutorial.h"
#include <temple/dll.h>


Tutorial tutorial;

bool Tutorial::IsTutorialActive() const
{
	auto isTutorialActive = temple::GetRef<int(__cdecl)()>(0x1009AB80);
	return isTutorialActive();
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
