#pragma once

#include <temple/dll.h>


template <typename T>
struct ToEEHashtable : temple::TempleAlloc
{
	uint32_t numItems;
	uint32_t capacity;
	uint32_t powerOfTwo; // 2*capacity, rounded up to power of 2
	uint32_t * keyArray; // capacity: powerOfTwo
	T** dataArray;
	uint32_t * idxArray; // capacity: capacity
	uint32_t pad;

	uint32_t Init(uint32_t capacity);
};

template <typename T>
struct ToEEHashtableSystem : temple::AddressTable
{
	uint32_t HashtableInit(ToEEHashtable<T>* hashtableOut, uint32_t capacity)
	{
		hashtableOut->capacity = capacity;
		uint32_t powerOfTwo = 1;
		hashtableOut->numItems = 0;
		for (; powerOfTwo < 2 * capacity; powerOfTwo *= 2);

		hashtableOut->powerOfTwo = powerOfTwo;
		hashtableOut->dataArray = _dataArrayNew(powerOfTwo);
		hashtableOut->keyArray = _keyArrayNew(powerOfTwo);
		hashtableOut->idxArray = _keyArrayNew(capacity);
		
		for (uint32_t i = 0; i < powerOfTwo; i++)
		{
			hashtableOut->keyArray[i] = 0;
		}
		return powerOfTwo;
	}

	uint32_t HashtableNumItems(ToEEHashtable<T> * hashtable)
	{
		return hashtable->numItems;
	}

	T* HashtableGetDataPtr(ToEEHashtable<T>*hashtable, uint32_t n)
	{
		return hashtable->dataArray[hashtable->idxArray[n]];
	}

	uint32_t HashtableGetKey(ToEEHashtable<T>*hashtable, uint32_t n)
	{
		return hashtable->keyArray[hashtable->idxArray[n]];
	}

	uint32_t HashtableAddItem(ToEEHashtable<T> * hashtable, uint32_t key, T* structIn)
	{
		if (hashtable->numItems >= hashtable->capacity){ return 3; }
		if (key == 0){ return 0x11; }
		uint32_t bitmask = hashtable->powerOfTwo - 1;
		uint32_t idxNew = key & bitmask;
		uint32_t storedKey = hashtable->keyArray[idxNew];
		
		while (storedKey && storedKey != key)
		{
			idxNew = bitmask & (idxNew + 1);
			storedKey = hashtable->keyArray[idxNew];
		}
		if (storedKey == key)
		{
			logger->info("Hashtable collision detected!");
			assert(key != storedKey);
			return 0x11;
		}

		hashtable->keyArray[idxNew] = key;
		hashtable->dataArray[idxNew] = structIn;
		hashtable->idxArray[hashtable->numItems] = idxNew;
		hashtable->numItems += 1;
		return 0;
	}

	uint32_t HashtableSearch(ToEEHashtable<T>* hashtable, uint32_t key, T** structOut)
	{
		uint32_t bitmask = (hashtable->powerOfTwo - 1);
		uint32_t idx = key & bitmask;
		uint32_t * keyArray = hashtable->keyArray;
		assert(keyArray);
		uint32_t storedKey = keyArray[idx];
		if (!storedKey){ return 0x11; }
		while (storedKey != key)
		{
			idx = bitmask & (idx + 1);
			storedKey = keyArray[idx];
			if (!storedKey){ return 0x11; }
		}

		if (idx != -1)
		{
			if (structOut)
				*structOut = hashtable->dataArray[idx];
			return 0;
		}
		return 0x11;	
	}

	uint32_t StringHash(char * stringIn)
	{
		return ELFhash(stringIn);
	}

	uint32_t HashtableOverwriteItem(ToEEHashtable<T> * hashtable, uint32_t key, T* structIn)
	{
		if (key == 0){ return 0x11; }
		uint32_t bitmask = hashtable->powerOfTwo - 1;
		uint32_t idxNew = key & bitmask;
		uint32_t storedKey = hashtable->keyArray[idxNew];

		while (storedKey != key)
		{
			if (!storedKey)
			{
				if (hashtable->numItems >= hashtable->capacity){ return 3; }
				hashtable->keyArray[idxNew] = key;
				hashtable->dataArray[idxNew] = structIn;
				hashtable->idxArray[hashtable->numItems] = idxNew;
				hashtable->numItems += 1;
				return 0;
			}
			idxNew = bitmask & (idxNew + 1);
			storedKey = hashtable->keyArray[idxNew];
		}

		hashtable->dataArray[idxNew] = structIn;
		return 0x111;
	}

	ToEEHashtableSystem()
	{
		rebase(ELFhash, 0x101EBB00);
	}

private:

	T ** _dataArrayNew(uint32_t capacity)
	{
		return (T**)operator new(sizeof(T*) * capacity);
	}

	uint32_t * _keyArrayNew(uint32_t capacity)
	{
		return (uint32_t *)operator new(sizeof(uint32_t) * capacity);
	}

	uint32_t(__cdecl* ELFhash)(char * stringIn);
};

template<typename T>
inline uint32_t ToEEHashtable<T>::Init(uint32_t capacity)
{
	return uint32_t();
}
