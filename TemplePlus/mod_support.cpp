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

	wchar_t tfexIniPath[MAX_PATH];
	wcscpy_s(tfexIniPath, config.toeeDir.c_str());
	PathAppend(tfexIniPath, L"TFE-X.ini");
	
	if (!PathFileExists(tfexIniPath)) {
		return;
	}

	wfstream tfexIni(tfexIniPath);
	if (!tfexIni.is_open())
		return;
		
	wstring shit;
	while (std::getline(tfexIni, shit) && !mIsCo8NC){
		if (shit.find(L"New Content") != shit.npos) {
			mIsCo8NC = true;
			mIsIWD = false;
			mIsPalCove = false;
		}
		if (shit.find(L"Keep on the Borderlands") != shit.npos)
		{
			mIsIWD = false;
			mIsKotB = true;
			mIsCo8NC = false;
			mIsPalCove = false;
		}
		if (shit.find(L"Icewind Dale") != shit.npos)
		{
			mIsIWD = true;
			mIsKotB = false;
			mIsCo8NC = false;
			mIsPalCove = false;
		}
		if (shit.find(L"Paladin") != shit.npos)
		{
			mIsIWD = false;
			mIsKotB = false;
			mIsCo8NC = false;
			mIsPalCove = true;
		}
	}
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
