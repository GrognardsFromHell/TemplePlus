#pragma once

#include "stdafx.h"

extern void* templeImageBase;

/**
 * This is a utility class that helps with access to function pointers.
 * It provides an automatic callback to be notified as soon as 
 */
struct Rebaser
{
	/*
		Modifies a pointer argument of arbitrary type
	*/
	template <typename T>
	void operator()(T& arg)
	{
		uint32_t relativeOffset = reinterpret_cast<uint32_t>(arg) - 0x10000000;
		BOOST_ASSERT(relativeOffset > 0 && relativeOffset < 0x10000000);
		auto realAddress = static_cast<char*>(templeImageBase) + relativeOffset;
		arg = reinterpret_cast<T>(realAddress);
	}

	template <typename T>
	void operator()(T& arg, uint32_t address)
	{
		uint32_t relativeOffset = address - 0x10000000;
		BOOST_ASSERT(relativeOffset > 0 && relativeOffset < 0x10000000);
		auto realAddress = static_cast<char*>(templeImageBase) + relativeOffset;
		arg = reinterpret_cast<T>(realAddress);
	}
};

struct AddressInitializer
{
	friend struct AddressTable;

	typedef function<void(Rebaser rebaser)> Callback;

	AddressInitializer(const Callback& callback)
	{
		if (rebaseDone)
		{
			callback(Rebaser());
		}
		else
		{
			initializers.push_back(callback);
		}
	}

	static void performRebase();
private:
	static vector<Callback> initializers;
	static bool rebaseDone;
};

template <typename T, uint32_t offsetPreset = 0>
struct GlobalStruct
{
	GlobalStruct() : mPtr(reinterpret_cast<T*>(offsetPreset))
	{
		static_assert(offsetPreset != 0, "This constructor should only be used with a template argument offset");
		AddressInitializer([this](Rebaser rebase)
		{
			rebase(mPtr);
		});
	}

	GlobalStruct(uint32_t offset)
	{
		mPtr = reinterpret_cast<T*>(offset);
		AddressInitializer([this](Rebaser rebase)
			{
				rebase(mPtr);
			});
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

template <typename T, uint32_t offsetPreset = 0>
struct GlobalPrimitive
{
	GlobalPrimitive() : mPtr(reinterpret_cast<T*>(offsetPreset))
	{
		static_assert(offsetPreset != 0, "This constructor should only be used with a template argument offset");
		AddressInitializer([this](Rebaser rebase)
		{
			rebase(mPtr);
		});
	}

	GlobalPrimitive(uint32_t offset)
	{
		mPtr = reinterpret_cast<T*>(offset);
		AddressInitializer([this](Rebaser rebase)
		{
			rebase(mPtr);
		});
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

template<uint32_t offset = 0> using GlobalBool = GlobalPrimitive<bool, offset>;

struct AddressTable
{
	AddressTable()
	{
		AddressInitializer::Callback callback = [this](Rebaser rebaser)
			{
				rebase(rebaser);
			};
		AddressInitializer::initializers.push_back(callback);
	}

	virtual ~AddressTable()
	{
	}

	virtual void rebase(Rebaser rebaser) = 0;
};

/**
	Allows allocations and deallocations within ToEEs internal memory heap.
*/
struct TempleAllocFuncs : AddressTable
{
	void* (__cdecl *opNew)(size_t count);
	void (__cdecl *free)(void *ptr);

	void rebase(Rebaser rebase) override {
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
