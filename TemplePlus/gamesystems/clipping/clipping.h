#pragma once

#include "../gamesystem.h"

namespace gfx {
	class RenderingDevice;
}

class BinaryReader;

class ClippingSystem : public GameSystem {
public:	
	static constexpr auto Name = "Clipping";

	explicit ClippingSystem(gfx::RenderingDevice& g);
	~ClippingSystem();

	void Load(const std::string& directory);
	void Render();
	void Unload();


	void SetDebug(bool enable);
	bool IsDebug() const;

	size_t GetTotal() const;
	size_t GetRenderered() const;

	const std::string &GetName() const override;

private:
	class Impl;

	void LoadMeshes(const std::string &directory);
	void LoadObjects(const std::string &directory);
	void LoadObject(BinaryReader& reader);

	std::unique_ptr<Impl> mImpl;		
};
