#include "stdafx.h"
#include "mod_support.h"
#include "config/config.h"
#include <fstream>
#include <iostream>
#include <temple/dll.h>
#include <QDir>

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
	QDir toeeDir(QString::fromStdWString(config.toeeDir));
	QString tfexIniPath = toeeDir.absoluteFilePath("TFE-X.ini");
	
	if (!QFile::exists(tfexIniPath)) {
		return;
	}

	wfstream tfexIni(tfexIniPath.toStdWString());
	if (!tfexIni.is_open())
		return;
		
	wstring shit;
	while (std::getline(tfexIni, shit) && !mIsCo8NC){
		if (shit.find(L"New Content") != shit.npos) {
			mIsCo8NC = true;
			mIsIWD = false;
		}
		if (shit.find(L"Keep on the Borderlands") != shit.npos)
		{
			mIsIWD = false;
			mIsKotB = true;
			mIsCo8NC = false;
		}
		if (shit.find(L"Icewind Dale") != shit.npos)
		{
			mIsIWD = true;
			mIsKotB = false;
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

bool ModSupport::IsIWD() const
{
	return mIsIWD;
}

bool ModSupport::IsCo8() const
{
	return mIsCo8;
}

void ModSupport::SetNCGameFlag(bool value){
	modSupportAddresses.Set(500, value);
}
