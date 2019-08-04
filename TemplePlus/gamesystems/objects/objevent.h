#pragma once
#include <idxtables.h>
#include <obj.h>
#include <objlist.h>


#define OBJ_EVENT_WALL_ENTERED_HANDLER_ID 50
#define OBJ_EVENT_WALL_EXITED_HANDLER_ID 51

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

	bool IsWall();
	LocAndOffsets GetWallEndpoint();
	void UpdateObjectNodes(); // copies current objlist results to objNodesPrev and clears
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
	

	ObjEventSystem();

	int ListRangeUpdate(ObjEventAoE &data, int id, ObjEventListItem* listItem) const;
	BOOL ObjEventLoadGame(GameSystemSaveFile* saveFile) const;
	BOOL ObjEventHandler(ObjEventAoE* const aoeEvt, int id, ObjEventListItem& evt) const;
	void AdvanceTime();
	void TablePruneNullAoeObjs() const;

#pragma region ObjEventList functions

	int EventAppend(objHndl aoeObj, int onEnterFuncIdx, int onLeaveFuncIdx, ObjectListFilter olcFilter, float radiusInch, float angleBase, float angleSize) const; // registers an object event
	void PrependEvtListNode(ObjEventListItem& evtListNode);
	void ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc);
	bool ListHasItems() const;
	void ListPrune(); //  remove consecutive duplicates and null obj handles
	bool ObjEvtLoader(int* id, ObjEventAoE* evt, TioFile* file) const;
	void FlushEvents();
private:
	std::vector<ObjEventListItem>  objEvtList;
	bool mLockEvtList = false; // so that the EvtList isn't altered when processing it; can happen on damage events and subsequently cause crashes
#pragma endregion
	bool ObjEventLocIsInAoE(ObjEventAoE* const aoeEvt, LocAndOffsets aoeObjLoc, float objRadius) const;



protected:
	BOOL FreeObjectNodes(ObjEventAoE* aoeEvt, int id) const;
	void FreeObjectNodesRecursive(ObjListResultItem* item) const;
} ;

extern ObjEventSystem objEvents;
