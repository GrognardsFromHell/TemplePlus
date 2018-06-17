#pragma once

#include <infrastructure/vfs.h>
#include <vector>

typedef struct _GUID GUID;

namespace temple {
	
	/*
		Wraps Troika IO (tio.dll) to provide VFS services.		
	*/
	class TioVfs : public Vfs {
	public:
		TioVfs();
		~TioVfs();
		
		// Adds a data item to the vfs (i.e. .dat file, folder)
		bool AddPath(std::string_view path);

		// Removes a previously added path
		bool RemovePath(std::string_view path);

		bool GetArchiveGUID(std::string_view path, GUID &guidOut);

		bool FileExists(std::string_view  filename) override;
		bool DirExists(std::string_view  path) override;

		bool MkDir(std::string_view  path) override;
		
		std::vector<VfsSearchResult> Search(std::string_view globPattern) override;
		
		bool RemoveDir(std::string_view path) override;
		
		bool RemoveFile(std::string_view path) override;
		void Pack(const std::vector<std::string> &args);
		
		FileHandle Open(std::string_view name, std::string_view mode) override;
		size_t Read(void* buffer, size_t size, FileHandle handle) override;
		size_t Write(const void* buffer, size_t size, FileHandle handle) override;
		size_t Length(FileHandle handle) override;
		void Close(FileHandle handle) override;
		size_t Tell(FileHandle handle) override;
	private:

		std::unique_ptr<class TioVfsImpl> mImpl;

	};

}
