#include "stdafx.h"

#include <fstream>
#include <infrastructure/logging.h>
#include <infrastructure/crypto.h>
#include <experimental/filesystem>

#include "installationdir.h"
#include "config/config.h"

InstallationDir::InstallationDir() {
}

InstallationDir::InstallationDir(const std::wstring& directory) : mDirectory(directory) {

	Normalize();
	DetectDllVersion();
	DetectCo8();
	DetectMissingData();
}

std::wstring InstallationDir::GetNotUsableReason() const {

	if (mTempleDllVersion == TempleDllVersion::MISSING) {
		return L"A required file (temple.dll) is missing.";
	}

	// So temple.dll seems to be okay... What about data files?
	if (!mDataMissing.empty()) {
		return std::wstring(L"One or more required files ({}) are missing.");
	}

	return std::wstring(); // Everything seems to be okay...

}

wstring InstallationDir::GetDllPath() const {
	return mDirectory + L"temple.dll";
}

void InstallationDir::Normalize() {
	// Ensure backslash
	if (mDirectory.back() != L'/' && mDirectory.back() != L'\\') {
		mDirectory.append(L"\\");
	}
}

bool InstallationDir::RevertTfeXChange(gsl::array_view<uint8_t> data,
                                       uint32_t address,
                                       const std::vector<uint8_t>& original,
                                       const std::vector<uint8_t>& patched) {

	Expects(original.size() == patched.size());

	if (address + original.size() >= data.size()) {
		return false;
	}

	auto begin = data.begin() + address;
	auto end = begin + original.size();
	if (std::mismatch(begin, end, patched.begin()).first == end) {
		// The area matches the given TFE-X patched data, so we replace it with the
		// original data.
		std::copy(original.begin(), original.end(), data.begin() + address);
		return true;
	}
	return false;

}

bool InstallationDir::RevertTfeXChanges(gsl::array_view<uint8_t> dllData) {

	bool changeDetected = false;

	// Max HP per level change
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x000733AC,
	                                   {0xE8, 0xAF, 0x57, 0xFC, 0xFF},
	                                   {0x90, 0x90, 0x90, 0x90, 0x90});

	// Max NPC HP from levels
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0007F7BB,
	                                   {0xE8, 0xA0, 0x93, 0xFB, 0xFF},
	                                   {0x8B, 0xC5, 0x90, 0x90, 0x90});

	// Max NPC HP from HD
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0007F828,
	                                   {0xE8, 0x33, 0x93, 0xFB, 0xFF},
	                                   {0x0F, 0xAF, 0xDD, 0x03, 0xC3});

	// Show Exact HP
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0012441D,
	                                   {0xA9},
	                                   {0x00});

	// Humble NPCs
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x000668D1,
	                                   {0xE8, 0x8A, 0x4A, 0xFC, 0xFF},
	                                   {0xB8, 0x00, 0x00, 0x00, 0x00});
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0006C935,
	                                   {0xE8, 0x26, 0xEA, 0xFB, 0xFF},
	                                   {0xB8, 0x00, 0x00, 0x00, 0x00});
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x000B61BE,
	                                   {0x14},
	                                   {0x00});
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0014DFCB,
	                                   {0xA9},
	                                   {0x0D});
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x0014DFD0,
	                                   {0x75},
	                                   {0xEB});
	changeDetected |= RevertTfeXChange(dllData,
	                                   0x000813F7,
	                                   {0x0D, 0x00, 0x00, 0x00, 0x04},
	                                   {0x90, 0x90, 0x90, 0x90, 0x90});

	// Party size is variable
	for (uint8_t pcs = 3; pcs <= 8; ++pcs) {
		if (pcs == 5)
			continue; // this is the vanilla version

		uint8_t npcs = 8 - pcs;

		changeDetected |= RevertTfeXChange(dllData,
		                                   0x0002BBEF,
		                                   {0x05},
		                                   {pcs});
		changeDetected |= RevertTfeXChange(dllData,
		                                   0x0002BC4F,
		                                   {3},
		                                   {npcs});
		changeDetected |= RevertTfeXChange(dllData,
		                                   0x000B0187,
		                                   {3},
		                                   {npcs});
	}

	// Point buy is variable
	for (uint8_t points :{28, 30, 32, 35}) {
		changeDetected |= RevertTfeXChange(dllData,
		                                   0x0018B534,
		                                   {25},
		                                   {points});
		changeDetected |= RevertTfeXChange(dllData,
		                                   0x0018B576,
		                                   {25},
		                                   {points});
	}

	// Width / Height hack is more problematic
	auto resHackPresent = RevertTfeXChange(dllData,
	                                       0x00147293,
	                                       {0xE8, 0x68, 0x25, 0x0B, 0x00},
	                                       {0xB8, 0x20, 0x03, 0x00, 0x00});
	if (resHackPresent) {
		changeDetected = true;
		RevertTfeXChange(dllData,
		                 0x001472FA,
		                 {0xE8, 0xF1, 0x24, 0x0B, 0x00},
		                 {0xB8, 0x58, 0x02, 0x00, 0x00});

		// Original values are for 800x600
		dllData[0x0000164E] = 0x04;
		dllData[0x0000164F] = 0x00;
		dllData[0x00001656] = 0x03;
		dllData[0x00001657] = 0x00;
	}
	return resHackPresent;
}

