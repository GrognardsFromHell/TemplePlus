#include "stdafx.h"
#include "util/addresses.h"

void* templeImageBase = nullptr;

bool AddressInitializer::rebaseDone = false;

// This is the image base of the temple.dll as the PE header says
const uint32_t orgTempleBase = 0x10000000;

static void rebase(void** ptr) {
	auto ptrValue = reinterpret_cast<uint32_t>(*ptr);

	auto relativeOffset = ptrValue - orgTempleBase;
	assert(relativeOffset > 0 && relativeOffset < 0x10000000);
	auto realAddress = reinterpret_cast<uint32_t>(templeImageBase) + relativeOffset;
	*ptr = reinterpret_cast<void*>(realAddress);
}

void AddressInitializer::performRebase() {
	assert(!rebaseDone);
	rebaseDone = true;
	logger->info("Rebasing {} addresses", rebaseQueue().size());
	for (size_t i = 0; i < rebaseQueue().size(); ++i) {
		rebase(rebaseQueue()[i]);
	}
	rebaseQueue().clear();
}

// This is a way around the static initialization order
vector<void**>& AddressInitializer::rebaseQueue() {
	static auto queue = new vector<void**>();
	return *queue;
}

TempleAllocFuncs allocFuncs;
