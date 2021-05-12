
#pragma once
#include <cstdint>
#include <obj_structs.h>
#include <common.h>

struct TioFile;
class OutputStream;
struct GameTime;

class InputStream {
public:
	virtual ~InputStream() {}

	int8_t ReadInt8() {
		int8_t value;
		ReadRaw(&value, sizeof(int8_t));
		return value;
	}
	uint8_t ReadUInt8() {
		uint8_t value;
		ReadRaw(&value, sizeof(uint8_t));
		return value;
	}

	int16_t ReadInt16() {
		int16_t value;
		ReadRaw(&value, sizeof(int16_t));
		return value;
	}
	uint16_t ReadUInt16() {
		uint16_t value;
		ReadRaw(&value, sizeof(uint16_t));
		return value;
	}

	int32_t ReadInt32() {
		int32_t value;
		ReadRaw(&value, sizeof(int32_t));
		return value;
	}
	uint32_t ReadUInt32() {
		uint32_t value;
		ReadRaw(&value, sizeof(uint32_t));
		return value;
	}

	int64_t ReadInt64() {
		int64_t value;
		ReadRaw(&value, sizeof(int64_t));
		return value;
	}
	uint64_t ReadUInt64() {
		uint64_t value;
		ReadRaw(&value, sizeof(uint64_t));
		return value;
	}

	ObjectId ReadObjectId() {
		ObjectId id;
		ReadRaw(&id, sizeof(id));
		return id;
	}

	void ReadGameTime(GameTime& gameTimeOut);

	float ReadFloat() {
		float value;
		ReadRaw(&value, sizeof(float));
		return value;
	}

	void ReadBytes(uint8_t* buffer, size_t count) {
		ReadRaw(buffer, count);
	}

	std::string ReadLine(size_t maxLen) {
		std::string line;
		line.reserve(maxLen);
		for (size_t i = 0; i < maxLen; ++i) {
			char ch = ReadUInt8();
			if (ch == '\n') {
				return line;
			}
			line.push_back(ch);
		}
		throw TempleException("Could not read a line");
	}

	void CopyTo(OutputStream &out, size_t bytes);

	/**
	 * Reads a string with a 4byte length prefix that also includes
	 * a null-byte in both the length and the actual data.
	 */
	std::string ReadZStringPrefixed() {
		auto len = ReadUInt32();
		if (len < 1) {
			throw TempleException("Expected at least the null-byte for a null-terminated string.");
		}
		std::string result;
		result.resize(len - 1);
		ReadRaw(&result[0], len - 1);
		if (ReadUInt8() != 0) {
			throw TempleException("String was not null-terminated.");
		}
		return result;
	}

	/**
	 * Reads a string with a 4byte length prefix that DOES NOT include
	 * a null-byte in either the length or the actual data.
	 */
	std::string ReadStringPrefixed() {
		auto len = ReadUInt32();
		std::string result;
		if (len == 0) {
			return result;
		}
		result.resize(len);
		ReadRaw(&result[0], len);
		return result;
	}

	/**
	 * Reads a fixed length string from the file. Ensures the string is properly null terminated.
	 */
	std::string ReadStringFixed(size_t size) {
		std::vector<char> buffer(size + 1, 0);
		ReadRaw(&buffer[0], size);
		std::string result;
		result.assign(buffer.data(), size);
		return result;
	}

	/**
	 * Reads a finite length string from the file. Ensures the string is properly null terminated.
	 */
	std::string ReadStringMaxSize(size_t maxSize) {
		auto size = ReadUInt32();
		if (size > maxSize) {
			throw TempleException("ReadStringMaxSize: String length greater than expected");
		}
		std::vector<char> buffer(size+ 1, 0);
		ReadRaw(&buffer[0], size);
		std::string result;
		result.assign(buffer.data(), size);
		return result;
	}

	LocFull ReadLocFull() {
		LocFull result;
		result.location.location = locXY::fromField(ReadUInt64());
		result.location.off_x = ReadFloat();
		result.location.off_y = ReadFloat();
		result.off_z = ReadFloat();
		return result;
	}

	locXY ReadLoc() {
		return locXY::fromField(ReadUInt64());
	}

