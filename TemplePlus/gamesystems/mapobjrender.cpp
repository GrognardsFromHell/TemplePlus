#include "stdafx.h"
#include "mapobjrender.h"
#include <util/fixes.h>
#include <obj.h>
#include "gamesystems/maps/sector.h"
#include <util/config.h>
#include <graphics/renderer.h>
#include <temple/meshes.h>
#include "gamesystems/gamesystems.h"
#include <graphics/renderstates_hooks.h>
#include <temple/aasrenderer.h>

using namespace gfx;
using namespace temple;

static struct MapRenderAddresses : temple::AddressTable {
	bool (*IsPosExplored)(locXY loc, float offsetX, float offsetY);
	void (*WorldToLocalScreen)(vector3f pos, float* xOut, float* yOut);
	void (*ObjUpdateRenderHeight)(objHndl obj, int animId);
	void (*EnableLights)(LocAndOffsets forLocation, float radius);

	void (*Particles_Kill)(int handle);
	int (*Particles_CreateAtPos)(int hashcode, float x, float y, float z);

	bool* globalLightEnabled;
	LegacyLight* globalLight;
	BOOL* isNight;

	MapRenderAddresses() {
		rebase(IsPosExplored, 0x1002ECB0);
		rebase(WorldToLocalScreen, 0x10029040);
		rebase(ObjUpdateRenderHeight, 0x10021360);
		rebase(EnableLights, 0x100A5BA0);
		rebase(isNight, 0x10B5DC80);

		rebase(Particles_Kill, 0x10049BE0);
		rebase(Particles_CreateAtPos, 0x10049BD0);

		rebase(globalLightEnabled, 0x118691E0);
		rebase(globalLight, 0x11869200);
	}
} addresses;

MapObjectRenderer::MapObjectRenderer(GameSystems& gameSystems, gfx::RenderingDevice& device, temple::AasRenderer &aasRenderer)
	: mGameSystems(gameSystems),
	  mDevice(device),
	  mAasRenderer(aasRenderer) {
}

MapObjectRenderer::~MapObjectRenderer() {
}

void MapObjectRenderer::RenderMapObjects(int tileX1, int tileX2, int tileY1, int tileY2) {

	for (auto secY = tileY1 / 64; secY <= tileY2 / 64; ++secY) {
		for (auto secX = tileX1 / 64; secX <= tileX2 / 64; ++secX) {

			LockedMapSector sector(secX, secY);

			for (auto tx = 0; tx < 64; ++tx) {
				for (auto ty = 0; ty < 64; ++ty) {

					auto obj = sector.GetObjectsAt(tx, ty);
					while (obj) {
						RenderObject(obj->handle, true);
						obj = obj->next;
					}

				}
			}
		}
	}

}

