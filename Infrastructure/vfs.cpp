
#define _CRT_SECURE_NO_WARNINGS
#include "infrastructure/vfs.h"

#include <sys/stat.h>

#include "infrastructure/exception.h"
#include "infrastructure/format.h"

std::unique_ptr<Vfs> vfs;

class StdIoVfs : public Vfs {
public:
	~StdIoVfs() {}

protected:
	
	// Inherited via Vfs
	virtual FileHandle Open(const char * name, const char * mode) override
	{
		return (FileHandle)fopen(name, mode);
	}

	virtual size_t Length(FileHandle handle) override
	{
		struct stat st;
		auto fn = _fileno((FILE*)handle);
		fstat(fn, &st);
		return st.st_size;
	}

	virtual size_t Read(void * buffer, size_t size, FileHandle handle) override
	{
		return fread(buffer, 1, size, (FILE*) handle);
	}

	virtual void Close(FileHandle handle) override
	{
		fclose((FILE*)handle);
	}

};

Vfs* Vfs::CreateStdIoVfs() {
	return new StdIoVfs;
}

std::string Vfs::ReadAsString(const std::string & filename)
{
	auto fh = Open(filename.c_str(), "rt");
	if (!fh) {
		throw TempleException("Unable to find file {}", filename);
	}
	auto fileSize = Length(fh);
	std::string result;
	result.resize(fileSize);
	Read(const_cast<char*>(result.data()), fileSize, fh);
	Close(fh);
	return result;
}

std::vector<uint8_t> Vfs::ReadAsBinary(const std::string& filename) {
	auto fh = Open(filename.c_str(), "rt");
	if (!fh) {
		throw TempleException("Unable to find file {}", filename);
	}
	auto fileSize = Length(fh);
	std::vector<uint8_t> result;
	result.resize(fileSize);
	Read(&result[0], fileSize, fh);
	Close(fh);
	return result;
}
