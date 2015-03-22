
#include "stdafx.h"
#include "addresses.h"

void* templeImageBase = nullptr;

vector<void**> AddressInitializer::rebaseQueue;

bool AddressInitializer::rebaseDone = false;

// This is the image base of the temple.dll as the PE header says
const uint32_t orgTempleBase = 0x10000000;

static void rebase(void **ptr) {
	auto ptrValue = reinterpret_cast<uint32_t>(*ptr);

	auto relativeOffset = ptrValue - orgTempleBase;
	BOOST_ASSERT(relativeOffset > 0 && relativeOffset < 0x10000000);
	auto realAddress = reinterpret_cast<uint32_t>(templeImageBase) + relativeOffset;
	*ptr = reinterpret_cast<void*>(realAddress);
}

void AddressInitializer::performRebase() {
	BOOST_ASSERT(!rebaseDone);
	rebaseDone = true;
	LOG(info) << "Rebasing " << rebaseQueue.size() << " addresses";
	for (size_t i = 0; i < rebaseQueue.size(); ++i) {
		rebase(rebaseQueue[i]);
	}
	rebaseQueue.clear();
}

TempleAllocFuncs allocFuncs;
