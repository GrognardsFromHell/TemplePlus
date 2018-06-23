#pragma once

#include <memory>
#include <string>
#include <vector>

#include <gsl/span>

struct VfsSearchResult {
	std::string filename;
	bool dir;
	size_t sizeInBytes;
	uint32_t lastModified;
};

enum class SeekDir {
	Start = 0,
	Current = 1,
	End = 2
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
	std::string ReadAsString(std::string_view filename);

	/*
	Reads a binary file fully into a vector of uint8_t.
	*/
	std::vector<uint8_t> ReadAsBinary(std::string_view filename);

	/**
	 * Does the file exist?
	 */
	virtual bool FileExists(std::string_view path) = 0;

	/**
	 * Does the directory exist?
	 */
	virtual bool DirExists(std::string_view path) = 0;

	/**
	 * Creates a directory.
	 */
	virtual bool MkDir(std::string_view path) = 0;

	/**
	 * Will return all files and directories that match the given glob pattern, which
	 * is not recursive.
	 * Example: mes\*.mes will return the names of all files in mes (not full paths) that
	            end with .mes.
	 */
	virtual std::vector<VfsSearchResult> Search(std::string_view globPattern) = 0;

	/**
	 * Removes an empty directory.
	 */
	virtual bool RemoveDir(std::string_view path) = 0;

	/**
	 * Removes a file (no directories).
	 */
	virtual bool RemoveFile(std::string_view path) = 0;

	/**
	* Deletes all files and dirctories within the given directory.
	*/
	bool CleanDir(std::string_view path);

	/**
	 * Returns true if the given directory does not contain anything.
	 */
	bool IsDirEmpty(std::string_view path);

	/**
	 * Writes binary data to a file.
	 */
	void WriteBinaryFile(std::string_view path, gsl::span<uint8_t> data);

	using FileHandle = void*;
	virtual FileHandle Open(std::string_view name, std::string_view mode) = 0;
	virtual size_t Read(void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t Write(const void* buffer, size_t size, FileHandle handle) = 0;
	virtual size_t Length(FileHandle handle) = 0;
	virtual size_t Tell(FileHandle handle) = 0;
	virtual void Seek(FileHandle handle, int position, SeekDir dir = SeekDir::Start) = 0;
	virtual void Close(FileHandle handle) = 0;
	
};

class VfsPath {
public:
	VfsPath() = delete;

	static bool IsFileSystem(std::string_view path);
	static std::string Concat(std::string_view a, std::string_view b);
};

extern std::unique_ptr<Vfs> vfs;