void MapObjectRenderer::RenderObject(objHndl handle, bool showInvisible) {

	auto type = objects.GetType(handle);
	auto flags = objects.GetFlags(handle);

	// Dont render destroyed or disabled objects
	constexpr auto dontDrawFlags = OF_OFF | OF_DESTROYED | OF_DONTDRAW;
	if ((flags & dontDrawFlags) != 0) {
		return;
	}

	// Hide invisible objects we're supposed to show them
	if ((flags & OF_INVISIBLE) != 0 && !showInvisible) {
		return;
	}

	// Dont draw secret doors that haven't been found yet
	auto secretDoorFlags = objects.GetSecretDoorFlags(handle);
	if (secretDoorFlags & OSDF_SECRET_DOOR) {
		auto found = ((secretDoorFlags & OSDF_SECRET_DOOR_FOUND) != 0);
		if (!found && type != obj_t_portal)
			return;
	}

	auto animatedModel = objects.GetAnimHandle(handle);

	auto animParams(objects.GetAnimParams(handle));

	locXY worldLoc;

	objHndl parent = 0;
	if (type >= obj_t_weapon && type <= obj_t_generic || type == obj_t_bag) {
		parent = inventory.GetParent(handle);
	}

	auto alpha = objects.GetAlpha(handle);

	if (parent) {
		auto parentAlpha = objects.GetAlpha(parent);
		alpha = (alpha + parentAlpha) / 2;

		worldLoc = objects.GetLocation(parent);
	} else {
		worldLoc = objects.GetLocation(handle);
	}

	if (alpha == 0) {
		return;
	}

	// Handle fog occlusion of the world position
	if (type != obj_t_container
		&& (type == obj_t_projectile
			|| type == obj_t_pc
			|| type == obj_t_npc
			|| type >= obj_t_weapon && type <= obj_t_generic
			|| type == obj_t_bag)
		&& !addresses.IsPosExplored(worldLoc, animParams.offsetX, animParams.offsetY)) {
		return;
	}

	LocAndOffsets worldPosFull;
	worldPosFull.off_x = animParams.offsetX;
	worldPosFull.off_y = animParams.offsetY;
	worldPosFull.location = worldLoc;
	auto centerOfTile = worldPosFull.ToCenterOfTileAbs();
	vector3f centerOfTileFull{centerOfTile.x, centerOfTile.y, 0};
	float screenX, screenY;
	addresses.WorldToLocalScreen(centerOfTileFull, &screenX, &screenY);

	auto radius = objects.GetRadius(handle);
	auto renderHeight = objects.GetRenderHeight(handle);

	// Take render height from the animation if necessary
	if (renderHeight < 0) {
		addresses.ObjUpdateRenderHeight(handle, animatedModel->GetAnimId());
		renderHeight = objects.GetRenderHeight(handle);
	}

	// This checks if the object's screen bounding box is off screen
	auto bbLeft = (int)(screenX - radius);
	auto bbRight = (int)(screenX + radius);
	auto bbTop = (int)(screenY - (animParams.offsetZ + renderHeight + radius) * cos45);
	auto bbBottom = (bbTop + (int)((2 * radius + renderHeight) * cos45));

	if (bbRight < 0 || bbBottom < 0 || bbLeft > config.renderWidth || bbTop > config.renderHeight) {
		return;
	}

	/*
	DrawBoundingCylinder(
		centerOfTile.x,
		animParams.offsetZ,
		centerOfTile.y,
		radius,
		renderHeight
	);
	*/

	auto lightSearchRadius = 0.0f;
	if (!(flags & OF_DONTLIGHT)) {
		lightSearchRadius = radius;
	}

	LocAndOffsets locAndOffsets;
	locAndOffsets.location = worldLoc;
	locAndOffsets.off_x = animParams.offsetX;
	locAndOffsets.off_y = animParams.offsetY;
	auto lights(FindLights(locAndOffsets, lightSearchRadius));

	// TODO: Render highlighting and barbarian rage highlighting for weapons here

	static D3DCOLOR sDiffuse[32767];
	D3DCOLOR* diffuse = nullptr;
	if (alpha != 255) {
		for (auto i = 0; i < 32767; ++i) {
			sDiffuse[i] = D3DCOLOR_ARGB(alpha, 255, 255, 255);
		}
		diffuse = &sDiffuse[0];
	}

	// renderStates->SetLighting(true);
	
	mAasRenderer.Render(animatedModel.get(), animParams, lights);

	/*auto materialIds(animatedModel->GetSubmeshes());
	for (size_t i = 0; i < materialIds.size(); ++i) {
		auto materialId = materialIds[i];
		auto submesh(animatedModel->GetSubmesh(animParams, i));
		RenderHooks::ShaderRender3d(
			submesh->GetVertexCount(),
			(D3DXVECTOR4*)&submesh->GetPositions()[0],
			(D3DXVECTOR4*)&submesh->GetNormals()[0],
			diffuse,
			(D3DXVECTOR2*)&submesh->GetUV()[0],
			submesh->GetPrimitiveCount(),
			&submesh->GetIndices()[0],
			materialId
		);
	}*/

}

