
#include "stdafx.h"
#include "tio.h"
#include "tio_utils.h"

bool TioDirExists(const std::string &filename) {
	TioFileListFile f;
	return tio_fileexists(filename.c_str(), &f) && f.attribs & 0x20;
}

bool TioClearDir(const std::string& path) {
	
	if (!TioDirExists(path)) {
		return false;
	}

	TioFileList list;
	auto globPattern = fmt::format("{}\\*.*", path);
	tio_filelist_create(&list, globPattern.c_str());

	bool success = true;

	for (int i = 0; i < list.count; ++i) {
		auto &file = list.files[i];
		auto filepath = fmt::format("{}\\{}", path, file.name);
				
		if (!(file.attribs & 0x20)) {
			if (tio_remove(filepath.c_str())) {
				success = false;
			}
		} else if (strcmp(file.name, ".") && strcmp(file.name, "..")) {
			// Recursively delete directories other than . and ..
			if (!TioClearDir(filepath)) {
				success = false;
			}

			if (tio_rmdir(filepath.c_str())) {
				success = false;
			}
		}
	}

	tio_filelist_destroy(&list);
	return success;

}
