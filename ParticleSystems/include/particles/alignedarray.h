
#pragma once

#include <malloc.h>

class AlignedFloatArray {
public:
	explicit AlignedFloatArray(int capacity)
		: mCapacity(RoundUpToBatchSize(capacity)),
		mData((float*)_aligned_malloc(sizeof(float) * mCapacity, 16))  {
	}
	~AlignedFloatArray() {
		if (mData) {
			_aligned_free(const_cast<float*>(mData));
		}
	}
	
	int GetCapacity() const {
		return mCapacity;
	}

	float operator[](int index) const {
		return mData[index];
	}

	float& operator[](int index) {
		return mData[index];
	}

	// Fill the array with the given constant value.
	void Fill(int start, int endExcl, float value) {
		for (auto i = start; i < endExcl; ++i) {
			mData[i] = value;
		}
	}

	void Multiply(int start, int endExcl, float factor) {
		for (auto i = start; i < endExcl; ++i) {
			mData[i] *= factor;
		}
	}

	void Add(int start, int endExcl, const AlignedFloatArray &other) {
		assert(endExcl <= mCapacity);
		assert(endExcl <= other.mCapacity);
		for (auto i = start; i < endExcl; ++i) {
			mData[i] += other.mData[i];
		}
	}

	void AddScaled(int start, int endExcl, const AlignedFloatArray &other, float scale) {
		assert(endExcl <= mCapacity);
		assert(endExcl <= other.mCapacity);
		for (auto i = start; i < endExcl; ++i) {
			mData[i] += other.mData[i] * scale;
		}
	}

	void Sum(int start, int endExcl, const AlignedFloatArray &op1, const AlignedFloatArray &op2) {
		assert(endExcl <= mCapacity);
		assert(endExcl <= op1.mCapacity);
		assert(endExcl <= op2.mCapacity);
		for (auto i = start; i < endExcl; ++i) {
			mData[i] = op1.mData[i] + op2.mData[i];
		}
	}

private:

	constexpr static const int BatchSize = 4;
	
	template<typename TAlgo, class... TAlgoArg>
	void BatchProcess(int start, int endExcl, TAlgoArg... args) {
		TAlgo algo(std::forward<TAlgoArg>(args)...);

		auto startOfBatch = RoundUpToBatchSize(start);
		auto endOfBatch = RoundDownToBatchSize(endExcl);

		auto dest = mData + start;
		
		// Fill the non 4 byte aligned area before the batch area
		if (start < startOfBatch) {
			auto end = mData + startOfBatch;
			while (dest != end) {
				algo.ProcessSingle(dest++);
			}
		}

		if (startOfBatch < endOfBatch) {
			auto batchDest = mData + startOfBatch;
			auto batchEnd = mData + endOfBatch;
			while (batchDest != batchEnd) {
				algo.ProcessBatch(batchDest);
				batchDest += BatchSize;
			}
		}

		// Fill the non 4 byte aligned area after the batch area
		auto end = mData + endExcl;
		while (dest != end) {
			algo.ProcessSingle(dest++);
		}
	}

	static int RoundUpToBatchSize(int size) {
		// Round size up to multiples of 4
		if (size % BatchSize) {
			return (size / BatchSize + 1) * BatchSize;
		}
		else {
			return size;
		}
	}

	static int RoundDownToBatchSize(int size) {
		// Round size down to multiples of 4
		if (size % BatchSize) {
			return (size / BatchSize) * BatchSize;
		}
		else {
			return size;
		}
	}

	const int mCapacity;
	float* const mData;
};
