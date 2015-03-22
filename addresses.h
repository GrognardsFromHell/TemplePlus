#pragma once

#include "stdafx.h"
#include "exception.h"

extern void* templeImageBase;

struct AddressInitializer
{
	// Add a pointer to the queue for being rebased
	static void queueRebase(void **ptr) {
		rebaseQueue.push_back(ptr);
	}

	// Perform all queued rebase operations
	static void performRebase();
private:
	static vector<void**> rebaseQueue;
	static bool rebaseDone;
};

template <typename T, uint32_t offsetPreset>
struct GlobalStruct
{
	GlobalStruct() : mPtr(reinterpret_cast<T*>(offsetPreset))
	{
		static_assert(offsetPreset != 0, "This constructor should only be used with a template argument offset");
		AddressInitializer::queueRebase(reinterpret_cast<void**>(&mPtr));
	}

	operator T*()
	{
		return mPtr;
	}

	T* operator ->()
	{
		return mPtr;
	}

	T* ptr()
	{
		return mPtr;
	}

private:
	T* mPtr;
};

template <typename T, uint32_t offsetPreset>
struct GlobalPrimitive
{
	GlobalPrimitive() : mPtr(reinterpret_cast<T*>(offsetPreset))
	{
		BOOST_ASSERT(mPtr != nullptr);
		AddressInitializer::queueRebase(reinterpret_cast<void**>(&mPtr));
	}

	T operator =(T value)
	{
		return (*mPtr = value);
	}

	operator T()
	{
		return *mPtr;
	}

	T* ptr()
	{
		return mPtr;
	}

	GlobalPrimitive & operator =(const GlobalPrimitive &) = delete;
	GlobalPrimitive(const GlobalPrimitive &) = delete;
private:
	T* mPtr;
};

template<uint32_t offset> using GlobalBool = GlobalPrimitive<bool, offset>;

struct AddressTable
{
	AddressTable() {}

	AddressTable & operator =(const AddressTable &) = delete;
	AddressTable(const AddressTable &) = delete;

protected:
	template<typename T>
	void rebase(T &ref, uint32_t offset) {
		ref = reinterpret_cast<T>(offset);
		AddressInitializer::queueRebase(reinterpret_cast<void**>(&ref));
	}
};

/**
	Allows allocations and deallocations within ToEEs internal memory heap.
*/
struct TempleAllocFuncs : AddressTable
{
	void* (__cdecl *opNew)(size_t count);
	void (__cdecl *free)(void *ptr);

	TempleAllocFuncs() {
		rebase(opNew, 0x10256432);
		rebase(free, 0x10254209);
	}
};
extern TempleAllocFuncs allocFuncs;

/*
Utility base class that ensures memory is allocated within the heap of temple.dll
Simply using new will otherwise cause issues since the free&malloc functions within
temple.dll do not come from the same CRT.
*/
struct TempleAlloc {
	void* operator new (size_t count){
		return allocFuncs.opNew(count);
	}

	void operator delete(void* ptr, size_t) {
		allocFuncs.free(ptr);
	}
};

template <uint32_t address>
inline void* temple_address()
{
	static_assert(address > 0x10000000 && address < 0x20000000,
		"address is not within temple.dll address space");
	BOOST_ASSERT(templeImageBase != 0);
	return ((char*)templeImageBase) + (address - 0x10000000);
}

inline void* temple_address(uint32_t address)
{
	BOOST_ASSERT(address > 0x10000000 && address < 0x20000000);
	BOOST_ASSERT(templeImageBase != 0);
	return ((char*)templeImageBase) + (address - 0x10000000);
}

template <uint32_t address, typename T>
inline T temple_get()
{
	return *(T*)temple_address<address>();
}

template <typename T>
inline T temple_get(uint32_t address)
{
	return *(T*)temple_address(address);
}

template <typename T>
inline void temple_set(uint32_t address, T value)
{
	*(T*)temple_address(address) = value;
}

template <uint32_t address, typename T>
inline void temple_set(T value)
{
	*(T*)temple_address(address) = value;
}