void InstallationDir::DetectDllVersion() {

	// Does it even exist?
	auto dllPath(GetDllPath());
	ifstream file(dllPath, ios_base::in | ios_base::binary);

	if (!file) {
		auto errorMsg(strerror(errno));
		logger->info("Couldn't analyze temple.dll in {}: {}", ucs2_to_utf8(mDirectory), errorMsg);
		return;
	}

	// Read the entire file into memory
	file.seekg(0, std::ios_base::end);
	auto fileSize = static_cast<size_t>(file.tellg());

	auto fileData(std::make_unique<uint8_t[]>(fileSize));

	file.seekg(0, std::ios_base::beg);
	file.read(reinterpret_cast<char*>(fileData.get()), fileSize);

	file.close();

	gsl::array_view<uint8_t> fileDataView{fileData.get(), fileSize};

	if (RevertTfeXChanges(fileDataView)) {
		logger->info("TFE-X changes detected");
		mTfeXDetected = true;
		config.usingCo8 = true;
	}

	auto md5Hash = crypto::MD5AsString(fileDataView);
	logger->info("Hash value for {} is: {}", ucs2_to_utf8(dllPath), md5Hash);

	if (md5Hash == "f915db404bd5e765374581e8b05eb691") {
		mTempleDllVersion = TempleDllVersion::CO8;
		config.usingCo8 = true;
	} else if (md5Hash == "67758f8d4841f9590d5ade9369d3c8e1") {
		mTempleDllVersion = TempleDllVersion::GOG;
		config.usingCo8 = false;
	} else if (md5Hash == "f73c049fc2324a527f589533a0dfb8e0") {
		mTempleDllVersion = TempleDllVersion::PATCH2;
		config.usingCo8 = false;
	} else {
		if (fileSize > 4000 * 1000) // cheap but works :P
		{
			mTempleDllVersion = TempleDllVersion::CO8;
		} 
		else
		{
			mTempleDllVersion = TempleDllVersion::UNKNOWN;
		}
	}

}

/*
	Detects the presence of a Co8 installation based on the Co8
	configuration file.
*/
void InstallationDir::DetectCo8() {
	mCo8Present = std::experimental::filesystem::exists(mDirectory);
}

void InstallationDir::DetectMissingData() {

	// Check for existing data files that we know must be there
	auto requiredFiles = {L"ToEE1.dat", L"ToEE2.dat", L"ToEE3.dat", L"ToEE4.dat", L"tig.dat",
		L"Modules\\ToEE.dat"};

	std::wstring missingInfo;
	for (auto requiredFile : requiredFiles) {
		auto path = mDirectory + requiredFile;
		if (!std::experimental::filesystem::exists(path)) {
			missingInfo.append(requiredFile);
		}
	}

}
