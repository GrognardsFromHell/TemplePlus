#pragma once
#include <map>

struct BuffDebuffSub
{
	uint32_t effectTypeId;
	const char * text;
	int spellEnum;
};

struct BuffDebuffPacket
{
	int buffCount;
	BuffDebuffSub buffs[8];
	int debuffCount;
	BuffDebuffSub debuffs[8];
	int innerCount;
	BuffDebuffSub innerStatuses[6]; // the icons appearing inside the portrait (e.g. Flatfooted)
};


struct BuffDebuffSpec{
	std::string textureFilename;
	std::string helpTopicName;
	int textureId;

};

class UiParty
{
	std::map<int, BuffDebuffSpec> buffDebuffSpecs;
};

extern UiParty uiParty;