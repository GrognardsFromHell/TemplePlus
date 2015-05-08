#pragma once
#include "stdafx.h"
#include "common.h"


struct HistoryEntry;
struct HistoryArrayEntry;

struct HistorySystem : AddressTable
{
	int32_t(__cdecl * RollHistoryAdd)(HistoryEntry * hist);
	// todo: the annoying AppendHistoryId
	HistorySystem();
};

extern HistorySystem histSys;
