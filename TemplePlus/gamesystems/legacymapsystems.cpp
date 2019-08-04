
#include "stdafx.h"
#include "config/config.h"
#include <infrastructure/exception.h>

#include "legacymapsystems.h"

#include "gamesystems/map/sector.h"
#include "gamesystems.h"
#include <tig/tig_tabparser.h>
#include "objects/objsystem.h"

//*****************************************************************************
//* Scroll
//*****************************************************************************

ScrollSystem::ScrollSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10005e70);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Scroll");
	}
}
ScrollSystem::~ScrollSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100056e0);
	shutdown();
}
void ScrollSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10005870);
	resetBuffers(&rebuildInfo);
}
void ScrollSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10005700);
	reset();
}
void ScrollSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x10006000);
	advanceTime(time);
}
const std::string &ScrollSystem::GetName() const {
	static std::string name("Scroll");
	return name;
}

void ScrollSystem::SetMapId(int mapId)
{
	static auto map_scroll_set_map_id = temple::GetPointer<void(int mapId)>(0x10005720);
	map_scroll_set_map_id(mapId);

	/*struct MapLimits{
		int left = 9000;
		int top = 0;
		int right = -9000;
		int bottom = -18000;
	};*/

	if (mapId == 5000)
		return;

	auto maplimitsMes = temple::GetRef<MesHandle>(0x102AC234);
	if (maplimitsMes == -1)
		return;

	auto &leftLim = temple::GetRef<int64_t>(0x10307340);
	auto &rightLim = temple::GetRef<int64_t>(0x103072F0);
	auto &topLim = temple::GetRef<int64_t>(0x10307368);
	auto &botLim = temple::GetRef<int64_t>(0x10307358);

	auto deltaW = leftLim - rightLim;
	auto deltaH = topLim - botLim;
	if (deltaW < config.renderWidth + 100){
		leftLim += (config.renderWidth - deltaW )/ 2 + 50;
		rightLim -= (config.renderWidth - deltaW ) / 2 + 50;
	}

	if (deltaH < config.renderHeight + 100) {
		topLim +=(config.renderHeight - deltaH )/ 2 + 50;
		botLim -= (config.renderHeight - deltaH )/ 2 + 50;
	}
}

//*****************************************************************************
//* Location
//*****************************************************************************

LocationSystem::LocationSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1002a9a0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Location");
	}
}
void LocationSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10028db0);
	resetBuffers(&rebuildInfo);
}
const std::string &LocationSystem::GetName() const {
	static std::string name("Location");
	return name;
}

void LocationSystem::CenterOn(int tileX, int tileY)
{
	auto centerOn = temple::GetPointer<void(int, int)>(0x1002A580);
	centerOn(tileX, tileY);
}

void LocationSystem::CenterOnSmooth(int tileX, int tileY) {

	auto map_location_get_translation_delta = temple::GetRef<BOOL(__cdecl)(int , int , int64_t &, int64_t&)>(0x10029810);
	int64_t deltax, deltay;

	map_location_get_translation_delta(tileX, tileY, deltax, deltay);

	auto scrollButter = temple::GetRef<int>(0x102AC238);

	if (scrollButter && sqrt(deltay*deltay + deltax*deltax) <= 2400.0){ // fixed: didn't check for non-zero scroll butter in vanilla, which would cause this thing to fail (i.e. when Scroll Acceleration was disabled)
		temple::GetRef<int>(0x10307338) = deltay; //map_scroll_rate_y
		temple::GetRef<int>(0x10307370) = deltax; //map_scroll_rate_x
	}
	else {
		gameSystems->GetLocation().CenterOn(tileX, tileY);
	}
}

void LocationSystem::SetLimits(uint64_t limitX, uint64_t limitY)
{
	static auto location_set_limits = temple::GetPointer<BOOL(uint64_t, uint64_t)>(0x1002a8f0);
	location_set_limits(limitX, limitY);
}

locXY LocationSystem::GetLimitsCenter()
{
	static auto GetLocationLimitCenterXY = temple::GetPointer<uint64_t()>(0x1002a170);
	return locXY::fromField(GetLocationLimitCenterXY());
}

//*****************************************************************************
//* Light
//*****************************************************************************

