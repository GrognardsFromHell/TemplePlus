
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

/*
	A utility class that unprotects an area of memory while it is in scope.
	Addresses are specified according to the default image base of temple.dll
	and are rebased to the actual image base automatically.
*/
class MemoryUnprotector {
public:
	explicit MemoryUnprotector(uint32_t dllAddress, size_t size) noexcept;
	~MemoryUnprotector() noexcept;

	MemoryUnprotector(const MemoryUnprotector&) = delete;
	MemoryUnprotector(const MemoryUnprotector&&) = delete;
	MemoryUnprotector& operator=(const MemoryUnprotector&) = delete;
private:
	void *mAddress; // This is the real address
	size_t mSize;
	uint32_t mOldProtection;
};

class TempleFix {
public:
	TempleFix();
	virtual ~TempleFix();

	virtual void apply() = 0;

protected:
	void write(uint32_t offset, const void *buffer, size_t size);
	void read(uint32_t offset, void *buffer, size_t size);
	void writeHex(uint32_t offset, const std::string &hexPattern);
	void redirectCall(uint32_t offset, void* redirectTo);
	void redirectJump(uint32_t offset, void* redirectTo);

	template<typename T>
	void redirectToLambda(uint32_t offset, T* redirectTo) {
		redirectCall(offset, redirectTo);
	};

	// Intelligently replaces the instruction at the given offset with noops
	void writeNoops(uint32_t offset);

    // Overwrites an entire area with INT3 instructions as a canary to detect
    // usage of functions that should have been fully replaced
    void breakRegion(uint32_t begin, uint32_t end);

	template<typename T>
	T* replaceFunction(uint32_t offset, T* replaceWith) {
		return (T*)replaceFunctionInternal(offset, replaceWith);
	}
	void writeCall(uint32_t offset, void* redirectTo);
	void writeJump(uint32_t offset, void* redirectTo);

private:
	void *replaceFunctionInternal(uint32_t offset, void *replaceWith);
};

class TempleFixes {
	friend class TempleFix;
public:
	static void apply();
private:
	static std::vector<TempleFix*> &fixes();
};
