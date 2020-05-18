#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "maps.h"

#include "ui_worldmap.h"
#include "ui_systems.h"

#include <temple/dll.h>
#include "tig/tig_msg.h"
#include "fade.h"
#include "party.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "mod_support.h"

//*****************************************************************************
//* Worldmap-UI
//*****************************************************************************

constexpr int HOMMLET_MAIN = 5001;
constexpr int HOMMLET_NORTH = 15001;
constexpr int HOMMLET_CASTLE = 25001;

class UiWorldmapImpl {
	friend class UiWorldmap;
	friend class WorldmapFix;
public:
	UiWorldmapImpl();

	bool OnDestinationReached();

	std::vector<int> areaToLocId;

	int &mNeedToClearEncounterMap = temple::GetRef<int>(0x10BEF800);
	int &mTeleportMapId = temple::GetRef<int>(0x102FB3CC);
};

constexpr int ACQUIRED_LOCATIONS_CO8 = 21;
struct WorldmapWidgetsCo8
{
	LgcyWindow * mainWnd;
	LgcyButton * worldmapBtn;
	LgcyButton * curMapBtn;
	LgcyButton * locationBtns[14];
	LgcyButton * locationRingBtns[14];
	LgcyButton * scriptBtns[14];
	LgcyButton * URHereBtn;
	LgcyButton * trailDots[80];
	LgcyButton * exitBtn;
	LgcyWindow * selectionWnd;
	LgcyButton * acquiredLocations[ACQUIRED_LOCATIONS_CO8];
	LgcyButton * centerOnPartyBtn;
};
struct WorldmapPath
{
	int startX;
	int startY;
	int unk8;
	int unkC;
	int count;
	char* directions;
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
	auto load = temple::GetPointer<int(const UiSaveFile*)>(0x1015e0f0);
	return load(&save) == 1;
}
const std::string &UiWorldmap::GetName() const {
	static std::string name("Worldmap-UI");
	return name;
}

void UiWorldmap::Show(int mode)
{
	static auto ui_show_worldmap = temple::GetPointer<void(int mode)>(0x1015f140);
	ui_show_worldmap(mode);
}

void UiWorldmap::Hide(){
	static auto ui_hide_worldmap = temple::GetRef<void(__cdecl)()>(0x1015E210);
	return ui_hide_worldmap();
}

