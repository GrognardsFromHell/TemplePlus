
#include "stdafx.h"

#include <infrastructure/exception.h>

#include "legacymapsystems.h"

#include "gamesystems/map/sector.h"

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
	static auto map_location_center_on_smooth = temple::GetPointer<void(int x, int y)>(0x10005bc0);
	map_location_center_on_smooth(tileX, tileY);
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

ProtoSystem::ProtoSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x1003b7a0);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Proto");
	}
}
ProtoSystem::~ProtoSystem() {
	auto shutdown = temple::GetPointer<void()>(0x1003b9b0);
	shutdown();
}
const std::string &ProtoSystem::GetName() const {
	static std::string name("Proto");
	return name;
}

//*****************************************************************************
//* Object
//*****************************************************************************

ObjectSystem::ObjectSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10021670);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system Object");
	}
}
ObjectSystem::~ObjectSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10020da0);
	shutdown();
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
