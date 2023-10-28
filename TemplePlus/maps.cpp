
#include "stdafx.h"
#include "maps.h"
#include <temple/dll.h>
#include "location.h"
#include "util/fixes.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/legacysystems.h"
#include "fade.h"
#include "gamesystems/legacymapsystems.h"
#include "infrastructure/tabparser.h"
#include "tig/tig_tabparser.h"
#include <infrastructure/vfs.h>

struct MapAddresses : temple::AddressTable {
	
	/*
		Writes at most size visited map ids to pMapIdsOut and returns
		the real number of visited maps, which can be larger.
	*/
	size_t (__cdecl *GetVisitedMaps)(int *pMapIdsOut, int size);

	/*
		Gets the id of the current area, which is based on the current map.
		TODO There is a hardcoded table in this function that should be replaced.
	*/
	int (__cdecl *GetCurrentArea)();

	/*
		Gets the id of the current map.
	*/
	int (__cdecl *GetCurrentMap)();

	/*
		Returns true if the given map id is valid.
	*/
	bool (__cdecl *IsValidMapId)(int mapId);
	
	/*
		Returns true if the current map is an outdoor map.
	*/
	bool (__cdecl *IsCurrentMapOutdoor)();

	/*
		Reveals a Flag on the Townmap UI.
	*/
	void (__cdecl *RevealFlag)(int mapId, int flagId);

	/*
		Gets a jump point definition.
	*/
	bool (__cdecl *GetJumpPoint)(int jmpPntID, char *mapNameOut, size_t mapNameOutSize, int *mapNumOut, locXY *locXYOut);
	
	locXY (*GetMapCenterTile)();
	
	int * mapLoaded;
	int * mapIsOpen;

	MapAddresses() {
		rebase(GetVisitedMaps, 0x1006FE50);
		rebase(GetCurrentArea, 0x1006ED50);
		rebase(GetCurrentMap, 0x10070F90);
		rebase(IsValidMapId, 0x10070EF0);
		rebase(RevealFlag, 0x10128360);
		rebase(IsCurrentMapOutdoor, 0x1006FE80);
		rebase(GetJumpPoint, 0x100BDE20);
		rebase(mapLoaded, 0x10AA9524);
		rebase(GetMapCenterTile, 0x1002A170);
		rebase(mapIsOpen, 0x10AA9588);
	}
};
MapAddresses mapAddresses;

bool ReadDaynightXferEntry(Vfs::FileHandle file, DayNightXfer*& dnx);
DayNightXfer* CreateDaynightEntry(ObjectId id, int daymapId, LocAndOffsets daymapLoc, int nightmapId, LocAndOffsets nightmapLoc);
bool TeleportSystem::DaynightXferLoad()
{
	auto dnxFile = fmt::format("Save\\Current\\daynight.nxd");
	if (!vfs->FileExists(dnxFile)) {
		dnxFile = fmt::format("rules\\daynight.nxd");
	}
	auto f = vfs->Open(dnxFile, "rb");
	if (!f) {
		return false;
	}

	auto& dnxList = temple::GetRef<DayNightXfer*>(0x10AB7548);
	// free all current nodes
	for (auto node = dnxList; dnxList; node = dnxList) {
		auto nextNode = node->next;
		dnxList = nextNode;
		free(node);
	}

	// parse nodes from file
	DayNightXfer* node = nullptr;
	while (ReadDaynightXferEntry(f, node)) {
		//preprend node
		node->next = dnxList;
		dnxList = node;
	}
	vfs->Close(f);
	return true;
}