void MapObjectRenderer::DrawBoundingCylinder(float x, float y, float z, float radius, float height) {

	// Draw the 3d bounding cylinder of the object
	std::vector<Line> lines;
	constexpr auto diffuse = D3DCOLOR_ARGB(128, 0, 255, 0);

	auto scaledRadius = radius * cos45;
	D3DXVECTOR3 from, to;

	from.x = x + scaledRadius;
	from.y = y;
	from.z = z - scaledRadius;
	to.x = from.x;
	to.y = y + height;
	to.z = from.z;

	lines.push_back({from, to, diffuse});

	from.x = x - scaledRadius;
	from.z = z + scaledRadius;
	to.x = from.x;
	to.z = from.z;
	lines.push_back({from, to, diffuse});

	/*
		Draw the circle on top and on the bottom 
		of the cylinder.
	*/
	for (auto i = 0; i < 24; ++i) {
		// We rotate 360° in 24 steps of 15° each
		auto rot = i * deg2rad(15);
		auto nextRot = rot + deg2rad(15);

		// This is the bottom cap
		from.x = x + cosf(rot) * radius;
		from.y = y;
		from.z = z - sinf(rot) * radius;
		to.x = x + cosf(nextRot) * radius;
		to.y = y;
		to.z = z - sinf(nextRot) * radius;
		lines.push_back({from, to, diffuse});

		// This is the top cap
		from.x = x + cosf(rot) * radius;
		from.y = y + height;
		from.z = z - sinf(rot) * radius;
		to.x = x + cosf(nextRot) * radius;
		to.y = y + height;
		to.z = z - sinf(nextRot) * radius;
		lines.push_back({from, to, diffuse});
	}

	// TODO
	/*Renderer renderer(*graphics);
	renderer.DrawLines(lines);*/

}

class SectorIterator {
public:
	SectorIterator(int fromX, int toX, int fromY, int toY)
		: mFromX(fromX / 64), mFromY(fromY / 64),
		  mToX(toX / 64), mToY(toY / 64),
		  x(mFromX), y(mFromY) {
	}

	~SectorIterator() = default;

	bool HasNext() const {
		return y < mToY || (y == mToY && x < mToX);
	}

	LockedMapSector& Next() {
		if (++x > mToX) {
			x = mFromX;
			++y;
			Expects(y <= mToY);
		}
		mLockedSector = std::make_unique<LockedMapSector>(x, y);
		return *mLockedSector;
	}

private:
	int mFromX, mFromY, mToX, mToY;
	int x, y;
	std::unique_ptr<LockedMapSector> mLockedSector;
};

