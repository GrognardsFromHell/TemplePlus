
#pragma once

#include "obj_structs.h"
#include "arrayidxbitmaps.h"

#pragma pack(push, 1)
struct ArrayHeader {
	uint32_t elSize;
	uint32_t count;
	ArrayIdxMapId idxBitmapId;

	// Pointer to the data that starts behind this header
	uint8_t* GetData() {
		return reinterpret_cast<uint8_t*>(this) + sizeof(ArrayHeader);
	}

	void* GetData(size_t packedIdx) {
		return GetData() + packedIdx * elSize;
	}
};
#pragma pack(pop)

template<typename T>
class GameObjectReadOnlyArrayHelper {
public:
	GameObjectReadOnlyArrayHelper(ArrayHeader** storageLocation)
		: mStorageLocation(storageLocation) {
		assert(!mStorageLocation || !(*mStorageLocation) || (*mStorageLocation)->elSize == sizeof(T));
	}

	size_t GetSize() const {
		if (!mStorageLocation) {
			// The type of the object didn't support this field
			return 0;
		}
		if (!*mStorageLocation) {
			return 0;
		}
		return (*mStorageLocation)->count;
	}

	bool HasIndex(size_t index) const {
		if (!mStorageLocation) {
			// The type of the object didn't support this field
			return false;
		}
		return arrayIdxBitmaps.HasIndex((*mStorageLocation)->idxBitmapId, index);
	}

	const T& operator[](size_t index) const;

protected:
	ArrayHeader** mStorageLocation;

	size_t GetPackedIndex(size_t index) const {
		return arrayIdxBitmaps.GetPackedIndex((*mStorageLocation)->idxBitmapId, index);
	}
};


template<typename T>
class GameObjectArrayHelper : public GameObjectReadOnlyArrayHelper<T> {
public:
	GameObjectArrayHelper(ArrayHeader** storageLocation)
		: GameObjectReadOnlyArrayHelper<T>(storageLocation) {
	}

	void Set(size_t index, const T& value) {
		if (!this->mStorageLocation) {
			// The type of the object didn't support this field
			return;
		}

		// Initialize the array storage if necessary
		if (!*this->mStorageLocation) {
			*this->mStorageLocation = (ArrayHeader*)malloc(sizeof(ArrayHeader));
			(*this->mStorageLocation)->count = 0;
			(*this->mStorageLocation)->elSize = sizeof(T);
			(*this->mStorageLocation)->idxBitmapId = arrayIdxBitmaps.Allocate();
		}

		auto packedIdx = this->GetPackedIndex(index);

		// Add the corresponding index position
		if (!this->HasIndex(index)) {
			arrayIdxBitmaps.AddIndex((*this->mStorageLocation)->idxBitmapId, index);

			// Resize the array storage
			*this->mStorageLocation = (ArrayHeader*)realloc(*this->mStorageLocation,
				sizeof(ArrayHeader) 
				+ (*this->mStorageLocation)->count * sizeof(T)
				+ sizeof(T));

			// Move back everything behind the packed Idx
			for (size_t i = (*this->mStorageLocation)->count; i > packedIdx; --i) {
				*reinterpret_cast<T*>((*this->mStorageLocation)->GetData(i))
					= *reinterpret_cast<T*>((*this->mStorageLocation)->GetData(i - 1));
			}

			(*this->mStorageLocation)->count++;
		}
		
		*reinterpret_cast<T*>((*this->mStorageLocation)->GetData(packedIdx)) = value;
	}

	void Remove(size_t index) {
	    if (!this->mStorageLocation) {
			// The type of the object didn't support this field
			return;
		}

	    if (!*this->mStorageLocation) {
			return; // No storage allocated anyway
		}

	    if (!this->HasIndex(index)) {
			return; // Index is already removed
		}

		// If the array only consist of the element we are removing, deallocate it
		if ((*this->mStorageLocation)->count == 1) {
			Clear();
			return;
		}

		size_t storageIndex = this->GetPackedIndex(index);
		arrayIdxBitmaps.RemoveIndex((*this->mStorageLocation)->idxBitmapId, index);

		T* arr = reinterpret_cast<T*>((*this->mStorageLocation)->GetData());
		// Copy all the data from the back one place forward
		for (size_t i = storageIndex; i < (*this->mStorageLocation)->count - 1; ++i) {
			arr[i] = arr[i + 1];
		}		
		(*this->mStorageLocation)->count--;
		*this->mStorageLocation = reinterpret_cast<ArrayHeader*>(
			realloc(*this->mStorageLocation, sizeof(ArrayHeader) + sizeof(T) * (*this->mStorageLocation)->count)
		);
	}

	void Clear() {
		if (!this->mStorageLocation) {
			// The type of the object didn't support this field
			return;
		}

		if (!*this->mStorageLocation) {
			return; // Already reset
		}
		arrayIdxBitmaps.Free((*this->mStorageLocation)->idxBitmapId);
		free(*this->mStorageLocation);
		*this->mStorageLocation = nullptr;
	}

	/**
	 * Calls the given callback for every stored index in the array.
	 * Also passes a mutable data pointer.
	 */
	void ForEachIndex(std::function<void(size_t)> callback) {
	    if (!this->mStorageLocation) {
			// The type of the object didn't support this field
			return;
		}

	    if (!*this->mStorageLocation) {
			return;
		}

	    arrayIdxBitmaps.ForEachIndex((*this->mStorageLocation)->idxBitmapId, [=](size_t realIdx) {
			callback(realIdx);
			return true;
		});
	}

	void Append(const T& value)	{
	    Set(this->GetSize(), value);
	};

};

struct ObjectScript;
struct SpellStoreData;

using GameInt32Array = GameObjectArrayHelper<int32_t>;
using GameInt64Array = GameObjectArrayHelper<int64_t>;
using GameObjectIdArray = GameObjectArrayHelper<ObjectId>;
using GameScriptArray = GameObjectArrayHelper<ObjectScript>;
using GameSpellArray = GameObjectArrayHelper<SpellStoreData>;

using GameInt32ReadOnlyArray = GameObjectReadOnlyArrayHelper<int32_t>;
using GameInt64ReadOnlyArray = GameObjectReadOnlyArrayHelper<int64_t>;
using GameObjectIdReadOnlyArray = GameObjectReadOnlyArrayHelper<ObjectId>;
using GameScriptReadOnlyArray = GameObjectReadOnlyArrayHelper<ObjectScript>;
using GameSpellReadOnlyArray = GameObjectReadOnlyArrayHelper<SpellStoreData>;

template<typename T>
inline const T & GameObjectReadOnlyArrayHelper<T>::operator[](size_t index) const
{
	if (!mStorageLocation) {
		static T empty;
		memset(&empty, 0, sizeof(empty));
		// The type of the object didn't support this field
		return empty;
	}

	static T sEmpty = T();
	if (!*mStorageLocation || !HasIndex(index)) {
		return sEmpty;
	}

	auto packedIdx = GetPackedIndex(index);
	return *reinterpret_cast<const T*>((*mStorageLocation)->GetData(packedIdx));
}
