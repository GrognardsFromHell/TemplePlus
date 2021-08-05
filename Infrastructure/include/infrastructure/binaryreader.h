
#pragma once

#include <cassert>
#include <span>

/*
	Helper class for reading structured binary data from a 
	memory stream.
*/
class BinaryReader {
public:

	explicit BinaryReader(std::span<const uint8_t> data) : mData(data) {}

	template<typename T>
	T Read() {
		assert(mData.size() >= sizeof(T));
		auto result{ *reinterpret_cast<const T*>(&mData[0]) };
		mData = mData.subspan(sizeof(T));
		return result;
	}

	std::string ReadFixedString(size_t length) {
		assert(mData.size() >= (int) length);
		std::string result(length, '\0');
		memcpy(&result[0], &mData[0], length);
		result.resize(result.length());
		mData = mData.subspan(length);
		return result;
	}

	bool AtEnd() const {
		return mData.empty();
	}

private:
	std::span<const uint8_t> mData;
};
