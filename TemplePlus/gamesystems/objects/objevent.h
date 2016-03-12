#pragma once
//#include <idxtables.h>
//#include <obj.h>
//
//
//struct ObjEventAoE;
//struct ObjEventListItem;
//
//class ObjEventSystem
//{
//	ObjEventListItem * objEvtList;
//public:
//	IdxTableWrapper<ObjEventAoE> * objEvtTable;
//
//	ObjEventSystem();
//
//	int ListRangeUpdate(ObjEventAoE &data, int id, ObjEventListItem* listItem) const;
//	void AdvanceTime() const;
//	void TablePruneNullAoeObjs() const;
//
//#pragma region ObjEventList functions
//	void PrependEvtListNode(ObjEventListItem* evtListNode);
//	void ListItemNew(objHndl obj, LocAndOffsets loc, LocAndOffsets aoeObjLoc);
//	bool ListHasItems() const;
//	void ListPrune() const; //  remove consecutive duplicates and null obj handles
//
//
//private:
//	ObjEventListItem * ListPruneNext(ObjEventListItem*) const;
//#pragma endregion
//} objEvents;
