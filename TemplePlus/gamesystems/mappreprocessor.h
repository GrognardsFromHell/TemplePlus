#pragma once

class MapMobilePreprocessor {
public:

	explicit MapMobilePreprocessor(GUID moduleGuid) : mModuleGuid(moduleGuid) {
	}

	void Preprocess(const std::string &mapName) const;

private:
	static char EncodeChar(char ch);
	static std::string EncodeMapCacheFilename(const std::string& filename);
	bool IsValidCacheFile(const std::string &path) const;

	GUID mModuleGuid;
};
