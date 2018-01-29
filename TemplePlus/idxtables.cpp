
#include "stdafx.h"
#include "idxtables.h"
#include "util/fixes.h"

//temple::GlobalPrimitive<IdxTableListsNode*, 0x10EF2E70> idxTablesList;
IdxTableListsNode* idxTablesList;

#define BUCKET_COUNT 256


//template<typename T>
//inline void IdxTableWrapper<T>::free(){
//	/*auto idxtableFree = temple::GetRef<void(__cdecl)(IdxTable<T>*)>(0x101EC690);
//	idxtableFree(mTable);*/
//	
//}
//
//template <typename T>
//void IdxTableWrapper<T>::newTable(size_t size, const char* filename, int line){
//
//	
//
//	/*auto newIdxTable = temple::GetRef<void(__cdecl)(IdxTable<T>*, size_t, const char*, int)>(0x101EC620);
//	newIdxTable(mTable, size, filename, line);*/
//	
//}


class IdxTableHooks : TempleFix
{
public:

	static int IdxTableInit();
	static void IdxTableShutdown();

	static void IdxTableFree(IdxTable<void*>*);
	static void IdxTableNew(IdxTable<void*>* mTable, size_t size, const char* filename, int line);

	void apply() override	{

		replaceFunction(0x101EC400, IdxTableInit);
		replaceFunction(0x101ECAD0, IdxTableShutdown);
		replaceFunction(0x101EC690, IdxTableFree);
		replaceFunction(0x101EC620, IdxTableNew);
	}
} idxTableHooks;

int IdxTableHooks::IdxTableInit(){
	idxTablesList = nullptr;
	return 0;
}

void IdxTableHooks::IdxTableShutdown(){

	for (auto node = idxTablesList; node; node= idxTablesList){
		
		logger->error("{}({}): unreleased idx table.", node->sourceFile, node->lineNumber);
		IdxTableFree(reinterpret_cast<IdxTable<void*>*>(node->table));
		//idxTablesList = node->next;
		//free(node->table);
		//free(node);
	}

}

void IdxTableHooks::IdxTableFree(IdxTable<void*>* mTable) {


	for (auto i = 0; i < mTable->bucketCount; i++) {
		auto buckets = mTable->buckets;

		for (auto buck = buckets[i]; buck; buck = buckets[i]) {
			buckets[i] = buck->next;
			free(buck->data);
			free(buck);
			buckets = mTable->buckets;
		}
	}
	free(mTable->buckets);


	//IdxTableListsNode* &nodeList = temple::GetRef<IdxTableListsNode*>(0x10EF2E70);
	auto &nodeList = idxTablesList;
	auto node = nodeList;

	if ( reinterpret_cast<int>(node->table) == reinterpret_cast<int>(mTable)) {
		nodeList = node->next;
		free(node);
		return;
	}

	auto prevNode = node;
	while (node) {

		auto next = node->next;
		if (reinterpret_cast<int>(node->table) != reinterpret_cast<int>(mTable)) {
			prevNode = node;
			node = next;
			continue;
		}


		prevNode->next = node->next;
		free(node);
		return;
	}
}

void IdxTableHooks::IdxTableNew(IdxTable<void*>* mTable, size_t size, const char * filename, int line){

	mTable->bucketCount = BUCKET_COUNT;

	auto buckets = (IdxTableNode<void*>**)malloc(4 * BUCKET_COUNT);
	mTable->buckets = buckets;

	for (auto i = 0; i < mTable->bucketCount; i++) {
		mTable->buckets[i] = nullptr;
	}

	mTable->itemSize = size;
	mTable->itemCount = 0;

	auto newListEntry = new IdxTableListsNode;
	newListEntry->sourceFile = filename;

	//void* mTableVal = &mTable;
	//void* ptr = &newListEntry->table;
	//*ptr = *mTableVal;
	//newListEntry->table = *mTableVal;
	newListEntry->table = reinterpret_cast<IdxTable<void>*>(mTable);

	newListEntry->lineNumber = line;

	//auto &list = temple::GetRef<IdxTableListsNode*>(0x10EF2E70);
	auto &list = idxTablesList;

	newListEntry->next = list;
	list = newListEntry;

}
