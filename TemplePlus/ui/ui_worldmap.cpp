#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "maps.h"
#include <tig/tig_keyboard.h>

#include "ui_worldmap.h"
#include "ui_systems.h"
#include "ui_legacysystems.h"
#include "ui_townmap.h"

#include <temple/dll.h>
#include "tig/tig_msg.h"
#include "fade.h"
#include "party.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "mod_support.h"
#include <infrastructure/vfs.h>
#include "util/streams.h"
#include <infrastructure/binaryreader.h>
#include <ui/widgets/widgets.h>


//*****************************************************************************
//* Worldmap-UI
//*****************************************************************************

constexpr int HOMMLET_MAIN = 5001;
constexpr int HOMMLET_NORTH = 15001;
constexpr int HOMMLET_CASTLE = 25001;
constexpr int LOCATION_NONE = -1; // for locations with no fixed worldmap position, e.g. random encounters

typedef int LocationId; // Location on the worldmap
typedef int AreaId; // Game Area
typedef int MapId; // Map ID

struct LocationProps {
	bool knownByDefault = false;
	MapId mapId = 0; // Note: the actual MapId is % 10000 of this; e.g. Hommlet hotspots are 5001, 15001, 25001
	AreaId areaId = 0;
	locXY startLoc = locXY::fromField( (uint64_t)-1); // if left at -1, the location will be retrieved from MapList.mes values
	std::string name; // originally 0x11EA4000
};

enum MapVisibility : uint32_t {
	MV_HIDDEN = 0,
	MV_FADING = 1,
	MV_VISIBLE =2,
	// above this - time stamp
};
struct LocationState {
	MapVisibility scriptWriting; // originally array at 0x11EA4A00
	MapVisibility mapBtn; // originally array at 0x11EA4900
};

enum LocationVisitedFlags : uint32_t {
	LVF_LocationVisible = 1,
	LVF_ScriptVisible = 2
};
struct LocationVisited {
	LocationId locId;
	int flags;
};
struct LocationWidgets {
	LgcyWidgetId locationBtn;
	LgcyWidgetId locationRingBtn;
	LgcyWidgetId scriptBtn;
};

struct LgcyWorldmapPath {
	int startX;
	int startY;
	int endX;
	int endY;
	int count;
	char* directions;
};

struct WorldmapPath {
	int startX;
	int startY;
	int endX;
	int endY;
	int count;
	std::vector<uint8_t> directions;
};


class UiWorldmapImpl {
	friend class UiWorldmap;
	friend class WorldmapFix;
public:
	
	UiWorldmapImpl();

	bool LoadGame(const UiSaveFile& save);

	void Show(int state);

	bool OnDestinationReached();

	std::vector<LocationId> mAreaToLocId;
	std::vector<MapId> locToMapId;
	std::vector<uint32_t> scriptWritingStatus;
	std::vector<LocationProps> locationProps;
	std::vector<LocationState> locationStates;
	std::vector<LocationVisited> locationsVisited;

	int &mNeedToClearEncounterMap = temple::GetRef<int>(0x10BEF800);
	int &mTeleportMapId = temple::GetRef<int>(0x102FB3CC);
	int& mRandomEncounterStatus = temple::GetRef<int>(0x102FB3D0);
	int& mRandomEncounterX = temple::GetRef<int>(0x102FB3D4);
	int& mRandomEncounterY = temple::GetRef<int>(0x102FB3D8);
	BOOL& mPermanentPermissionToExitEncounterMap = temple::GetRef<BOOL>(0x10BF37A4); // actually a UiRandomEncounter member

	LocationId& mDialogueDestLocId = temple::GetRef<LocationId>(0x102FB3E8);

	BOOL& mIsMakingTrip = temple::GetRef<BOOL>(0x10BEF7FC);
	/*
	0 - Switched from townmap UI, or door with obj_f_scenery_flags: OSCF_USE_OPEN_WORLDMAP
	1- ??
	2 - travel by dialogue
	3 - entered from random encounter map
	*/
	int& mWorldmapState = temple::GetRef<int>(0x10BEF808);
	BOOL& mIsVisible = temple::GetRef<BOOL>(0x10BEF7DC);

protected:
	void InitLocations();
		void ReadLocationNames();
	bool mIsCo8 = false;

	MesFile::Content mWidgetLocations;
	MesFile::Content mLargeTexturesMes;  // 0_worldmap_ui_large_textures.mes

	std::vector<int> mTeleportMapIdTable; // map ID to teleport for each location; originally 0x102972BC (Vanilla) 0x11EA3710 (Co8 5.0.2+)
	int& mLocationsVisitedCount = temple::GetRef<int>(0x10BEF810);

	void InitWidgets(int w, int h);
		void GetUiLocations();
	void UpdateWidgets();
	void LoadPaths();
	std::vector<int> mPathIdTable;
	
	int mPathIdTableWidth;
	std::vector<WorldmapPath> mPaths; // originally 0x10BEEE98 (vanilla) 0x11EA2406 (Co8 5.0.2+)
	int mPathIdMax;

	void SetMapVisited(MapId mapId);
	LocationId GetLocationFromMap(MapId mapId); // note: this is strictly for the "start" maps of the area, i.e. indoors or connected maps don't count
	LocationId GetLocationFromArea(AreaId areaId);
	int GetLocationVisited(LocationId); // for the text list of "Locations Visited"

	void SetLocationVisited(LocationId locId);

	int GetAreaFromMap(MapId mapId);
	void MarkAreaKnown(AreaId areaId);

	

	BOOL MakeTripFromAreaToArea(int fromId, int toId);

	void UnhideLocationBtns();
	void UpdateAcquiredLocationsBtns();

	// Widgets
	std::unique_ptr<WidgetContainer> mWnd;
	LgcyWidgetId mCurrentMapBtn = -1;
	LgcyWidgetId mCenterOnPartyBtn = -1;
	std::vector<LgcyWidgetId> acquiredLocationBtns;
	std::vector<LocationWidgets> locWidgets;

	bool AcquiredLocationBtnMsg(LgcyWidgetId widId, TigMsg* msg);

	int& mLocationsRevealingCount = temple::GetRef<int>(0x10BEF7F4);
};

constexpr int ACQUIRED_LOCATIONS_CO8 = 21;
constexpr int ACQUIRED_LOCATIONS_VANILLA = 14;
constexpr int TRAILDOT_MAX = 80;


