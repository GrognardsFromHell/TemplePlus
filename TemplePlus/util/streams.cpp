
#include "stdafx.h"
#include "streams.h"

#include <infrastructure/vfs.h>
#include <tio/tio.h>
#include "gametime.h"

void InputStream::CopyTo(OutputStream& out, size_t bytes) {

	uint8_t buffer[4096];

	// Copy in blocks
	while (bytes >= sizeof(buffer)) {
		ReadBytes(&buffer[0], sizeof(buffer));
		out.WriteBytes(&buffer[0], sizeof(buffer));
		bytes -= sizeof(buffer);
	}

	// Now copy the rest
	if (bytes > 0) {
		ReadBytes(&buffer[0], bytes);
		out.WriteBytes(&buffer[0], bytes);
	}

}

void InputStream::ReadGameTime(GameTime& gameTimeOut)
{
	ReadRaw(&gameTimeOut, sizeof(GameTime));
}

VfsInputStream::VfsInputStream(const std::string& filename) : mFilename(filename) {
	mHandle = vfs->Open(filename.c_str(), "rb");
	if (!mHandle) {
		throw TempleException("Cannot open {} for reading.", filename);
	}
}

VfsInputStream::~VfsInputStream() {
	vfs->Close(mHandle);
}

void VfsInputStream::ReadRaw(void* buffer, size_t count) {
	if (vfs->Read(buffer, count, mHandle) != count) {
		throw TempleException("Error while reading {} bytes from {}", count, mFilename);
	}
}

size_t VfsInputStream::GetPos() const {
	return vfs->Tell(mHandle);
}

VfsOutputStream::VfsOutputStream(const std::string& filename, bool binary, bool append) : mFilename(filename) {
	char mode[3] = { 'w', 'b' };
	if (append) {
		mode[0] = 'a';
	}
	if (!binary) {
		mode[1] = 't';
	}

	mHandle = vfs->Open(filename.c_str(), mode);
	if (!mHandle) {
		throw TempleException("Cannot open {} for writing.", filename);
	}
}

VfsOutputStream::~VfsOutputStream() {
	vfs->Close(mHandle);
}

void VfsOutputStream::WriteRaw(const void* buffer, size_t count) {
	if (vfs->Write(buffer, count, mHandle) != count) {
		throw TempleException("Error while writing {} bytes to {}", count, mFilename);
	}
}

size_t VfsOutputStream::GetPos() const {
	return vfs->Tell(mHandle);
}

TioOutputStream::TioOutputStream(TioFile* file) : mHandle(file) {
}

void TioOutputStream::WriteRaw(const void* buffer, size_t count) {
	if (tio_fwrite(buffer, count, 1, mHandle) != 1) {
		throw TempleException("Unable to write {} bytes to file.", count);
	}
}

size_t TioOutputStream::GetPos() const {
	return tio_ftell(mHandle);
}

void MemoryOutputStream::WriteRaw(const void* buffer, size_t count) {
	auto begin = reinterpret_cast<const uint8_t*>(buffer);
	auto end = begin + count;

	mBuffer.insert(mBuffer.end(), begin, end);
}

size_t MemoryOutputStream::GetPos() const {
	return mBuffer.size();
}

void OutputStream::WriteGameTime(const GameTime& time) {
	WriteRaw(&time, sizeof(GameTime));
}