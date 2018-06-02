#define _CRT_SECURE_NO_WARNINGS
#include "infrastructure/vfs.h"

#include <cstdio>
#include <sys/stat.h>
#include <direct.h>

#include "infrastructure/exception.h"
#include <fmt/format.h>

std::unique_ptr<Vfs> vfs;

class StdIoVfs : public Vfs {
public:
	~StdIoVfs() {
	}

protected:
	
	// Inherited via Vfs
	virtual FileHandle Open(const char* name, const char* mode) override {
		return (FileHandle)fopen(name, mode);
	}

	virtual size_t Length(FileHandle handle) override {
		struct stat st;
		auto fn = _fileno((FILE*)handle);
		fstat(fn, &st);
		return st.st_size;
	}

	virtual size_t Read(void* buffer, size_t size, FileHandle handle) override {
		return fread(buffer, 1, size, (FILE*)handle);
	}
	
	virtual size_t Write(const void* buffer, size_t size, FileHandle handle) override {
		return fwrite(buffer, 1, size, (FILE*)handle);
	}

	virtual void Close(FileHandle handle) override {
		fclose((FILE*)handle);
	}


public:
	bool MkDir(const std::string& path) override {
		return _mkdir(path.c_str()) == 0;
	}

	virtual bool FileExists(const std::string& path) override {
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0) {
			return false;
		}
		return (buffer.st_mode & _S_IFREG) != 0;
	}
	
	virtual bool DirExists(const std::string& path) override {
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0) {
			return false;
		}
		return (buffer.st_mode & _S_IFDIR) != 0;
	}

	std::vector<VfsSearchResult> Search(const std::string& globPattern) override {
		throw TempleException("Unsupported Operation");
	}
	bool RemoveDir(const std::string& path) override {
		throw TempleException("Unsupported Operation");
	}
	bool RemoveFile(const std::string& path) override {
		throw TempleException("Unsupported Operation");
	}
	size_t Tell(FileHandle handle) override {
		return ftell((FILE*)handle);
	}
};

Vfs* Vfs::CreateStdIoVfs() {
	return new StdIoVfs;
}

std::string Vfs::ReadAsString(const std::string& filename) {
	auto fh = Open(filename.c_str(), "rt");
	if (!fh) {
		throw TempleException("Unable to find file {}", filename);
	}
	// Length is just an upper bound because it will count the \r's in the string
	auto fileSize = Length(fh);
	std::string result;
	result.reserve(fileSize);
	size_t bytesRead;
	do {
		char buffer[4096];
		bytesRead = Read(&buffer[0], 4096, fh);
		result.append(&buffer[0], bytesRead);
	} while (bytesRead == 4096);
	Close(fh);
	return result;
}

std::vector<uint8_t> Vfs::ReadAsBinary(const std::string& filename) {
	auto fh = Open(filename.c_str(), "rb");
	if (!fh) {
		throw TempleException("Unable to find file {}", filename);
	}
	auto fileSize = Length(fh);
	std::vector<uint8_t> result;
	if (fileSize > 0) {
		result.resize(fileSize);
		Read(&result[0], fileSize, fh);
	}
	Close(fh);
	return result;
}

bool Vfs::IsDirEmpty(const std::string& path) {
	if (!DirExists(path)) {
		return false;
	}

	auto globPattern(Path::Concat(path, "*.*"));
	return Search(globPattern).empty();
}

void Vfs::WriteBinaryFile(const std::string &path, gsl::span<uint8_t> data) {
	
	auto fh(Open(path.c_str(), "wb"));
	if (Write(&data[0], data.size(), fh) != data.size()) {
		Close(fh);
		RemoveFile(path.c_str());
		throw TempleException("Unable to write file {}", path);
	}
	Close(fh);

}

bool Vfs::CleanDir(const std::string& path) {
	if (!DirExists(path)) {
		return false;
	}

	auto result = true;

	for (const auto &entry : Search(Path::Concat(path, "*.*"))) {
		auto fullPath(Path::Concat(path, entry.filename));

		if (!entry.dir) {
			result &= RemoveFile(fullPath);
		}
		else {
			CleanDir(fullPath);
			result &= RemoveDir(fullPath);
		}
	}

	return result;
}

bool Path::IsFileSystem(const std::string& path) {

	if (path.empty()) {
		return false;
	}

	if (path[0] == '.' || path[0] == '\\' || path[0] == '/') {
		return true;
	}

	if (path.length() < 3) {
		return false;
	}

	return isalpha(path[0]) && path[1] == ':' && path[2] == '\\';

}

std::string Path::Concat(const std::string& a, const std::string& b) {

	std::string result;
	result.reserve(a.length() + b.length() + 1);
	result.append(a);

	if (!result.empty() && result.back() != '\\' && result.back() != '/') {
		result.push_back('\\');
	}

	result.append(b);

	return result;

}