struct WorldmapWidgetsVanilla
{
	LgcyWindow* mainWnd;
	LgcyButton* worldmapBtn;
	LgcyButton* curMapBtn;
	LgcyButton* locationBtns[14];
	LgcyButton* locationRingBtns[14];
	LgcyButton* scriptBtns[14];
	LgcyButton* URHereBtn;
	LgcyButton* trailDots[TRAILDOT_MAX];
	LgcyButton* exitBtn;
	LgcyWindow* selectionWnd;
	LgcyButton* acquiredLocations[ACQUIRED_LOCATIONS_VANILLA];
	LgcyButton* centerOnPartyBtn;
};
struct WorldmapWidgetsCo8
{
	LgcyWindow * mainWnd;
	LgcyButton * worldmapBtn;
	LgcyButton * curMapBtn;
	LgcyButton * locationBtns[14];
	LgcyButton * locationRingBtns[14];
	LgcyButton * scriptBtns[14];
	LgcyButton * URHereBtn;
	LgcyButton * trailDots[TRAILDOT_MAX];
	LgcyButton * exitBtn;
	LgcyWindow * selectionWnd;
	LgcyButton * acquiredLocations[ACQUIRED_LOCATIONS_CO8];
	LgcyButton * centerOnPartyBtn;
};

UiWorldmapImpl *wmImpl = nullptr;

UiWorldmap::UiWorldmap(int width, int height) {
	struct LgcyConf {
		int x;
		int width;
		int height;
	} lgcyConf;
	lgcyConf.width = width;
	lgcyConf.height = height;

	auto startup = temple::GetPointer<int(const LgcyConf*)>(0x10160470);
	if (!startup(&lgcyConf)) {
		throw TempleException("Unable to initialize game system Worldmap-UI");
	}

	mImpl = std::make_unique<UiWorldmapImpl>();
	wmImpl = mImpl.get();
}
UiWorldmap::~UiWorldmap() {
	auto shutdown = temple::GetPointer<void()>(0x1015e060);
	shutdown();
}
void UiWorldmap::ResizeViewport(const UiResizeArgs& resizeArg) {
	auto resize = temple::GetPointer<void(const UiResizeArgs*)>(0x101599c0);
	resize(&resizeArg);
}
void UiWorldmap::Reset() {
	auto reset = temple::GetPointer<void()>(0x101597b0);
	reset();
}
bool UiWorldmap::SaveGame(TioFile *file) {
	auto save = temple::GetPointer<int(TioFile*)>(0x101598b0);
	return save(file) == 1;
}
bool UiWorldmap::LoadGame(const UiSaveFile &save) {
	return wmImpl->LoadGame(save);
}
const std::string &UiWorldmap::GetName() const {
	static std::string name("Worldmap-UI");
	return name;
}

void UiWorldmap::Show(int mode)
{
	static auto ui_show_worldmap = temple::GetPointer<void(int mode)>(0x1015f140);
	ui_show_worldmap(mode);
	mImpl->Show(mode); 
}

void UiWorldmap::Hide(){
	static auto ui_hide_worldmap = temple::GetRef<void(__cdecl)()>(0x1015E210);
	mImpl->mWnd->Hide();
	return ui_hide_worldmap();
}

/* 0x10160450 */
void UiWorldmap::TravelToArea(int area)
{
	wmImpl->mDialogueDestLocId = wmImpl->mAreaToLocId[area];
	Show(2);
}

bool UiWorldmap::NeedToClearEncounterMap(){
	static auto uiWorldmapNeedToClearEncounterMap = temple::GetRef<BOOL(__cdecl)()>(0x10159790);
	return (uiWorldmapNeedToClearEncounterMap() != FALSE) ? true: false;
}

// UI crash at exit fix
class UiExitCrashFix : TempleFix
{
public: 
	void apply() override 
	{
		int asdf;
		read(0x1015B3E8 + 2, &asdf, sizeof(int));
		
		write(0x1015B3F5 + 2, &asdf, sizeof(int));// need to free the "centered on party" button; Spellslinger forgot to change this
		//read(0x1015B3F5 + 2, &asdf, sizeof(int));

	}
} uiExitCrashFix;

UiWorldmap &ui_worldmap() {
	return uiSystems->GetWorldmap();
}

void UiWorldmapMakeTripWrapper();

// Worldmap extension by Spellslinger
class WorldmapFix : TempleFix
{
public:
	static int CanAccessWorldmap();
	static BOOL UiWorldmapMakeTripCdecl(int fromId, int toId);

	void apply() override {
		// UiWorldmapTravelByDialogue
		replaceFunction<void(int)>(0x10160450, [](int destArea) {
			ui_worldmap().TravelToArea(destArea);
		});

		
		if (modSupport.IsCo8())
		{
			writeNoops(0x10159ABC); // This fixes an out of bounds write caused by spell slinger hacks
		}
		replaceFunction<void()>(0x1015EA20, UiWorldmapMakeTripWrapper);
		
		// UiWorldmapAcquiredLocationBtnMsg 
		replaceFunction<BOOL(__cdecl)(LgcyWidgetId, TigMsg *)>(0x1015F320, [](LgcyWidgetId widId, TigMsg *msg){
			return wmImpl->AcquiredLocationBtnMsg(widId, msg) ? TRUE: FALSE;
		});

		replaceFunction<int(__cdecl)(int)>(0x1006EC30, [](int mapId)->int {
			return wmImpl->GetAreaFromMap(mapId);
		});

		replaceFunction<BOOL(__cdecl)()>(0x1015E840, []()->BOOL {
			return wmImpl->OnDestinationReached() ? TRUE : FALSE;
		});

		replaceFunction<BOOL(__cdecl)(UiSaveFile&)>(0x1015E0F0, [](UiSaveFile& save)->BOOL {
			return wmImpl->LoadGame(save) ? TRUE:FALSE;
		});
	}

} worldmapFix;

