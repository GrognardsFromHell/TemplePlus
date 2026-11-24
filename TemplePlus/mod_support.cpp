#include "stdafx.h"
#include "mod_support.h"
#include "config/config.h"
#include <fstream>
#include <iostream>
#include <temple/dll.h>

ModSupport modSupport;


static struct GlobalFlagAddresses : temple::AddressTable {
	int **globalFlags;

	bool Get(int flagIdx) {
		int idx = flagIdx / 32;
		uint32_t bit = flagIdx % 32;
		int flagWord = (*globalFlags)[idx];
		return (flagWord & (1 << bit)) != 0;
	}
	void Set(int flagIdx, bool value) {
		int idx = flagIdx / 32;
		int bit = flagIdx % 32;
		int &flagWord = (*globalFlags)[idx];
		uint32_t mask = (1 << bit);
		if (value) {
			flagWord |= mask;
		}
		else {
			flagWord &= ~mask;
		}
	}

	GlobalFlagAddresses() {
		rebase(globalFlags, 0x103073A8);
	}
} modSupportAddresses;


void ModSupport::DetectCo8ActiveModule(){

	mIsCo8NC = mIsKotB = mIsIWD = mIsPalCove = false;
	mHasTfexIni = false;
	mActiveModuleName.clear();

	wchar_t tfexIniPath[MAX_PATH];
	wcscpy_s(tfexIniPath, config.toeeDir.c_str());
	PathAppend(tfexIniPath, L"TFE-X.ini");
	
	if (!PathFileExists(tfexIniPath)) {
		logger->info("ModSupport: no TFE-X.ini found; assuming a standalone Co8 install.");
		return;
	}

	std::ifstream tfexIni(tfexIniPath);
	if (!tfexIni.is_open())
		return;
	mHasTfexIni = true;

	// TFE-X writes the name of the currently active module on the first line.
	// Only that line determines the active module; don't scan the rest of the file.
	std::string line;
	while (std::getline(tfexIni, line)) {
		while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
			line.pop_back();
		auto start = line.find_first_not_of(" \t\xEF\xBB\xBF");
		if (start == std::string::npos)
			continue;
		mActiveModuleName = line.substr(start);
		break;
	}

	auto &name = mActiveModuleName;
	if (name.find("New Content") != std::string::npos) {
		mIsCo8NC = true;
	}
	else if (name.find("Keep on the Borderlands") != std::string::npos) {
		mIsKotB = true;
	}
	else if (name.find("Icewind Dale") != std::string::npos) {
		mIsIWD = true;
	}
	else if (name.find("Paladin") != std::string::npos) {
		mIsPalCove = true;
	}
	logger->info("ModSupport: TFE-X active module: \"{}\" (Co8 NC: {}, KotB: {}, IWD: {}, Paladin's Cove: {})",
		name, mIsCo8NC, mIsKotB, mIsIWD, mIsPalCove);
} 

bool ModSupport::HasTfexIni() const
{
	return mHasTfexIni;
}

const std::string& ModSupport::GetActiveModuleName() const
{
	return mActiveModuleName;
}

void ModSupport::DetectZMOD() {
	mIsZMOD = false;
	if (!config.defaultModule.empty() && (tolower(config.defaultModule).find("zmod") != std::string::npos))
	{
		mIsZMOD = true;
	}
}

bool ModSupport::IsCo8NCEdition() const
{
	if (!mInited)
		throw TempleException("ModSupport uninited");
	return mIsCo8NC;
}

bool ModSupport::IsKotB() const
{
	if (!mInited)
		throw TempleException("ModSupport uninited");
	return mIsKotB;
}

bool ModSupport::IsPalCove() const
{
	if (!mInited)
		throw TempleException("ModSupport uninited");
	return mIsPalCove;
}

bool ModSupport::IsIWD() const
{
	if (!mInited)
		throw TempleException("ModSupport uninited");
	return mIsIWD;
}

bool ModSupport::IsCo8() const
{
	if (!mInited) {
		return temple::Dll::GetInstance().HasCo8Hooks();
	}
		
	return mIsCo8;
}

bool ModSupport::IsZMOD() const
{
	if (!mInited)
		throw TempleException("ModSupport uninited");
	return mIsZMOD;
}

void ModSupport::SetIsZMOD(bool value)
{
	mIsZMOD = value;
}

void ModSupport::SetNCGameFlag(bool value){
	modSupportAddresses.Set(500, value);
}

const std::vector<std::string>& ModSupport::GetOverrides()
{
	return mOverridesLoaded;
}

void ModSupport::AddOverride(const std::string& overrideName)
{
	mOverridesLoaded.push_back(overrideName);
}
