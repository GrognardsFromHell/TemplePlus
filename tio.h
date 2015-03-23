
#pragma once

enum TioFileFlag
{
	TIO_FILE_NATIVE = 0x2,
};

struct TioFile {
	const char* filename;
	uint32_t flags;
	FILE fileHandle;
	uint32_t field_c;
	uint32_t field_10;
	uint32_t field_14;
	uint32_t field_18;
	uint32_t field_1c;
	uint32_t field_20;
	uint32_t field_24;
	uint32_t field_28;
	uint32_t field_2c;
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
	TioFileListFile *files;
};
#pragma pack(push, 1)
struct TioFileListFile {
	char name[260];
	uint32_t attribs; // See TioFileAttribs
	uint32_t sizeInBytes;
	time_t lastModified;
};
#pragma pack(pop)

extern "C" {

	#define TIOAPI __declspec(dllimport) __cdecl
		
	/*
		Performs a search (including the current search path) for the given file pattern, such as
		*.dat or similar and writes the result to list.
	*/
	void TIOAPI tio_filelist_create(TioFileList *list, const char *globPattern);

	/*
		Destroys a result list previously created with tio_filelist_create.
	*/
	void TIOAPI tio_filelist_destroy(TioFileList *list);

	/*
		Adds a file (either a directory or a .dat file) to the TIO search path.
	*/
	int TIOAPI tio_path_add(const char *path);

	/*
		Removes a previously added path.
	*/
	bool TIOAPI tio_path_remove(const char *path);

	/*
		Creates a new directory.
	*/
	void TIOAPI tio_mkdir(const char *path);

	// The following functions have the wrong signature
	void TIOAPI tio_fgetpos();
	void TIOAPI tio_fclose();
	void TIOAPI tio_fread();
	void TIOAPI tio_fopen();
	void TIOAPI tio_remove();
	void TIOAPI tio_fwrite();
	void TIOAPI tio_fstat();
	void TIOAPI tio_fileexists();
	void TIOAPI tio_rename();
	void TIOAPI tio_path_guid();
	void TIOAPI tio_fseek();	
	void TIOAPI tio_file_extract();
	void TIOAPI tio_fgets();
	void TIOAPI tio_ftell();
	void TIOAPI tio_fsetpos();
	void TIOAPI tio_filelength();
	void TIOAPI tio_feof();
	void TIOAPI tio_fprintf();
	void TIOAPI tio_fputc();
	void TIOAPI tio_fputs();
	void TIOAPI tio_ungetc();
	void TIOAPI tio_file_from_funcs();
	void TIOAPI tio_rewind();
	void TIOAPI tio_vfprintf();
	void TIOAPI tio_tmpfile();
	void TIOAPI tio_setvbuf();
	void TIOAPI tio_isatty();
	void TIOAPI tio_fgetc();
	void TIOAPI tio_fopen_exclusive();
	void TIOAPI tio_file_from_stdio();
	void TIOAPI tio_fflush();
	void TIOAPI tio_ferror();
	void TIOAPI tio_clearerr();
	void TIOAPI tio_rmdir();

}
