
#include "stdafx.h"
#include "util/fixes.h"

#include "arrayidxbitmaps.h"
#include "objarrays.h"
#include <util/streams.h>

static class ArrayIndexBitmapsHooks : public TempleFix {
public:

	virtual const char * name() override
	{
		return "Array Idx Bitmap Hooks";
	}

	virtual void apply() override
	{
		replaceFunction<ArrayIdxMapId()>(0x1010b290, []() -> ArrayIdxMapId {
			return arrayIdxBitmaps.Allocate();
		});
		
		replaceFunction<void(ArrayIdxMapId)>(0x1010b320, [](ArrayIdxMapId id) {
			arrayIdxBitmaps.Free(id);
		});

		replaceFunction<void(ArrayIdxMapId, size_t, BOOL)>(0x1010b420, [](auto id, auto idx, auto used) {
			if (used) {
				arrayIdxBitmaps.AddIndex(id, idx);
			} else {
				arrayIdxBitmaps.RemoveIndex(id, idx);
			}
		});

		replaceFunction<size_t(ArrayIdxMapId, size_t)>(0x1010b040, [](auto id, auto idx) {
			return arrayIdxBitmaps.GetPackedIndex(id, idx);
		});

		replaceFunction<ArrayIdxMapId(ArrayIdxMapId)>(0x1010b3c0, [](auto id) {
			return arrayIdxBitmaps.Clone(id);
		});

		replaceFunction<BOOL(ArrayIdxMapId, size_t)>(0x1010aff0, [](auto id, auto idx) {
			return arrayIdxBitmaps.HasIndex(id, idx) ? TRUE : FALSE;
		});

		replaceFunction<size_t(ArrayIdxMapId)>(0x1010ae30, [](auto id) {
			return arrayIdxBitmaps.GetSerializedSize(id);
		});

		replaceFunction<void(ArrayIdxMapId*, uint8_t**)>(0x1010b520, [](auto id, auto buffer) {
			*id = arrayIdxBitmaps.DeserializeFromMemory(buffer);
		});

		// Sadly the array idx map code was inlined here, so we have to replace the entire function
		replaceFunction<void(const uint8_t *, void*)>(0x1010b7c0, [](auto array, auto bufferOut) {
			auto dwords = reinterpret_cast<const uint32_t*>(array);

			// Serialize the array itself
			auto elSize = *dwords;
			auto elCount = *(dwords + 1);
			auto idxMapId = static_cast<ArrayIdxMapId>(*(dwords + 2));
			auto fullSize = 12 + elSize * elCount; // 12 is for the header
			memcpy(bufferOut, array, fullSize);

			// Now write the array index map
			uint8_t *extraDataOut = reinterpret_cast<uint8_t*>(bufferOut) + fullSize;
			arrayIdxBitmaps.SerializeToMemory(idxMapId, &extraDataOut);
		});

		replaceFunction<BOOL(ArrayIdxMapId, TioFile*)>(0x1010add0, [](auto id, auto file) {
			TioOutputStream output(file);
			arrayIdxBitmaps.SerializeToStream(id, output);
			return TRUE;
		});

		replaceFunction<BOOL(ArrayIdxMapId*, TioFile*)>(0x1010b4a0, [](auto idOut, auto file) {
			try {
				*idOut = arrayIdxBitmaps.DeserializeFromFile(file);
				return TRUE;
			} catch (TempleException&) {
				*idOut = 0;
				return FALSE;
			}
		});

		// This iteration function seems to mix obj arrays and the array idx map internals
		using Callback = BOOL(*)(void* data, size_t idx);
		replaceFunction<BOOL(ArrayHeader**, Callback)>(0x1010b700, [](auto arrayPtr, auto callback) {
			auto array = *arrayPtr;
			
			size_t packedIdx = 0;
			bool result = arrayIdxBitmaps.ForEachIndex(array->idxBitmapId, [&](auto idx) {
				auto elem = array->GetData(packedIdx++);
				return callback(elem, idx) == TRUE;
			});

			return result ? TRUE : FALSE;
		});

		replaceFunction<int(uint32_t, int)>(0x1010ae90, [](auto value, auto bitIdx) {
			// Some calls are made with bitIdx == 32 to count all bits
			if (bitIdx >= 32) {
				return (int)arrayIdxBitmaps.PopCnt(value);
			} else {
				return (int)arrayIdxBitmaps.PopCntConstrained(value, bitIdx);
			}
		});

		replaceFunction<void(ArrayHeader**, size_t, void*)>(0x1010b730, [](auto arrayPtr, auto idx, auto dataOut) {
			auto array = *arrayPtr;

			auto hasIndex = arrayIdxBitmaps.HasIndex(array->idxBitmapId, idx);
			if (!hasIndex) {
				memset(dataOut, 0, array->elSize);
				return;
			}

			auto packedIdx = arrayIdxBitmaps.GetPackedIndex(array->idxBitmapId, idx);
			auto elData = array->GetData(packedIdx);

			memcpy(dataOut, elData, array->elSize);
		});

	}

} hooks;
