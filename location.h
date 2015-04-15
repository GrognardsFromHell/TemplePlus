#pragma once
#include "stdafx.h"
#include "common.h"

const float tileToOffset = 12.0;

struct LocationSys : AddressTable
{
	float distBtwnLocAndOffs(LocAndOffsets, LocAndOffsets);
	void(__cdecl * getLocAndOff)(objHndl objHnd, LocAndOffsets * locAndOff);
	void(__cdecl * TOEEdistBtwnLocAndOffs)(LocAndOffsets, LocAndOffsets); // outputs to the FPU (st0);is basically  sqrt(dx^2+dy^2)/ 12 where a tile is sqrt(800)xsqrt(800);  I think it's in Inches because some functions divide this result by 12 (inches->feet)
	LocationSys();
};

extern LocationSys locSys;