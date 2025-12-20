#include "stdafx.h"
#include "mapobjrender.h"
#include <util/fixes.h>
#include <obj.h>
#include "gamesystems/map/sector.h"
#include "gamesystems/particlesystems.h"
#include <config/config.h>
#include <temple/meshes.h>
#include "gamesystems/gamesystems.h"
#include "gamesystems/gamerenderer.h"
#include <graphics/renderstates_hooks.h>
#include <infrastructure/images.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/shaperenderer2d.h>
#include <aas/aas_renderer.h>
#include "../critter.h"
#include "gameview.h"
#include <graphics/dynamictexture.h>

#include "froggrapplecontroller.h"

#include "tig/tig_startup.h"

using namespace gfx;
using namespace temple;
using namespace DirectX;

static struct MapRenderAddresses : temple::AddressTable {
	uint8_t (*GetFogStatus)(locXY loc, float offsetX, float offsetY);
	void (*WorldToLocalScreen)(vector3f pos, float* xOut, float* yOut);
	void (*EnableLights)(LocAndOffsets forLocation, float radius);

	uint32_t (*GetWeaponGlowType)(objHndl wielder, objHndl item);

	bool* globalLightEnabled;
	LegacyLight* globalLight;
	BOOL* isNight;

	MapRenderAddresses() {
		rebase(GetFogStatus, 0x1002ECB0);
		rebase(WorldToLocalScreen, 0x10029040);
		rebase(EnableLights, 0x100A5BA0);
		rebase(isNight, 0x10B5DC80);
		rebase(GetWeaponGlowType, 0x1004E620);

		rebase(globalLightEnabled, 0x118691E0);
		rebase(globalLight, 0x11869200);
	}
} addresses;

MapObjectRenderer::MapObjectRenderer(GameSystems& gameSystems, 
	gfx::RenderingDevice& device, 
	gfx::WorldCamera& camera,
	gfx::MdfMaterialFactory &mdfFactory,
	aas::Renderer &aasRenderer)
	: mGameSystems(gameSystems),
	  mDevice(device),
	  mCamera(camera),
	  mAasRenderer(aasRenderer) {

	mHighlightMaterial = mdfFactory.LoadMaterial("art/meshes/hilight.mdf");
	mBlobShadowMaterial = mdfFactory.LoadMaterial("art/meshes/shadow.mdf");
	mOccludedMaterial = mdfFactory.LoadMaterial("art/meshes/occlusion.mdf");

	mGrappleController = std::make_unique<FrogGrappleController>(device, mdfFactory);

	config.AddVanillaSetting("shadow_type", "2", [&]() {
		int shadowType = config.GetVanillaInt("shadow_type");
		switch (shadowType) {
		case 0:
			mShadowType = ShadowType::Blob;
			break;
		case 1:
			mShadowType = ShadowType::Geometry;
			break;
		case 2:
			mShadowType = ShadowType::ShadowMap;
			break;
		}
	});

	switch (config.GetVanillaInt("shadow_type")) {
	case 0:
		mShadowType = ShadowType::Blob;
		break;
	case 1:
		mShadowType = ShadowType::Geometry;
		break;
	case 2:
		mShadowType = ShadowType::ShadowMap;
		break;
	}

	// Load the weapon glow effect files
	mGlowMaterials[0] = mdfFactory.LoadMaterial("art/meshes/wg_magic.mdf");
	mGlowMaterials[1] = mdfFactory.LoadMaterial("art/meshes/wg_acid.mdf");
	mGlowMaterials[2] = mdfFactory.LoadMaterial("art/meshes/wg_bane.mdf");
	mGlowMaterials[3] = mdfFactory.LoadMaterial("art/meshes/wg_chaotic.mdf");
	mGlowMaterials[4] = mdfFactory.LoadMaterial("art/meshes/wg_cold.mdf");
	mGlowMaterials[5] = mdfFactory.LoadMaterial("art/meshes/wg_fire.mdf");
	mGlowMaterials[6] = mdfFactory.LoadMaterial("art/meshes/wg_holy.mdf");
	mGlowMaterials[7] = mdfFactory.LoadMaterial("art/meshes/wg_law.mdf");
	mGlowMaterials[8] = mdfFactory.LoadMaterial("art/meshes/wg_shock.mdf");
	mGlowMaterials[9] = mdfFactory.LoadMaterial("art/meshes/wg_unholy.mdf");
}

