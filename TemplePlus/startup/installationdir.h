#pragma once

#include <string>
#include <vector>

enum class TempleDllVersion : uint32_t {
	GOG,
	CO8,
	PATCH2,
	UNKNOWN,
	MISSING
};

class InstallationDir {
public:
	InstallationDir();
	explicit InstallationDir(const std::wstring& directory);

	const std::wstring& GetDirectory() const {
		return mDirectory;
	}

	bool IsSupportedDllVersion() const {
		return mTempleDllVersion == TempleDllVersion::GOG
			|| mTempleDllVersion == TempleDllVersion::CO8
			|| mTempleDllVersion == TempleDllVersion::PATCH2;
	}

	bool IsUsable() const {
		return mDataMissing.empty() && mTempleDllVersion != TempleDllVersion::MISSING;
	}

	// Returns a human readable reason for why IsUsable returned false
	std::wstring GetNotUsableReason() const;
	
	std::wstring GetDllPath() const;
	bool IsCo8();
private:

	std::wstring mDirectory;
	std::wstring mDataMissing;
	TempleDllVersion mTempleDllVersion = TempleDllVersion::MISSING;
	bool mCo8Present = false;
	bool mDataPresent = false;
	bool mTfeXDetected = false;

	void Normalize();
	void DetectDllVersion();
	void DetectCo8();
	void DetectMissingData();
	bool RevertTfeXChanges(gsl::array_view<uint8_t> dllData);
	bool RevertTfeXChange(gsl::array_view<uint8_t> data,
	                      uint32_t address,
	                      const std::vector<uint8_t>& original,
	                      const std::vector<uint8_t>& patched);


};
