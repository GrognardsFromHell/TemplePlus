
#pragma once

#include "gamesystem.h"
struct GameSystemConf;

#pragma once

#include <string>
#include <cstdint>

#include "gamesystem.h"

#include "legacy.h"
#include "obj.h"

class ScrollSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem {
public:
	static constexpr auto Name = "Scroll";
	ScrollSystem(const GameSystemConf &config);
	~ScrollSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;

	void SetMapId(int mapId);
};

class LocationSystem : public GameSystem, public BufferResettingGameSystem {
public:
	static constexpr auto Name = "Location";
	LocationSystem(const GameSystemConf &config);
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;

	void CenterOn(int x, int y);
	void SetLimits(uint64_t limitX, uint64_t limitY);
	locXY GetLimitsCenter();
};

class LightSystem : public GameSystem, public BufferResettingGameSystem {
public:
	static constexpr auto Name = "Light";
	LightSystem(const GameSystemConf &config);
	~LightSystem();
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;

	void Load(const std::string &dataDir);

	// Sets the info from daylight.mes based on map id
	void SetMapId(int mapId);
};

class TileSystem : public GameSystem, public BufferResettingGameSystem {
public:
	static constexpr auto Name = "Tile";
	TileSystem(const GameSystemConf &config);
	~TileSystem();
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	const std::string &GetName() const override;
};

class ONameSystem : public GameSystem, public ModuleAwareGameSystem {
public:
	static constexpr auto Name = "OName";
	ONameSystem(const GameSystemConf &config);
	~ONameSystem();
	void LoadModule() override;
	void UnloadModule() override;
	const std::string &GetName() const override;
};

class ObjectNodeSystem : public GameSystem {
public:
	static constexpr auto Name = "ObjectNode";
	~ObjectNodeSystem();
	const std::string &GetName() const override;
};

class ProtoSystem : public GameSystem {
public:
	static constexpr auto Name = "Proto";
	ProtoSystem(const GameSystemConf &config);
	~ProtoSystem();
	const std::string &GetName() const override;
};

class ObjectSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "Object";
	ObjectSystem(const GameSystemConf &config);
	~ObjectSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	void CloseMap() override;
	const std::string &GetName() const override;

};

class MapSectorSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "MapSector";
	MapSectorSystem(const GameSystemConf &config);
	~MapSectorSystem();
	void Reset() override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	void CloseMap() override;
	const std::string &GetName() const override;

	void Clear();
	void SetDirectories(const std::string &dataDir, const std::string &saveDir);

	bool IsSectorLoaded(SectorLoc location);

	void RemoveSectorLight(objHndl handle);
};

class SectorVBSystem : public GameSystem, public ResetAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "SectorVB";
	SectorVBSystem(const GameSystemConf &config);
	~SectorVBSystem();
	void Reset() override;
	void CloseMap() override;
	const std::string &GetName() const override;

	void SetDirectories(const std::string &dataDir, const std::string &saveDir);
};

class TextBubbleSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "TextBubble";
	TextBubbleSystem(const GameSystemConf &config);
	~TextBubbleSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	void CloseMap() override;
	const std::string &GetName() const override;
};

class TextFloaterSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public TimeAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "TextFloater";
	TextFloaterSystem(const GameSystemConf &config);
	~TextFloaterSystem();
	void Reset() override;
	void AdvanceTime(uint32_t time) override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	void CloseMap() override;
	const std::string &GetName() const override;
};

class JumpPointSystem : public GameSystem, public ModuleAwareGameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "JumpPoint";
	void LoadModule() override;
	void UnloadModule() override;
	void Reset() override;
	const std::string &GetName() const override;
};

class HeightSystem : public GameSystem {
public:
	static constexpr auto Name = "Height";
	HeightSystem(const GameSystemConf &config);
	const std::string &GetName() const override;

	void SetDataDirs(const std::string &dataDir, const std::string &saveDir);

	void Clear();
};

class GMeshSystem : public GameSystem {
public:
	static constexpr auto Name = "GMesh";
	GMeshSystem(const GameSystemConf &config);
	~GMeshSystem();
	const std::string &GetName() const override;

	void Load(const std::string &dataDir);
};

class PathNodeSystem : public GameSystem, public ResetAwareGameSystem {
public:
	static constexpr auto Name = "PathNode";
	PathNodeSystem(const GameSystemConf &config);
	~PathNodeSystem();
	void Reset() override;
	const std::string &GetName() const override;

	// This seems actually redundant since it'll be set by load as well
	void SetDataDirs(const std::string &dataDir, const std::string &saveDir);

	void Load(const std::string &dataDir, const std::string &saveDir);

};