MapObjectRenderer::~MapObjectRenderer() {
	config.RemoveVanillaCallback("shadow_type");
}

void MapObjectRenderer::RenderMapObjects(int tileX1, int tileX2, int tileY1, int tileY2) {

	gfx::PerfGroup perfGroup(mDevice, "Map Objects");

	mTotalLastFrame = 0;
	mRenderedLastFrame = 0;

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

	mTotalLastFrame++;

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

	if (objects.IsCritter(handle)){
		if (d20Sys.d20Query(handle, DK_QUE_Polymorphed))		{
			int dummy = 1;
		}
	}


	locXY worldLoc;

	objHndl parent = objHndl::null;
	if (objects.IsEquipmentType(type)) {
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
			|| objects.IsCritterType(type)
			|| objects.IsEquipmentType(type))
		&& !(addresses.GetFogStatus(worldLoc, animParams.offsetX, animParams.offsetY) & 1)) {
		return;
	}

	LocAndOffsets worldPosFull;
	worldPosFull.off_x = animParams.offsetX;
	worldPosFull.off_y = animParams.offsetY;
	worldPosFull.location = worldLoc;
	
	auto radius = objects.GetRadius(handle);
	auto renderHeight = objects.GetRenderHeight(handle);

	// Take render height from the animation if necessary
	if (renderHeight < 0 
		|| objects.IsCritterType(type) && renderHeight > 10000.0f) // added in Temple+ as failsafe for bugged heights. For reference, Zuggtmoy is 381
	{
		objects.UpdateRenderHeight(handle, *animatedModel);
		renderHeight = objects.GetRenderHeight(handle);
	}

	if (!IsObjectOnScreen(worldPosFull, animParams.offsetZ, radius, renderHeight)) {
		return;
	}
	
	if (config.drawObjCylinders) {
		tig->GetShapeRenderer3d().DrawCylinder(
			worldPosFull.ToInches3D(animParams.offsetZ),
			radius,
			renderHeight
			);
	}
	
	auto lightSearchRadius = 0.0f;
	if (!(flags & OF_DONTLIGHT)) {
		lightSearchRadius = radius;
	}

	LocAndOffsets locAndOffsets;
	locAndOffsets.location = worldLoc;
	locAndOffsets.off_x = animParams.offsetX;
	locAndOffsets.off_y = animParams.offsetY;
	auto lights(FindLights(locAndOffsets, lightSearchRadius));

	if (type == obj_t_weapon) {
		uint32_t glowType;
		if ((flags & OF_INVENTORY) && parent) {
			glowType = addresses.GetWeaponGlowType(parent, handle);
		} else {
			glowType = addresses.GetWeaponGlowType(objHndl::null, handle);
		}

		if (glowType && glowType <= mGlowMaterials.size()) {
			auto& glowMaterial = mGlowMaterials[glowType - 1];
			if (glowMaterial) {
				RenderObjectHighlight(handle, glowMaterial);
			}
		}
	}

	if (mShowHighlights
		&& (objects.IsEquipmentType(type) && !(flags & (OF_INVENTORY | OF_CLICK_THROUGH))
			|| critterSys.IsLootableCorpse(handle)
			|| type == obj_t_portal
			|| (config.highlightContainers && type == obj_t_container)))
	{
		RenderObjectHighlight(handle, mHighlightMaterial);

		// Add a single light with full ambient color to make the object appear fully lit
		lights.clear();
		Light3d fullBrightLight;
		fullBrightLight.ambient = XMFLOAT4(1, 1, 1, 1);
		fullBrightLight.color = XMFLOAT4(0, 0, 0, 0);
		fullBrightLight.dir = XMFLOAT4(0, 0, 1, 0);
		fullBrightLight.type = Light3dType::Directional;
		lights.push_back(fullBrightLight);
	}

	mRenderedLastFrame++;	
	MdfRenderOverrides overrides;
	overrides.alpha = alpha / 255.0f;
	mAasRenderer.Render(animatedModel.get(), animParams, lights, &overrides);

	Light3d globalLight;
	if (!lights.empty()) {
		globalLight = lights[0];
	}

	if (objects.IsCritterType(type)) {
		if (alpha > 16) {
			if (mShadowType == ShadowType::ShadowMap)
			{
				RenderShadowMapShadow(handle, animParams, *animatedModel, globalLight, alpha);
			} else if (mShadowType == ShadowType::Geometry) {
				mAasRenderer.RenderGeometryShadow(animatedModel.get(),
					animParams,
					globalLight,
					alpha / 255.0f);
			} else if (mShadowType == ShadowType::Blob) {
				RenderBlobShadow(handle, *animatedModel, animParams, alpha);
			}
		}

		/*
			This renders the equipment in a critter's hand separately, but
			I am not certain *why* exactly. I thought this would have been
			handled by addmeshes, but it might be that there's a distinct
			difference between addmeshes that are skinned onto the mobile's
			skeleton and equipment that is unskinned and just positioned 
			in the player's hands.
		*/
		auto weaponPrim = critterSys.GetWornItem(handle, EquipSlot::WeaponPrimary);
		if (weaponPrim) {
			RenderObject(weaponPrim, showInvisible);
		}
		auto weaponSec = critterSys.GetWornItem(handle, EquipSlot::WeaponSecondary);
		if (weaponSec) {
			RenderObject(weaponSec, showInvisible);
		}
		auto shield = critterSys.GetWornItem(handle, EquipSlot::Shield);
		if (shield) {
			RenderObject(shield, showInvisible);
		}

	}
	else if (objects.IsEquipmentType(type) && mShadowType == ShadowType::Geometry)
	{
		mAasRenderer.RenderGeometryShadow(animatedModel.get(),
			animParams,
			globalLight,
			alpha / 255.0f);
	}

	RenderMirrorImages(handle,
		animParams,
		*animatedModel,
		lights);

	if (mGrappleController->IsGiantFrog(handle)) {
		mGrappleController->AdvanceAndRender(handle, 
			animParams, 
			*animatedModel, 
			lights, 
			alpha / 255.0f);
	}

}

