#pragma once

#include <temple/dll.h>

template <typename T>
struct IdxTableNode : temple::TempleAlloc
{
	T* data;
	IdxTableNode<T>* next;
	int id;

	IdxTableNode(int _id, const T&_data, IdxTableNode<T> *_next) : data(nullptr), next(_next), id(_id) {
		data = static_cast<T*>(operator new(sizeof(T)));
		new (data)T(_data);
	}
};

template <typename T>
struct IdxTable : temple::TempleAlloc
{
	int bucketCount;
	IdxTableNode<T>** buckets;
	int itemSize;
	int itemCount;
};

struct IdxTableListsNode : temple::TempleAlloc
{
	const char* sourceFile;
	int lineNumber;
	IdxTable<void>* table;
	IdxTableListsNode* next;
};

template <typename T>
class IdxTableWrapper;

template <typename T>
class IdxTableIterator
{
public:
	friend class IdxTableWrapper<T>;

	IdxTableIterator& operator ++()
	{
		// find next
		if (mBucket >= mTable->bucketCount)
		{
			return *this;
		}

		// Try scanning within the bucket first
		if (mNode && mNode->next)
		{
			mNode = mNode->next;
			return *this; // found one!
		}

		// Find the next bucket with data
		mNode = nullptr;
		for (++mBucket; mBucket < mTable->bucketCount; ++mBucket)
		{
			if (mTable->buckets[mBucket])
			{
				mNode = mTable->buckets[mBucket];
				break;
			}
		}

		return *this;
	}

	const IdxTableNode<T>& operator *()
	{
		return *mNode;
	}

	inline bool operator==(const IdxTableIterator<T>& b)
	{
		return mTable == b.mTable &&
			mNode == b.mNode &&
			mBucket == b.mBucket;
	}

	template <typename T>
	inline bool operator!=(const IdxTableIterator<T>& b)
	{
		return !(*this == b);
	}

private:
	explicit IdxTableIterator(const IdxTable<T>* table, IdxTableNode<T>* node, int bucket) : mTable(table), mNode(node), mBucket(bucket)
	{
	}

	explicit IdxTableIterator(const IdxTable<T>* table) : mTable(table), mNode(nullptr), mBucket(0)
	{
		for (mBucket = 0; mBucket < table->bucketCount; ++mBucket)
		{
			if (table->buckets[mBucket])
			{
				mNode = table->buckets[mBucket];
				break;
			}
		}
	}

	const IdxTable<T>* mTable;
	IdxTableNode<T>* mNode;
	int mBucket = 0;
};

/*
	Utility class for handling internal ToEE index tables that point to structures of 
	type T.
*/
template <typename T>
class IdxTableWrapper
{
public:
	IdxTableWrapper(uint32_t address) : mTable(reinterpret_cast<IdxTable<T>*>(address))
	{
		temple::Dll::RegisterAddressPtr(reinterpret_cast<void**>(&mTable));
	}

	IdxTableWrapper(IdxTable<T>* pointer) : mTable(pointer)
	{
	}

	uint32_t itemCount() const
	{
		return mTable->itemCount;
	}

	uint32_t itemSize() const
	{
		return mTable->itemSize;
	}

	T *get(int id)
	{
		auto node = mTable->buckets[id % mTable->bucketCount];

		while (node)
		{
			if (node->id == id)
			{
				return node->data;
			}
			node = node->next;
		}

		return nullptr;
	}

	uint32_t copy(int id, T * dataOut)
	{
		auto node = mTable->buckets[id % mTable->bucketCount];

		while (node)
		{
			if (node->id == id)
			{
				memcpy(dataOut, node->data, sizeof(T));
				return 1;
			}
			node = node->next;
		}

		return 0;
	}



	void put(int id, const T &data)
	{
		auto bucketId = id % mTable->bucketCount;
		IdxTableNode<T> *node = mTable->buckets[bucketId];

		// Check if an entry for id is already there
		while (node)
		{
			if (node->id == id)
			{
				*(node->data) = data;
				return;
			}
			node = node->next;
		}

		// In case the ID didn't exist yet, prepend a node for it
		mTable->buckets[bucketId] = new IdxTableNode<T>(id, data, node);
		++(mTable->itemCount);
	}

	void remove(int id)
	{
		auto bucketId = id % mTable->bucketCount;
		IdxTableNode<T> *prevNode = nullptr;
		IdxTableNode<T> *node = mTable->buckets[bucketId];

		while (node)
		{
			if (node->id == id)
			{
				delete node->data;
				if (prevNode) {
					prevNode->next = node->next;
				} else {
					mTable->buckets[bucketId] = node->next;
				}
				delete node;
				--(mTable->itemCount);
				return;
			}
			prevNode = node;
			node = node->next;
		}
	}

	IdxTableIterator<T> erase(IdxTableIterator<T> it)
	{
		Expects(it.mTable == this->mTable);
		Expects(it.mNode != nullptr);
		auto bucketId = it.mBucket;
		IdxTableNode<T> *prevNode = nullptr;
		IdxTableNode<T> *node = mTable->buckets[bucketId];

		auto nodeToDelete = it.mNode;
		++it;

		while (node)
		{
			if (node == nodeToDelete)
			{
				delete node->data;
				if (prevNode) {
					prevNode->next = node->next;
				} else {
					mTable->buckets[bucketId] = node->next;
				}
				delete node;
				++(mTable->itemCount);
				return it;
			}
			prevNode = node;
			node = node->next;
		}

		throw TempleException("Iterator wasn't pointing to an element in this table.");
	}

	IdxTableIterator<T> begin()
	{
		return IdxTableIterator<T>(mTable);
	}

	IdxTableIterator<T> end()
	{
		return IdxTableIterator<T>(mTable, nullptr, mTable->bucketCount);
	}

private:
	IdxTable<T>* mTable;
};

extern temple::GlobalPrimitive<IdxTableListsNode*, 0x10EF2E70> idxTablesList;