bool ReadDaynightXferEntry(Vfs::FileHandle file, DayNightXfer*& dnx)
{
	DayNightXfer tmpNode;
	if (!vfs->Read(&tmpNode.objId, sizeof(tmpNode.objId), file)
		|| !vfs->Read(&tmpNode.currentMap, sizeof(tmpNode.currentMap), file)
		|| !vfs->Read(&tmpNode.dayMapId, sizeof(tmpNode.dayMapId), file)
		|| !vfs->Read(&tmpNode.loc, sizeof(tmpNode.loc), file)
		|| !vfs->Read(&tmpNode.nightMapId, sizeof(tmpNode.nightMapId), file)
		|| !vfs->Read(&tmpNode.nightLoc, sizeof(tmpNode.nightLoc), file)
		) {
		return false;
	}

	auto newNode = CreateDaynightEntry(tmpNode.objId, tmpNode.dayMapId, tmpNode.loc, tmpNode.nightMapId, tmpNode.nightLoc);
	dnx = newNode;
	if (!newNode)
		return false;

	newNode->currentMap = tmpNode.currentMap;
	return true;
}

DayNightXfer* CreateDaynightEntry(ObjectId id, int daymapId, LocAndOffsets daymapLoc, int nightmapId, LocAndOffsets nightmapLoc)
{
	return temple::GetRef< DayNightXfer*(__cdecl)(ObjectId , int , LocAndOffsets , int , LocAndOffsets )>(
		0x10084C00)(id , daymapId, daymapLoc, nightmapId, nightmapLoc);
}

class GameSystemReplacements : TempleFix
{
public:
	static int(__cdecl*orgField1C)(GameSystemConf* conf);
	static int field1c(GameSystemConf * conf) {
		return orgField1C(conf);
	};

	static int JumpPointModLoad();
	static int JumpPointInit();

	void apply() override
	{
		replaceFunction(0x100BDF50, JumpPointInit);
		replaceFunction(0x100BDFE0, JumpPointModLoad);

		orgField1C = replaceFunction(0x1006FC60, field1c); // doesn't seem to get called anywhere, not even when editor mode is enabled. Possibly ripped out code.

		static void(__cdecl*orgTeleportProcess)(FadeAndTeleportArgs&) = replaceFunction<void(__cdecl)(FadeAndTeleportArgs&)>(0x10085AA0, [](FadeAndTeleportArgs &args)
		{
			orgTeleportProcess(args);
		});

		static int(__cdecl*orgFadeAndTeleport)(FadeAndTeleportArgs&) = replaceFunction<int(__cdecl)(FadeAndTeleportArgs&)>(0x10084A50, [](FadeAndTeleportArgs &args)
		{

			auto &fadeAndTeleportActive = temple::GetRef<BOOL>(0x10AB74C0);
			auto &teleportPacket = temple::GetRef<FadeAndTeleportArgs>(0x10AB74C8);

			return orgFadeAndTeleport(args);
		});

		// fix for Co8 bug with daynight transfer using map 5199 (placeholder) for the night map of 5189
		static DayNightXfer* (__cdecl*orgDaynightListPop)() = replaceFunction< DayNightXfer*(__cdecl)()>(0x10084D00, []()
		{
			auto result = orgDaynightListPop();
			if (result && result->currentMap == 5199 && result->dayMapId == 5189){
				result->currentMap = 5189;
			}
			return result;
		});

		static BOOL(__cdecl * DaynightXferLoad)() = replaceFunction<BOOL(__cdecl)()>(0x100851C0, []()
			{
				return gameSystems->GetTeleport().DaynightXferLoad() ? TRUE: FALSE;
			});


		static BOOL(__cdecl * orgTeleportPrepareObj)(objHndl, locXY) 
			= replaceFunction<BOOL(objHndl, locXY)>(0x100856D0,
			[](objHndl handle, locXY loc)->BOOL {
				auto result = orgTeleportPrepareObj(handle, loc);
				logger->debug("TeleportPrepareObj: prepared obj {}, result = {}", handle, result);
				return result;
			});
		
		static BOOL(__cdecl * orgMoveObjToMap)(objHndl, int, LocAndOffsets) 
			= replaceFunction<BOOL(objHndl, int, LocAndOffsets)>(0x10025F70,
			[](objHndl handle, int mapId, LocAndOffsets loc)->BOOL {
					auto result = orgMoveObjToMap(handle, mapId, loc);
					logger->debug("MoveObjToMap: obj {} , tgtMap = {}, result = {}", handle, mapId, result);
					return result;
			});
		

		replaceFunction<void(__cdecl)(locXY)>(0x10005BC0, [](locXY locXy){
			gameSystems->GetLocation().CenterOnSmooth(locXy.locx, locXy.locy);
		});



	}
} gameSystemFix;