void MapObjectRenderer::RenderObjectInUi(objHndl handle, int x, int y, float rotation, float scale) {

	// The y-15 is a hack to get it to be centered, in reality this would really need a rewrite of the entire camera system :-|
	auto worldPos = mCamera.ScreenToWorld((float)x, (float)y - 15);

	auto animatedModel = objects.GetAnimHandle(handle);

	auto animParams = objects.GetAnimParams(handle);
	animParams.x = 0;
	animParams.y = 0;
	animParams.offsetX = 0;
	animParams.offsetY = 0;
	animParams.offsetZ = 0;
	animParams.rotation = XM_PI + rotation;
	animParams.rotationPitch = 0;
	
	std::vector<gfx::Light3d> lights;
	lights.resize(2);
	lights[0].type = gfx::Light3dType::Directional;
	lights[0].color = { 1, 1, 1, 0 };
	lights[0].dir = { -0.7071200013160706f, -0.7071200013160706f , 0, 0};

	lights[1].type = gfx::Light3dType::Directional;
	lights[1].color = { 1, 1, 1, 0 };
	lights[1].dir = { 0, 0.7071200013160706, -0.7071200013160706f, 0 };

	// Use a GPU-side transform matrix instead of modifying the AAS model
	MdfRenderOverrides overrides;
	overrides.useWorldMatrix = true;
	auto transform = DirectX::XMMatrixAffineTransformation(
		DirectX::XMVectorSet(scale, scale, scale, 1.0f),
		DirectX::XMVectorZero(),
		DirectX::XMQuaternionIdentity(),
		DirectX::XMVectorSet(
			worldPos.x + 865.70398f,
			1200.0f,
			worldPos.z + 865.70398f,
			0
		)
	);
	XMStoreFloat4x4(&overrides.worldMatrix, transform);

	mAasRenderer.Render(animatedModel.get(), animParams, lights, &overrides);

	if (objects.IsCritter(handle)) {
		auto weaponPrim = critterSys.GetWornItem(handle, EquipSlot::WeaponPrimary);
		if (weaponPrim) {
			RenderObjectInUi(weaponPrim, x, y, rotation, scale);
		}
		auto weaponSec = critterSys.GetWornItem(handle, EquipSlot::WeaponSecondary);
		if (weaponSec) {
			RenderObjectInUi(weaponSec, x, y, rotation, scale);
		}
		auto shield = critterSys.GetWornItem(handle, EquipSlot::Shield);
		if (shield) {
			RenderObjectInUi(shield, x, y, rotation, scale);
		}
	}

}

