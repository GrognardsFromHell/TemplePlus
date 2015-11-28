#pragma once

#include <infrastructure/vfs.h>
#include <vector>

namespace temple {
	
	/*
		Wraps Troika IO (tio.dll) to provide VFS services.		
	*/
	class TioVfs : public Vfs {
	public:
		TioVfs();
		~TioVfs();
		
		// Adds a data item to the vfs (i.e. .dat file, folder)
		bool AddPath(const std::string &path);

		// Removes a previously added path
		bool RemovePath(const std::string &path);

		bool GetArchiveGUID(const std::string &path, GUID &guidOut);

		bool FileExists(const std::string& filename) override;
		bool DirExists(const std::string& path) override;

		bool MkDir(const std::string& path) override;
		
		std::vector<VfsSearchResult> Search(const std::string &globPattern) override;
		
		bool RemoveDir(const std::string &path) override;
		
		bool RemoveFile(const std::string &path) override;
		void Pack(const std::vector<std::string> &args);

	protected:
		FileHandle Open(const char* name, const char* mode) override;
		size_t Read(void* buffer, size_t size, FileHandle handle) override;
		size_t Write(void* buffer, size_t size, FileHandle handle) override;
		size_t Length(FileHandle handle) override;
		void Close(FileHandle handle) override;

	private:

		std::unique_ptr<class TioVfsImpl> mImpl;

	};

}
