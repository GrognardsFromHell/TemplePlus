
#include "stdafx.h"

#include <temple/dll.h>

#include "util/fixes.h"
#include "trampoline.h"
#include "hde/hde32.h"

vector<TempleFix*> &TempleFixes::fixes() {
	static vector<TempleFix*> activeFixes;
	return activeFixes;
}

MemoryUnprotector::MemoryUnprotector(uint32_t dllAddress, size_t size) noexcept
	: mAddress(temple::Dll::GetInstance().GetAddress(dllAddress)), mSize(size), mOldProtection(0) {
	// rebase the address using temple::GetPointer
	auto result = VirtualProtect(mAddress, mSize, PAGE_READWRITE, (DWORD*)&mOldProtection);
	assert(result);
}

MemoryUnprotector::~MemoryUnprotector() noexcept {
	auto result = VirtualProtect(mAddress, mSize, mOldProtection, (DWORD*)&mOldProtection);
	assert(result);
}

TempleFix::TempleFix() {
	TempleFixes::fixes().push_back(this);
}

TempleFix::~TempleFix() {
}

void TempleFix::write(uint32_t offset, const void* buffer, size_t size) {
	MemoryUnprotector unprotector(offset, size);
	memcpy(temple::Dll::GetInstance().GetAddress(offset), buffer, size);
}

void TempleFix::read(uint32_t offset, void* buffer, size_t size) {
	memcpy(buffer, temple::Dll::GetInstance().GetAddress(offset), size);
}

void TempleFix::writeHex(uint32_t offset, const string &hexPattern) {
	// at most 128 bytes supported due to the buffer below
	assert(hexPattern.size() <= 256);
	assert(hexPattern.size() >= 2);

	// Parse hex pattern into a vector of uint8_t
	uint8_t buffer[128];
	size_t totalSize = 0;
	for (size_t i = 0; i < hexPattern.size();) {
		buffer[totalSize++] = static_cast<uint8_t>(stoul(hexPattern.substr(i, 2), nullptr, 16) & 0xFF);

		i += 2;
		// Skip whitespace until next char
		while (i < hexPattern.size() && hexPattern.at(i) == ' ') {
			++i;
		}
	}

	MemoryUnprotector unprotector(offset, totalSize);
	memcpy(temple::Dll::GetInstance().GetAddress(offset), buffer, totalSize);
}

void *TempleFix::replaceFunctionInternal(uint32_t offset, void* replaceWith) {
	void* original = nullptr;
	auto target = temple::Dll::GetInstance().GetAddress(offset);
	
	auto status = MH_CreateHook(target, replaceWith, &original);
	if (status != MH_OK) {
		logger->error("Unable to hook the function @ {:x}: {}", offset, status);
		return nullptr;
	}

	status = MH_QueueEnableHook(target);
	if (status != MH_OK) {
		logger->error("Unable to enable hook for function @ {:x}: {}", offset, status);
		return nullptr;
	}

	return original;
}

void TempleFix::redirectCall(uint32_t offset, void* redirectTo) {

	// Read what's there...
	auto realAddress = temple::Dll::GetInstance().GetAddress(offset);
	hde32s oldInstruction;
	hde32_disasm(realAddress, &oldInstruction);

	uint32_t oldCallTo;

	switch (oldInstruction.opcode) {
	// relative call opcode
	case 0xE8:
		assert(oldInstruction.len == 5);
		oldCallTo = offset + oldInstruction.len + static_cast<int32_t>(oldInstruction.imm.imm32);
		break;
	default:
		logger->error("Unsupported opcode for replacing a call: {:x}", oldInstruction.opcode);
		break;
	}

	logger->trace("Replacing old call to 0x{:x}", oldCallTo);
	
	writeCall(offset, redirectTo);
}


void TempleFix::redirectJump(uint32_t offset, void* redirectTo) {

	// Read what's there...
	auto realAddress = temple::Dll::GetInstance().GetAddress(offset);
	hde32s oldInstruction;
	hde32_disasm(realAddress, &oldInstruction);

	uint32_t oldCallTo;

	switch (oldInstruction.opcode) {
		// relative call opcode
	case 0xE9:
		assert(oldInstruction.len == 5);
		oldCallTo = offset + oldInstruction.len + static_cast<int32_t>(oldInstruction.imm.imm32);
		break;
	default:
		logger->error("Unsupported opcode for replacing a jump: {:x}", oldInstruction.opcode);
		break;
	}

	logger->trace("Replacing old jump to 0x{:x}", oldCallTo);

	writeJump(offset, redirectTo);
}

void TempleFix::writeNoops(uint32_t offset) {

	// Read what's there...
	auto realAddress = temple::Dll::GetInstance().GetAddress(offset);
	hde32s oldInstruction;
	hde32_disasm(realAddress, &oldInstruction);

	uint8_t noopBytes[32];
	Expects(oldInstruction.len <= sizeof(noopBytes));
	memset(noopBytes, 0x90, oldInstruction.len);
	write(offset, &noopBytes[0], oldInstruction.len);

}

void TempleFix::breakRegion(uint32_t from, uint32_t to)
{
	size_t size = to - from;

	Expects(to > from);

	// Unprotect the entire area
	MemoryUnprotector unprotector(from, size);

	// Fill with INT 3 instructions (which trigger breakpoints)
	auto realFrom = reinterpret_cast<char*>(temple::Dll::GetInstance().GetAddress(from));
	memset(realFrom, 0xCC, size);

}

void TempleFix::writeCall(uint32_t offset, void* redirectTo) {
	CALL_REL call = {
		0xE8,                   // E8 xxxxxxxx: CALL +5+xxxxxxxx
		0x00000000              // Relative destination address
	};

	auto startOfNextInstruction = reinterpret_cast<int32_t>(temple::Dll::GetInstance().GetAddress(offset + 5));
	auto targetAddress = reinterpret_cast<int32_t>(redirectTo);
	call.operand = targetAddress - startOfNextInstruction;
	write(offset, &call, sizeof(call));	
}


void TempleFix::writeJump(uint32_t offset, void* redirectTo) {
	CALL_REL call = {
		0xE9,                   // E9 xxxxxxxx: JMP +5+xxxxxxxx
		0x00000000              // Relative destination address
	};

	auto startOfNextInstruction = reinterpret_cast<int32_t>(temple::Dll::GetInstance().GetAddress(offset + 5));
	auto targetAddress = reinterpret_cast<int32_t>(redirectTo);
	call.operand = targetAddress - startOfNextInstruction;
	write(offset, &call, sizeof(call));
}


void TempleFixes::apply() {
	logger->info("Applying {} DLL fixes", fixes().size());

	for (auto fix : fixes()) {
		fix->apply();
	}

	auto status = MH_ApplyQueued();
	if (status != MH_OK) {
		logger->error("Unable to apply queued hooks: {}", status);
	}

	logger->info("Finished applying DLL fixes");

}
