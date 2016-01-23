#include "stdafx.h"
#include "arrayidxbitmaps.h"

#include <tio/tio.h>

static uint8_t PopCntSlow(uint32_t value) {
	uint8_t count = 0;
	while (value != 0) {
		if (value & 1) {
			++count;
		}
		value >>= 1;
	}
	return count;
}

ArrayIndexBitmaps::ArrayIndexBitmaps()
{
	mBitmapBlocks.reserve(8192);
	mArrays.reserve(4096);
	mFreeIds.reserve(4096);

	// Initialize the bitmasks
	uint32_t bitmask = 0; // No bits set
	mPartialBitmasks[0] = bitmask;
	
	for (auto i = 1; i < 32; ++i) {
		// Set one more bit
		bitmask = (bitmask << 1) | 1;
		mPartialBitmasks[i] = bitmask;
	}

	// Initialize the bitcount lookup tables
	// Do it for 0-xFF first, then use that to do the upper word as well
	for (size_t i = 0; i <= UINT8_MAX; ++i) {
		// Count bits set in i
		mBitCountLut[i] = PopCntSlow(i);
	}

	// Now initialize the rest
	for (size_t i = UINT8_MAX + 1; i <= UINT16_MAX; ++i) {
		// Count bits set in i
		uint8_t upper = (uint16_t)i >> 8;
		uint8_t lower = i & 0xFF;
		mBitCountLut[i] = mBitCountLut[upper] + mBitCountLut[lower];
	}
}

ArrayIdxMapId ArrayIndexBitmaps::Allocate()
{
	
	// Take one from the pool if available
	if (!mFreeIds.empty()) {
		auto result = mFreeIds.back();
		mFreeIds.pop_back();
		return result;
	}

	// Otherwise add a new one
	ArrayIdxMapId id = mArrays.size();

	// Start with 2 blocks initially, which is enough to track indices 0-63
	ArrayIndices indices;
	indices.count = 2;
	indices.firstIdx = mBitmapBlocks.size();
	mBitmapBlocks.push_back(0);
	mBitmapBlocks.push_back(0);
	mArrays.emplace_back(indices);

	return id;

}

void ArrayIndexBitmaps::Free(ArrayIdxMapId id)
{
	auto &arr = mArrays[id];

	// Shrink the array to 2 elements before returning it to the free list
	if (arr.count > 2) {
		Shrink(id, mArrays[id].count - 2);
	}

	Expects(arr.count == 2);

	// Reset the bitmaps
	for (size_t i = 0; i < arr.count; ++i) {
		mBitmapBlocks[arr.firstIdx + i] = 0;
	}

	mFreeIds.push_back(id);
}

ArrayIdxMapId ArrayIndexBitmaps::Clone(ArrayIdxMapId id)
{
	auto result = Allocate();
	auto &dest = mArrays[result];
	auto &src = mArrays[id];

	// Make the destination big enough
	if (src.count > dest.count) {
		auto extendBy = src.count - dest.count;
		Extend(result, extendBy);
	}
	
	// Copy over the bitmap blocks
	std::copy(mBitmapBlocks.begin() + src.firstIdx,
		mBitmapBlocks.begin() + src.firstIdx + src.count,
		mBitmapBlocks.begin() + dest.firstIdx);

	return result;
}

void ArrayIndexBitmaps::RemoveIndex(ArrayIdxMapId id, size_t index)
{
	auto& arr = mArrays[id];
	auto blockIdx = index / 32;

	// The index is out of bounds
	if (blockIdx >= arr.count) {
		// But since we're removing, it doesn't matter
		return;
	}

	// Retrieve a reference to the bitmap block that contains the
	// bit for the index
	auto& bitmapBlock = mBitmapBlocks[arr.firstIdx + blockIdx];

	// Clear the bit that represents the index
	uint32_t bit = 1 << (index % 32);
	bitmapBlock &= ~ bit;
}

void ArrayIndexBitmaps::AddIndex(ArrayIdxMapId id, size_t index)
{
	auto& arr = mArrays[id];
	auto blockIdx = index / 32;

	// The index is out of bounds
	if (blockIdx >= arr.count) {
		// We have to extend it to be able to store the index bit
		auto extendBy = blockIdx + 1 - arr.count;
		Extend(id, extendBy);
	}

	// Retrieve a reference to the bitmap block that contains the
	// bit for the index
	auto& bitmapBlock = mBitmapBlocks[arr.firstIdx + blockIdx];

	// Set the bit that represents the index
	uint32_t bit = 1 << (index % 32);
	bitmapBlock |= bit;
}

bool ArrayIndexBitmaps::HasIndex(ArrayIdxMapId id, size_t index) const
{
	auto& arr = mArrays[id];
	auto blockIdx = index / 32;
	auto bitIdx = index % 32;

	if (blockIdx >= arr.count) {
		return false; // No allocated bitmap block for this idx
	}

	auto bitmapBlock = mBitmapBlocks[arr.firstIdx + blockIdx];
	auto mask = 1 << bitIdx;
	
	return (bitmapBlock & mask) != 0;
}

size_t ArrayIndexBitmaps::GetPackedIndex(ArrayIdxMapId id, size_t index) const
{
	auto& arr = mArrays[id];
	auto blockIdx = index / 32;
	
	size_t count = 0;

	// The number of fully counted blocks
	auto fullCounted = std::min(arr.count, blockIdx);
	for (size_t i = 0; i < fullCounted; ++i) {
		count += PopCnt(mBitmapBlocks[arr.firstIdx + i]);
	}

	// The block that contains the actual index bit is counted
	// and and not including the index bit itself
	if (blockIdx < arr.count) {
		uint8_t bitIdx = index % 32;
		count += PopCntConstrained(mBitmapBlocks[arr.firstIdx + blockIdx], bitIdx);
	}

	return count;
}

