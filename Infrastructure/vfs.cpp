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
	virtual FileHandle Open(std::string_view name, std::string_view mode) override {
		return (FileHandle)fopen(name.data(), mode.data());
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

	virtual void Seek(FileHandle handle, int position, SeekDir dir) override {
		int origin;
		switch (dir) {
		default:
		case SeekDir::Start:
			origin = SEEK_SET;
			break;
		case SeekDir::Current:
			origin = SEEK_CUR;
			break;
		case SeekDir::End:
			origin = SEEK_END;
			break;
		}

		fseek((FILE*)handle, position, origin);
	}

public:
	bool MkDir(std::string_view path) override {
		return _mkdir(path.data()) == 0;
	}

	virtual bool FileExists(std::string_view path) override {
		struct stat buffer;
		if (stat(path.data(), &buffer) != 0) {
			return false;
		}
		return (buffer.st_mode & _S_IFREG) != 0;
	}
	
	virtual bool DirExists(std::string_view path) override {
		struct stat buffer;
		if (stat(path.data(), &buffer) != 0) {
			return false;
		}
		return (buffer.st_mode & _S_IFDIR) != 0;
	}

	std::vector<VfsSearchResult> Search(std::string_view globPattern) override {
		throw TempleException("Unsupported Operation");
	}
	bool RemoveDir(std::string_view path) override {
		throw TempleException("Unsupported Operation");
	}
	bool RemoveFile(std::string_view path) override {
		throw TempleException("Unsupported Operation");
	}
	size_t Tell(FileHandle handle) override {
		return ftell((FILE*)handle);
	}
};

Vfs* Vfs::CreateStdIoVfs() {
	return new StdIoVfs;
}

std::string Vfs::ReadAsString(std::string_view filename) {
	auto fh = Open(filename, "rt");
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

std::vector<uint8_t> Vfs::ReadAsBinary(std::string_view filename) {
	auto fh = Open(filename, "rb");
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

bool Vfs::IsDirEmpty(std::string_view path) {
	if (!DirExists(path)) {
		return false;
	}

	auto globPattern(VfsPath::Concat(path, "*.*"));
	return Search(globPattern).empty();
}

void Vfs::WriteBinaryFile(std::string_view path, gsl::span<uint8_t> data) {
	
	auto fh(Open(path, "wb"));
	if (!fh) {
		throw TempleException("Unable to open file {} for writing", path);
	}
	if (Write(&data[0], data.size(), fh) != data.size()) {
		Close(fh);
		RemoveFile(path);
		throw TempleException("Unable to write file {}", path);
	}
	Close(fh);

}

bool Vfs::CleanDir(std::string_view path) {
	if (!DirExists(path)) {
		return false;
	}

	auto result = true;

	for (const auto &entry : Search(VfsPath::Concat(path, "*.*"))) {
		auto fullPath(VfsPath::Concat(path, entry.filename));

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

bool VfsPath::IsFileSystem(std::string_view path) {

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

std::string VfsPath::Concat(std::string_view a, std::string_view b) {

	std::string result;
	result.reserve(a.length() + b.length() + 1);
	result.append(a);

	if (!result.empty() && result.back() != '\\' && result.back() != '/') {
		result.push_back('\\');
	}

	result.append(b);

	return result;

}
