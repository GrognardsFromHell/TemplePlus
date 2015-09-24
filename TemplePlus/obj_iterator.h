#pragma once

#include "common.h"

enum ObjIteratorFlags : uint32_t
{
	ObjItFlag_1 = 1,
	ObjItFlag_2 = 2,
	ObjItFlag_4 = 4
};

struct ObjIteratorResult
{
	int flags;
	int field4;
	int field8;
	int fieldC;
	int field10;
	int field14;
	objHndl obj;
	int field20;
	int field24;
	int field28;
	int field2C;
	int field30;
	int field34;
};

struct ObjIterator
{
	ObjIteratorFlags flags;
	int field4;
	LocAndOffsets origin;
	LocAndOffsets targetLoc;
	float radius;
	int field2C;
	objHndl performer;
	objHndl target;
	float someFloat;
	ObjIteratorResult * results;
	int resultCount;
	ObjIterator()
	{
		this->flags = ObjItFlag_1;
		this->resultCount = 0;
		this->results = 0;
		this->origin.location.locx = 0;
		this->origin.location.locy = 0;
		this->origin.off_x = 0;
		this->origin.off_y = 0;
		this->targetLoc.location.locx = 0;
		this->targetLoc.location.locy = 0;
		this->targetLoc.off_x = 0;
		this->targetLoc.off_y = 0;
	}

	int TargettingSthg_100BACE0();
	~ObjIterator();
};