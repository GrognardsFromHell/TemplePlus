#pragma once

#include <memory>
#include <string>
#include <vector>

#include "gsl/array_view.h"

struct VfsSearchResult {
	std::string filename;
	bool dir;
	size_t sizeInBytes;
	uint32_t lastModified;
};

/*
	Abstractions for accessing files in the virtual file system.
	The virtual file system is backed by TIO in a normal game,
	which means that the files are scattered across the .dat files
	and data directories.
*/
class Vfs {
public:
	virtual ~Vfs() {
	};

	static Vfs* CreateStdIoVfs();

	/*
		Reads a file fully into a string.
	*/
	std::string ReadAsString(const std::string& filename);

	/*
		Reads a binary file fully into a vector of uint8_t.
	*/
	std::vector<uint8_t> ReadAsBinary(const std::string& filename);

	/**
	 * Does the file exist?
	 */
	virtual bool FileExists(const std::string& path) = 0;

	/**
	 * Does the directory exist?
	 */
	virtual bool DirExists(const std::string& path) = 0;

	/**
	 * Creates a directory.
	 */
	virtual bool MkDir(const std::string& path) = 0;

	/**
	 * Will return all files and directories that match the given glob pattern, which
	 * is not recursive.
	 * Example: mes\*.mes will return the names of all files in mes (not full paths) that
	            end with .mes.
	 */
	virtual std::vector<VfsSearchResult> Search(const std::string& globPattern) = 0;

	/**
	 * Removes an empty directory.
	 */
	virtual bool RemoveDir(const std::string& path) = 0;

	/**
	 * Removes a file (no directories).
	 */
	virtual bool RemoveFile(const std::string& path) = 0;

	/**
	* Deletes all files and dirctories within the given directory.
	*/
	bool CleanDir(const std::string& path);

	/**
	 * Returns true if the given directory does not contain anything.
	 */
	bool IsDirEmpty(const std::string& path);

	/**
	 * Writes binary data to a file.
	 */
	void WriteBinaryFile(const std::string &path, gsl::array_view<uint8_t> data);

	using FileHandle = void*;
	virtual FileHandle Open(const char* name, const char* mode) = 0;
	virtual size_t Read(void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t Write(void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t Length(FileHandle handle) = 0;
	virtual void Close(FileHandle handle) = 0;
	
};

class Path {
public:
	Path() = delete;

	static bool IsFileSystem(const std::string& path);
	static std::string Concat(const std::string& a, const std::string& b);
};

extern std::unique_ptr<Vfs> vfs;