/* 0x1006EC30 */
int UiWorldmapImpl::GetAreaFromMap(MapId mapId)
{

	if (mIsCo8) {
		if (mapId == 5078 && maps.IsCurrentMapOutdoor()) {
			// Fix for Livonya's slaughtered caravan mod in Co8 5.0.2+
			// Co8 has changed its map to 5078, but the area is still defined as 4
			// This causes you to start map navigation as if on the Temple, which can cause a hang if you go on to Nulb (since it's a short path)
			return 0; // set to random enc map
		}
	}

	int result;
	switch (mapId)
	{
	case 5000:
	case 5001:
	case 5006:
	case 5007:
	case 5008:
	case 5009:
	case 5010:
	case 5011:
	case 5012:
	case 5013:
	case 5014:
	case 5015:
	case 5016:
	case 5017:
	case 5018:
	case 5019:
	case 5020:
	case 5021:
	case 5022:
	case 5023:
	case 5024:
	case 5025:
	case 5026:
	case 5027:
	case 5028:
	case 5029:
	case 5030:
	case 5031:
	case 5032:
	case 5033:
	case 5034:
	case 5035:
	case 5036:
	case 5037:
	case 5038:
	case 5039:
	case 5040:
	case 5041:
	case 5042:
	case 5043:
	case 5044:
	case 5045:
	case 5046:
	case 5047:
	case 5048:
	case 5049:
	case 5063:
	case 5096:
	case 5097:
	case 5098:
	case 5099:
	case 5100:
	case 5101:
	case 5102:
	case 5103:
	case 5104:
	case 5115:
		result = 1;
		break;
	case 5002:
	case 5003:
	case 5004:
	case 5005:
		result = 2;
		break;
	case 5051:
	case 5052:
	case 5053:
	case 5054:
	case 5055:
	case 5056:
	case 5057:
	case 5058:
	case 5059:
	case 5060:
	case 5061:
	case 5085:
	case 5086:
	case 5087:
	case 5088:
		result = 3;
		break;
	case 5062:
	case 5064:
	case 5065:
	case 5066:
	case 5067:
	case 5078:
	case 5079:
	case 5080:
	case 5081:
	case 5082:
	case 5083:
	case 5084:
	case 5106:
		result = 4;
		break;
	case 5094:
		result = 5;
		break;
	case 5068:
		result = 6;
		break;
	case 5092:
	case 5093:
		result = 7;
		break;
	case 5091:
		result = 8;
		break;
	case 5095:
	case 5114:
		result = 9;
		break;
	case 5069:
		result = 10;
		break;
	case 5112:
		result = 11;
		break;
	case 5113:
		result = 12;
		break;

	// Co8 new areas
	case 5121:
		result = 14;
		break;
	case 5132:
		result = 15;
		break;
	case 5108:
		result = 16;
		break;
	default:
		result = 0;
		break;
	}

	return result;
}

int WorldmapFix::CanAccessWorldmap()
{
	signed int result;
	int v1;

	v1 = maps.GetCurrentMapId() - 5001;
	switch (v1)
	{
	case 120:
		result = 1;
		break;
	case 131:
		result = 1;
		break;
	case 107:
		result = 1;
		break;
	default:
		switch (v1)
		{
		case 0:
		case 1:
		case 50:
		case 61:
		case 67:
		case 68:
		case 90:
		case 92:
		case 93:
		case 94:
		case 111:
		case 112:
			result = 1;
			break;
		default:
			result = 0;
			break;
		}
		break;
	}
	return result;
}

BOOL WorldmapFix::UiWorldmapMakeTripCdecl(int fromId, int toId)
{
	return wmImpl->MakeTripFromAreaToArea(fromId, toId);
}

int convertSpecialId(int id) {
	if (id == 10 || id == 11)
		return 9;
	if (id == 12 || id == 13)
		return 6;
	return id;
};
void GetWorldXyById(int id, int& toX, int&toY){
	switch (id) {
	case 0:
		toX = temple::GetRef<int>(0x10BEF3B4);
		toY = temple::GetRef<int>(0x10BEF3B8);
		break;
	case 1:
		toX = temple::GetRef<int>(0x10BEF3E0);
		toY = temple::GetRef<int>(0x10BEF3E4);
		break;
	case 2:
		toX = temple::GetRef<int>(0x10BEF40C);
		toY = temple::GetRef<int>(0x10BEF410);
		break;
	case 3:
		toX = temple::GetRef<int>(0x10BEF438);
		toY = temple::GetRef<int>(0x10BEF43C);
		break;
	case 4:
		toX = temple::GetRef<int>(0x10BEF464);
		toY = temple::GetRef<int>(0x10BEF468);
		break;
	case 5:
		toX = temple::GetRef<int>(0x10BEF490);
		toY = temple::GetRef<int>(0x10BEF494);
		break;
	case 6:
		toX = temple::GetRef<int>(0x10BEF4BC);
		toY = temple::GetRef<int>(0x10BEF4C0);
		break;
	case 7:
		toX = temple::GetRef<int>(0x10BEF4E8);
		toY = temple::GetRef<int>(0x10BEF4EC);
		break;
	case 8:
		toX = temple::GetRef<int>(0x10BEF514);
		toY = temple::GetRef<int>(0x10BEF518);
		break;
	case 9:
		toX = temple::GetRef<int>(0x10BEF540);
		toY = temple::GetRef<int>(0x10BEF544);
		break;
	case 10:
		toX = temple::GetRef<int>(0x10BEF55C);
		toY = temple::GetRef<int>(0x10BEF560);
		break;
	case 11:
		toX = temple::GetRef<int>(0x10BEF578);
		toY = temple::GetRef<int>(0x10BEF57C);
		break;
	default:
		toX = toY = 0;
	}
}

