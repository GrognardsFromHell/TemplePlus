
#include "stdafx.h"
#include "objlist.h"

#include <temple/dll.h>
#include "gamesystems/objects/objsystem.h"
#include "raycast.h"

static struct ObjListAddresses : temple::AddressTable {
	void(__cdecl *ObjListTile)(locXY loc, int flags, ObjListResult &result);
	void(__cdecl *ObjListRect)(TileRect &trect, ObjectListFilter olcCritters, ObjListResult& result);
	void(__cdecl *ObjListVicinity)(locXY loc, int flags, ObjListResult &result);
	void(__cdecl *ObjListRadius)(LocAndOffsets loc, float radius, float angleMin, float angleMax, int flags, ObjListResult &result);
	void(__cdecl *ObjListFollowers)(objHndl critter, ObjListResult &result);
	int(__cdecl *ObjListFree)(ObjListResult &result);
	void(__cdecl *ReturnToPool)(ObjListResultItem* objListResultItem);
	ObjListResultItem *(__cdecl*ObjlistPop)();
	
	ObjListAddresses() {
		rebase(ObjListTile, 0x1001E970);
		rebase(ObjListRect, 0x1001ECF0);
		rebase(ObjListVicinity, 0x1001F1C0);
		rebase(ObjListRadius, 0x10022E50);
		rebase(ObjListFollowers, 0x1001F450);
		rebase(ObjListFree, 0x1001F2C0);
		rebase(ObjlistPop, 0x100C0CA0);
		rebase(ReturnToPool, 0x100C0C20);
	}

	
} addresses;

void ObjListResult::Init(){
	this->objects = nullptr;
	this->numSectorObjects = 0;
	IncreaseObjListCount();
}

int ObjListResult::Free(){
	return addresses.ObjListFree(*this);
}

void ObjListResult::PrependHandle(objHndl handle){
	auto objNodeNew = addresses.ObjlistPop();
	objNodeNew->handle = handle;
	objNodeNew->next = this->objects;
	this->objects = objNodeNew;
}

void ObjListResultItem::ReturnToPool() {
	addresses.ReturnToPool(this);
}


void ObjListResult::IncreaseObjListCount(){
	temple::GetRef<int>(0x10808CF8)++;
}

int ObjListResult::CountResults(){
	auto node = this->objects;
	auto count = 0;
	for (count = 0; node; count++){
		node = node->next;
	}
	return count;
}

void ObjListResult::ListRadius(LocAndOffsets origin, float rangeInches, float angleMin, float angleSize, int filter){
	addresses.ObjListRadius(origin, rangeInches, angleMin, angleSize, filter, *this);
}

void ObjListResult::ListRaycast(LocAndOffsets & origin, LocAndOffsets & endPt, float rangeInches, float radiusInches){
	RaycastPacket rayPkt;
	rayPkt.sourceObj = objHndl::null;
	rayPkt.origin = origin;
	rayPkt.rayRangeInches = rangeInches;
	rayPkt.targetLoc = endPt;
	rayPkt.radius = radiusInches;
	rayPkt.flags = (RaycastFlags)(RaycastFlags::ExcludeItemObjects | RaycastFlags::HasRadius | RaycastFlags::HasRangeLimit);
	rayPkt.Raycast();
	this->Init();

	if (rayPkt.resultCount) {
		for (auto i = 0; i < rayPkt.resultCount; i++) {
			auto &rayRes = rayPkt.results[i];
			if (rayRes.obj) {
				this->PrependHandle(rayRes.obj);
			}
		}
	}
}

bool ObjListResult::ContainsHandle(objHndl tgt)
{
	auto node = this->objects;
	while (node) {
		if (node->handle == tgt) return true;
		node = node->next;
	}
	return false;
}

bool ObjListResult::AnyHandle(std::function<bool(objHndl)> pred)
{
	auto node = this->objects;
	while (node) {
		if (pred(node->handle)) return true;
		node = node->next;
	}
	return false;
}

bool ObjListResult::AllHandles(std::function<bool(objHndl)> pred)
{
	auto node = this->objects;
	while (node) {
		if (!pred(node->handle)) return false;
		node = node->next;
	}
	return true;
}

ObjList::ObjList() {
	FreeResult();
}

ObjList::~ObjList() {
	FreeResult();
}

void ObjList::ListTile(locXY loc, int flags) {
	FreeResult();
	addresses.ObjListTile(loc, flags, mResult);
	mHasToFree = true;
}

void ObjList::ListRect(TileRect& trect, ObjectListFilter olcCritters)
{
	FreeResult();
	addresses.ObjListRect(trect, olcCritters, mResult);
	mHasToFree = true;
}

void ObjList::ListVicinity(locXY loc, int flags) {
	FreeResult();
	addresses.ObjListVicinity(loc, flags, mResult);
	mHasToFree = true;
}

void ObjList::ListVicinity(objHndl handle, int flags){
	ListVicinity(objSystem->GetObject(handle)->GetLocation(), flags);
}

void ObjList::ListRadius(LocAndOffsets loc, float radiusInches, int flags) {
	FreeResult();
	addresses.ObjListRadius(loc, radiusInches, 0.0f, (float)(M_PI * 2), flags, mResult);
	mHasToFree = true;
}

void ObjList::ListRange(LocAndOffsets loc, float radius, float angleMin, float angleMax, int flags)
{
	FreeResult();
	addresses.ObjListRadius(loc, radius, angleMin, angleMax, flags, mResult);
	mHasToFree = true;
}

void ObjList::ListRangeTiles(objHndl handle, int rangeTiles, ObjectListFilter filter){
	if (!handle){
		logger->error("Null handle given in ObjList::ListRangeTiles");
		return;
	}
		
	auto objBody = objSystem->GetObject(handle);
	auto loc = objBody->GetLocationFull();
	auto rangeInches = rangeTiles *INCH_PER_TILE; //28.284271; ;
	ListRadius(loc, rangeInches, filter);

}

void ObjList::ListCone(LocAndOffsets loc, float radius, float coneStartAngleRad, float coneArcRad, int flags) {
	FreeResult();
	addresses.ObjListRadius(loc, radius, coneStartAngleRad, coneArcRad, flags, mResult);
	mHasToFree = true;
}

void ObjList::ListFollowers(objHndl critter) {
	FreeResult();
	addresses.ObjListFollowers(critter, mResult);
	mHasToFree = true;
}

int ObjList::size() {
	if (mSizeValid) {
		return mSize;
	}
	mSize = CountObjects();
	mSizeValid = true;
	return mSize;
}

std::vector<objHndl> ObjList::GetListResult(){
	std::vector<objHndl> result;
	if (!mResult.objects) {
		return result;
	}
	auto item = mResult.objects;
	while (item) {
		result.push_back(item->handle);
		item = item->next;
	}
	return result;
}

int ObjList::CountObjects() const {
	if (!mResult.objects) {
		return 0;
	}
	auto item = mResult.objects;
	auto result = 0;
	while (item) {
		result++;
		item = item->next;
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
