#include "stdafx.h"
#include "objevent.h"
#include <util/fixes.h>
#include <gamesystems/gamesystems.h>
#include "objsystem.h"

#include <obj.h>
#include "common.h"
#include <objlist.h>
#include <gamesystems/map/sector.h>
#include <location.h>
#include <gamesystems/legacy.h>
#include <maps.h>
#include "raycast.h"


struct ObjEventListItem
{
	objHndl obj;
	LocAndOffsets loc;
	LocAndOffsets aoeObjLoc;
	ObjEventListItem * next;
	int pad;
};


ObjEventSystem objEvents;


const int testSizeOfAoeEvent = sizeof(ObjEventAoE); // should be 200 (0xC8)

IdxTableWrapper<ObjEventAoE> _objEvtTable(0x109DD320);

class ObjEventHooks : public TempleFix
{
public: 
	static void ObjectEventAdvanceTime();
	//static int EventAppend(objHndl obj, int onEnterFuncIdx, int onLeaveFuncIdx, ObjectListFilter olcFilter, float radiusInch, float angleMin, float angleMax);
	static void ObjEventListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets newLoc); // called from location updater functions like ObjMove

	void apply() override 
	{
		replaceFunction(0x10045290, ObjEventListItemNew);
		replaceFunction<int(__cdecl)(objHndl , int , int , ObjectListFilter , float , float , float )>(0x10045580, [](objHndl aoeObj, int onEnterIdx, int onLeaveIdx, ObjectListFilter olcFilter, float radiusInch, float angleBase, float angleSize){
			return objEvents.EventAppend( aoeObj, onEnterIdx, onLeaveIdx, olcFilter, radiusInch, angleBase, angleSize);
		});
		replaceFunction(0x10045740, ObjectEventAdvanceTime);
		
		
	}
} objEvtHooks;



void ObjEventHooks::ObjectEventAdvanceTime()
{
	objEvents.AdvanceTime();
}


void ObjEventHooks::ObjEventListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets newLoc)
{
	objEvents.ListItemNew(obj, loc, newLoc);
}

BOOL ObjEventSystem::FreeObjectNodes(ObjEventAoE* aoeEvt, int id) const
{
	auto objNode = aoeEvt->objNodesPrev;
	if (objNode){
		if (objNode->next){
			FreeObjectNodesRecursive(objNode->next);
		}
		free(objNode);
	}
	return 1;
}

void ObjEventSystem::FreeObjectNodesRecursive(ObjListResultItem* item) const
{
	if (item){
		if (item->next)
			FreeObjectNodesRecursive(item->next);
		free(item);
	}
}

ObjEventSystem::ObjEventSystem()
{
	objEvtTable = &_objEvtTable;
}

int ObjEventSystem::ListRangeUpdate(ObjEventAoE& evt, int id, ObjEventListItem* listItem) const
{
	if (!evt.aoeObj) {
		logger->warn("ObjectEventListRangeUpdate: AoeObj not found! Evt ID: {}", id);
		return 0;
		//objEvents.objEvtTable->remove(id);
	}
	
	auto obj = gameSystems->GetObj().GetObject(evt.aoeObj);
	if (gameSystems->GetObj().IsValidHandle(evt.aoeObj)){

		LocAndOffsets locFull = obj->GetLocationFull();

		if (evt.IsWall()){
			
			auto startPt = objSystem->GetObject(evt.aoeObj)->GetLocationFull();
			LocAndOffsets endPt = evt.GetWallEndpoint();
			evt.objListResult.ListRaycast(startPt, endPt, evt.radiusInch, 5.0f * INCH_PER_FEET / 2.0f);

		}
		else{
			evt.objListResult.ListRadius(locFull,evt.radiusInch, evt.angleMin, evt.angleSize, evt.filter);
			objEvtTable->put(id, evt);
		}

	} else
	{
		int dummy = 1;
		logger->debug("A ha! Caught an invalid handle! {} GUID {}", evt.aoeObj, gameSystems->GetObj().GetIdByHandle(evt.aoeObj).ToString());
	};
		

	
	return 1;
}

