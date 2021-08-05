
#pragma once

#include <array>
#include <EASTL/vector.h>

class OutputStream;
struct TioFile;

using ArrayIdxMapId = uint32_t;

struct ArrayIndices {
	size_t firstIdx; // Index of first bitmap block used
	size_t count; // Number of bitmap blocks used
};

/**
 * Manages storage for array index bitmaps, which are used to
 * more efficiently store sparse arrays.
 */
class ArrayIndexBitmaps {
public:
	ArrayIndexBitmaps();

	// Allocate an array index map
	ArrayIdxMapId Allocate();

	// Free an array index map
	void Free(ArrayIdxMapId id);

	// Copies an array index map and returns the id of the created copy
	ArrayIdxMapId Clone(ArrayIdxMapId id);

	// Removes an index from an index map
	void RemoveIndex(ArrayIdxMapId id, size_t index);
	
	// Adds an index to an index map if it's not already in it
	void AddIndex(ArrayIdxMapId id, size_t index);

	// Checks if an index is present in the array index map
	bool HasIndex(ArrayIdxMapId id, size_t index) const;

	// Get the index mapped to a range with no gaps (for storage purposes)
	size_t GetPackedIndex(ArrayIdxMapId id, size_t index) const;

	// Returns the serialized size of an array index map in bytes
	size_t GetSerializedSize(ArrayIdxMapId id) const;

	// Deserializes an array index map from the given memory buffer and returns
	// the id of the newly allocated map
	ArrayIdxMapId DeserializeFromMemory(uint8_t **buffer);
	
	// Serializes an array index map to the given memory buffer
	void SerializeToMemory(ArrayIdxMapId id, uint8_t **buffer) const;

	// Serializes an array index map to the given stream
	void SerializeToStream(ArrayIdxMapId id, OutputStream &stream) const;

	// Deserializes an array index map from the given file and returns
	// the id of the newly allocated map or throws on failure.
	ArrayIdxMapId DeserializeFromFile(TioFile *file);

	// Calls a callback for all indices that are present in the index map.
	// Stops iterating when false is returned. Returns false if any call to
	// callback returned false, true otherwise.
	bool ForEachIndex(ArrayIdxMapId id, std::function<bool(size_t)> callback) const;

	// Count of set bits in given 32-bit integer up to and not including the given bit (0-31)
	uint8_t PopCntConstrained(uint32_t value, uint8_t upToExclusive) const;
	
	// Count of set bits in given 32-bit integer
	uint8_t PopCnt(uint32_t value) const;

private:
	// The bitmap blocks that contain the actual index bitmaps
	eastl::vector<uint32_t> mBitmapBlocks;

	// State stored for each array
	eastl::vector<ArrayIndices> mArrays;

	// IDs of free entries in mArrays
	eastl::vector<ArrayIdxMapId> mFreeIds;

	// Bitmask lookup table that contains bitmasks that have the lower 0 - 31
	// bits set. This is used in counting the bits up and until position i in
	// a DWORD
	std::array<uint32_t, 32> mPartialBitmasks;

	// A bit count lookup table for all values of a 16-bit integer
	std::array<uint8_t, UINT16_MAX + 1> mBitCountLut;
	
	// Shrinks an index map by the specified number of bitmap blocks
	void Shrink(ArrayIdxMapId id, size_t shrinkBy);

	// Extends an index map by the specified number of bitmap blocks
	void Extend(ArrayIdxMapId id, size_t extendBy);
};

extern ArrayIndexBitmaps arrayIdxBitmaps;
