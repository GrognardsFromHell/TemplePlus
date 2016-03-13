#pragma once
#include <idxtables.h>
#include <obj.h>
#include <objlist.h>
struct GameSystemSaveFile;
struct ObjEventListItem;
struct ObjEventAoE
{
	int64_t sectorLoc;
	objHndl aoeObj;
	int onEnterFuncIdx;
	int onLeaveFuncIdx;
	ObjectListFilter filter;
	float radiusInch;
	float angleMin; // radians
	float angleSize;
	ObjListResultItem* objNodesPrev;
	int field2C;
	ObjListResult objListResult; // result for the current time tick; is copied to objNodesPrev at the end of the tick
};
struct LocAndOffsets;

//#include <idxtables.h>
//#include <obj.h>
//
//
//struct ObjEventAoE;
//struct ObjEventListItem;
//
class ObjEventSystem
{

public:
	IdxTableWrapper<ObjEventAoE> * objEvtTable;
	
	BOOL FreeObjectNodes(ObjEventAoE* aoeEvt, int id) const;
		void FreeObjectNodesRecursive(ObjListResultItem* item) const;
	ObjEventSystem();

	int ListRangeUpdate(ObjEventAoE &data, int id, ObjEventListItem* listItem) const;
	BOOL ObjEventLoadGame(GameSystemSaveFile* saveFile) const;
	BOOL ObjEventHandler(ObjEventAoE* const aoeEvt, int id, ObjEventListItem& evt) const;
	void AdvanceTime();
	void TablePruneNullAoeObjs() const;

#pragma region ObjEventList functions
	void PrependEvtListNode(ObjEventListItem& evtListNode);
	void ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc);
	bool ListHasItems() const;
	void ListPrune(); //  remove consecutive duplicates and null obj handles
	bool ObjEvtLoader(int* id, ObjEventAoE* evt, TioFile* file) const;
	void FlushEvents();
private:
	std::vector<ObjEventListItem>  objEvtList;
#pragma endregion
	bool ObjEventLocIsInAoE(ObjEventAoE* const aoeEvt, LocAndOffsets aoeObjLoc, float objRadius) const;
} ;

extern ObjEventSystem objEvents;