BOOL ObjEventSystem::ObjEventLoadGame(GameSystemSaveFile* saveFile) const
{
	for (auto it : *objEvents.objEvtTable) {
		auto nodeData = it.data;
		objEvents.FreeObjectNodes(nodeData, it.id);
	}

	objEvents.objEvtTable->free();
	objEvents.objEvtTable->newTable(sizeof(ObjEventAoE), "ObjectEvent.cpp", 193);

	int count = 0;
	if (tio_fread(&count, sizeof(int), 1, saveFile->file) != 1)
		return FALSE;



	int id;
	ObjEventAoE aoeEvt;
	for (int i = 0; i < count; i++) {
		if (objEvents.ObjEvtLoader(&id, &aoeEvt, saveFile->file) == 1) {
			objEvents.objEvtTable->put(id, aoeEvt);
		}
		else
		{
			logger->warn("ObjEventLoadGame: failed to read!");
			return FALSE;
		}
	}

	int _serial = 0;
	if (tio_fread(&_serial, sizeof(int), 1, saveFile->file) != 1)
		return FALSE;
	auto& objEvtSerial = temple::GetRef<int>(0x109DD330);
	objEvtSerial = _serial;
	for (auto it : *objEvents.objEvtTable) {
		auto nodeData = it.data;
		logger->debug("ObjectEventLoad: {} ({}) for handler id {}", nodeData->aoeObj, gameSystems->GetObj().GetIdByHandle(nodeData->aoeObj).ToString(),nodeData->onLeaveFuncIdx);
	}
	return TRUE;
}

BOOL ObjEventSystem::ObjEventHandler(ObjEventAoE* const aoeEvt, int id,ObjEventListItem& evt) const
{

	auto objEventHandlerFuncs = temple::GetPointer<void(__cdecl*)(objHndl, objHndl, int)>(0x102AFB40); // an array of 50 callbacks, though there are only 2 duplicated for them all
	auto onLeaveHandler = objEventHandlerFuncs[1]; // does  a dispatch for DK_OnLeaveAoE
	auto onEnterHandler = objEventHandlerFuncs[0]; // does  a dispatch for DK_OnEnterAoE

	if (aoeEvt->aoeObj != evt.obj)
	{
		// find the event obj in the aoeEvt list of previously appearing objects, to determine leaving / entering status
		bool foundInNodes = false;
		auto objNode = aoeEvt->objNodesPrev;
		
		while(objNode && objNode->handle != evt.obj){
			objNode = objNode->next;	
		}
		if (objNode)
			foundInNodes = true;

		if (!evt.obj){
			throw TempleException("Null event node after pruning???");
		}

		auto objBody = gameSystems->GetObj().GetObject(evt.obj);
		auto objRadius = objects.GetRadius(evt.obj);
		auto aoeObjLoc = evt.aoeObjLoc;
		bool isInAreaOfEffect = ObjEventLocIsInAoE(aoeEvt, aoeObjLoc, objRadius);

		if (foundInNodes){
			if(!isInAreaOfEffect)
			{
				onLeaveHandler(aoeEvt->aoeObj, evt.obj, id);
				return 1;
			}
		} else if (isInAreaOfEffect)
		{
			onEnterHandler(aoeEvt->aoeObj, evt.obj, id);
		}
		return 1;
	}

	// obj is aoeObj - i.e. the object itself moved. iterate over the aoeEvt's objlist
	for (auto it = aoeEvt->objListResult.objects; it; it=it->next){
		
		bool foundInNodes = false;

		// found the obj in the previous list
		auto prevObj = aoeEvt->objNodesPrev;
		while (prevObj && prevObj->handle != it->handle){
			prevObj = prevObj->next;
		}

		// if not found in the previous list, execute an onEnter event
		if (!prevObj){
			onEnterHandler(aoeEvt->aoeObj, it->handle, id);
		}
	}


	// check if any from the prevlist are gone, if so, execute the onLeave function for them
	auto prevObj = aoeEvt->objNodesPrev;
	while(prevObj){
		auto it = aoeEvt->objListResult.objects;

		while (it && it->handle != prevObj->handle){
			it = it->next;
		}

		// not found in the current list
		if (!it){
			onLeaveHandler(aoeEvt->aoeObj, prevObj->handle, id);
		}

		prevObj = prevObj->next;
	}

	return 1;
}

void ObjEventSystem::AdvanceTime() 
{
	TablePruneNullAoeObjs();

	if (!ListHasItems())
		return;

	ListPrune();

	//update ranges from aoeObj
	for (auto it : *objEvents.objEvtTable) {
		ObjEventAoE* data = it.data;
		ListRangeUpdate(*data, it.id, nullptr);
	}

	mLockEvtList = true;

	// cycle through the event list and expire the events
	for (auto evt = objEvtList.begin(); evt != objEvtList.end();)
	{
	
		for (auto it = objEvtTable->begin(); it != objEvtTable->end(); ++it) {
			auto& node = *it;
			ObjEventListItem evtt = *evt;
			if (!ObjEventHandler(node.data, node.id, evtt)) {
				logger->warn("ObjectEventAdvanceTime: fail! Id {}", node.id);
			}

		}
		
		evt = objEvtList.erase(evt);
		
	}

	mLockEvtList = false;

	auto objEvtUpdater = temple::GetRef<int(__cdecl)(ObjEventAoE*, int id)>(0x100450B0);
	for (auto it : *objEvents.objEvtTable){
		ObjEventAoE* data = it.data;
		//objEvtUpdater(data, it.id );
		data->UpdateObjectNodes();
		//objEvents.objEvtTable->put(it.id, *data);
	}
}

