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
};
#pragma pack(pop)

namespace temple {

	class TioVfsImpl {
	public:

		// Function typedefs for TIO
		TioFile* (*OpenFile)(const char* file, const char* mode);

		// Reads data from an opened file. Largely equivalent to fread.
		int (*Read)(void* buffer, size_t size, size_t count, TioFile* file);

		// Close an opened file
		void (*CloseFile)(TioFile*);

		// Return the size of the given file in bytes
		int (*FileLength)(TioFile* file);

		// Adds a file (either a directory or a .dat file) to the TIO search path.
		int (*AddPath)(const char* path);

		// Setter of function pointers for error and info output messages ( printf("[ERROR] %s", msg); )
		void(__cdecl* TioPackFuncs)(TioMsgCb erroCb, TioMsgCb msgCb);

		// Pack file(s)
		void(__cdecl* TioPack)(int argc, char* argv[]);

		BOOL(*FileExists)(const char* file, int unk);

		TioVfsImpl() {
			Resolve("tio_fopen", OpenFile);
			Resolve("tio_fread", Read);
			Resolve("tio_fclose", CloseFile);
			Resolve("tio_filelength", FileLength);
			Resolve("tio_path_add", AddPath);
			Resolve("tio_pack_funcs", TioPackFuncs);
			Resolve("tio_pack", TioPack);
			Resolve("tio_fileexists", FileExists);
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
		int (*remove)(const char* file);
		int (*fwrite)(const void* buffer, int size, int count, TioFile* file);
		int (*fstat)(TioFile* file, TioFileListFile* pFileInfo);
		bool (*fileexists)(const char* path, TioFileListFile* pInfoOut);
		void (*rename)(const char* from, const char* to);
		void (*path_guid)();
		void (*fseek)();
		void (*file_extract)();
		void (*fgets)(char* buffer, int size, TioFile* file);
		void (*ftell)();
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

	void TioVfs::AddPath(const std::string& path) {
		mImpl->AddPath(path.c_str());
	}

	void TioVfs::Pack(const std::vector<std::string>& args)
	{
		char * argsC[100];
		for (int i = 0; i < args.size(); i++)
		{
			argsC[i] = (char*)args[i].c_str();
		}
		mImpl->TioPack(args.size(), argsC);
	}

	Vfs::FileHandle TioVfs::Open(const char* name, const char* mode) {
		return mImpl->OpenFile(name, mode);
	}

	size_t TioVfs::Read(void* buffer, size_t size, FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->Read(buffer, 1, size, tioFile);
	}

	size_t TioVfs::Length(FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->FileLength(tioFile);
	}

	void TioVfs::Close(FileHandle handle) {
		auto tioFile = static_cast<TioFile*>(handle);
		return mImpl->CloseFile(tioFile);
	}

}
