
#include "stdafx.h"
#include "objlist.h"

static struct ObjListAddresses : AddressTable {
	void(__cdecl *ObjListTile)(locXY loc, int flags, ObjListResult &result);
	void(__cdecl *ObjListVicinity)(locXY loc, int flags, ObjListResult &result);
	void(__cdecl *ObjListRadius)(LocAndOffsets loc, float radius, float unk1, float unk2, int flags, ObjListResult &result);
	void(__cdecl *ObjListFollowers)(objHndl critter, ObjListResult &result);
	void(__cdecl *ObjListFree)(ObjListResult &result);

	ObjListAddresses() {
		rebase(ObjListTile, 0x1010D4D0);
		rebase(ObjListVicinity, 0x1001F1C0);
		rebase(ObjListRadius, 0x1010D650);
		rebase(ObjListFollowers, 0x1001F450);
		rebase(ObjListFree, 0x1001F2C0);
	}
} addresses;

ObjList::ObjList() {
	FreeResult();
}

ObjList::~ObjList() {
	FreeResult();
}

void ObjList::ListTile(locXY loc, int flags) {
	FreeResult();
	addresses.ObjListTile(loc, flags, mResult);
}

void ObjList::ListVicinity(locXY loc, int flags) {
	FreeResult();
	addresses.ObjListVicinity(loc, flags, mResult);
}

void ObjList::ListRadius(LocAndOffsets loc, float radius, int flags) {
	FreeResult();
	addresses.ObjListRadius(loc, radius, 0.0f, (float)(M_PI * 2), flags, mResult);
}

void ObjList::ListCone(LocAndOffsets loc, float radius, float coneStartAngleRad, float coneArcRad, int flags) {
	FreeResult();
	addresses.ObjListRadius(loc, radius, coneStartAngleRad, coneArcRad, flags, mResult);
}

void ObjList::ListFollowers(objHndl critter) {
	FreeResult();
	addresses.ObjListFollowers(critter, mResult);
}

int ObjList::size() {
	if (mSizeValid) {
		return mSize;
	}
	mSize = CountObjects();
	mSizeValid = true;
	return mSize;
}

int ObjList::CountObjects() const {
	if (!mResult.objects) {
		return 0;
	}
	auto ptr = mResult.objects;
	auto result = 0;
	while (*ptr) {
		result++;
		ptr++;
	}
	return result;
}

void ObjList::FreeResult() {
	if (mHasToFree) {
		addresses.ObjListFree(mResult);
	}
	mSizeValid = false;
	mSize = 0;
	mHasToFree = false;
}