void UiWorldmap::TravelToArea(int area)
{
	static auto ui_worldmap_travel_by_dialog = temple::GetPointer<void(int)>(0x10160450);
	ui_worldmap_travel_by_dialog(area);
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
	static int GetAreaFromMap(int mapId);
	static int CanAccessWorldmap();
	static BOOL UiWorldmapMakeTripCdecl(int fromId, int toId);

	void apply() override {
		replaceFunction<void(int)>(0x10160450, [](int destArea) {
			auto &uiWorldmapDestLocId = temple::GetRef<int>(0x102FB3E8);
			//auto &uiWorldmapAreaToLocId_Table = temple::GetRef<int[]>(0x11EA3610); // this is the Co8 location
			
			uiWorldmapDestLocId = wmImpl->areaToLocId[destArea];

			ui_worldmap().Show(2);
		});

		// This fixes an out of bounds write caused by spell slinger hacks
		writeNoops(0x10159ABC);


		if (modSupport.IsCo8()){
			replaceFunction<void()>(0x1015EA20, UiWorldmapMakeTripWrapper);
		}
		
		
		// UiWorldmapAcquiredLocationBtnMsg 
		static BOOL(__cdecl*orgUiWorldmapAcquiredLocationBtnMsg)(LgcyWidgetId, TigMsg *) = replaceFunction<BOOL(__cdecl)(LgcyWidgetId, TigMsg *)>(0x1015F320, [](LgcyWidgetId widId, TigMsg *msg){
			
			if (msg->type == TigMsgType::MOUSE){
				return TRUE;
			}
			if (!msg->IsWidgetEvent(TigMsgWidgetEvent::MouseReleased)){
				return FALSE;
			}

			if (!modSupport.IsCo8()) {
				return orgUiWorldmapAcquiredLocationBtnMsg(widId, msg);
			}

			auto & mIsMakingTrip = temple::GetRef<int>(0x10BEF7FC);
			auto & mWorldmapWidgets = temple::GetRef<WorldmapWidgetsCo8*>(0x10BEF2D0);
			auto & mLocationsVisitedCount = temple::GetRef<int>(0x10BEF810);
			auto & mLocationVisitedFlags = temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x11EA4800);
			
			if (mIsMakingTrip){
				return FALSE;
			}

			
			
			auto btn = uiManager->GetButton(widId);
			auto locIdx = -1;
			auto foundLocIdx = false;
			for (auto i = 0; i < ACQUIRED_LOCATIONS_CO8; ++i){
				if (btn == mWorldmapWidgets->acquiredLocations[i]){
					locIdx = i; foundLocIdx = true;
					break;
				}
			}
			if (!foundLocIdx){
				auto locationRingIdx = -1;
				for (auto i = 0; i < ACQUIRED_LOCATIONS_CO8; ++i){
					if (btn == mWorldmapWidgets->locationRingBtns[i]){
						locationRingIdx = i;
						break;
					}
				}
				for (auto i =0; i < mLocationsVisitedCount; ++i){
					//if (modSupport.IsCo8())
					{
						// auto & mLocationVisitedFlags = temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x11EA4800);
						if ((mLocationVisitedFlags[i] >> 8) == locationRingIdx) {
							locIdx = i;
							break;
						}
					}
					/*else{
						auto & mLocationVisitedFlags = temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x10BEF78C);
						if ((mLocationVisitedFlags[i] >> 8) == locationRingIdx) {
							locIdx = i;
							break;
						}
					}*/
					
				}
			}
			auto curLocId = temple::GetRef<int(__cdecl)()>(0x1015DF70)();

			return orgUiWorldmapAcquiredLocationBtnMsg(widId, msg);

			/*UiWorldmapMakeTripCdecl(curLocId, mLocationVisitedFlags[locIdx] >> 8);
			return TRUE;*/
		});
	}

} worldmapFix;

