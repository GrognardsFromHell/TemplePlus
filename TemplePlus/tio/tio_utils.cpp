
#include "stdafx.h"
#include "tio.h"
#include "tio_utils.h"

bool TioDirExists(const string &filename) {
	TioFileListFile f;
	return tio_fileexists(filename.c_str(), &f) && f.attribs & 0x20;
}

vector<uint8_t> *TioReadBinaryFile(const string &filename) {
	TioFileListFile info;

	if (!tio_fileexists(filename.c_str(), &info)) {
		return nullptr;
	}

	auto result = new vector<uint8_t>(info.sizeInBytes);
	
	auto fh = tio_fopen(filename.c_str(), "rb");
	if (tio_fread(result->data(), 1, result->size(), fh) != result->size()) {
		tio_fclose(fh);
		return nullptr;
	}

	tio_fclose(fh);

	return result;
}

bool TioClearDir(const string& path) {
	
	if (!TioDirExists(path)) {
		return false;
	}

	TioFileList list;
	auto globPattern = format("{}\\*.*", path);
	tio_filelist_create(&list, globPattern.c_str());

	bool success = true;

	for (int i = 0; i < list.count; ++i) {
		auto &file = list.files[i];
		auto filepath = format("{}\\{}", path, file.name);
				
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
