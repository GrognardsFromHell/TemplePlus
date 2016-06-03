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
		}
		if (shit.find(L"Keep on the Borderlands") != shit.npos)
		{
			mIsKotB = true;
			mIsCo8NC = false;
		}
	}
} 

bool ModSupport::IsCo8NCEdition() const
{
	return mIsCo8NC;
}

bool ModSupport::IsKotB() const
{
	return mIsKotB;
}

void ModSupport::SetNCGameFlag(bool value){
	modSupportAddresses.Set(500, value);
}
