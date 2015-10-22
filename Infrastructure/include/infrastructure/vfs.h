
#pragma once

#include <memory>
#include <string>
#include <vector>

/*
	Abstractions for accessing files in the virtual file system.
	The virtual file system is backed by TIO in a normal game,
	which means that the files are scattered across the .dat files
	and data directories.
*/
class Vfs {
public:
	virtual ~Vfs() {};

	static Vfs* CreateStdIoVfs();

	/*
		Reads a file fully into a string.
	*/
	std::string ReadAsString(const std::string &filename);

	/*
		Reads a binary file fully into a vector of uint8_t.
	*/
	std::vector<uint8_t> ReadAsBinary(const std::string &filename);
		
protected:
	typedef void* FileHandle;
	virtual FileHandle Open(const char *name, const char *mode) = 0;
	virtual size_t Read(void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t Length(FileHandle handle) = 0;
	virtual void Close(FileHandle handle) = 0;
};

extern std::unique_ptr<Vfs> vfs;