int WorldmapFix::GetAreaFromMap(int mapId)
{ // this might be co8 specific so not sure if to hook this
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

BOOL WorldmapFix::UiWorldmapMakeTripCdecl(int fromId, int toId) {
	auto &mTeleportMapId = temple::GetRef<int>(0x102FB3CC);
	auto &mTeleportMapIdTable = temple::GetRef<int[ACQUIRED_LOCATIONS_CO8]>(0x11EA3710);
	auto &mToId = temple::GetRef<int>(0x102FB3E0);
	auto &mFromId = temple::GetRef<int>(0x102FB3E4);
	auto & mIsMakingTrip = temple::GetRef<int>(0x10BEF7FC);
	auto &mRandomEncounterStatus = temple::GetRef<int>(0x102FB3D0);
	auto &mTrailDotInitialIdx = temple::GetRef<int>(0x102FB3DC);
	auto &mTrailDotCount = temple::GetRef<int>(0x10BEF7F8);
	auto &mXoffset = temple::GetRef<int>(0x102FB3EC);
	auto &mYoffset = temple::GetRef<int>(0x102FB3F0);
	auto &someX_10BEF7CC = temple::GetRef<int>(0x10BEF7CC);
	auto &someY_10BEF7D0 = temple::GetRef<int>(0x10BEF7D0);
	auto &mRandomEncounterX = temple::GetRef<int>(0x102FB3D4);
	auto &mRandomEncounterY = temple::GetRef<int>(0x102FB3D8);
	auto & mWorldmapWidgets = temple::GetRef<WorldmapWidgetsCo8*>(0x10BEF2D0);
	auto &mWorldmapState = temple::GetRef<int>(0x10BEF808);
	auto &mPaths = temple::GetRef<WorldmapPath[190]>(0x11EA2406);

	constexpr int PATH_ID_TABLE_WIDTH = 20;
	auto &mPathIdTable = temple::GetRef<int[PATH_ID_TABLE_WIDTH*PATH_ID_TABLE_WIDTH]>(0x11EA4100);

	mTeleportMapId = mTeleportMapIdTable[toId];
	auto toId_adj = toId;
	if ( fromId == -1 ){
		toId_adj = mToId;
	}
	else{
		mFromId = fromId;
		if (fromId == 10 || fromId == 11){
			mFromId = 9;
		}
		else if (fromId == 12 || fromId == 13){
			mFromId = 6;
		}
		toId_adj = toId;
		mToId = toId;
		if (toId == 10 || toId == 11 ){
			mToId = toId_adj = 9;
		}
		else if (toId == 12 || toId == 13){
			mToId = toId_adj = 6;
		}
	}
	mIsMakingTrip = 1;
	
	if (fromId == 10 || fromId == 11){
		fromId = 9;
	}
	else if (fromId == 12 || fromId == 13) {
		fromId = 6;
	}

	if (toId == 10 || toId == 11){
		toId = 9;
	}
	else if (toId == 12 || toId == 13) {
		toId = 6;
	}

	auto toX = 0, toY = 0;
	switch (toId){
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
		break;
	}

	// If exiting a random encounter
	if (mRandomEncounterStatus == 2 && toId != toId_adj && toId != mFromId ){
		auto fromX = mXoffset + someX_10BEF7CC + mRandomEncounterX + 10;
		auto fromY = mYoffset + someY_10BEF7D0 + mRandomEncounterY + 10;
		logger->info("Travelling on the map from {}, {} to {}, {}", fromX, fromY, toX, toY);


		auto& mTrailDotTextureW = temple::GetRef<int>(0x10BEF5C8);
		auto& mTrailDotTextureH = temple::GetRef<int>(0x10BEF5CC);

		auto deltaX = fromX - toX;
		auto deltaY = fromY - toY;
		mTrailDotCount = sqrt(deltaY*deltaY + deltaX * deltaX) / 12 + 1;
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

	auto v44 = false, v49 = false;
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
				fromId = toId_adj;
				v49 = true;
			}
			else if (toId == toId_adj){
				fromId = mFromId;
				v44 = true;
			}
		}
	}

	
	if (!endTrip){
		auto pathId_raw = mPathIdTable[toId + PATH_ID_TABLE_WIDTH * fromId];
		logger->info("***************************from {} to {}", fromId, toId);
		
		if (!(pathId_raw > -191 && pathId_raw <= 191)){
			endTrip = true;
			logger->info("Bad pathId, setting to no path");
		};
		if (pathId_raw == 191){
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
				for (auto i = 1; i < wmPath.count; ++i) {
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
				for (auto i = 1; i < wmPath.count; ++i) {
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

				}
			}


			

			if (v44){
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
			else if (v49){
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

	if (wmImpl->OnDestinationReached()){
		return TRUE;
	}
	mIsMakingTrip = FALSE;
	return FALSE;
}

UiWorldmapImpl::UiWorldmapImpl(){

	// Todo read this from file to support new modules
	areaToLocId.resize(64, 0);
	int i = 0;
	for (auto it : {
	-1,
	9,
	1, // Area 2 - Moathouse
	3,
	6,
	0,
	4,
	7,
	2,
	5,
	8,
	12,
	13,
	0,
	14,
	15,
	16,
		}){
		areaToLocId[i++] = it;
	}
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
	// Todo: modularize
	if (mTeleportMapId == HOMMLET_MAIN){
		fadeArgs.destLoc = { 623, 421 };
	}
	else if (mTeleportMapId == HOMMLET_NORTH){
		fadeArgs.destLoc = { 512, 221 };
		mTeleportMapId = HOMMLET_MAIN;
	}
	else if (mTeleportMapId == HOMMLET_CASTLE){
		fadeArgs.destLoc = { 433, 626 };
		mTeleportMapId = HOMMLET_MAIN;
	}
	else{
		fadeArgs.destLoc = mapSystem.GetStartPos(mTeleportMapId);
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
	//return temple::GetRef<BOOL(__cdecl)()>(0x1015E840)() != FALSE;
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