std::vector<Light3d> MapObjectRenderer::FindLights(LocAndOffsets atLocation, float radius) {

	std::vector<Light3d> lights;

	if (*addresses.globalLightEnabled) {
		Light3d light;
		auto& legacyLight = *addresses.globalLight;
		light.type = (Light3dType)legacyLight.type;
		light.color.x = legacyLight.colorR;
		light.color.y = legacyLight.colorG;
		light.color.z = legacyLight.colorB;
		light.dir.x = legacyLight.dir.x;
		light.dir.y = legacyLight.dir.y;
		light.dir.z = legacyLight.dir.z;
		light.pos.x = legacyLight.pos.x;
		light.pos.y = legacyLight.pos.y;
		light.pos.z = legacyLight.pos.z;
		light.range = legacyLight.range;
		light.phi = legacyLight.phi;
		lights.push_back(light);
	}

	if (radius == 0) {
		return lights;
	}

	// Build a box that has twice the radius convert to tiles as it's width/height
	// For some reason, ToEE will add one more INCH_PER_TILE here, which translates to
	// roughly 28 tiles more search radius than is needed
	auto boxDimensions = static_cast<int>(radius / INCH_PER_TILE + INCH_PER_TILE);
	auto tileX1 = atLocation.location.locx - 1 - boxDimensions;
	auto tileX2 = atLocation.location.locx + 1 + boxDimensions;
	auto tileY1 = atLocation.location.locy - 1 - boxDimensions;
	auto tileY2 = atLocation.location.locy + 1 + boxDimensions;

	SectorIterator sectorIterator(tileX1, tileX2, tileY1, tileY2);

	auto atPos = atLocation.ToCenterOfTileAbs();

	while (sectorIterator.HasNext()) {
		auto& sector = sectorIterator.Next();

		auto lightIt = sector.GetLights();
		while (lightIt.HasNext()) {
			auto& light = lightIt.Next();

			int type;
			uint32_t color;
			D3DVECTOR direction;
			float range, phi;
			auto lightPos = light.position.ToCenterOfTileAbs();

			if (light.flags & 0x40) {
				if (*addresses.isNight) {
					type = light.light2.type;
					color = light.light2.color;
					direction = light.light2.direction;
					range = light.range; // Notice how it's using light 1's range
					phi = light.light2.phi;

					/*
					Kill the daytime particle system if it's night and the
					daytime particle system is still alive.
					*/
					if (light.partSys.handle) {
						addresses.Particles_Kill(light.partSys.handle);
						light.partSys.handle = 0;
					}

					/*
					If the nighttime particle system has not yet been started,
					do it here.
					*/
					auto& nightPartSys = light.light2.partSys;
					if (!nightPartSys.handle && nightPartSys.hashCode) {
						auto centerOfTile = light.position.ToCenterOfTileAbs();
						nightPartSys.handle = addresses.Particles_CreateAtPos(
							nightPartSys.hashCode,
							centerOfTile.x,
							light.offsetZ,
							centerOfTile.y);
					}
				}
				else {
					type = light.type;
					color = light.color;
					direction = light.direction;
					range = light.range;
					phi = light.phi;

					// This is just the inverse of what we're doing at night (see above)
					auto& nightPartSys = light.light2.partSys;
					if (nightPartSys.handle) {
						addresses.Particles_Kill(nightPartSys.handle);
						nightPartSys.handle = 0;
					}

					auto& dayPartSys = light.partSys;
					if (!dayPartSys.handle && dayPartSys.hashCode) {
						dayPartSys.handle = addresses.Particles_CreateAtPos(
							dayPartSys.hashCode,
							lightPos.x,
							light.offsetZ,
							lightPos.y);
					}
				}
			}
			else {
				type = light.type;
				color = light.color;
				direction = light.direction;
				range = light.range;
				phi = light.phi;
			}

			if (!type) {
				continue;
			}

			// Distance (Squared) between pos and light pos
			auto acceptableDistance = (int)(radius + light.range);
			auto acceptableDistanceSquared = acceptableDistance * acceptableDistance;
			auto diffX = static_cast<int>(atPos.x - lightPos.x);
			auto diffY = static_cast<int>(atPos.y - lightPos.y);
			auto distanceSquared = diffX * diffX + diffY * diffY;
			if (distanceSquared > acceptableDistanceSquared) {
				continue;
			}

			Light3d light3d;
			if (type == 2) {
				light3d.type = Light3dType::Directional;
				D3DXVec3Normalize((D3DXVECTOR3*)&light3d.dir, (D3DXVECTOR3*)&direction);
			}
			else if (type == 3) {
				light3d.type = Light3dType::Spot;
				D3DXVec3Normalize((D3DXVECTOR3*)&light3d.dir, (D3DXVECTOR3*)&direction);
			}
			else if (type == 1) {
				light3d.type = Light3dType::Point;
				light3d.dir.x = direction.x;
				light3d.dir.y = direction.y;
				light3d.dir.z = direction.z;
			}

			light3d.pos.x = lightPos.x;
			light3d.pos.y = light.offsetZ;
			light3d.pos.z = lightPos.y;

			light3d.color.x = (color & 0xFF) / 255.0f;
			light3d.color.y = ((color >> 16) & 0xFF) / 255.0f;
			light3d.color.z = ((color >> 8) & 0xFF) / 255.0f;

			light3d.range = range;
			light3d.phi = phi;
			lights.push_back(light3d);
		}
	}

	return lights;

}

static class RenderFix : public TempleFix {
public:

	static void obj_render(objHndl handle, int flag, locXY unk, int x);

	const char* name() override {
		return "B";
	}

	void apply() override {
		replaceFunction(0x10026560, obj_render);
	}

} fix;

void RenderFix::obj_render(objHndl handle, int flag, locXY loc, int x) {
}

#pragma pack(push, 8)
struct PreciseSectorCol {
	int colCount;
	uint64_t sectorIds[4];
	uint64_t sectorIds2[4];
	int sectorIds3[4];
	int sectorIds4[4];
	int x;
	int y;
};

struct PreciseSectorRows {
	int rowCount;
	PreciseSectorCol rows[6];
};
#pragma pack(pop)