/* 0x1015EA20 */
BOOL UiWorldmapImpl::MakeTripFromAreaToArea(int fromId, int toId) {
	auto &mTeleportMapId = temple::GetRef<int>(0x102FB3CC);
	auto &mToId = temple::GetRef<int>(0x102FB3E0);
	auto &mFromId = temple::GetRef<int>(0x102FB3E4);
	
	auto &mTrailDotInitialIdx = temple::GetRef<int>(0x102FB3DC);
	auto &mTrailDotCount = temple::GetRef<int>(0x10BEF7F8);
	auto &mXoffset = temple::GetRef<int>(0x102FB3EC);
	auto &mYoffset = temple::GetRef<int>(0x102FB3F0);
	auto &someX_10BEF7CC = temple::GetRef<int>(0x10BEF7CC);
	auto &someY_10BEF7D0 = temple::GetRef<int>(0x10BEF7D0);
	auto & mWorldmapWidgets = temple::GetRef<WorldmapWidgetsCo8*>(0x10BEF2D0);
	

	
	mTeleportMapId = mTeleportMapIdTable[toId];
	auto toIdOrg = toId;


	if ( fromId == -1 ){ // random encounter map
		toIdOrg = mToId;
	}
	else{
		mFromId = fromId = convertSpecialId(fromId);
		mToId = toIdOrg = toId = convertSpecialId(toId);
	}
	mIsMakingTrip = 1;
	
	fromId = convertSpecialId(fromId);
	toId = convertSpecialId(toId);
	
	auto toX = 0, toY = 0;
	GetWorldXyById(toId, toX, toY);
	
		

	// If exiting a random encounter and going to a different location than origin/prev. destination
	// Make a beeline path
	if (mRandomEncounterStatus == 2 && toId != toIdOrg && toId != mFromId ){
		auto fromX = mXoffset + someX_10BEF7CC + mRandomEncounterX + 10;
		auto fromY = mYoffset + someY_10BEF7D0 + mRandomEncounterY + 10;
		logger->info("Travelling on the map from {}, {} to {}, {}", fromX, fromY, toX, toY);


		auto& mTrailDotTextureW = temple::GetRef<int>(0x10BEF5C8);
		auto& mTrailDotTextureH = temple::GetRef<int>(0x10BEF5CC);

		auto deltaX = fromX - toX;
		auto deltaY = fromY - toY;
		mTrailDotCount = sqrt(deltaY*deltaY + deltaX * deltaX) / 12 + 1;
		if (mTrailDotCount >= TRAILDOT_MAX){ // fixes crash
			mTrailDotCount = 80;
		}
		auto sumX = 0, sumY = 0;
		for (auto i = 0; i <= mTrailDotCount; ++i){
			auto dotWidget = mWorldmapWidgets->trailDots[i];
			dotWidget->x = mTrailDotTextureW / -2
			- sumX / mTrailDotCount
			+ fromX + 9;
			dotWidget->y = mTrailDotTextureH / -2
				- sumY / mTrailDotCount
				+ fromY + 10;
			uiManager->SetHidden(dotWidget->widgetId, false);
			sumY += deltaY;
			sumX += deltaX;

		}
		mRandomEncounterStatus = 0;
		mFromId = -1;
		mToId = -1;
		mTrailDotInitialIdx = -1;
		return FALSE;
	}

	auto rePathForward = false, rePathBackward = false;
	auto endTrip = true;
	if (mWorldmapState != 1){
		endTrip = false;
		if (mWorldmapState == 2){
			if (fromId >= 1 || fromId <= 10) // original code... bug??
				endTrip = false;
			else{
				endTrip = true;
			}
		}
		else if (mRandomEncounterStatus == 2){
			if (toId == mFromId){
				fromId = toIdOrg;
				rePathBackward = true;
			}
			else if (toId == toIdOrg){
				fromId = mFromId;
				rePathForward = true;
			}
		}
	}

	
	if (!endTrip){
		auto pathId_raw = mPathIdTable[toId + mPathIdTableWidth * fromId];
		logger->info("***************************from {} to {}", fromId, toId);

		if (!(pathId_raw > -mPathIdMax && pathId_raw <= mPathIdMax)){
			endTrip = true;
			logger->info("Bad pathId, setting to no path");
		};
		if (pathId_raw == mPathIdMax){
			endTrip = true;
		}
		else{
			auto deltaX = 14;
			auto deltaY = 10;
			logger->info(" ************************ path id : {}", pathId_raw);
			auto pathId = pathId_raw - 1;
			
			if (pathId_raw > 0) {
				logger->info(" ************************ path id changed to: {}", pathId);
				auto &wmPath = mPaths[pathId];
				mTrailDotCount = wmPath.count / 12;

				auto traildot = mWorldmapWidgets->trailDots[0];
				traildot->x = wmPath.startX + deltaX;
				traildot->y = wmPath.startY + deltaY;
				auto dotIdx = 1;
				for (auto i = 1; i <= wmPath.directions.size(); ++i) {
					traildot = mWorldmapWidgets->trailDots[dotIdx];
					switch (wmPath.directions[i - 1]) {
					case 5:
						--deltaY; break;
					case 7:
						--deltaX; break;
					case 8:
						++deltaX; break;
					case 9:
						--deltaX; --deltaY; break;
					case 10:
						++deltaX; --deltaY; break;
					case 11:
						--deltaX; ++deltaY; break;
					case 12:
						++deltaX; ++deltaY; break;
					case 6:
						++deltaY; break;
					default:
						break;
					}
					if (!(i % 12)) {
						traildot->x = deltaX + wmPath.startX;
						traildot->y = deltaY + wmPath.startY;
						dotIdx++;
					}
					if (dotIdx >= TRAILDOT_MAX){
						break;
					}
						
				}
			}
			else{
				pathId = -1 - pathId_raw;
				logger->info(" ************************ path id : {}", pathId);

				if (pathId < 0) {
					logger->info(" ************************ BAD PATH reseting to 0...path id : {}", pathId);
					pathId = 0;
				}

				auto &wmPath = mPaths[pathId];
				mTrailDotCount = wmPath.count / 12;

				auto traildot = mWorldmapWidgets->trailDots[mTrailDotCount];
				traildot->x = wmPath.startX + deltaX;
				traildot->y = wmPath.startY + deltaY;
				auto dotIdx = 1;
				for (auto i = 1; i <= wmPath.directions.size(); ++i) {
					traildot = mWorldmapWidgets->trailDots[mTrailDotCount- dotIdx];
					switch (wmPath.directions[i - 1]) {
					case 5:
						--deltaY; break;
					case 7:
						--deltaX; break;
					case 8:
						++deltaX; break;
					case 9:
						--deltaX; --deltaY; break;
					case 10:
						++deltaX; --deltaY; break;
					case 11:
						--deltaX; ++deltaY; break;
					case 12:
						++deltaX; ++deltaY; break;
					case 6:
						++deltaY; break;
					default:
						break;
					}
					if (!(i % 12)) {
						traildot->x = deltaX + wmPath.startX;
						traildot->y = deltaY + wmPath.startY;
						++dotIdx;
					}
					if (dotIdx >= TRAILDOT_MAX) {
						break;
					}
				}
			}


			

			if (rePathForward){
				auto dotDelta = mTrailDotCount - mTrailDotInitialIdx;
				mTrailDotCount = dotDelta;
				auto dotCount = dotDelta + 1;
				for (auto i =0; i < dotCount; ++i){
					auto dotsDest = mWorldmapWidgets->trailDots[i];
					auto dotsSrc = mWorldmapWidgets->trailDots[mTrailDotInitialIdx + i];
					dotsDest->x = dotsSrc->x;
					dotsDest->y = dotsSrc->y;
				}
			}
			else if (rePathBackward){
				auto dotDelta = mTrailDotCount - mTrailDotInitialIdx;
				mTrailDotCount = mTrailDotInitialIdx;
				auto dotCount = mTrailDotInitialIdx + 1;
				for (auto i = 0; i < dotCount; ++i) {
					auto dotsDest = mWorldmapWidgets->trailDots[i];
					auto dotsSrc = mWorldmapWidgets->trailDots[mTrailDotInitialIdx + i];
					dotsDest->x = dotsSrc->x;
					dotsDest->y = dotsSrc->y;
				}
			}

			return FALSE;
		}
	}

	if (OnDestinationReached()){
		return TRUE;
	}
	mIsMakingTrip = FALSE;
	return FALSE;
}