LightSystem::LightSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a7d40);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Light");
	}
}
LightSystem::~LightSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100a7f80);
	shutdown();
}
void LightSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x100a5b30);
	resetBuffers(&rebuildInfo);
}
const std::string &LightSystem::GetName() const {
	static std::string name("Light");
	return name;
}

void LightSystem::Load(const std::string & dataDir)
{
	static auto sector_globallight_load = temple::GetPointer<void(const char *)>(0x100a7860);	
	sector_globallight_load(dataDir.c_str());
}

void LightSystem::SetMapId(int mapId)
{
	static auto map_daylight_loadmap = temple::GetPointer<void(int mapId)>(0x100a7040);
	map_daylight_loadmap(mapId);
}

//*****************************************************************************
//* Tile
//*****************************************************************************

TileSystem::TileSystem() {
	// Was previously 0x100ab590 (see class comment)
}
TileSystem::~TileSystem() {
	// Was previously 0x100ac8a0 (see class comment)
}
const std::string &TileSystem::GetName() const {
	static std::string name("Tile");
	return name;
}

SectorTile TileSystem::GetTile(locXY location) {
	static auto map_tile_get_tile = temple::GetPointer<SectorTile *(int x, int y)>(0x100ab810);
	auto tile = map_tile_get_tile(location.locx, location.locy);
	if (tile) {
		return *tile;
	}
	return { TileMaterial::Grass, (TileFlags)0 };
}

TileMaterial TileSystem::GetMaterial(locXY location) {
	static auto map_tile_get_footstep_sound = temple::GetPointer<int(locXY loc)>(0x100ab870);
	return (TileMaterial) map_tile_get_footstep_sound(location);
}

//*****************************************************************************
//* OName
//*****************************************************************************

ONameSystem::ONameSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10105770);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system OName");
	}
}
ONameSystem::~ONameSystem() {
	auto shutdown = temple::GetPointer<void()>(0x101057b0);
	shutdown();
}
void ONameSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x101057e0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system OName");
	}
}
void ONameSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x10105850);
	unloadModule();
}
const std::string &ONameSystem::GetName() const {
	static std::string name("OName");
	return name;
}

//*****************************************************************************
//* ObjectNode
//*****************************************************************************

ObjectNodeSystem::~ObjectNodeSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100c0c70);
	shutdown();
}
const std::string &ObjectNodeSystem::GetName() const {
	static std::string name("ObjectNode");
	return name;
}

//*****************************************************************************
//* Proto
//*****************************************************************************

struct ProtoIdRange {
	uint16_t start;
	uint16_t end;
};

static ProtoIdRange sProtoIdRanges[17] = {
	{ 0, 999 },
	{ 1000, 1999 },
	{ 2000, 2999 },
	{ 3000, 3999 },
	{ 4000, 4999 },
	{ 5000, 5999 },
	{ 6000, 6999 },
	{ 7000, 7999 },
	{ 8000, 8999 },
	{ 9000, 9999 },
	{ 10000, 10999 },
	{ 11000, 11999 },
	{ 12000, 12999 },
	{ 13000, 13999 },
	{ 14000, 14999 },
	{ 15000, 15999 },
	{ 16000, 16999 },
};

ProtoSystem::ProtoSystem(const GameSystemConf &config) {
	static auto protos_tab_parse_line = temple::GetPointer<TigTabLineParser>(0x1003b640);

	// do parsing for additive protos files (for usermade content)
	TioFileList protosFlist;
	tio_filelist_create(&protosFlist, "rules\\protos\\*.tab");

	for (auto i=0; i < protosFlist.count; i++){

		TigTabParser tabParser;
		tabParser.Init(protos_tab_parse_line);

		std::string combinedFname(fmt::format("rules\\protos\\{}", protosFlist.files[i].name) ) ;
		if (tabParser.Open(combinedFname.c_str() ) ) {
			throw TempleException("Unable to open rules\\protos_override.tab");
		}

		tabParser.Process();
		tabParser.Close();

	}

	tio_filelist_destroy(&protosFlist);

	// protos override (for Temple+)
	{
		TigTabParser tabParser;
		tabParser.Init(protos_tab_parse_line);

		if (tabParser.Open("rules\\protos_override.tab")) {
			throw TempleException("Unable to open rules\\protos_override.tab");
		}

		tabParser.Process();
		tabParser.Close();
	}


	{
		TigTabParser tabParser;
		tabParser.Init(protos_tab_parse_line);

		if (tabParser.Open("rules\\protos.tab")) {
			throw TempleException("Unable to open rules\\protos.tab");
		}

		tabParser.Process();
		tabParser.Close();
	}
	
}

