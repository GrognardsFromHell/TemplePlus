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


struct ObjEventListItem
{
	objHndl aoeObj;
	LocAndOffsets loc;
	LocAndOffsets aoeObjLoc;
	ObjEventListItem * next;
	int pad;
};



//class ObjEventSystem
//{
//	
//public:
//	IdxTableWrapper<ObjEventAoE> * objEvtTable;
//
//	ObjEventSystem();
//
//	int ListRangeUpdate(ObjEventAoE &data, int id, ObjEventListItem* listItem) const;
//	BOOL ObjEventHandler(ObjEventAoE* const aoeEvt,int id, ObjEventListItem& evt) const;
//	void AdvanceTime() ;
//	void TablePruneNullAoeObjs() const;
//
//	#pragma region ObjEventList functions
//	void PrependEvtListNode(ObjEventListItem& evtListNode);
//	void ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc);
//	bool ListHasItems() const;
//	void ListPrune() ; //  remove consecutive duplicates and null obj handles
//	
//	
//private:
//	std::vector<ObjEventListItem>  objEvtList;
//	std::vector<ObjEventListItem>  objEvtDebugList;
//#pragma endregion
//	bool ObjEventLocIsInAoE(ObjEventAoE* const aoeEvt, LocAndOffsets aoeObjLoc, float objRadius) const;
//} objEvents;
ObjEventSystem objEvents;


const int testSizeOfAoeEvent = sizeof(ObjEventAoE); // should be 200 (0xC8)

IdxTableWrapper<ObjEventAoE> _objEvtTable(0x109DD320);

class ObjEventHooks : public TempleFix
{
public: 
	const char* name() override {
		return "ObjEvent Hooks";
	} 
	static void ObjectEventAdvanceTime();
	static int EventAppend(objHndl obj, int onEnterFuncIdx, int onLeaveFuncIdx, ObjectListFilter olcFilter, float radiusInch, float angleMin, float angleMax);
	static void ObjEventListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc);

	void apply() override 
	{
		replaceFunction(0x10045290, ObjEventListItemNew);
		replaceFunction(0x10045580, EventAppend);
		replaceFunction(0x10045740, ObjectEventAdvanceTime);
		
		
	}
} objEvtHooks;



void ObjEventHooks::ObjectEventAdvanceTime()
{
	objEvents.AdvanceTime();
}