void ObjEventSystem::PrependEvtListNode(ObjEventListItem& evtListNode)
{
	if (!mLockEvtList)
		objEvtList.push_back(evtListNode);
}

void ObjEventSystem::ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc)
{
	if (objEvtTable->itemCount())
	{
		ObjEventListItem listItem;
		listItem.obj = obj;
		listItem.loc = loc;
		listItem.aoeObjLoc = aoeObjLoc;
		PrependEvtListNode(listItem);
	}
}

bool ObjEventSystem::ListHasItems() const
{
	return (!objEvtList.empty());
}

void ObjEventSystem::ListPrune()
{
	for (auto it = objEvtList.begin(); it != objEvtList.end(); ) {
		if (!it->obj){
			it = objEvtList.erase(it);
		} 
		else{
			++it;
		}
	}

	for (auto it = objEvtList.begin(); it != objEvtList.end(); ++it){
		auto nextIt = it + 1;
		while(nextIt != objEvtList.end()){

			if (it->obj != nextIt->obj)
				++nextIt;
			else{
				nextIt = objEvtList.erase(nextIt);
			}
		}
	}
}

bool ObjEventSystem::ObjEvtLoader(int* id, ObjEventAoE* evt, TioFile* file) const
{

	if (tio_fread(id, sizeof(int), 1, file) != 1
		|| tio_fread(&evt->sectorLoc, sizeof(SectorLoc), 1, file) != 1	)
		return false;
	ObjectId objId;
	if (tio_fread(&objId, sizeof(ObjectId), 1, file) != 1)
		return false;
	evt->aoeObj = gameSystems->GetObj().GetHandleById(objId);
	if (tio_fread(&evt->onEnterFuncIdx, sizeof(int), 1, file) != 1)
		return false;
	if (tio_fread(&evt->onLeaveFuncIdx, sizeof(int), 1, file) != 1)
		return false;
	if (tio_fread(&evt->filter, sizeof(int), 1, file) != 1)
		return false;
	if (tio_fread(&evt->radiusInch, sizeof(float), 1, file) != 1)
		return false;
	if (tio_fread(&evt->angleMin, sizeof(int), 1, file) != 1)
		return false;
	if (tio_fread(&evt->angleSize, sizeof(int), 1, file) != 1)
		return false;

	evt->objNodesPrev = nullptr;
	auto loadObjNodes = temple::GetRef<int(__cdecl)(ObjEventAoE*, TioFile*)>(0x10044E20);
	return loadObjNodes(evt, file) != 0;

}

void ObjEventSystem::FlushEvents()
{
	objEvtList.clear();

	for (auto it : *objEvents.objEvtTable) {
		auto nodeData = it.data;
		objEvents.FreeObjectNodes(nodeData, it.id);
	}

	objEvents.objEvtTable->free();
	objEvents.objEvtTable->newTable(sizeof(ObjEventAoE), "ObjectEvent.cpp", 423);
	
}

void ObjEventSystem::TablePruneNullAoeObjs() const
{

	for (auto it = objEvtTable->begin(); it != objEvtTable->end(); )
	{
		bool shouldPrune = false;
		auto& node = *it;
		if (!node.data->aoeObj)
			shouldPrune = true;


		if (shouldPrune) {
			it = objEvtTable->erase(it); // erase advances the iterator too so there shouldn't be one in the loop execution section
		}
		else {
			++it;
		}
	}

}

int ObjEventSystem::EventAppend(objHndl aoeObj, int onEnterFuncIdx, int onLeaveFuncIdx, ObjectListFilter olcFilter, float radiusInch, float angleBase, float angleSize) const
{
	if (!aoeObj) {
		logger->warn("ObjectEventAppend: Null aoeObj!");
		return 0;
	}
	auto aoeObjBody = gameSystems->GetObj().GetObject(aoeObj);
	auto objLoc = aoeObjBody->GetLocationFull();
	SectorLoc secLoc;
	secLoc.GetFromLoc(objLoc.location);

	ObjEventAoE evt;
	evt.aoeObj = aoeObj;
	evt.sectorLoc = secLoc;
	evt.onEnterFuncIdx = onEnterFuncIdx;
	evt.onLeaveFuncIdx = onLeaveFuncIdx;
	evt.radiusInch = radiusInch;
	evt.angleMin = angleBase;
	evt.angleSize = angleSize;
	evt.objNodesPrev = nullptr;
	evt.filter = olcFilter;

	auto getNewId = temple::GetRef<int(__cdecl)()>(0x10044AE0);
	auto id = getNewId();
	objEvents.objEvtTable->put(id, evt);

	if (objEvents.objEvtTable->itemCount()) {
		ObjEventListItem evtListNode;
		evtListNode.obj = aoeObj;
		evtListNode.aoeObjLoc = objLoc;
		evtListNode.loc.location.locx = 0;
		evtListNode.loc.location.locy = 0;
		evtListNode.loc.off_x = 0;
		evtListNode.loc.off_y = 0;
		evtListNode.pad = evt.onLeaveFuncIdx; // for debug
		objEvents.PrependEvtListNode(evtListNode);
	}
	return id;
}


