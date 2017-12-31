#include "stdafx.h"
#include "mod_support.h"
#include "config/config.h"
#include <fstream>
#include <iostream>
#include <temple/dll.h>

#include "gamesystems/gamesystems.h"
#include "gamesystems/scripting.h"

ModSupport modSupport;

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

void ModSupport::SetNCGameFlag(bool value) {
	gameSystems->GetScript().SetGlobalFlag(500, value);
}
