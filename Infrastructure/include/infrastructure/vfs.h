
#pragma once

#include <memory>
#include <string>

/*
	Abstractions for accessing files in the virtual file system.
	The virtual file system is backed by TIO in a normal game,
	which means that the files are scattered across the .dat files
	and data directories.
*/
class Vfs {
public:
	virtual ~Vfs() = 0;

	static Vfs* CreateStdIoVfs();

	/*
		Reads a file fully into a string.
	*/
	std::string ReadAsString(const std::string &filename);

protected:
	typedef void* FileHandle;
	virtual FileHandle open(const char *name, const char *mode) = 0;
	virtual size_t read(void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t length(FileHandle handle) = 0;
	virtual void close(FileHandle handle) = 0;
};

inline Vfs::~Vfs() {
}

extern std::unique_ptr<Vfs> vfs;
