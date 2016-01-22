
#include "stdafx.h"
#include "streams.h"

#include <infrastructure/vfs.h>

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

VfsOutputStream::VfsOutputStream(const std::string& filename) : mFilename(filename) {
	mHandle = vfs->Open(filename.c_str(), "wb");
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