	XMFLOAT3 ReadXMFLOAT3() {
		XMFLOAT3 result;
		ReadRaw(&result, sizeof(result));
		return result;
	}

protected:
	virtual void ReadRaw(void* buffer, size_t count) = 0;
	virtual size_t GetPos() const = 0;
};

class VfsInputStream : public InputStream {
public:
	VfsInputStream(const std::string &filename);
	~VfsInputStream();
protected:
	void ReadRaw(void* buffer, size_t count) override;
	size_t GetPos() const override;
private:
	void* mHandle = nullptr;
	std::string mFilename;
};

class MemoryInputStream : public InputStream {
public:
	MemoryInputStream(gsl::span<const uint8_t> data)
		: mCurrent(data.data()), mData(data) {
		Expects(mData.size() >= 0);
	}

	void ReadRaw(void* buffer, size_t count) override {
		Expects(GetPos() + count <= static_cast<size_t>(mData.size()));
		memcpy(buffer, mCurrent, count);
		mCurrent += count;
	}

	size_t GetPos() const override {
		return (size_t)(mCurrent - mData.data());
	}

	bool AtEnd() const {
		return (mCurrent >= mData.data() + mData.size() );
	}
private:
	const uint8_t* mCurrent;
	gsl::span<const uint8_t> mData;
};

class OutputStream {
public:
	virtual ~OutputStream() {}

	void WriteInt8(int8_t value) {
		WriteRaw(&value, sizeof(int8_t));
	}

	void WriteUInt8(uint8_t value) {
		WriteRaw(&value, sizeof(uint8_t));
	}

	void WriteInt16(int16_t value) {
		WriteRaw(&value, sizeof(int16_t));
	}

	void WriteUInt16(uint16_t value) {
		WriteRaw(&value, sizeof(uint16_t));
	}

	void WriteInt32(int32_t value) {
		WriteRaw(&value, sizeof(int32_t));
	}

	void WriteUInt32(uint32_t value) {
		WriteRaw(&value, sizeof(uint32_t));
	}

	void WriteUInt64(uint64_t value) {
		WriteRaw(&value, sizeof(uint64_t));
	}

	void WriteInt64(int64_t value) {
		WriteRaw(&value, sizeof(int64_t));
	}

	void WriteBytes(const uint8_t* buffer, size_t count) {
		WriteRaw(buffer, count);
	}

	void WriteFloat(float value) {
		WriteRaw(&value, sizeof(float));
	}

	void WriteStringPrefixed(const std::string &str) {
		WriteUInt32(str.size());
		WriteRaw(str.c_str(), str.size());
	}

	void WriteStringMaxSize(const std::string& str, size_t maxSize) {
		auto size = min(maxSize, str.size());
		WriteUInt32(size);
		WriteRaw(str.c_str(), size);
	}
	
	void WriteObjectId(const ObjectId &id) {
		WriteRaw(&id, sizeof(ObjectId));
	}

	void WriteGameTime(const GameTime& time);

protected:
	virtual void WriteRaw(const void* buffer, size_t count) = 0;
	virtual size_t GetPos() const = 0;
};

class VfsOutputStream : public OutputStream {
public:
	explicit VfsOutputStream(const std::string &filename, bool binary = true, bool append = false);
	~VfsOutputStream();

protected:
	void WriteRaw(const void* buffer, size_t count) override;
	size_t GetPos() const override;
private:
	void* mHandle = nullptr;
	std::string mFilename;
};

class TioOutputStream : public OutputStream {
public:
	explicit TioOutputStream(TioFile *file);

protected:
	void WriteRaw(const void* buffer, size_t count) override;
	size_t GetPos() const override;
private:
	TioFile * mHandle = nullptr;
};

class MemoryOutputStream : public OutputStream {
public:
	explicit MemoryOutputStream(size_t initialCapacity = 512) {
		mBuffer.reserve(initialCapacity);		
	}
	
	gsl::span<const uint8_t> GetBuffer() const {
		return mBuffer;
	}

protected:
	void WriteRaw(const void* buffer, size_t count) override;
	size_t GetPos() const override;
	
private:
	std::vector<uint8_t> mBuffer;
};