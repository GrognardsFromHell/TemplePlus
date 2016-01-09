#pragma once
#include "common.h"

struct AooShaderPacket
{
	LocAndOffsets loc;
	int shaderId;
	int field14;
};

void HourglassUpdate(int, int, int);