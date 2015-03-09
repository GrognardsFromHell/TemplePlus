
#include "stdafx.h"
#include "addresses.h"

void* templeImageBase = nullptr;

vector<AddressInitializer::Callback> AddressInitializer::initializers;

bool AddressInitializer::rebaseDone = false;

void AddressInitializer::performRebase()
{
	if (rebaseDone)
	{
		return;
	}

	rebaseDone = true;

	Rebaser rebaser;	
	for (auto callback : initializers)
	{
		callback(rebaser);
	}
	initializers.clear();
}

TempleAllocFuncs templeAllocFuncs;