void MapObjectRenderer::RenderOccludedMapObjects(int tileX1, int tileX2, int tileY1, int tileY2) {

	gfx::PerfGroup perfGroup(mDevice, "Occluded Map Objects");

	for (auto secY = tileY1 / 64; secY <= tileY2 / 64; ++secY) {
		for (auto secX = tileX1 / 64; secX <= tileX2 / 64; ++secX) {

			LockedMapSector sector(secX, secY);

			for (auto tx = 0; tx < 64; ++tx) {
				for (auto ty = 0; ty < 64; ++ty) {

					auto obj = sector.GetObjectsAt(tx, ty);
					while (obj) {
						RenderOccludedObject(obj->handle);
						obj = obj->next;
					}

				}
			}
		}
	}

}

void MapObjectRenderer::RenderOccludedObject(objHndl handle) {

	mTotalLastFrame++;

	auto type = objects.GetType(handle);
	auto flags = objects.GetFlags(handle);

	// Dont render destroyed or disabled objects
	constexpr auto dontDrawFlags = OF_OFF | OF_DESTROYED | OF_DONTDRAW;
	if ((flags & dontDrawFlags) != 0) {
		return;
	}
	if (flags & OF_INVISIBLE || flags & OF_INVENTORY) {
		return;
	}

	switch (type) {
		case obj_t_scenery:
		case obj_t_trap:
			return;
		case obj_t_pc:
		case obj_t_npc:
			if (critterSys.IsConcealed(handle)) {
				return;
			}
			break;
		default:
			break;
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

	objHndl parent = objHndl::null;
	if (objects.IsEquipmentType(type)) {
		parent = inventory.GetParent(handle);
	}

	auto alpha = objects.GetAlpha(handle);

	if (parent) {
		auto parentAlpha = objects.GetAlpha(parent);
		alpha = (alpha + parentAlpha) / 2;

		worldLoc = objects.GetLocation(parent);
	}
	else {
		worldLoc = objects.GetLocation(handle);
	}

	if (alpha == 0) {
		return;
	}

	// Handle fog occlusion of the world position, but handle it differently for portals
	if (type != obj_t_portal) {
		auto fogStatus = addresses.GetFogStatus(worldLoc, animParams.offsetX, animParams.offsetY);
		if (!(fogStatus & 0xB0) || !(fogStatus & 1)) {
			return;
		}
	} else {
		LocAndOffsets loc;
		loc.location = worldLoc;
		loc.off_x = animParams.offsetX - INCH_PER_SUBTILE;
		loc.off_y = animParams.offsetY - INCH_PER_SUBTILE;
		loc.Normalize();

		auto fogStatus = addresses.GetFogStatus(loc.location, loc.off_x, loc.off_y);
		if (!(fogStatus & 0xB0) || !(fogStatus & 1)) {
			return;
		}
	}
	
	LocAndOffsets worldPosFull;
	worldPosFull.off_x = animParams.offsetX;
	worldPosFull.off_y = animParams.offsetY;
	worldPosFull.location = worldLoc;

	auto radius = objects.GetRadius(handle);
	auto renderHeight = objects.GetRenderHeight(handle);

	// Take render height from the animation if necessary
	if (renderHeight < 0) {
		objects.UpdateRenderHeight(handle, *animatedModel);
		renderHeight = objects.GetRenderHeight(handle);
	}

	if (!IsObjectOnScreen(worldPosFull, animParams.offsetZ, radius, renderHeight)) {
		return;
	}
	
	auto lightSearchRadius = 0.0f;
	if (!(flags & OF_DONTLIGHT)) {
		lightSearchRadius = radius;
	}

	LocAndOffsets locAndOffsets;
	locAndOffsets.location = worldLoc;
	locAndOffsets.off_x = animParams.offsetX;
	locAndOffsets.off_y = animParams.offsetY;
	auto lights(FindLights(locAndOffsets, lightSearchRadius));
	
	mRenderedLastFrame++;
	MdfRenderOverrides overrides;
	overrides.alpha = alpha / 255.0f;

	if (type != obj_t_portal) {
		mOccludedMaterial->Bind(mDevice, lights, &overrides);
		mAasRenderer.RenderWithoutMaterial(animatedModel.get(), animParams);

		if (objects.IsCritterType(type)) {
			/*
			This renders the equipment in a critter's hand separately, but
			I am not certain *why* exactly. I thought this would have been
			handled by addmeshes, but it might be that there's a distinct
			difference between addmeshes that are skinned onto the mobile's
			skeleton and equipment that is unskinned and just positioned
			in the player's hands.
			*/
			auto weaponPrim = critterSys.GetWornItem(handle, EquipSlot::WeaponPrimary);
			if (weaponPrim) {
				RenderOccludedObject(weaponPrim);
			}
			auto weaponSec = critterSys.GetWornItem(handle, EquipSlot::WeaponSecondary);
			if (weaponSec) {
				RenderOccludedObject(weaponSec);
			}
			auto shield = critterSys.GetWornItem(handle, EquipSlot::Shield);
			if (shield) {
				RenderOccludedObject(shield);
			}

		}
	} else {
		if (mShowHighlights) {
			overrides.ignoreLighting = true;
		}
		mAasRenderer.Render(animatedModel.get(), animParams, lights, &overrides);
	}

}

void MapObjectRenderer::RenderObjectHighlight(objHndl handle, const gfx::MdfRenderMaterialPtr & material)
{

	mTotalLastFrame++;

	auto type = objects.GetType(handle);
	auto flags = objects.GetFlags(handle);

	// Dont render destroyed or disabled objects
	constexpr auto dontDrawFlags = OF_OFF | OF_DESTROYED | OF_DONTDRAW;
	if ((flags & dontDrawFlags) != 0) {
		return;
	}

	// Hide invisible objects we're supposed to show them
	if ((flags & OF_INVISIBLE) != 0) {
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

	objHndl parent = objHndl::null;
	if (objects.IsEquipmentType(type)) {
		parent = inventory.GetParent(handle);
	}

	auto alpha = objects.GetAlpha(handle);

	if (parent) {
		auto parentAlpha = objects.GetAlpha(parent);
		alpha = (alpha + parentAlpha) / 2;

		worldLoc = objects.GetLocation(parent);
	}
	else {
		worldLoc = objects.GetLocation(handle);
	}

	if (alpha == 0) {
		return;
	}

	// Handle fog occlusion of the world position
	if (type != obj_t_container
		&& (type == obj_t_projectile
			|| objects.IsCritterType(type)
			|| objects.IsEquipmentType(type))
		&& !(addresses.GetFogStatus(worldLoc, animParams.offsetX, animParams.offsetY) & 1)) {
		return;
	}

	LocAndOffsets worldPosFull;
	worldPosFull.off_x = animParams.offsetX;
	worldPosFull.off_y = animParams.offsetY;
	worldPosFull.location = worldLoc;

	auto radius = objects.GetRadius(handle);
	auto renderHeight = objects.GetRenderHeight(handle);

	// Take render height from the animation if necessary
	if (renderHeight < 0) {
		objects.UpdateRenderHeight(handle, *animatedModel);
		renderHeight = objects.GetRenderHeight(handle);
	}

	if (!IsObjectOnScreen(worldPosFull, animParams.offsetZ, radius, renderHeight)) {
		return;
	}

	auto lightSearchRadius = 0.0f;
	if (!(flags & OF_DONTLIGHT)) {
		lightSearchRadius = radius;
	}

	LocAndOffsets locAndOffsets;
	locAndOffsets.location = worldLoc;
	locAndOffsets.off_x = animParams.offsetX;
	locAndOffsets.off_y = animParams.offsetY;
	auto lights(FindLights(locAndOffsets, lightSearchRadius));

	mRenderedLastFrame++;

	MdfRenderOverrides overrides;
	overrides.alpha = alpha / 255.0f;
	material->Bind(mDevice, lights, &overrides);
	mAasRenderer.RenderWithoutMaterial(animatedModel.get(), animParams);

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
		return y <= mToY || (y == mToY && x <= mToX);
	}

	LockedMapSector& Next() {
		mLockedSector = std::make_unique<LockedMapSector>(x, y);
		if (++x > mToX) {
			x = mFromX;
			++y;
		}
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

	auto atPos = atLocation.ToInches2D();

	while (sectorIterator.HasNext()) {
		auto& sector = sectorIterator.Next();

		auto lightIt = sector.GetLights();
		while (lightIt.HasNext()) {
			auto& light = lightIt.Next();

			int type;
			uint32_t color;
			XMFLOAT3 direction;
			float range, phi;
			auto lightPos = light.position.ToInches2D();

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
						gameSystems->GetParticleSys().Remove(light.partSys.handle);
						light.partSys.handle = 0;
					}

					/*
					If the nighttime particle system has not yet been started,
					do it here.
					*/
					auto& nightPartSys = light.light2.partSys;
					if (!nightPartSys.handle && nightPartSys.hashCode) {
						auto centerOfTile = light.position.ToInches3D(light.offsetZ);
						nightPartSys.handle = gameSystems->GetParticleSys().CreateAt(
							nightPartSys.hashCode, centerOfTile
						);
					}
				} else {
					type = light.type;
					color = light.color;
					direction = light.direction;
					range = light.range;
					phi = light.phi;

					// This is just the inverse of what we're doing at night (see above)
					auto& nightPartSys = light.light2.partSys;
					if (nightPartSys.handle) {
						gameSystems->GetParticleSys().Remove(nightPartSys.handle);
						nightPartSys.handle = 0;
					}

					auto& dayPartSys = light.partSys;
					if (!dayPartSys.handle && dayPartSys.hashCode) {
						auto centerOfTile = light.position.ToInches3D(light.offsetZ);
						dayPartSys.handle = gameSystems->GetParticleSys().CreateAt(
							dayPartSys.hashCode, centerOfTile
						);
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
				auto normalizedDir = XMVector3Normalize(XMLoadFloat3(&direction));
				XMStoreFloat4(&light3d.dir, normalizedDir);
				light3d.dir.w = 0;
			}
			else if (type == 3) {
				light3d.type = Light3dType::Spot;
				auto normalizedDir = XMVector3Normalize(XMLoadFloat3(&direction));
				XMStoreFloat4(&light3d.dir, normalizedDir);
				light3d.dir.w = 0;
			}
			else if (type == 1) {
				light3d.type = Light3dType::Point;
				light3d.dir.x = direction.x;
				light3d.dir.y = direction.y;
				light3d.dir.z = direction.z;
			}

			// Some vanilla lights are broken
			if (light3d.dir.x == 0.0f && light3d.dir.y == 0.0f && light3d.dir.z == 0.0f) {
				light3d.dir.x = 0.0f;
				light3d.dir.y = 0.0f;
				light3d.dir.z = 1.0f;
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

bool MapObjectRenderer::IsObjectOnScreen(LocAndOffsets &location, float offsetZ, float radius, float renderHeight)
{

	auto centerOfTile3d = location.ToInches3D();
	auto screenPos = gameView->WorldToScreenUi(centerOfTile3d);

	// This checks if the object's screen bounding box is off screen
	auto bbLeft = screenPos.x - radius;
	auto bbRight = screenPos.x + radius;
	auto bbTop = screenPos.y - (offsetZ + renderHeight + radius) * cos45;
	auto bbBottom = bbTop + (2 * radius + renderHeight) * cos45;

	auto screenWidth = gameView->GetWidth();
	auto screenHeight = gameView->GetHeight();
	if (bbRight < 0 || bbBottom < 0 || bbLeft > screenWidth || bbTop > screenHeight) {
		return false;
	}

	return true;

}

void MapObjectRenderer::RenderMirrorImages(objHndl obj,
										   const gfx::AnimatedModelParams &animParams,
										   gfx::AnimatedModel &model,
										   gsl::span<Light3d> lights)
{
	auto mirrorImages = d20Sys.d20Query(obj, DK_QUE_Critter_Has_Mirror_Image);

	if (!mirrorImages) {
		return;
	}

	// The rotation of the mirror images is animated
	static uint32_t lastRenderTime = -1;
	static float rotation = 0;
	if (lastRenderTime != -1)
	{
		float elapsedSecs = (timeGetTime() - lastRenderTime) / 1000.0f;
		// One full rotation (2PI) in 16 seconds
		rotation += elapsedSecs * XM_2PI / 16.0f;
		
		// Wrap the rotation around
		while (rotation >= XM_2PI) {
			rotation -= XM_2PI;
		}
	}
	lastRenderTime = timeGetTime();

	// The images should partially overlap the actual model
	auto radius = objects.GetRadius(obj) * 0.75f;

	for (size_t i = 0; i < mirrorImages; ++i) {
		// Draw one half on the left and the other on the right, 
	    // if there are an uneven number, the excess image is drawn on the left
		int pos = i + 1;
		if (pos > (int) mirrorImages / 2) {
			pos = pos - mirrorImages - 1;
		}

		// Generate a world matrix that applies the translation
		MdfRenderOverrides overrides;
		overrides.useWorldMatrix = true;
		auto xTrans = cosf(rotation) * pos * radius;
		auto yTrans = sinf(rotation) * pos * radius;
		XMStoreFloat4x4(&overrides.worldMatrix, XMMatrixTranslation(xTrans, 0, yTrans));
		overrides.alpha = 0.31f;

		mAasRenderer.Render(&model, animParams, lights, &overrides);
	}
}

void MapObjectRenderer::RenderShadowMapShadow(objHndl obj,
	const gfx::AnimatedModelParams &animParams,
	gfx::AnimatedModel & model,
	const Light3d &globalLight,
	int alpha)
{
	
	LocAndOffsets loc{ { animParams.x, animParams.y }, animParams.offsetX, animParams.offsetY };
	auto worldPos{ loc.ToInches3D(animParams.offsetZ) };

	auto radius = objects.GetRadius(obj);
	auto height = objects.GetRenderHeight(obj);

	std::array<AnimatedModel*, 3> models;
	std::array<const AnimatedModelParams*, 3> params;
	
	// The critter model always has a shadow
	size_t modelCount = 1;
	models[0] = &model;
	params[0] = &animParams;

	auto primaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	gfx::AnimatedModelPtr primaryWeaponModel, secondaryWeaponModel;
	gfx::AnimatedModelParams primaryWeaponParams, secondaryWeaponParams;
	if (primaryWeapon)
	{
		primaryWeaponModel = objects.GetAnimHandle(primaryWeapon);
		primaryWeaponParams = objects.GetAnimParams(primaryWeapon);
		
		models[modelCount] = primaryWeaponModel.get();
		params[modelCount] = &primaryWeaponParams;
		modelCount++;
	}

	auto secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	if (secondaryWeapon) {
		secondaryWeaponModel = objects.GetAnimHandle(secondaryWeapon);
		secondaryWeaponParams = objects.GetAnimParams(secondaryWeapon);

		models[modelCount] = secondaryWeaponModel.get();
		params[modelCount] = &secondaryWeaponParams;
		modelCount++;
	}

	mAasRenderer.RenderShadowMapShadow(
		gsl::span(&models[0], modelCount),
		gsl::span(&params[0], modelCount),
		worldPos,
		radius,
		height,
		globalLight.dir,
		alpha / 255.0f,
		config.softShadows
	);

}

void MapObjectRenderer::RenderBlobShadow(objHndl handle, gfx::AnimatedModel &model, gfx::AnimatedModelParams &animParams, int alpha)
{
	auto& shapeRenderer3d = tig->GetShapeRenderer3d();

	std::array<gfx::ShapeVertex3d, 4> corners;

	LocAndOffsets loc{ { animParams.x, animParams.y}, animParams.offsetX, animParams.offsetY };
	auto center = loc.ToInches3D(animParams.offsetZ);

	auto radius = objects.GetRadius(handle);

	corners[0].pos.x = center.x - radius;
	corners[0].pos.y = center.y;
	corners[0].pos.z = center.z - radius;
	corners[0].uv = { 0, 0 };
	
	corners[1].pos.x = center.x + radius;
	corners[1].pos.y = center.y;
	corners[1].pos.z = center.z - radius;
	corners[1].uv = { 1, 0 };

	corners[2].pos.x = center.x + radius;
	corners[2].pos.y = center.y;
	corners[2].pos.z = center.z + radius;
	corners[2].uv = { 1, 1 };
			
	corners[3].pos.x = center.x - radius;
	corners[3].pos.y = center.y;
	corners[3].pos.z = center.z + radius;
	corners[3].uv = { 0, 1 };

	XMCOLOR color(mBlobShadowMaterial->GetSpec()->diffuse);
	color.a = (color.a * alpha) / 255;
	shapeRenderer3d.DrawQuad(corners, *mBlobShadowMaterial, color);
}

static class RenderFix : public TempleFix {
public:

	static void obj_render(objHndl handle, int flag, locXY unk, int x);
	static void obj_render_highlight(objHndl handle, int shaderId);
	static void SetShadowType(int type);
	static int GetShadowType();
	static void SetShowHighlight(BOOL enable);

	void apply() override {
		replaceFunction(0x10026560, obj_render);
		replaceFunction(0x10023EC0, obj_render_highlight);
		replaceFunction(0x10020AA0, SetShadowType);
		replaceFunction(0x10020B00, GetShadowType);
		replaceFunction(0x1001D7D0, SetShowHighlight);
		// obj_render_in_ui
		replaceFunction<int(objHndl, int, int, float, float)>(0x100243b0, [](objHndl objId, int x, int y, float rotation, float scale) {

			auto charGen = scale != 1.5f;

			// Char creation makes assumption about the main menu which no longer hold, so we
			// manually restore the 1x scale of the normal world before we render
			auto orgScale = gameView->GetZoom();
			gameView->SetZoom(1.0f);
			if (charGen) {
				x = (int)((gameView->GetWidth() - 788) / 2) + 120;
				y = (int)((gameView->GetHeight() - 497) / 2) + 180;
			}
			gameRenderer->GetMapObjectRenderer().RenderObjectInUi(objId, x, y, rotation, 1.5f);
			gameView->SetZoom(orgScale);
			return 0;
		});
	}

} fix;

void RenderFix::obj_render(objHndl handle, int flag, locXY loc, int x) {
	throw TempleException("Should not be called directly.");
}

void RenderFix::obj_render_highlight(objHndl handle, int shaderId)
{
	auto mdfMaterial{ tig->GetMdfFactory().GetById(shaderId) };

	gameRenderer->GetMapObjectRenderer().RenderObjectHighlight(handle, mdfMaterial);

}

void RenderFix::SetShadowType(int type) {
	config.SetVanillaInt("shadow_type", type);
}

int RenderFix::GetShadowType() {
	int shadowType = config.GetVanillaInt("shadow_type");
	return shadowType;
}

void RenderFix::SetShowHighlight(BOOL enable)
{
	if (!gameRenderer) {
		return;
	}
	gameRenderer->GetMapObjectRenderer().SetShowHighlight(enable == TRUE);
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