int(__cdecl*GameSystemReplacements::orgField1C)(GameSystemConf*) ;

Maps maps;

vector<int> Maps::GetVisited() {

	vector<int> result(100);
	auto realCount = mapAddresses.GetVisitedMaps(result.data(), result.size());

	if (realCount > result.size()) {
		result.resize(realCount);
		realCount = mapAddresses.GetVisitedMaps(result.data(), result.size());
	}
	assert(realCount <= result.size());
	result.resize(realCount);

	return result;

}

int Maps::GetCurrentMapId() {
	return mapAddresses.GetCurrentMap();
}

int Maps::GetCurrentAreaId() {
	return mapAddresses.GetCurrentArea();
}

bool Maps::IsValidMapId(int mapId) {
	return mapAddresses.IsValidMapId(mapId);
}

void Maps::RevealFlag(int mapId, int flagId) {
	mapAddresses.RevealFlag(mapId, flagId);
}

bool Maps::IsCurrentMapOutdoor() {
	return mapAddresses.IsCurrentMapOutdoor();
}

locXY Maps::GetMapCenterTile() {
	return mapAddresses.GetMapCenterTile();
}

bool Maps::GetJumpPoint(int id, JumpPoint& jumpPoint, bool withMapName) {
	char mapName[256];
	size_t mapNameLen = withMapName ? 256 : 0;
	bool result = mapAddresses.GetJumpPoint(id, mapName, mapNameLen, &jumpPoint.mapId, &jumpPoint.location);
	if (result) {
		jumpPoint.mapName = mapName;
	}
	return result;
}

int GameSystemReplacements::JumpPointModLoad(){

	auto jmpPntTable = temple::GetPointer<IdxTable<LgcyJumpPoint>>(0x10BCAAA4);

	auto makeNewIdxTable = temple::GetRef<void(__cdecl)(IdxTable<LgcyJumpPoint>*, int, const char*, int)>(0x101EC620);

	makeNewIdxTable(jmpPntTable, 0x18, "jumppoint.c", 117);

	auto initJumpTable = temple::GetRef<int(__cdecl)()>(0x100BDF50);
	auto res = initJumpTable();
	if (initJumpTable()){
		temple::GetRef<int>(0x10BCAAB4) = 1; // jump point inited
	}

	return res;
}


IdxTableWrapper<LgcyJumpPoint> jmpPointIdxTable(0x10BCAAA4);
int GameSystemReplacements::JumpPointInit(){

	for (auto it: jmpPointIdxTable){
		auto data = it.data;
		free(data->mapName);
	}

	auto jmpPntTable = temple::GetPointer<IdxTable<LgcyJumpPoint>>(0x10BCAAA4);
	/*auto idxTableFree = temple::GetRef<void(__cdecl)(IdxTable<LgcyJumpPoint>*)>(0x101EC690);
	auto makeNewIdxTable = temple::GetRef<void(__cdecl)(IdxTable<LgcyJumpPoint>*, int, const char*, int)>(0x101EC620);
*/
	jmpPointIdxTable.free();
	jmpPointIdxTable.newTable(sizeof LgcyJumpPoint, "jumppoint.c", 97);

	TigTabParser tabFile;
	auto jmpPtParser = temple::GetRef<TigTabLineParser>(0x100BDE90);
	tabFile.Init(jmpPtParser);
	tabFile.Open("rules\\jumppoint.tab");
	tabFile.Process();

	auto &jmpPointMaxId = temple::GetRef<int>(0x10BCAAB8);
	jmpPointMaxId = 0;

	for (auto it:jmpPointIdxTable){
		auto data = it.data;
		if (data->id > jmpPointMaxId){
			jmpPointMaxId = data->id;
		}
	}

	tabFile.Close();
	
	return 1;
}