ProtoSystem::~ProtoSystem() {

	// Remove the prototype objects for each type
	for (auto &range : sProtoIdRanges) {
		for (auto protoId = range.start; protoId <= range.end; ++protoId) {
			auto id = ObjectId::CreatePrototype(protoId);
			auto handle = objSystem->GetHandleById(id);
			if (handle) {
				objSystem->Remove(handle);
			}
		}
	}
	
}

const std::string &ProtoSystem::GetName() const {
	static std::string name("Proto");
	return name;
}

//*****************************************************************************
//* Object
//*****************************************************************************

ObjectSystem::ObjectSystem(const GameSystemConf &config) {

	// This is used by obj_list_vicinity
	static auto& viewportId = temple::GetRef<int>(0x104C7F74);
	viewportId = config.viewportId;

	// Still evaluated in advancetime
	static auto& always0 = temple::GetRef<int>(0x104C7F8C);
	always0 = 0;

	static auto& isEditor = temple::GetRef<BOOL>(0x10788098);
	isEditor = FALSE;

	static auto& screenRect = temple::GetRef<TigRect>(0x10307B90);
	screenRect.x = 0;
	screenRect.y = 0;
	screenRect.width = config.width;
	screenRect.height = config.height;

	// This is still used in raycasting...
	static auto visibleTypes = temple::GetPointer<BOOL>(0x10427EF0); // 17 entries
	for (size_t i = 0; i < 17; ++i) {
		visibleTypes[i] = TRUE;
	}

	static auto& objFlagsHidden = temple::GetRef<uint32_t>(0x10527F98);
	objFlagsHidden = OF_OFF | OF_DESTROYED;

	static auto& objFlagsDontDraw = temple::GetRef<uint32_t>(0x104C7F70);
	objFlagsDontDraw = OF_DONTDRAW;

	// Looks like an Arcanum leftover
	static auto& objLighting = temple::GetRef<int>(0x10788234);
	objLighting = 1;

	static auto& showObjHighlight = temple::GetRef<BOOL>(0x10788CEC);
	showObjHighlight = FALSE;

	static auto init_astar_result_cache = temple::GetPointer<void()>(0x1003ff00);
	init_astar_result_cache();
}
ObjectSystem::~ObjectSystem() {
	// Was @ 0x10020da0
	
	/*
		Notes:
		The trap.mes file is never used
		The materials.mes file is now processed elsewhere and the associated
		index table is also unused.
		The shadow map buffers are also handled elsewhere.
	*/
	Reset();
	
	// I do not believe this is used in any way
	static auto map_obj_free_rendercolors_pool = temple::GetPointer<int()>(0x100201a0);
	map_obj_free_rendercolors_pool();
}
void ObjectSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x1001d2b0);
	resetBuffers(&rebuildInfo);
}
void ObjectSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x1001d270);
	reset();
}
void ObjectSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x1001d4e0);
	advanceTime(time);
}
void ObjectSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x1001d7c0);
	mapClose();
}
const std::string &ObjectSystem::GetName() const {
	static std::string name("Object");
	return name;
}

//*****************************************************************************
//* SectorVB
//*****************************************************************************

SectorVBSystem::SectorVBSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100aa5c0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system SectorVB");
	}
}
SectorVBSystem::~SectorVBSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100aaa80);
	shutdown();
}
void SectorVBSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100aa630);
	reset();
}
void SectorVBSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x100aa630);
	mapClose();
}
const std::string &SectorVBSystem::GetName() const {
	static std::string name("SectorVB");
	return name;
}

void SectorVBSystem::SetDirectories(const std::string & dataDir, const std::string & saveDir)
{
	static auto map_sectorvb_set_dirs = temple::GetPointer<BOOL(const char *dataDir, const char *saveDir)>(0x100aa430);
	map_sectorvb_set_dirs(dataDir.c_str(), saveDir.c_str());
}

