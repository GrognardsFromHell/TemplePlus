#pragma once

struct BuffDebuffSub
{
	uint32_t effectTypeId;
	const char * text;
	uint32_t spellEnum;
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