UiWorldmapImpl::UiWorldmapImpl(){

	mIsCo8 = temple::Dll::GetInstance().IsExpandedWorldmapDll();

	InitLocations();
	
	for (auto locId = 0; locId < locationProps.size(); ++locId) {
		
		if (locationProps[locId].knownByDefault) {
			LocationVisited locVisit;
			locVisit.locId = locId;
			locVisit.flags = LVF_LocationVisible | LVF_ScriptVisible; 
			locationsVisited.push_back(locVisit);
			// uiManager->SetHidden_Safe(locWidgets[locId].locationRingBtn, false); // TODO
		}
	}

	LoadPaths();

	for (auto i = 0; i < (mIsCo8 ? ACQUIRED_LOCATIONS_CO8 : ACQUIRED_LOCATIONS_VANILLA); ++i) {
		locWidgets.push_back(LocationWidgets());
	}
	
	InitWidgets(config.renderWidth, config.renderHeight);
	UpdateWidgets();
	
}

void UiWorldmapImpl::Show(int state) {
	
	mWorldmapState = state;
	mIsVisible = TRUE;

	mWnd->Show();
	((WidgetButton*)uiManager->GetAdvancedWidget(mCurrentMapBtn))->SetDisabled(mWorldmapState == 3);
	((WidgetButton*)uiManager->GetAdvancedWidget(mCenterOnPartyBtn))->SetDisabled(mWorldmapState == 3);
	
}

