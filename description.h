#pragma once

#include "common.h"



struct DescriptionSystem : AddressTable
{
	uint32_t * customNamesCount;
	char** customNamesArray;

	DescriptionSystem()
	{
		rebase(customNamesCount, 0x10AB757C);
		rebase(customNamesArray, 0x10AB7578);
	};
};

extern DescriptionSystem description;