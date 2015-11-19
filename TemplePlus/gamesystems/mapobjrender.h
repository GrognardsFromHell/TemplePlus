#pragma once

#include <obj.h>

namespace gfx {
	struct AnimatedModelParams;
	class RenderingDevice;
	struct Light3d;
}
namespace temple {
	class AasRenderer;
}

class GameSystems;

class MapObjectRenderer {
public:

	MapObjectRenderer(GameSystems& gameSystems, gfx::RenderingDevice& device, temple::AasRenderer &aasRenderer);
	~MapObjectRenderer();

	void RenderMapObjects(int tileX1, int tileX2, int tileY1, int tileY2);
	void RenderObject(objHndl handle, bool showInvisible);

private:
	GameSystems& mGameSystems;
	gfx::RenderingDevice& mDevice;
	temple::AasRenderer &mAasRenderer;

	void DrawBoundingCylinder(float x, float y, float z, float radius, float height);

	gfx::AnimatedModelParams GetAnimParamsForObject(objHndl handle);

	const std::string& GetAttachedBoneName(objHndl handle);

	std::vector<gfx::Light3d> FindLights(LocAndOffsets atLocation, float radius);

	/*
	Same as sin45 incidentally.
	The idea seems to be that the vertical height of the model is scaled
	according to the camera inclination of 45°.
	*/
	static constexpr float cos45 = 0.70709997f;

};
