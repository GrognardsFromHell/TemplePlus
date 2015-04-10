#pragma once

#include "stdafx.h"
#include "addresses.h"


template <typename T>
struct ToEEHashtable : public TempleAlloc
{
	uint32_t numItems;
	uint32_t capacity;
	uint32_t powerOfTwo;
	uint32_t * keyArray;
	T** dataArray;
	uint32_t * idxArray;
	uint32_t pad;
};

template <typename T>
struct ToEEHashtableSystem : AddressTable
{
	uint32_t HashtableInit(ToEEHashtable<T>* hashtableOut, uint32_t capacity)
	{
		hashtableOut->capacity = capacity;
		uint32_t powerOfTwo = 1;
		for (hashtableOut->numItems = 0; powerOfTwo < 2 * capacity; powerOfTwo *= 2);

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
			logger->info("Hashtable collission detected!");
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
		uint32_t storedKey = hashtable->keyArray[idx];
		if (!storedKey){ return 0x11; }
		while (storedKey != key)
		{
			idx = bitmask & (idx + 1);
			storedKey = hashtable->keyArray[idx];
			if (!storedKey){ return 0x11; }
		}

		if (idx != -1)
		{
			*structOut = hashtable->dataArray[idx];
			return 0;
		}
		return 0x11;	
	}

	uint32_t StringHash(char * stringIn)
	{
		return ELFhash(stringIn);
	}

	ToEEHashtableSystem()
	{
		rebase(ELFhash, 0x101EBB00);
	}

private:

	T ** _dataArrayNew(uint32_t capacity)
	{
		return (T**)allocFuncs._malloc_0(sizeof(T*) * capacity);
	}

	uint32_t * _keyArrayNew(uint32_t capacity)
	{
		return (uint32_t *)allocFuncs._malloc_0(sizeof(uint32_t) * capacity);
	}

	uint32_t(__cdecl* ELFhash)(char * stringIn);
};