/* 0x1015E0F0 */
bool UiWorldmapImpl::LoadGame(const UiSaveFile& save)
{
	// Co8: 4 * (1 + ACQUIRED_LOCATIONS_CO8 + 5)
	// Vanilla: 4 * (1 + ACQUIRED_LOCATIONS_VANILLA + 5)

	if (tio_fread(&mLocationsVisitedCount, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read mLocationsVisitedCount");
		return false;
	}

	auto isCo8Save = false;
	auto curpos = vfs->Tell(save.file);
	auto remaining = vfs->Length(save.file) - curpos;
	if (remaining >= sizeof(int) * (ACQUIRED_LOCATIONS_CO8 + 5 + 1) ){
		vfs->Seek(save.file, 4 * (20 + 5), SeekDir::Current);
		int sentinelCheck;
		vfs->Read(&sentinelCheck, sizeof(int), save.file);
		if (sentinelCheck == 0xBEEFCAFE) {
			isCo8Save = true;
		}
		vfs->Seek(save.file, -4 * (20 + 5 + 1), SeekDir::Current);
	}

	if (mIsCo8) {
		// todo check if you can detect vanilla save and modify this...
		auto& mLocationsVisitedFlags = temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x11EA4800);
		if (tio_fread(&mLocationsVisitedFlags, 20*4, 1, save.file) != 1) {
			logger->error("UiWorldmap: failed to read mLocationsVisitedFlags");
			return false;
		}
		locationsVisited.clear();
		for (auto i = 0; i < mLocationsVisitedCount; ++i) {
			LocationVisited locVis;
			locVis.locId = mLocationsVisitedFlags[i] >> 8;
			locVis.flags = mLocationsVisitedFlags[i] & 0xFF;
			locationsVisited.push_back(locVis);
		}
	}
	else {
		auto& mLocationsVisitedFlags = temple::GetRef<int[ACQUIRED_LOCATIONS_VANILLA]>(0x10BEF78C);
		if (tio_fread(&mLocationsVisitedFlags, ACQUIRED_LOCATIONS_VANILLA*4, 1, save.file) != 1) {
			logger->error("UiWorldmap: failed to read mLocationsVisitedFlags");
			return false;
		}
		locationsVisited.clear();
		for (auto i = 0; i < mLocationsVisitedCount; ++i) {
			LocationVisited locVis;
			locVis.locId = mLocationsVisitedFlags[i] >> 8;
			locVis.flags = mLocationsVisitedFlags[i] & 0xFF;
			locationsVisited.push_back(locVis);
		}
	}
	temple::GetRef<void(__cdecl)()>(0x1015DFD0)(); // UnhideLocations
	UpdateAcquiredLocationsBtns();

	if (tio_fread(&mRandomEncounterX, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read ");
		return false;
	}

	if (tio_fread(&mRandomEncounterY, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read ");
		return false;
	}

	if (tio_fread(&mRandomEncounterStatus, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read ");
		return false;
	}

	if (tio_fread(&mNeedToClearEncounterMap, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read ");
		return false;
	}

	if (tio_fread(&mPermanentPermissionToExitEncounterMap, 4, 1, save.file) != 1) {
		logger->error("UiWorldmap: failed to read ");
		return false;
	}

	return true;
}

// 0x1015E840
bool UiWorldmapImpl::OnDestinationReached()
{
	//rndEncDetails = nullptr;
	auto &mRndEncDetails = temple::GetRef<void*>(0x10BEF814);
	mRndEncDetails = nullptr;

	mNeedToClearEncounterMap = FALSE;
	FadeAndTeleportArgs fadeArgs;
	auto &mapSystem = gameSystems->GetMap();
	

	auto locId = GetLocationFromMap(mTeleportMapId);
	if (locId != LOCATION_NONE) {
		auto wrappedMapId = mTeleportMapId % 10000;

		auto &locProps = locationProps[locId];
		if (locProps.startLoc.locx != -1 && locProps.startLoc.locy != -1) {
			fadeArgs.destLoc = locProps.startLoc;
		}
		else {
			fadeArgs.destLoc = mapSystem.GetStartPos(wrappedMapId);
		}
		mTeleportMapId = wrappedMapId;
	}
	else {
		logger->error("OnDestinationReached(): Could not find teleport map! Using hardcoded alternative.");

		if (mTeleportMapId == HOMMLET_MAIN) {
			fadeArgs.destLoc = { 623, 421 };
		}
		else if (mTeleportMapId == HOMMLET_NORTH) {
			fadeArgs.destLoc = { 512, 221 };
			mTeleportMapId = HOMMLET_MAIN;
		}
		else if (mTeleportMapId == HOMMLET_CASTLE) {
			fadeArgs.destLoc = { 433, 626 };
			mTeleportMapId = HOMMLET_MAIN;
		}
		else {
			fadeArgs.destLoc = mapSystem.GetStartPos(mTeleportMapId);
		}
	}
	
	

	
	fadeArgs.flags = FadeAndTeleportFlags::ftf_unk200;
	fadeArgs.somehandle = party.GroupPCsGetMemberN(0);
	
	auto curMap = mapSystem.GetCurrentMapId();
	fadeArgs.destMap = (mTeleportMapId == curMap) ? -1 : mTeleportMapId;
	
	fadeArgs.movieId = mapSystem.GetEnterMovie(fadeArgs.destMap, false);
	if (fadeArgs.movieId){
		fadeArgs.flags |= FadeAndTeleportFlags::ftf_play_movie;
		fadeArgs.field20 = 0;
	}

	ui_worldmap().Hide();
	fade.FadeAndTeleport(fadeArgs);
	return FALSE;
}

void UiWorldmapImpl::InitLocations()
{

	const int MAX_NUM_AREAS = 64;
	const int MAX_NUM_LOCATIONS = 64;

	// Todo read this from file to support new modules
	mTeleportMapIdTable.resize(MAX_NUM_LOCATIONS, 0);
	mAreaToLocId.resize(MAX_NUM_AREAS, 0);

	if (mIsCo8) {
		memcpy(&mTeleportMapIdTable[0], temple::GetPointer(0x11EA3710), ACQUIRED_LOCATIONS_CO8 * sizeof(int));
		memcpy(&mAreaToLocId[0], temple::GetPointer(0x11EA3610), 64 * sizeof(int));
		// In Co8 this should be
		//mAreaToLocId = {
		//-1, // Area 0 - unknown area
		//9, // Area 1  - Hommlet
		//1, // Area 2 - Moathouse
		//3,
		//6,
		//0,
		//4,
		//7,
		//2,
		//5,
		//8,
		//12,
		//13,
		//0, // vanilla limit
		//14, // new Co8 areas
		//15,
		//16,
		//};
	}
	else {
		memcpy(&mTeleportMapIdTable[0], temple::GetPointer(0x102972BC), ACQUIRED_LOCATIONS_VANILLA * sizeof(int));
		memcpy(&mAreaToLocId[0], temple::GetPointer(0x10297250), 13 * sizeof(int));
	}
	

	locToMapId = { // Co8 setup
		5094,  //0
		5002,
		5091,
		5051,
		5068,
		5095,  // 5
		5062,
		5093,
		5069,
		HOMMLET_NORTH,
		HOMMLET_MAIN, // 10
		HOMMLET_CASTLE,
		5112,
		5113,
		// new Co8 areas:
		5121,  // 14
		5132,  // 15
		5108   // 16
	};
	locToMapId.resize(MAX_NUM_LOCATIONS, 0);
	
	for (auto i = 0; i < MAX_NUM_LOCATIONS; ++i) {
		auto props = LocationProps();
		props.mapId = locToMapId[i];
		locationProps.push_back(props);
	}
	
	// Hommlet special locations
	for (auto locId = 9; locId <= 11; locId++) {
		locationProps[locId].knownByDefault = true;
	}
	locationProps[9].startLoc.locx = temple::GetRef<int>(0x1015E8AB + 1);
	locationProps[9].startLoc.locy = temple::GetRef<int>(0x1015E8B0 + 1);

	locationProps[10].startLoc.locx = temple::GetRef<int>(0x1015E8B7 + 1);
	locationProps[10].startLoc.locy = temple::GetRef<int>(0x1015E8BC + 1);

	locationProps[11].startLoc.locx = temple::GetRef<int>(0x1015E89F + 1);
	locationProps[11].startLoc.locy = temple::GetRef<int>(0x1015E8A4 + 1);

	ReadLocationNames();
}

// 0x1015DE80
void UiWorldmapImpl::ReadLocationNames()
{
	auto mesContent = MesFile::ParseFile("mes/worldmap_location_names_text.mes");
	auto count = 0;
	
	for (auto &it : mesContent) {
		if (count >= locationProps.size()) {
			break;
		}
		it.second;
		locationProps[count].name = it.second;
		count++;
	}
}

void UiWorldmapImpl::InitWidgets(int w, int h)
{
	std::string basePath ("art/interface/worldmap_ui/");
	std::string townmapPath("art/interface/townmap_ui/");

	mLargeTexturesMes = MesFile::ParseFile(basePath + "0_worldmap_ui_large_textures.mes");
	auto townmapTexts = MesFile::ParseFile("mes/townmap_ui_text.mes");
	GetUiLocations();
	
	// TODO
	auto refX = 0, refY = 0;

	auto getLoc = [&](int idx)->int {
		return std::stol(mWidgetLocations[idx]);
	};
	refX = getLoc(10); refY = getLoc(11);

	auto SetWidFromMes = [&](WidgetBase& widg, int baseIdx)->void { // used for the buttons, since the mesfile coords are absolute but our widget system uses relative coords
		//widg.SetAutoSizeHeight(false); widg.SetAutoSizeWidth(false);
		//widg.SetMargins({ 0,0,0,0 });
		widg.SetPos(getLoc(baseIdx + 0), getLoc(baseIdx + 1) );
		widg.SetSize({ getLoc(baseIdx + 2), getLoc(baseIdx + 3) });
	};
	auto AdjChildPos = [&](WidgetBase& widg, WidgetBase& parent) {
		widg.SetX(widg.GetX() - parent.GetX());
		widg.SetY(widg.GetY() - parent.GetY());
	};
	
	// TODO tooltips

	
	// Main Window
	{
		mWnd = std::make_unique<WidgetContainer>(0,0);
		
		mWnd->SetSize({ getLoc(12), getLoc(13) });
		SetWidFromMes(*mWnd, 10);
		auto mainBg = std::make_unique<WidgetImage>(basePath + mLargeTexturesMes[10]);
		mWnd->AddContent(std::move(mainBg));
		mWnd->SetUpdateTimeMsgHandler([](uint32_t) {
			auto handler = temple::GetRef<BOOL(__cdecl)(int widId, TigMsg& msg)>(0x1015E990);
			handler(-1, TigMsg()); // args aren't used in practice
			});
		mWnd->SetKeyStateChangeHandler([&](const TigKeyStateChangeMsg& msg)->bool {
			if (mIsMakingTrip) {
				return true;
			}
			if (msg.key == DIK_ESCAPE && !msg.down) {
				ui_worldmap().Hide();
			}
			return true;
			});
	}
	
	// Exit Button
	{
		auto exitBtn = std::make_unique<WidgetButton>();
		
		SetWidFromMes(*exitBtn, 20);
		exitBtn->SetStyle("main-exit-button");
		exitBtn->SetClickHandler([&](int x, int y) {
			if (mIsMakingTrip)
				return false;
			ui_worldmap().Hide();
			});
		AdjChildPos(*exitBtn, *mWnd);
		mWnd->Add(std::move(exitBtn));
	}

	// "World Map" Button
	{
		auto wmBtn = std::make_unique<WidgetButton>();

		SetWidFromMes(*wmBtn, 30);
		auto myStyle = wmBtn->GetStyle();
		wmBtn->SetStyle("worldmap-worldmap-button"); // TODO the vanilla text had kerning = 0 which gave it different look
		wmBtn->SetText(townmapTexts[11]);
		AdjChildPos(*wmBtn, *mWnd);
		mWnd->Add(std::move(wmBtn));
	}
	// "Current Map" Button
	{
		auto btn = std::make_unique<WidgetButton>();

		SetWidFromMes(*btn, 40);
		btn->SetStyle("worldmap-currentmap-button");
		btn->SetText(townmapTexts[12]);
		btn->SetWidgetMsgHandler([&](const TigMsgWidget& msg) ->bool{
			if (mWorldmapState == 3) {
				return true;
			}
			if (msg.widgetEventType == TigMsgWidgetEvent::MouseReleased) {
				if (mIsMakingTrip) {
					return true;
				}
				ui_worldmap().Hide();
				uiSystems->GetTownmap().Show();
				return true;
			}
			return false;
			});
		mCurrentMapBtn = btn->GetWidgetId();
		AdjChildPos(*btn, *mWnd);
		mWnd->Add(std::move(btn));
	}
	
	// Selection window (acquired locations text entries)
	{
		auto wnd = std::make_unique<WidgetContainer>(0,0);
		SetWidFromMes(*wnd, 60);
		
		
		auto btnSpacing = getLoc(79);
		auto btnHeight = getLoc(78);
		for (auto i = 0u; i < locWidgets.size(); ++i) {
			auto btn = std::make_unique<WidgetButton>();
			auto btnId = btn->GetWidgetId();
			// btn->SetText("East Hommlet");
			acquiredLocationBtns.push_back(btnId);

			SetWidFromMes(*btn, 75);
			btn->SetMargins({ 10,0,0,0 });
			btn->SetY(btn->GetY() + i * (btnSpacing + btnHeight));
			btn->SetStyle("worldmap-acquired-location");
			
			btn->SetWidgetMsgHandler([&, btnId](const TigMsgWidget& msg) ->bool {
				return AcquiredLocationBtnMsg(btnId, (TigMsg*) &msg);
				});
			AdjChildPos(*btn, *wnd);
			wnd->Add(std::move(btn));
		}
		AdjChildPos(*wnd, *mWnd);
		mWnd->Add(std::move(wnd));
	}
	
	// Center on party button
	{
		auto btn = std::make_unique<WidgetButton>();

		SetWidFromMes(*btn, 90);
		btn->SetStyle("worldmap-center-on-party-button");
		btn->SetWidgetMsgHandler([&](const TigMsgWidget& msg) ->bool {
			if (mWorldmapState == 3) {
				return true;
			}
			if (msg.widgetEventType == TigMsgWidgetEvent::MouseReleased) {
				if (!mIsMakingTrip) {
					ui_worldmap().Hide();
					uiSystems->GetTownmap().Show();
				}
				temple::GetRef<void(__cdecl)(int, const TigMsgWidget&)>(0x1012C660)(-1, msg); // UiTownmap center on party
				return true;
			}
			return false;
			});
		mCenterOnPartyBtn = btn->GetWidgetId();
		AdjChildPos(*btn, *mWnd);
		mWnd->Add(std::move(btn));
	}


	mWnd->SetPos(getLoc(10) + (w - 800) / 2, getLoc(11) + (h - 600) / 2);
}

void UiWorldmapImpl::GetUiLocations()
{
	// Todo: separate this file to modular locations that can have satellite locations
	mWidgetLocations = MesFile::ParseFile("art/interface/worldmap_ui/0_worldmap_ui_locations.mes");

}

void UiWorldmapImpl::UpdateWidgets()
{
	UpdateAcquiredLocationsBtns();
}

void UiWorldmapImpl::LoadPaths()
{
	auto rawData (vfs->ReadAsBinary("art\\interface\\worldmap_ui\\worldmap_ui_paths.bin"));
	MemoryInputStream reader(rawData);
	
	auto numPaths = reader.ReadInt32();
	logger->info("Loading worldmap paths ({})", numPaths);
	try {
		while (!reader.AtEnd()) {
			WorldmapPath wmp;
			wmp.startX = reader.ReadInt32();
			wmp.startY = reader.ReadInt32();
			wmp.endX = reader.ReadInt32();
			wmp.endY = reader.ReadInt32();
			wmp.count = reader.ReadInt32();

			if ((wmp.count % 4) > 0) {
				wmp.count += 4 - (wmp.count % 4);
			}
			wmp.directions.resize(wmp.count);
			reader.ReadBytes(&wmp.directions[0], wmp.count);
			mPaths.push_back(wmp);
		}
	}
	catch(...) {

	}
	logger->info("Loaded {} worldmap paths", mPaths.size());


	if (mIsCo8) {
		mPathIdMax = 191;
		mPathIdTableWidth = 20;
		mPathIdTable.resize(mPathIdTableWidth * mPathIdTableWidth);
		memcpy(&mPathIdTable[0], temple::GetPointer(0x11EA4100), mPathIdTable.size() * sizeof(int) );
	}
	else { // vanilla
		mPathIdMax = 46;
		mPathIdTableWidth = 12;
		mPathIdTable.resize(mPathIdTableWidth * mPathIdTableWidth);
		memcpy(&mPathIdTable[0], temple::GetPointer(0x102972F8), mPathIdTable.size() * sizeof(int));
	}
}

LocationId UiWorldmapImpl::GetLocationFromMap(MapId mapId){
	for (auto i = 0; i < locationProps.size(); ++i) {
		auto& locProps = locationProps[i];
		if (locProps.mapId == mapId) {
			return (LocationId)i;
		}
	}
	return LOCATION_NONE;
}

LocationId UiWorldmapImpl::GetLocationFromArea(AreaId areaId) {
	for (auto i = 0; i < mAreaToLocId.size(); ++i) {
		if (mAreaToLocId[i] == areaId) {
			return (LocationId )i;
		}
	}
	return LOCATION_NONE;
}

int UiWorldmapImpl::GetLocationVisited(LocationId locId)
{
	for (auto i = 0; i < locationsVisited.size(); ++i) {
		if (locationsVisited[i].locId == locId) {
			return i;
		}
	}
	return LOCATION_NONE;
}

/* 0x101596A0 */
void UiWorldmapImpl::SetMapVisited(MapId mapId)
{
	auto locId = GetLocationFromMap(mapId);
	SetLocationVisited(locId);
}
// refactored from SetLocationVisited
void UiWorldmapImpl::SetLocationVisited(LocationId locId)
{
	if (locId == LOCATION_NONE || locId >= locationProps.size()) {
		return;
	}
	LocationProps& locProps = locationProps[locId];
	if (locProps.knownByDefault)
		return;
	
	auto& locState = locationStates[locId];
	if (locState.scriptWriting == MV_VISIBLE) {
		return;
	}

	MarkAreaKnown(locProps.areaId);

	auto locVisited = GetLocationVisited(locId);
	if (locVisited == LOCATION_NONE) {
		return;
	}
	locationsVisited[locVisited].flags |= LVF_ScriptVisible;
	locState.scriptWriting = MV_FADING;
	
	auto btnId = locWidgets[locId].scriptBtn;
	if (uiManager->GetButton(btnId)) {
		uiManager->SetHidden(btnId, false);
	}
	
}

/* 0x101595E0 */
void UiWorldmapImpl::MarkAreaKnown(AreaId areaId)
{
	auto locId = GetLocationFromArea(areaId);
	if (locId == LOCATION_NONE || locId >= locationProps.size()) {
		return;
	}
	LocationProps& locProps = locationProps[locId];
	if (locProps.knownByDefault)
		return;

	auto& locState = locationStates[locId];
	if (locState.mapBtn == MV_VISIBLE) {
		return;
	}

	auto locVisited = GetLocationVisited(locId);
	if (locVisited != LOCATION_NONE) {
		return;
	}

	// Add new location
	mLocationsRevealingCount++;
	
	auto locVisitedNew = LocationVisited();
	locVisitedNew.locId = locId;
	locVisitedNew.flags = LVF_LocationVisible;
	
	locationsVisited.push_back(locVisitedNew);

	locState.mapBtn = MV_FADING;
	uiManager->SetHidden_Safe(locWidgets[locId].locationBtn, false);
	uiManager->SetHidden_Safe(locWidgets[locId].locationRingBtn, false);
	
	uiSystems->GetUtilityBar().FlashMapBtn();

}

/* 0x1015DFD0 */
void UiWorldmapImpl::UnhideLocationBtns()
{
	for (auto i = 0; i < locationsVisited.size(); ++i) {
		auto locVis = locationsVisited[i];
		auto locId = locVis.locId;
		
		auto& widg = locWidgets[locId];
		auto& locStat = locationStates[locId];
		if (locVis.flags & LVF_LocationVisible) {
			uiManager->SetHidden_Safe(widg.locationBtn, false);
			uiManager->SetHidden_Safe(widg.locationRingBtn, false);
			locStat.mapBtn = MV_VISIBLE;
		}
		if (locVis.flags & LVF_ScriptVisible) {
			uiManager->SetHidden_Safe(widg.scriptBtn, false);
			locStat.scriptWriting = MV_VISIBLE;
		}
		
	}
}

void UiWorldmapImpl::UpdateAcquiredLocationsBtns()
{
	for (auto i = 0; i < locationsVisited.size() && i < acquiredLocationBtns.size(); ++i) {
		auto locVis = locationsVisited[i];
		
		

		auto widId = acquiredLocationBtns[i];
		auto widget = (WidgetButton*)uiManager->GetAdvancedWidget(widId);
		
		uiManager->SetHidden_Safe(widId, false);

		auto locId = locVis.locId;
		if (locId >=0 && locId < locationProps.size()) {
			auto& locProps = locationProps[locId];
			widget->SetText(locProps.name);
		}
		else {
			widget->SetText(fmt::format("BAD LOCATION ID: {}", locId));
		}

	}

}

bool UiWorldmapImpl::AcquiredLocationBtnMsg(LgcyWidgetId widId, TigMsg* msg)
{
	if (msg->type == TigMsgType::MOUSE) {
		return TRUE;
	}
	if (!msg->IsWidgetEvent(TigMsgWidgetEvent::MouseReleased)) {
		return FALSE;
	}

	auto& mWorldmapWidgets = temple::GetRef<WorldmapWidgetsCo8*>(0x10BEF2D0);
	auto& mLocationsVisitedCount = temple::GetRef<int>(0x10BEF810);
	auto mLocationVisitedFlags = mIsCo8 ? 
		temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x11EA4800) :
		temple::GetRef<int[ACQUIRED_LOCATIONS_VANILLA]>(0x10BEF78C);

	if (mIsMakingTrip) {
		return FALSE;
	}



	auto btn = uiManager->GetButton(widId);
	auto locIdx = -1;
	auto destLocId = LOCATION_NONE;
	auto foundLocIdx = false;
	// search for the button in the acquired locations buttons
	for (auto i = 0; i < ACQUIRED_LOCATIONS_CO8; ++i) {
		if (btn == mWorldmapWidgets->acquiredLocations[i]) {
			locIdx = i; foundLocIdx = true;
			destLocId = mLocationVisitedFlags[locIdx] >> 8;
			break;
		}
	}
	// Worldmap replacement:
	for (auto i = 0; i < acquiredLocationBtns.size(); ++i) {
		if (widId == acquiredLocationBtns[i]) {
			locIdx = i; foundLocIdx = true;
			destLocId = locationsVisited[locIdx].locId;
		}
	}

	if (!foundLocIdx) { // search for the button in the location ring buttons
		auto locationRingIdx = -1;
		for (auto i = 0; i < ACQUIRED_LOCATIONS_CO8; ++i) {
			if (btn == mWorldmapWidgets->locationRingBtns[i]) {
				locationRingIdx = i;
				break;
			}
		}

		for (auto i = 0; i < mLocationsVisitedCount; ++i) {
			{
				auto locIdVisited = mLocationVisitedFlags[i] >> 8;
				if (locIdVisited == locationRingIdx) {
					locIdx = i;
					destLocId = mLocationVisitedFlags[locIdx] >> 8;
					break;
				}
			}
		}
	}
	if (locIdx == -1) {
		logger->error("UiWorldmap: Couldn't find location button??");
		return FALSE;
	}

	auto curLocId = temple::GetRef<int(__cdecl)()>(0x1015DF70)();

	MakeTripFromAreaToArea(curLocId, destLocId);
	return TRUE;
}

void __declspec(naked) UiWorldmapMakeTripWrapper() {
	macAsmProl; // esp = esp0 - 16
	__asm {
		push ecx; // esp = esp0 - 24
		push eax; // esp = esp0 - 20
		mov esi, WorldmapFix::UiWorldmapMakeTripCdecl;
		call esi;
		add esp, 8;
	}
	macAsmEpil;
	__asm retn;
};