SectorVB * SectorVBSystem::GetSvb(SectorLoc secLoc){
	auto result = temple::GetRef<SectorVB*(__cdecl)(SectorLoc)>(0x100AA650)(secLoc);
	if (result)
		result->refCnt++;
	return result;
}

//*****************************************************************************
//* TextBubble
//*****************************************************************************

TextBubbleSystem::TextBubbleSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a2cb0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system TextBubble");
	}
}
TextBubbleSystem::~TextBubbleSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100a31c0);
	shutdown();
}
void TextBubbleSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x100a2a00);
	resetBuffers(&rebuildInfo);
}
void TextBubbleSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100a31b0);
	reset();
}
void TextBubbleSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100a2de0);
	advanceTime(time);
}
void TextBubbleSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x100a31b0);
	mapClose();
}
const std::string &TextBubbleSystem::GetName() const {
	static std::string name("TextBubble");
	return name;
}

//*****************************************************************************
//* TextFloater
//*****************************************************************************

TextFloaterSystem::TextFloaterSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a2040);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system TextFloater");
	}
}
TextFloaterSystem::~TextFloaterSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100a2980);
	shutdown();
}
void TextFloaterSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x100a1df0);
	resetBuffers(&rebuildInfo);
}
void TextFloaterSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100a2970);
	reset();
}
void TextFloaterSystem::AdvanceTime(uint32_t time) {
	auto advanceTime = temple::GetPointer<void(uint32_t)>(0x100a2480);
	advanceTime(time);
}
void TextFloaterSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x100a2970);
	mapClose();
}
const std::string &TextFloaterSystem::GetName() const {
	static std::string name("TextFloater");
	return name;
}

//*****************************************************************************
//* JumpPoint
//*****************************************************************************

void JumpPointSystem::LoadModule() {
	auto loadModule = temple::GetPointer<int()>(0x100bdfe0);
	if (!loadModule()) {
		throw TempleException("Unable to load module data for game system JumpPoint");
	}
}
void JumpPointSystem::UnloadModule() {
	auto unloadModule = temple::GetPointer<void()>(0x100be010);
	unloadModule();
}
void JumpPointSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100be040);
	reset();
}
const std::string &JumpPointSystem::GetName() const {
	static std::string name("JumpPoint");
	return name;
}

//*****************************************************************************
//* Height
//*****************************************************************************

HeightSystem::HeightSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a88f0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Height");
	}
}
const std::string &HeightSystem::GetName() const {
	static std::string name("Height");
	return name;
}

void HeightSystem::SetDataDirs(const std::string & dataDir, const std::string & saveDir)
{
	static auto setHeightDataDir = temple::GetPointer<void(const char*)>(0x100A8970);
	setHeightDataDir(dataDir.c_str());

	auto setHeightSaveDir = temple::GetPointer<void(const char*)>(0x100A8990);
	setHeightSaveDir(saveDir.c_str());
}

void HeightSystem::Clear()
{
	static auto clear = temple::GetPointer<void()>(0x100A8940);
	clear();
}

int8_t HeightSystem::GetDepth(LocAndOffsets location)
{
	static auto get_depth = temple::GetPointer<int8_t(LocAndOffsets)>(0x100a8cb0);
	return get_depth(location);
}

//*****************************************************************************
//* PathNode
//*****************************************************************************

PathNodeSystem::PathNodeSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x100a9b40);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system PathNode");
	}
}
PathNodeSystem::~PathNodeSystem() {
	auto shutdown = temple::GetPointer<void()>(0x100a9da0);
	shutdown();
}
void PathNodeSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x100a9da0);
	reset();
}
const std::string &PathNodeSystem::GetName() const {
	static std::string name("PathNode");
	return name;
}

void PathNodeSystem::SetDataDirs(const std::string & dataDir, const std::string & saveDir)
{
	static auto setDataDirs = temple::GetPointer<void(const char*, const char*)>(0x100A9720);
	setDataDirs(dataDir.c_str(), saveDir.c_str());
}

void PathNodeSystem::Load(const std::string & dataDir, const std::string & saveDir)
{
	static auto load = temple::GetPointer<BOOL(const char *dataDir, const char *saveDir)>(0x100A9DD0);
	if (!load(dataDir.c_str(), saveDir.c_str())) {
		throw TempleException("Unable to load path nodes from {}", dataDir);
	}
}
