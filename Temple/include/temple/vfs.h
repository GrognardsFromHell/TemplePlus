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
		void AddPath(const std::string &path);
		// void Pack(const std::vector<std::string> &args);
		
	protected:
		FileHandle Open(const char* name, const char* mode) override;
		size_t Read(void* buffer, size_t size, FileHandle handle) override;
		size_t Length(FileHandle handle) override;
		void Close(FileHandle handle) override;
	private:

		std::unique_ptr<class TioVfsImpl> mImpl;

	};

}
