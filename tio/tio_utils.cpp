
#include "stdafx.h"
#include "tio.h"
#include "tio_utils.h"


vector<char> *TioReadBinaryFile(const string &filename) {
	TioFileListFile info;

	if (!tio_fileexists(filename.c_str(), &info)) {
		return nullptr;
	}

	auto result = new vector<char>(info.sizeInBytes);
	
	auto fh = tio_fopen(filename.c_str(), "rb");
	if (tio_fread(result->data(), 1, result->size(), fh) != result->size()) {
		tio_fclose(fh);
		return nullptr;
	}

	tio_fclose(fh);

	return result;
}