int ObjEventHooks::EventAppend(objHndl aoeObj, int onEnterFuncIdx, int onLeaveFuncIdx, ObjectListFilter olcFilter, float radiusInch, float angleMin, float angleMax)
{
	if (!aoeObj){
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
	evt.angleMin = angleMin;
	evt.angleSize = angleMax;
	evt.objNodesPrev = nullptr;
	evt.filter = olcFilter;

	auto getNewId = temple::GetRef<int(__cdecl)()>(0x10044AE0);
	auto id = getNewId();
	objEvents.objEvtTable->put(id, evt);

	if (objEvents.objEvtTable->itemCount()){
		ObjEventListItem evtListNode;
		evtListNode.aoeObj = aoeObj;
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

void ObjEventHooks::ObjEventListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc)
{
	objEvents.ListItemNew(obj, loc, aoeObjLoc);
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
	if (gameSystems->GetObj().IsValidHandle(evt.aoeObj))
	{
		LocAndOffsets locFull = obj->GetLocationFull();
		auto listRange = temple::GetRef<void(__cdecl)(LocAndOffsets, float, float, float, ObjectListFilter, ObjListResult*)>(0x10022E50);
		listRange(locFull, evt.radiusInch, evt.angleMin, evt.angleSize, evt.filter, &evt.objListResult);
		objEvtTable->put(id, evt);
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
		logger->debug("ObjectEventLoad: {} ({}) for handler id {}", description.getDisplayName(nodeData->aoeObj), gameSystems->GetObj().GetIdByHandle(nodeData->aoeObj).ToString(),nodeData->onLeaveFuncIdx);
	}
	return TRUE;
}

BOOL ObjEventSystem::ObjEventHandler(ObjEventAoE* const aoeEvt, int id,ObjEventListItem& evt) const
{

	auto objEventHandlerFuncs = temple::GetPointer<void(*__cdecl)(objHndl , objHndl , int )>(0x102AFB40);
	if (aoeEvt->aoeObj != evt.aoeObj)
	{
		// find the event obj in the aoeEvt list of previously appearing objects
		bool foundInNodes = false;
		auto objNode = aoeEvt->objNodesPrev;
		
		while(objNode && objNode->handle != evt.aoeObj){
			objNode = objNode->next;	
		}
		if (objNode)
			foundInNodes = true;

		if (!evt.aoeObj){
			throw TempleException("Null event node after pruning???");
		}
		auto objBody = gameSystems->GetObj().GetObject(evt.aoeObj);
		auto objRadius = objects.GetRadius(evt.aoeObj);
		auto aoeObjLoc = evt.aoeObjLoc;
		bool isInAreaOfEffect = ObjEventLocIsInAoE(aoeEvt, aoeObjLoc, objRadius);

		if (foundInNodes){
			if(!isInAreaOfEffect)
			{
				objEventHandlerFuncs[aoeEvt->onLeaveFuncIdx](aoeEvt->aoeObj, evt.aoeObj, id);
				return 1;
			}
		} else if (isInAreaOfEffect)
		{
			objEventHandlerFuncs[aoeEvt->onEnterFuncIdx](aoeEvt->aoeObj, evt.aoeObj, id);
		}
		return 1;
	}

	// obj is aoeObj - iterate over the aoeEvt's objlist
	for (auto it = aoeEvt->objListResult.objects; it; it=it->next){
		
		
		bool foundInNodes = false;

		// found the obj in the previous list
		auto prevObj = aoeEvt->objNodesPrev;
		while (prevObj && prevObj->handle != it->handle){
			prevObj = prevObj->next;
		}

		// if not found in the previous list, execute an onEnter event
		if (!prevObj){
			objEventHandlerFuncs[aoeEvt->onEnterFuncIdx](aoeEvt->aoeObj, it->handle, id);
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
			objEventHandlerFuncs[aoeEvt->onLeaveFuncIdx](aoeEvt->aoeObj, prevObj->handle, id);
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

	// cycle through the event list and expire the events
	for (auto evt = objEvtList.begin(); evt != objEvtList.end();)
	{
		for (auto it = objEvtTable->begin(); it != objEvtTable->end(); ++it) {
			auto& node = *it;
			node.data;
			auto& evtt = *evt;
			if (!ObjEventHandler(node.data, node.id, evtt)) {
				logger->warn("ObjectEventAdvanceTime: fail! Id {}", node.id);
			}
		}
		evt = objEvtList.erase(evt);
	}

	auto objEvtUpdater = temple::GetRef<int(__cdecl)(ObjEventAoE*, int id)>(0x100450B0);
	for (auto it : *objEvents.objEvtTable){
		ObjEventAoE* data = it.data;
		objEvtUpdater(data, it.id );
	}
}

void ObjEventSystem::PrependEvtListNode(ObjEventListItem& evtListNode)
{
	objEvtList.push_back(evtListNode);
}

void ObjEventSystem::ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc)
{
	if (objEvtTable->itemCount())
	{
		ObjEventListItem listItem;
		listItem.aoeObj = obj;
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
		if (!it->aoeObj){
			it = objEvtList.erase(it);
		} 
		else{
			++it;
		}
	}

	for (auto it = objEvtList.begin(); it != objEvtList.end(); ++it){
		auto nextIt = it + 1;
		while(nextIt != objEvtList.end()){

			if (it->aoeObj != nextIt->aoeObj)
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
	return loadObjNodes(evt, file);

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



bool ObjEventSystem::ObjEventLocIsInAoE(ObjEventAoE* const aoeEvt, LocAndOffsets loc, float objRadius) const
{
	auto aoeObjBody = gameSystems->GetObj().GetObject(aoeEvt->aoeObj);
	auto aoeObjLoc = aoeObjBody->GetLocationFull();
	float aoeAbsX, aoeAbsY, absX, absY;
	locSys.GetOverallOffset(aoeObjLoc, &aoeAbsX, &aoeAbsY);
	locSys.GetOverallOffset(loc, &absX, &absY);

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
