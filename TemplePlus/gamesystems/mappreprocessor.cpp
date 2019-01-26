#include "stdafx.h"

#include <infrastructure/vfs.h>
#include "mappreprocessor.h"

void MapMobilePreprocessor::Preprocess(const std::string& mapName) const {

	auto cachePath = VfsPath::Concat("maps\\", EncodeMapCacheFilename(mapName));

	if (IsValidCacheFile(cachePath)) {
		return;
	}

	// TODO: Finish the implementation of this

}

/// Encodes a filename in a way that makes it harder to read for players
/// This function is one-way
std::string MapMobilePreprocessor::EncodeMapCacheFilename(const std::string& filename) {
	auto result = filename;
	std::transform(result.begin(), result.end(), result.begin(), EncodeChar);
	std::reverse(result.begin(), result.end());
	return result;
}

/// Check if an existing cache file was created for the
/// same module as the currently loaded one by comparing
/// the GUID stored at the beginning of the file
bool MapMobilePreprocessor::IsValidCacheFile(const std::string& path) const {
	GUID fileGuid;
	auto fh = vfs->Open(path.c_str(), "rb");
	if (!fh) {
		return false;
	}

	if (vfs->Read(&fileGuid, sizeof(GUID), fh) != sizeof(GUID)) {
		vfs->Close(fh);
		return false;
	}

	vfs->Close(fh);
	return fileGuid == mModuleGuid;
}

char MapMobilePreprocessor::EncodeChar(char ch) {
	char lower, upper;

	if (ch >= 'A' && ch <= 'Z') {
		lower = 'A';
		upper = 'Z';
	} else if (ch >= 'a' && ch <= 'z') {
		lower = 'a';
		upper = 'z';
	} else {
		return ch;
	}

	auto newCh = ch + 13;
	if (newCh > upper)
		newCh = newCh - upper + lower + 1;
	return newCh;
}
