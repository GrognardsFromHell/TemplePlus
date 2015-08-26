
#define _CRT_SECURE_NO_WARNINGS
#include "vfs.h"

#include <sys/stat.h>

#include "exception.h"
#include "format.h"

std::unique_ptr<Vfs> vfs;

class StdIoVfs : public Vfs {
public:
	~StdIoVfs() {}

protected:
	
	// Inherited via Vfs
	virtual FileHandle open(const char * name, const char * mode) override
	{
		return (FileHandle)fopen(name, mode);
	}

	virtual size_t length(FileHandle handle) override
	{
		struct stat st;
		auto fn = _fileno((FILE*)handle);
		fstat(fn, &st);
		return st.st_size;
	}

	virtual size_t read(void * buffer, size_t size, FileHandle handle) override
	{
		return fread(buffer, 1, size, (FILE*) handle);
	}

	virtual void close(FileHandle handle) override
	{
		fclose((FILE*)handle);
	}

};

Vfs* Vfs::CreateStdIoVfs() {
	return new StdIoVfs;
}

std::string Vfs::ReadAsString(const std::string & filename)
{
	auto fh = open(filename.c_str(), "rt");
	if (!fh) {
		throw new TempleException(fmt::format("Unable to find file {}", filename));
	}
	auto fileSize = length(fh);
	std::string result;
	result.resize(fileSize);
	read(const_cast<char*>(result.data()), fileSize, fh);
	close(fh);
	return result;
}
