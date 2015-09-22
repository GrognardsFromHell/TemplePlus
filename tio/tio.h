#pragma once

enum TioFileFlag {
	TIO_FILE_NATIVE = 0x2,
};

#pragma pack(push, 1)
struct TioFileFuncs {
	int(__cdecl *flush)(void* handle) = nullptr;
	int(__cdecl *ungetc)(int ch, void* handle) = nullptr;
	size_t(__cdecl *read)(void* ptr, size_t size, size_t count, void* handle) = nullptr;
	size_t(__cdecl *write)(const void* ptr, size_t size, size_t count, void* handle) = nullptr;
	int(__cdecl *seek)(int offset, int origin, void* handle) = nullptr;
	int(__cdecl *tell)(void* handle) = nullptr;
	int(__cdecl *eof)(void* handle) = nullptr;
	int(__cdecl *stat)(struct stat*, void* handle) = nullptr;
	int(__cdecl *close)(void* handle) = nullptr;
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

extern "C" {

#define TIOAPI __cdecl
#define TIOLIB __declspec(dllimport)

	/*
	Performs a search (including the current search path) for the given file pattern, such as
	*.dat or similar and writes the result to list.
	*/
	TIOLIB void TIOAPI tio_filelist_create(TioFileList* list, const char* globPattern);

	/*
	Destroys a result list previously created with tio_filelist_create.
	*/
	TIOLIB void TIOAPI tio_filelist_destroy(TioFileList* list);

	/*
	Adds a file (either a directory or a .dat file) to the TIO search path.
	*/
	TIOLIB int TIOAPI tio_path_add(const char* path);

	/*
	Removes a previously added path.
	*/
	TIOLIB bool TIOAPI tio_path_remove(const char* path);

	/*
	Creates a new directory.
	*/
	TIOLIB void TIOAPI tio_mkdir(const char* path);

	/*
	Reads data from an opened file. Largely equivalent to fread.
	*/
	TIOLIB int TIOAPI tio_fread(void* buffer, size_t size, size_t count, TioFile* file);

	// The following functions have the wrong signature
	TIOLIB void TIOAPI tio_fgetpos(TioFile *file, uint64_t *filePos);
	TIOLIB int TIOAPI tio_fclose(TioFile *file);
	TIOLIB TioFile* TIOAPI tio_fopen(const char *file, const char *mode);
	TIOLIB int TIOAPI tio_remove(const char *file);
	TIOLIB int TIOAPI tio_fwrite(const void *buffer, int size, int count, TioFile *file);
	TIOLIB int TIOAPI tio_fstat(TioFile *file, TioFileListFile* pFileInfo);
	TIOLIB bool TIOAPI tio_fileexists(const char *path, TioFileListFile *pInfoOut = nullptr);
	TIOLIB void TIOAPI tio_rename(const char *from, const char *to);
	TIOLIB void TIOAPI tio_path_guid();
	TIOLIB void TIOAPI tio_fseek();
	TIOLIB void TIOAPI tio_file_extract();
	TIOLIB void TIOAPI tio_fgets(char *buffer, int size, TioFile *file);
	TIOLIB void TIOAPI tio_ftell();
	TIOLIB void TIOAPI tio_fsetpos();
	TIOLIB int TIOAPI tio_filelength(TioFile *file);
	TIOLIB void TIOAPI tio_feof();
	TIOLIB void TIOAPI tio_fprintf();
	TIOLIB void TIOAPI tio_fputc();
	TIOLIB void TIOAPI tio_fputs();
	TIOLIB void TIOAPI tio_ungetc();
	TIOLIB TioFile* TIOAPI tio_file_from_funcs(const TioFileFuncs* funcs, void* handle); // flags to 0x8
	TIOLIB void TIOAPI tio_rewind();
	TIOLIB void TIOAPI tio_vfprintf();
	TIOLIB void TIOAPI tio_tmpfile();
	TIOLIB void TIOAPI tio_setvbuf();
	TIOLIB void TIOAPI tio_isatty();
	TIOLIB void TIOAPI tio_fgetc();
	TIOLIB void TIOAPI tio_fopen_exclusive();
	TIOLIB TioFile* TIOAPI tio_file_from_stdio(FILE* fh); // flags to 0x12
	TIOLIB void TIOAPI tio_fflush();
	TIOLIB void TIOAPI tio_ferror();
	TIOLIB void TIOAPI tio_clearerr();
	TIOLIB int TIOAPI tio_rmdir(const char *path);


}