size_t ArrayIndexBitmaps::GetSerializedSize(ArrayIdxMapId id) const
{
	// Serialized form is: 32-bit int indicating the number of 32-bit blocks, followed by 
	// That number of 32-bit blocks
	return sizeof(uint32_t) + sizeof(uint32_t) * mArrays[id].count;
}

ArrayIdxMapId ArrayIndexBitmaps::DeserializeFromMemory(uint8_t **buffer)
{
	auto dwords = reinterpret_cast<uint32_t*>(*buffer);

	auto blockCount = *dwords++;

	auto result = Allocate();
	auto &arr = mArrays[result];

	// Extend to the right size
	if (blockCount > arr.count) {
		auto extendBy = blockCount - arr.count;
		Extend(result, extendBy);
	}

	// Read the data into the array map
	for (size_t i = 0; i < blockCount; ++i) {
		mBitmapBlocks[arr.firstIdx + i] = *dwords++;
	}

	// Write the new deserialization pointer back to the caller
	*buffer = reinterpret_cast<uint8_t*>(dwords);

	return result;
}

void ArrayIndexBitmaps::SerializeToMemory(ArrayIdxMapId id, uint8_t ** buffer) const
{
	auto dwords = reinterpret_cast<uint32_t*>(*buffer);

	auto &arr = mArrays[id];
	*dwords++ = arr.count;

	for (size_t i = 0; i < arr.count; ++i) {
		*dwords++ = mBitmapBlocks[arr.firstIdx + i];
	}

	*buffer = reinterpret_cast<uint8_t*>(dwords);
}

bool ArrayIndexBitmaps::SerializeToFile(ArrayIdxMapId id, TioFile * file) const
{

	auto& arr = mArrays[id];

	if (tio_fwrite(&arr.count, sizeof(uint32_t), 1, file) != 1) {
		return false;
	}

	if (tio_fwrite(&mBitmapBlocks[arr.firstIdx],
		sizeof(uint32_t),
		arr.count,
		file) != arr.count) {
		return false;
	}

	return true;

}

ArrayIdxMapId ArrayIndexBitmaps::DeserializeFromFile(TioFile * file)
{
	uint32_t count;
	if (tio_fread(&count, sizeof(uint32_t), 1, file) != 1) {
		throw TempleException("Unable to read array index map size");
	}

	auto result = Allocate();
	auto &arr = mArrays[result];

	if (count > arr.count) {
		auto extendBy = count - arr.count;
		Extend(result, extendBy);
	}

	if (tio_fread(&mBitmapBlocks[arr.firstIdx], sizeof(uint32_t), count, file) != count) {
		throw TempleException("Unable to read array index map data");
	}
	
	return result;
}

bool ArrayIndexBitmaps::ForEachIndex(ArrayIdxMapId id, std::function<bool(size_t)> callback) const
{
	auto &arr = mArrays[id];

	size_t index = 0;
	for (size_t i = 0; i < arr.count; ++i) {
		auto block = mBitmapBlocks[arr.firstIdx + i];
		
		for (uint8_t bitIdx = 0; bitIdx < 32; ++bitIdx) {
			// Index is present in the map
			if (block & (1 << bitIdx)) {
				if (!callback(index)) {
					return false;
				}
			}
			index++;
		}
	}

	return true;
}

uint8_t ArrayIndexBitmaps::PopCnt(uint32_t value) const
{
	uint16_t lower = value & UINT16_MAX;
	uint16_t upper = (value >> 16) & UINT16_MAX;
	return mBitCountLut[lower] + mBitCountLut[upper];
}

uint8_t ArrayIndexBitmaps::PopCntConstrained(uint32_t value, uint8_t upToExclusive) const
{
	// Use precomputed masks to unset the bits we don't want to count
	return PopCnt(value & mPartialBitmasks[upToExclusive]);
}

void ArrayIndexBitmaps::Shrink(ArrayIdxMapId id, size_t shrinkBy)
{
	auto& arr = mArrays[id];

	Expects(shrinkBy <= arr.count);
	arr.count -= shrinkBy;
	
	// Iterator to the first element to be removed
	auto first = mBitmapBlocks.begin() + arr.firstIdx + arr.count;
	auto last = first + shrinkBy;
	mBitmapBlocks.erase(first, last);

	// Now the "firstIdx" of all arrays after the one we modified have to be adjusted
	for (size_t i = id + 1; i < mArrays.size(); ++i) {
		mArrays[i].firstIdx -= shrinkBy;
	}
}

void ArrayIndexBitmaps::Extend(ArrayIdxMapId id, size_t extendBy)
{
	auto& arr = mArrays[id];
	
	// Iterator to the position before which the new elements will be inserted
	auto insertBefore = mBitmapBlocks.begin() + arr.firstIdx + arr.count;
	mBitmapBlocks.insert(insertBefore, extendBy, 0);

	arr.count += extendBy;

	// Now the "firstIdx" of all arrays after the one we modified have to be adjusted
	for (size_t i = id + 1; i < mArrays.size(); ++i) {
		mArrays[i].firstIdx += extendBy;
	}
}

ArrayIndexBitmaps arrayIdxBitmaps;
