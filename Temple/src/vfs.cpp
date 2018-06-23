#include <infrastructure/exception.h>
#include <platform/windows.h>

#include "temple/vfs.h"
#include <vector>

enum TioFileFlag {
	TIO_FILE_NATIVE = 0x2,
};

typedef void(__cdecl* TioMsgCb)(const char* msg);

#pragma pack(push, 1)
struct TioFileFuncs {
	int (__cdecl *flush)(void* handle) = nullptr;
	int (__cdecl *ungetc)(int ch, void* handle) = nullptr;
	size_t (__cdecl *read)(void* ptr, size_t size, size_t count, void* handle) = nullptr;
	size_t (__cdecl *write)(const void* ptr, size_t size, size_t count, void* handle) = nullptr;
	int (__cdecl *seek)(int offset, int origin, void* handle) = nullptr;
	int (__cdecl *tell)(void* handle) = nullptr;
	int (__cdecl *eof)(void* handle) = nullptr;
	int (__cdecl *stat)(struct stat*, void* handle) = nullptr;
	int (__cdecl *close)(void* handle) = nullptr;
};

struct TioFile {
	const char* filename;
	uint32_t flags;
	FILE fileHandle;
	int error;
	TioFileFuncs fileFuncs;
};

enum TioFileAttribs {
	TFA_READONLY = 0x2,
	TFA_HIDDEN = 0x4,
	TFA_SYSTEM = 0x10,
	TFA_SUBDIR = 0x20,
	TFA_ARCHIVE = 0x40
};

struct TioFileListFile;

struct TioFileList {
	int count;
	// This is a pointer to a contiguous array of count TioFileListFile entries.
	TioFileListFile* files;
};

struct TioFileListFile {
	char name[260];
	uint32_t attribs = 0; // See TioFileAttribs
	uint32_t sizeInBytes = 0;
	uint32_t lastModified = 0;

	TioFileListFile() {
		name[0] = '\0';
	}

	bool IsFile() const {
		return (attribs & TFA_SUBDIR) == 0;
	}

	bool IsDir() const {
		return (attribs & TFA_SUBDIR) != 0;
	}
};
#pragma pack(pop)

namespace temple {

	class TioVfsImpl {
	public:

		// Function typedefs for TIO
		TioFile* (*OpenFile)(const char* file, const char* mode);

		// Reads data from an opened file. Largely equivalent to fread.
		int(*Read)(void* buffer, size_t size, size_t count, TioFile* file);

		// Write data to an opened file. Largely equivalent to fwrite.
		int(*Write)(const void* buffer, size_t size, size_t count, TioFile* file);

		// Close an opened file
		void (*CloseFile)(TioFile*);

		// Return the size of the given file in bytes
		int (*FileLength)(TioFile* file);

		// Adds a file (either a directory or a .dat file) to the TIO search path.
		int (*AddPath)(const char* path);

		// Removes a previously added path
		int (*RemovePath)(const char* path);

		// Checks if a file exists
		int (*FileExists)(const char* path, TioFileListFile *info);

		// Creates a directory
		int (*MkDir)(const char* path);

		// Retrieves the GUID of a DAT file
		int(*PathGuid)(const char *path, GUID *guidOut);

		// Deletes a file
		int (*RemoveFile)(const char* file);
		
		// Removes an empty directory
		int (*RemoveDir)(const char* file);

		// Setter of function pointers for error and info output messages ( printf("[ERROR] %s", msg); )
		// void(__cdecl* TioPackFuncs)(TioMsgCb erroCb, TioMsgCb msgCb);

		//// Pack file(s)
		//void(__cdecl* TioPack)(int argc, char* argv[]);

		// Returns current pos of file handle
		int(*Tell)(TioFile *file);

		int(*Seek)(TioFile*file, int offset, int origin);

		TioVfsImpl() {
			Resolve("tio_path_add", AddPath);
			Resolve("tio_path_remove", RemovePath);
			Resolve("tio_path_guid", PathGuid);
			Resolve("tio_fopen", OpenFile);
			Resolve("tio_fread", Read);
			Resolve("tio_fwrite", Write);
			Resolve("tio_fclose", CloseFile);
			Resolve("tio_filelength", FileLength);
			Resolve("tio_fileexists", FileExists);
			Resolve("tio_mkdir", MkDir);			
			Resolve("tio_filelist_create", filelist_create);
			Resolve("tio_filelist_destroy", filelist_destroy);
			Resolve("tio_remove", RemoveFile);
			Resolve("tio_rmdir", RemoveDir);
			// Resolve("tio_pack_funcs", TioPackFuncs);
			// Resolve("tio_pack", TioPack);
			Resolve("tio_ftell", Tell);
			Resolve("tio_fseek", Seek);
		}

		/*
		Performs a search (including the current search path) for the given file pattern, such as
		*.dat or similar and writes the result to list.
		*/
		void (*filelist_create)(TioFileList* list, const char* globPattern);

		/*
		Destroys a result list previously created with tio_filelist_create.
		*/
		void (*filelist_destroy)(TioFileList* list);