bool ObjEventSystem::ObjEventLocIsInAoE(ObjEventAoE* const aoeEvt, LocAndOffsets loc, float objRadius) const
{
	auto aoeObjBody = gameSystems->GetObj().GetObject(aoeEvt->aoeObj);
	auto aoeObjLoc = aoeObjBody->GetLocationFull();
	float aoeAbsX, aoeAbsY, absX, absY;
	locSys.GetOverallOffset(aoeObjLoc, &aoeAbsX, &aoeAbsY);
	locSys.GetOverallOffset(loc, &absX, &absY);

	if (aoeEvt->IsWall()){
		auto wallEndpt = aoeEvt->GetWallEndpoint();
		float wallEndX, wallEndY;
		locSys.GetOverallOffset(wallEndpt, &wallEndX, &wallEndY);
		RaycastPointSearchPacket srchPkt({ aoeAbsX , aoeAbsY }, { wallEndX , wallEndY});
		srchPkt.radius = 5.0f * INCH_PER_FEET / 2.0f;
		
		auto result = srchPkt.IsPointInterceptedBySegment({ absX, absY }, objRadius);
		return result;
	}

	auto radius = objRadius + aoeEvt->radiusInch;
	auto deltaX = absX - aoeAbsX;
	auto absDeltaX = abs(deltaX);
	auto deltaY = absY - aoeAbsY;
	auto absDeltaY = abs(deltaY);

	// check distance
	if (absDeltaY > radius || radius*radius < absDeltaX*absDeltaX + absDeltaY*absDeltaY)
		return 0;

	// if the angle is 2PI return 1
	if (abs(aoeEvt->angleSize) >= M_PI*1.99)
		return 1;
	else
	{
		logger->warn("ObjEventLocIsInAoE: Cone section not implemented yet!");
	}

	// check conical case
	const float angleOffset = 0*M_PI*3.0 / 4.0; // thee quarters PI  (135°)
	auto angleMin = aoeEvt->angleMin + angleOffset; // angleMin of 0 is oriented along the x axis
	auto angleMax = aoeEvt->angleSize + aoeEvt->angleMin + angleOffset;
	
	auto angle = atan2(deltaX, deltaY) ; // atan2 returns between -pi to pi
	
	auto angleMinX = cos(angleMin);
	auto angleMinY = sin(angleMin);
	auto angleMaxX = cos(angleMax);
	auto angleMaxY = sin(angleMax);
	


	auto cosAngle = cos(angleMin);
	

	auto shit1 = -cos(angleMax);
	auto shit2 = sin(angleMax);

	return 0;

}

bool ObjEventAoE::IsWall(){
	return this->onEnterFuncIdx == OBJ_EVENT_WALL_ENTERED_HANDLER_ID && this->onLeaveFuncIdx == OBJ_EVENT_WALL_EXITED_HANDLER_ID;
}

LocAndOffsets ObjEventAoE::GetWallEndpoint(){
	auto aoeObjBody = gameSystems->GetObj().GetObject(this->aoeObj);
	auto aoeObjLoc = aoeObjBody->GetLocationFull();

	auto wallEndpt = aoeObjLoc;
	// use startPt + angleMin to find endPt
	auto vectorAngle = 5 * M_PI / 4 - angleMin;
	XMFLOAT2 dir(cos(vectorAngle), sin(vectorAngle));
	wallEndpt.off_x += dir.x * radiusInch;
	wallEndpt.off_y += dir.y * radiusInch;
	wallEndpt.Regularize();
	return wallEndpt;
}

void ObjEventAoE::UpdateObjectNodes(){
	// Free previous nodes
	auto prevNode = this->objNodesPrev;
	if (prevNode){
		if (prevNode->next)
			prevNode->next->FreeRecursive();
		free(prevNode);
	}
	
	// Copy objlist to objNodesPrev
	auto objNode = objListResult.objects;
	prevNode = nullptr;
	while (objNode){
		auto newNode = new ObjListResultItem();
		newNode->next = prevNode;
		newNode->handle = objNode->handle;
		prevNode = newNode;
		objNode = objNode->next;
	}
	this->objNodesPrev = prevNode;
	this->objListResult.Free();

}