		/*
		Removes a previously added path.
		*/
		bool (*path_remove)(const char* path);

		/*
		Creates a new directory.
		*/
		void (*mkdir)(const char* path);

		// The following functions have the wrong signature
		void (*fgetpos)(TioFile* file, uint64_t* filePos);

		int (*fstat)(TioFile* file, TioFileListFile* pFileInfo);
		bool (*fileexists)(const char* path, TioFileListFile* pInfoOut);
		void (*rename)(const char* from, const char* to);
		void (*path_guid)();
		void (*fseek)(TioFile *file, int offset, int origin);
		void (*file_extract)();
		void (*fgets)(char* buffer, int size, TioFile* file);
		void (*fsetpos)();
		void (*feof)();
		void (*fprintf)();
		void (*fputc)();
		void (*fputs)();
		void (*ungetc)();
		TioFile* (*file_from_funcs)(const TioFileFuncs* funcs, void* handle); // flags to 0x8
		void (*rewind)();
		void (*vfprintf)();
		void (*tmpfile)();
		void (*setvbuf)();
		void (*isatty)();
		void (*fgetc)();
		void (*fopen_exclusive)();
		TioFile* (*file_from_stdio)(FILE* fh); // flags to 0x12
		void (*fflush)();
		void (*ferror)();
		void (*clearerr)();
		int (*rmdir)(const char* path);

	private:
		template <typename T>
		void Resolve(const char* entryPoint, T* & funcPtr) {
			auto tioHandle = GetModuleHandle(L"tio");
			if (!tioHandle) {
				throw TempleException("tio.dll is not loaded");
			}

			funcPtr = (T*)GetProcAddress(tioHandle, entryPoint);

			if (!funcPtr) {
				throw TempleException("Entry point {} was not found in tio.dll",
				                      entryPoint);
			}
		}

	};

	TioVfs::TioVfs() : mImpl(std::make_unique<TioVfsImpl>()) {
	}

	TioVfs::~TioVfs() {
	}

	bool TioVfs::AddPath(std::string_view  path) {
		return mImpl->AddPath(path.data()) == 0;
	}

	bool TioVfs::RemovePath(std::string_view  path) {
		return mImpl->RemovePath(path.data()) == 0;
	}

	bool TioVfs::GetArchiveGUID(std::string_view  path, GUID& guidOut) {
		return mImpl->PathGuid(path.data(), &guidOut) == 0;
	}

	bool TioVfs::FileExists(std::string_view  path) {
		TioFileListFile info;
		return mImpl->FileExists(path.data(), &info) != 0
			&& info.IsFile();
	}

	bool TioVfs::DirExists(std::string_view  path) {
		TioFileListFile info;
		return mImpl->FileExists(path.data(), &info) != 0
			&& info.IsDir();
	}

	bool TioVfs::MkDir(std::string_view  path) {
		return mImpl->MkDir(path.data()) == 0;
	}

	std::vector<VfsSearchResult> TioVfs::Search(std::string_view  globPattern) {

		TioFileList list;
		mImpl->filelist_create(&list, globPattern.data());

		VfsSearchResult result;
		std::vector<VfsSearchResult> results;
		results.reserve(list.count);
		for (auto i = 0; i < list.count; ++i) {
			const auto &file = list.files[i];
			result.sizeInBytes = file.sizeInBytes;
			result.dir = (file.attribs & TFA_SUBDIR) != 0;
			result.lastModified = file.lastModified;
			result.filename = file.name;
			if (result.filename == "." || result.filename == "..") {
				continue;
			}
			results.emplace_back(result);
		}
	
		mImpl->filelist_destroy(&list);

		return results;

	}

	bool TioVfs::RemoveDir(std::string_view  path) {
		return (mImpl->RemoveDir(path.data()) == 0);
	}

	bool TioVfs::RemoveFile(std::string_view  path) {
		return (mImpl->RemoveFile(path.data()) == 0);
	}

	/*void TioVfs::Pack(const std::vector<std::string>& args)
	{
		char * argsC[100];
		for (size_t i = 0; i < args.size(); i++)
		{
			argsC[i] = (char*)args[i].c_str();
		}
		mImpl->TioPack(args.size(), argsC);
	}*/

	Vfs::FileHandle TioVfs::Open(std::string_view name, std::string_view mode) {
		return mImpl->OpenFile(name.data(), mode.data());
	}

	size_t TioVfs::Read(void* buffer, size_t size, FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->Read(buffer, 1, size, tioFile);
	}

	size_t TioVfs::Write(const void* buffer, size_t size, FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->Write(buffer, 1, size, tioFile);
	}

	size_t TioVfs::Length(FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->FileLength(tioFile);
	}

	void TioVfs::Close(FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->CloseFile(tioFile);
	}

	size_t TioVfs::Tell(FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->Tell(tioFile);
	}

	void TioVfs::Seek(FileHandle handle, int offset, SeekDir dir) {
		auto tioFile = static_cast<TioFile*>(handle);
		mImpl->Seek(tioFile, offset, (int) dir);
	}
}
