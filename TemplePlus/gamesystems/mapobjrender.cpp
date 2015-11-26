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
#include <infrastructure/images.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/shaperenderer2d.h>
#include <temple/aasrenderer.h>
#include "../critter.h"
#include <graphics/dynamictexture.h>

#include "tig/tig_startup.h"

using namespace gfx;
using namespace temple;

static struct MapRenderAddresses : temple::AddressTable {
	bool (*IsPosExplored)(locXY loc, float offsetX, float offsetY);
	void (*WorldToLocalScreen)(vector3f pos, float* xOut, float* yOut);
	void (*EnableLights)(LocAndOffsets forLocation, float radius);

	void (*Particles_Kill)(int handle);
	int (*Particles_CreateAtPos)(int hashcode, float x, float y, float z);

	bool* globalLightEnabled;
	LegacyLight* globalLight;
	BOOL* isNight;

	MapRenderAddresses() {
		rebase(IsPosExplored, 0x1002ECB0);
		rebase(WorldToLocalScreen, 0x10029040);
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

	auto displayName{ objects.GetDisplayName(handle, handle) };

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
		&& !addresses.IsPosExplored(worldLoc, animParams.offsetX, animParams.offsetY)) {
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
		objects.UpdateRenderHeight(handle, animatedModel->GetAnimId());
		renderHeight = objects.GetRenderHeight(handle);
	}

	if (!IsObjectOnScreen(worldPosFull, animParams.offsetZ, radius, renderHeight)) {
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

	mRenderedLastFrame++;	
	mAasRenderer.Render(animatedModel.get(), animParams, lights);

	Light3d globalLight;
	if (!lights.empty()) {
		globalLight = lights[0];
	}

	if (objects.IsCritterType(type)) {
		if (alpha > 16) {
			if (mShadowType == ShadowType::ShadowMap)
			{
				// TODO: pos
				RenderShadowMapShadow(handle, animParams, *animatedModel, globalLight);
			}
			else if (mShadowType == ShadowType::Geometry)
			{
				mAasRenderer.RenderGeometryShadow(animatedModel.get(),
					animParams,
					globalLight);
			}
			else if (mShadowType == ShadowType::Blob)
			{
				RenderBlobShadow(handle, *animatedModel);
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
	else if (!objects.IsEquipmentType(type) && mShadowType == ShadowType::Geometry)
	{
		mAasRenderer.RenderGeometryShadow(animatedModel.get(),
			animParams,
			globalLight);
	}

	RenderMirrorImages(handle);

	RenderGiantFrogTongue(handle);

}

void MapObjectRenderer::RenderObjectHighlight(objHndl handle, const gfx::MdfRenderMaterialPtr & material)
{
	// TODO
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

bool MapObjectRenderer::IsObjectOnScreen(LocAndOffsets &location, float offsetZ, float radius, float renderHeight)
{

	auto centerOfTile3d = location.ToCenterOfTileAbs3D();
	auto screenPos = mDevice.GetCamera().WorldToScreenUi(centerOfTile3d);

	// This checks if the object's screen bounding box is off screen
	auto bbLeft = screenPos.x - radius;
	auto bbRight = screenPos.x + radius;
	auto bbTop = screenPos.y - (offsetZ + renderHeight + radius) * cos45;
	auto bbBottom = bbTop + (2 * radius + renderHeight) * cos45;

	auto screenWidth = mDevice.GetCamera().GetScreenWidth();
	auto screenHeight = mDevice.GetCamera().GetScreenHeight();
	if (bbRight < 0 || bbBottom < 0 || bbLeft > screenWidth || bbTop > screenHeight) {
		return false;
	}

	return true;

}

void MapObjectRenderer::RenderMirrorImages(objHndl obj)
{
	auto mirrorImage = objects.d20.d20Query(obj, DK_QUE_Critter_Has_Mirror_Image);

	if (!mirrorImage) {
		return;
	}

	throw TempleException("NYI"); // TODO
}

void MapObjectRenderer::RenderGiantFrogTongue(objHndl handle) {
		
	// Special rendering for giant frogs of various types
	auto protoNum = objects.GetProtoNum(handle);
	if (protoNum != 14057 && protoNum != 14445 && protoNum != 14300) {
		return;
	}

	throw TempleException("NYI"); // TODO

	auto grappleState = objects.getInt32(handle, obj_f_grapple_state);
	if (!grappleState) {
		return;
	}
	/*
		v138 = GiantFrogGetGrappledOpponent(ObjHnd);
		colorOnlyAlpha = (unsigned __int16)v137;
		if (grappleState)
		{
			aasHandle = (v137 >> 16) & 0xFF;
			v139 = (long double)(signed int)aasHandle;
			aasHandle = (unsigned int)v137 >> 24;
			submeshIdxh = v139;
			radius = (long double)((unsigned int)v137 >> 24);
			AasAnimatedModelGetBoneWorldMatrixByName(aas_handle, &animParams, (D3DMATRIX *)&v225, "Tongue_Ref");
			if (__PAIR__(v138, HIDWORD(v138)))
			{
				*(float *)&shaderId = COERCE_FLOAT(obj_get_aas_handle((ObjHndl)v138));
				worldLoc = (location2d)Obj_Get_Internal_Field_64bit((ObjHndl)v138, 1);
				animParams.locx = (unsigned int)worldLoc.x;
				animParams.locy = (unsigned int)worldLoc.y;
				animParams.offsetx = Obj_Get_Internal_Field_Float((ObjHndl)v138, obj_f_offset_x);
				animParams.offsety = Obj_Get_Internal_Field_Float((ObjHndl)v138, obj_f_offset_y);
				parentOpacity = Obj_Get_Internal_Field_Float((ObjHndl)v138, obj_f_offset_z);
				aasHandle = (unsigned __int8)sector_get_elevation(
					worldLoc.x,
					worldLoc.y,
					animParams.offsetx,
					animParams.offsety);
				animParams.offsetz = parentOpacity - (long double)(signed int)aasHandle;
				animParams.rotation = Obj_Get_Internal_Field_Float((ObjHndl)v138, obj_f_rotation);
				LODWORD(animParams.rotationpitch) = 0;
				*(float *)&aasHandle = COERCE_FLOAT(Obj_Get_Internal_Field_32bit_Int((ObjHndl)v138, 8));
				animParams.scale = (long double)(signed int)aasHandle * 0.0099999998;
				AasAnimatedModelGetBoneWorldMatrixByName(shaderId, &animParams, (D3DMATRIX *)&infoOut, "Bip01 Spine1");
				objFlags = v245;
				offsetY = v247;
				Obj_Get_Internal_Field_Float((ObjHndl)v138, obj_f_3d_render_height);
				v140 = v246 - v237;
				v141 = v247 - absY;
				parentOpacity = v141;
				v142 = v245 - v236;
				*(float *)&aasHandle = v142;
				*(float *)&aas_handle = sqrt(v142 * v142 + v141 * v141 + v140 * v140);
				v143 = 1.0 / *(float *)&aas_handle;
				v233 = *(float *)&aasHandle * v143;
				v234 = v143 * v140;
				v235 = v143 * parentOpacity;
				if (*(float *)&aas_handle > 0.0)
					*(float *)&aas_handle = *(float *)&aas_handle - 6.0;
			}
			else
			{
				*(float *)&aas_handle = 120.0;
				objFlags = v233 * 120.0 + v236;
				offsetY = v235 * 120.0 + absY;
			}
			if (dword_102ACB64 == -1)
				tig_shader_register("art/meshes/Monsters/GiantFrog/tongue.mdf", &dword_102ACB64);
			obj_get_radius(ObjHnd);
			switch (colorOnlyAlpha)
			{
			case 1u:
				v144 = submeshIdxh + 28.284271;
				submeshIdxh = v144;
				if (v144 > *(float *)&aas_handle)
				{
					LOWORD(v137) = 2;
					submeshIdxh = *(float *)&aas_handle;
				}
				break;
			case 2u:
				v145 = submeshIdxh - 28.284271;
				submeshIdxh = v145;
				if ((v145 < 0.0) | __UNORDERED__(v145, 0.0))
				{
					Object_Set_Field_32bit(ObjHnd, obj_f_grapple_state, 7);
					LOWORD(v137) = 0;
					submeshIdxh = 0.0;
				}
				break;
			case 3u:
				v146 = submeshIdxh + 28.284271;
				submeshIdxh = v146;
				if (v146 > *(float *)&aas_handle)
				{
					LOWORD(v137) = 4;
					submeshIdxh = *(float *)&aas_handle;
					v147 = CritterGetWeaponAnimId(ObjHnd, 34);
					anim_obj_set_aas_anim_id(ObjHnd, (AnimationIds)v147);
					v148 = CritterGetWeaponAnimId((ObjHndl)v138, 28);
					anim_obj_set_aas_anim_id((ObjHndl)v138, (AnimationIds)v148);
				}
				break;
			case 4u:
				submeshIdxh = *(float *)&aas_handle;
				break;
			case 5u:
				v149 = *(float *)&aas_handle - 12.0;
				radius = v149;
				if ((v149 < 0.0) | __UNORDERED__(v149, 0.0))
					radius = 0.0;
				LOWORD(v137) = 6;
				goto LABEL_205;
			case 6u:
				LABEL_205:
					submeshIdxh = submeshIdxh - 14.142136;
					absX_4 = offsetY - v235 * 14.142136;
					absX = objFlags - v233 * 14.142136;
					abs_coord_to_location(absX, absX_4, &worldLoc, &objFlags, &offsetY);
					*(_QWORD *)&v152.offsetx = __PAIR__(LODWORD(offsetY), LODWORD(objFlags));
					v152.xy = worldLoc;
					Obj_Move((ObjHndl)v138, v152);
					if ((submeshIdxh < (long double)radius) | __UNORDERED__(submeshIdxh, radius))
					{
						v153 = v235 * radius + absY;
						v154 = v233 * radius + v236;
						abs_coord_to_location(v154, v153, &worldLoc, &objFlags, &offsetY);
						*(_QWORD *)&v155.offsetx = __PAIR__(LODWORD(offsetY), LODWORD(objFlags));
						v155.xy = worldLoc;
						Obj_Move((ObjHndl)v138, v155);
						submeshIdxh = radius;
						LOWORD(v137) = 4;
					}
					break;
			case 7u:
				submeshIdxh = submeshIdxh - 14.142136;
				v156 = offsetY - v235 * 14.142136;
				v157 = objFlags - v233 * 14.142136;
				abs_coord_to_location(v157, v156, &worldLoc, &objFlags, &offsetY);
				*(_QWORD *)&v158.offsetx = __PAIR__(LODWORD(offsetY), LODWORD(objFlags));
				v158.xy = worldLoc;
				Obj_Move((ObjHndl)v138, v158);
				if ((submeshIdxh < 0.0) | __UNORDERED__(submeshIdxh, 0.0))
				{
					abs_coord_to_location(v236, absY, &worldLoc, &objFlags, &offsetY);
					*(_QWORD *)&v159.offsetx = __PAIR__(LODWORD(offsetY), LODWORD(objFlags));
					v159.xy = worldLoc;
					Obj_Move((ObjHndl)v138, v159);
					Obj_Fade_To((ObjHndl)v138, 0, 0, 16, 0);
					LOWORD(v137) = 0;
					submeshIdxh = 0.0;
					v160 = CritterGetWeaponAnimId(ObjHnd, 35);
					anim_obj_set_aas_anim_id(ObjHnd, (AnimationIds)v160);
				}
				break;
			default:
				break;
			}
			Object_Set_Field_32bit(
				ObjHnd,
				obj_f_grapple_state,
				(unsigned __int16)v137 | ((((unsigned __int64)radius << 8) | (unsigned __int8)(unsigned __int64)submeshIdxh) << 16));
			v161 = 0;
			*(float *)&colorOnlyAlpha = v233 * 0.0;
			memset32(&obj_render_diffuse_96, color, 0x60u);
			obj_render_height = 0.0;
			v216 = v234 * 0.0;
			v162 = (char *)&obj_render_uv_96.v;
			*(float *)&objType__ = v235 * 0.0;
			do
			{
				v163 = (long double)SLODWORD(obj_render_height);
				*(float *)&v164 = 0.0;
				*(float *)&aasHandle = 0.0;
				v165 = (signed int)v162;
				v166 = v161;
				parentOpacity = ((cos(6.2831855 * v163 * 0.06666666666666667) - 1.0) * (submeshIdxh * 0.0014285714) * 0.5 + 1.0)
					* 3.0;
				v167 = submeshIdxh * v163 * 0.06666667;
				*(float *)&shaderId = v233 * v167;
				offsetY = v234 * v167;
				objFlags = v167 * v235;
				v168 = v163 * 0.06666667;
				do
				{
					v169 = (long double)(signed int)aasHandle;
					++v164;
					v166 += 16;
					v165 += 8;
					v170 = 1.0471976 * v169 + flt_10808CFC;
					*(float *)&aasHandle = v170;
					v171 = cos(v170);
					*(float *)&aas_handle = parentOpacity * v171;
					v172 = *(float *)&aasHandle;
					aasHandle = v164;
					v173 = sin(v172);
					v174 = parentOpacity * v173;
					*(float *)((char *)&flt_10307550 + v166) = v229 * v174
						+ v225 * *(float *)&aas_handle
						+ v236
						+ *(float *)&shaderId;
					*(float *)((char *)&flt_10307554 + v166) = v230 * v174 + v226 * *(float *)&aas_handle + v237 + offsetY;
					*(float *)((char *)&map_trap_mes + v166) = v231 * v174 + v227 * *(float *)&aas_handle + absY + objFlags;
					*(float *)&aas_handle = v171;
					*(float *)((char *)&flt_10788230 + v166) = v229 * v173
						+ v225 * *(float *)&aas_handle
						+ *(float *)&colorOnlyAlpha;
					*(float *)((char *)&map_obj_lighting + v166) = v230 * v173 + v226 * *(float *)&aas_handle + v216;
					*(float *)((char *)&map_object_rectlist + v166) = v231 * v173
						+ v227 * *(float *)&aas_handle
						+ *(float *)&objType__;
					*(float *)(v165 - 12) = v169 * 0.2;
					*(float *)(v165 - 8) = v168;
				} while ((signed int)v164 < 6);
				++LODWORD(obj_render_height);
				v162 = (char *)v165;
				v161 = v166;
			} while (v165 < (signed int)&dword_10307EA4);
			dword_10788228 = color;
			v175 = flt_10808CFC - 0.5235987901687622;
			*(float *)&aas_handle = cos(v175) * 4.5;
			offsetY = sin(v175) * 4.5;
			v176 = submeshIdxh + 9.0;
			flt_10307B60 = v229 * offsetY + v225 * *(float *)&aas_handle + v176 * v233 + v236;
			flt_10307B64 = v230 * offsetY + v226 * *(float *)&aas_handle + v176 * v234 + v237;
			flt_10307B68 = v231 * offsetY + v227 * *(float *)&aas_handle + v176 * v235 + absY;
			v177 = (v229 + v225) * 0.0 + v233;
			flt_10788840 = v177;
			v178 = (v230 + v226) * 0.0 + v234;
			*(float *)&aasHandle = v178;
			flt_10788844 = v178;
			v179 = (v231 + v227) * 0.0 + v235;
			parentOpacity = v179;
			flt_10788848 = v179;
			v180 = flt_10808CFC + 1.570796370506287;
			v181 = cos(v180) * 4.5;
			v182 = sin(v180) * 4.5;
			flt_10307B70 = v229 * v182 + v225 * v181 + v176 * v233 + v236;
			dword_10788854 = aasHandle;
			dword_10788858 = LODWORD(parentOpacity);
			dword_1078822C = color;
			dword_10788864 = aasHandle;
			dword_10788868 = LODWORD(parentOpacity);
			dword_10307EA0 = 1065353216;
			dword_10307EA4 = 1065353216;
			LODWORD(flt_10788230) = color;
			obj_render_height = 0.0;
			*(float *)&aasHandle = 0.0;
			LODWORD(offsetY) = &obj_render_indices180[1];
			flt_10307B74 = v230 * v182 + v226 * v181 + v176 * v234 + v237;
			flt_10307B78 = v231 * v182 + v227 * v181 + v176 * v235 + absY;
			flt_10788850 = v177;
			v183 = flt_10808CFC + 3.665191531181335;
			v184 = v183;
			v185 = cos(v183) * 4.5;
			v186 = sin(v184) * 4.5;
			flt_10307B80 = v229 * v186 + v225 * v185 + v176 * v233 + v236;
			flt_10307B84 = v230 * v186 + v226 * v185 + v176 * v234 + v237;
			flt_10307B88 = v231 * v186 + v227 * v185 + v176 * v235 + absY;
			flt_10788860 = v177;
			do
			{
				v187 = offsetY;
				v188 = 0;
				v189 = 6 * LODWORD(obj_render_height);
				v190 = 6 * LODWORD(obj_render_height) + 6;
				do
				{
					*(_WORD *)(LODWORD(v187) - 2) = v189 + v188;
					*LODWORD(v187) = v189 + v188 + 1;
					v191 = v189 + v188 + 7;
					*(_WORD *)(LODWORD(v187) + 4) = v189 + v188;
					*(_WORD *)(LODWORD(v187) + 2) = v191;
					*(_WORD *)(LODWORD(v187) + 6) = v191;
					*(_WORD *)(LODWORD(v187) + 8) = v190 + v188++;
					LODWORD(v187) += 12;
				} while (v188 < 5);
				v192 = aasHandle;
				v193 = 6 * (v188 + aasHandle);
				obj_render_indices180[v193 + 1] = v189;
				obj_render_indices180[v193 + 2] = v190;
				obj_render_indices180[v193 + 4] = v190;
				v194 = obj_render_height;
				obj_render_indices180[v193] = v189 + 5;
				obj_render_indices180[v193 + 3] = v189 + 5;
				obj_render_indices180[v193 + 5] = v189 + 11;
				v196 = __OFSUB__(LODWORD(offsetY) + 72, &word_10788CAA);
				v195 = LODWORD(offsetY) + 72 - (signed int)&word_10788CAA < 0;
				LODWORD(obj_render_height) = LODWORD(v194) + 1;
				LODWORD(offsetY) += 72;
				aasHandle = v192 + 6;
			} while (v195 ^ v196);
			word_10788CA8 = 90;
			word_10788CAE = 90;
			word_10788CB8 = 98;
			word_10788CBC = 98;
			word_10788CCA = 98;
			word_10788CD6 = 98;
			word_10788CDC = 98;
			word_10788CAA = 91;
			word_10788CAC = 97;
			word_10788CB0 = 97;
			word_10788CB2 = 96;
			word_10788CB4 = 92;
			word_10788CB6 = 93;
			word_10788CBA = 92;
			word_10788CBE = 97;
			word_10788CC0 = 94;
			word_10788CC2 = 95;
			word_10788CC4 = 96;
			word_10788CC6 = 94;
			word_10788CC8 = 96;
			word_10788CCC = 91;
			word_10788CCE = 92;
			word_10788CD0 = 97;
			word_10788CD2 = 93;
			word_10788CD4 = 94;
			word_10788CD8 = 96;
			word_10788CDA = 97;
			tig_shader_render_3d(
				96,
				&obj_render_pos_96,
				&obj_render_normals96,
				&obj_render_diffuse_96,
				&obj_render_uv_96,
				180,
				obj_render_indices180,
				dword_102ACB64);
		}
		*/
}

void MapObjectRenderer::ApplyGaussianBlur() {

	BlendState blendState;
	SamplerState samplerState;
	samplerState.addressU = D3DTADDRESS_CLAMP;
	samplerState.addressV = D3DTADDRESS_CLAMP;
	samplerState.magFilter = D3DTEXF_LINEAR;	
	samplerState.minFilter = D3DTEXF_LINEAR;
	samplerState.mipFilter = D3DTEXF_LINEAR;
	RasterizerState rasterizerState;
	rasterizerState.cullMode = D3DCULL_NONE;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;

	auto vs(mDevice.GetShaders().LoadVertexShader("gaussian_blur_vs"));
	Shaders::ShaderDefines horDefines;
	horDefines["HOR"] = "1";
	auto psHor(mDevice.GetShaders().LoadPixelShader("gaussian_blur_ps", horDefines));
	auto psVer(mDevice.GetShaders().LoadPixelShader("gaussian_blur_ps"));

	std::vector<MaterialSamplerBinding> samplersHor{
		{ mShadowTarget, samplerState }
	};
	std::vector<MaterialSamplerBinding> samplersVer{
		{ mShadowTargetTmp, samplerState }
	};

	Material materialHor{ blendState, depthStencilState, rasterizerState, samplersHor, vs, psHor };
	Material materialVer{ blendState, depthStencilState, rasterizerState, samplersVer, vs, psVer };

	mDevice.SetMaterial(materialHor);
	mDevice.GetDevice()->SetRenderTarget(0, mShadowTargetTmp->GetSurface());
	
	auto sw = mDevice.GetCamera().GetScreenWidth();
	auto sh = mDevice.GetCamera().GetScreenHeight();

	std::array<Vertex2d, 4> fullScreenCorners;
	fullScreenCorners[0].pos = { -1, -1, 0 };
	fullScreenCorners[0].uv = { 0, 0 };
	fullScreenCorners[1].pos = { 1, -1, 0 };
	fullScreenCorners[1].uv = { 1, 0 };
	fullScreenCorners[2].pos = { 1, 1, 0 };
	fullScreenCorners[2].uv = { 1, 1 };
	fullScreenCorners[3].pos = { -1, 1, 0 };
	fullScreenCorners[3].uv = { 0, 1 };
	tig->GetShapeRenderer2d().DrawRectangle(fullScreenCorners);

	mDevice.SetMaterial(materialVer);
	mDevice.GetDevice()->SetRenderTarget(0, mShadowTarget->GetSurface());
	tig->GetShapeRenderer2d().DrawRectangle(fullScreenCorners);
	
}

void MapObjectRenderer::RenderShadowMapShadow(objHndl obj,
	const gfx::AnimatedModelParams &animParams,
	gfx::AnimatedModel & model,
	const Light3d &globalLight)
{
	
	LocAndOffsets loc{ {animParams.x, animParams.y}, animParams.offsetX, animParams.offsetY };
	auto worldPos{ loc.ToCenterOfTileAbs() };

	auto objRadius = objects.GetRadius(obj);
	auto renderHeight = objects.GetRenderHeight(obj);

	constexpr auto shadowMapWidth = 256;
	constexpr auto shadowMapHeight = 256;
	
	XMFLOAT4 lightDir = globalLight.dir;

	float shadowMapWorldX, shadowMapWorldWidth, 
		shadowMapWorldZ, shadowMapWorldHeight;

	if (lightDir.x < 0.0) {
		shadowMapWorldX = worldPos.x - 2 * objRadius + lightDir.x * renderHeight;
		shadowMapWorldWidth = 4 * objRadius - lightDir.x * renderHeight;
	} else {
		shadowMapWorldX = worldPos.x - objRadius - objRadius;
		shadowMapWorldWidth = lightDir.x * renderHeight + 4 * objRadius;
	}

	if (lightDir.z < 0.0) {
		shadowMapWorldZ = worldPos.y - 2 * objRadius + lightDir.z * renderHeight;
		shadowMapWorldHeight = 4 * objRadius - lightDir.z * renderHeight;
	} else {
		shadowMapWorldZ = worldPos.y - 2 * objRadius;
		shadowMapWorldHeight = lightDir.z + renderHeight + 4 * objRadius;
	}
	
	/* RTT */
	if (!mShadowTarget) {
		mShadowTarget = mDevice.CreateRenderTargetTexture(D3DFMT_A8R8G8B8, shadowMapWidth, shadowMapHeight);
		mShadowTargetTmp = mDevice.CreateRenderTargetTexture(D3DFMT_A8R8G8B8, shadowMapWidth, shadowMapHeight);
	}
	CComPtr<IDirect3DSurface9> currentTarget;
	mDevice.GetDevice()->GetRenderTarget(0, &currentTarget);
	mDevice.GetDevice()->SetRenderTarget(0, mShadowTarget->GetSurface());

	BlendState blendState;
	RasterizerState rasterizerState;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	auto vs{ mDevice.GetShaders().LoadVertexShader("shadowmap_geom_vs") };
	auto ps{ mDevice.GetShaders().LoadPixelShader("diffuse_only_ps") };

	Material material{ blendState, depthStencilState, rasterizerState,{}, vs, ps };
	mDevice.SetMaterial(material);

	// Set shader params
	XMFLOAT4 floats{ shadowMapWorldX, shadowMapWorldZ, shadowMapWorldWidth, shadowMapWorldHeight };
	mDevice.GetDevice()->SetVertexShaderConstantF(0, &floats.x, 1);
	mDevice.GetDevice()->SetVertexShaderConstantF(1, &lightDir.x, 1);
	floats.x = animParams.offsetZ;
	mDevice.GetDevice()->SetVertexShaderConstantF(2, &floats.x, 1);
	XMCOLOR color(0, 0, 0, 0.5f);
	XMStoreFloat4(&floats, PackedVector::XMLoadColor(&color));
	mDevice.GetDevice()->SetVertexShaderConstantF(4, &floats.x, 1);

	mDevice.GetDevice()->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 0, 0);
	mAasRenderer.RenderWithoutMaterial(&model, animParams);

	/* RTT */

	auto primaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponPrimary);
	if (primaryWeapon)
	{
		auto weaponAnim = objects.GetAnimHandle(primaryWeapon);
		auto weaponAnimParams = objects.GetAnimParams(primaryWeapon);
		mAasRenderer.RenderWithoutMaterial(weaponAnim.get(), weaponAnimParams);
	}

	auto secondaryWeapon = critterSys.GetWornItem(obj, EquipSlot::WeaponSecondary);
	if (secondaryWeapon)
	{
		auto weaponAnim = objects.GetAnimHandle(secondaryWeapon);
		auto weaponAnimParams = objects.GetAnimParams(secondaryWeapon);
		mAasRenderer.RenderWithoutMaterial(weaponAnim.get(), weaponAnimParams);
	}

	ApplyGaussianBlur();

	mDevice.GetDevice()->SetRenderTarget(0, currentTarget);
	
	auto shadowMapWorldBottom = shadowMapWorldZ + shadowMapWorldHeight;
	auto shadowMapWorldRight = shadowMapWorldX + shadowMapWorldWidth;

	std::array<gfx::ShapeVertex3d, 4> corners;
	corners[0].pos = { shadowMapWorldX, animParams.offsetZ, shadowMapWorldZ };
	corners[1].pos = { shadowMapWorldX, animParams.offsetZ, shadowMapWorldBottom };
	corners[2].pos = { shadowMapWorldRight, animParams.offsetZ, shadowMapWorldBottom };
	corners[3].pos = { shadowMapWorldRight, animParams.offsetZ, shadowMapWorldZ };
	corners[0].uv = { 0, 0 };
	corners[1].uv = { 0, 1 };
	corners[2].uv = { 1, 1 };
	corners[3].uv = { 1, 0 };

	tig->GetShapeRenderer3d().DrawQuad(corners, 0xFFFFFFFF, mShadowTarget);

}

void MapObjectRenderer::RenderBlobShadow(objHndl handle, gfx::AnimatedModel & model)
{
	/*if (shadow_shader_id == -1)
		tig_shader_register("art/meshes/shadow.mdf", &shadow_shader_id);
	v216 = obj_get_radius(ObjHnd);
	obj_render_pos4[1].y = animParams.offsetz;
	v64 = (long double)animParams.locx * 28.284271;
	obj_render_diffuse4[0] = color;
	obj_render_diffuse4[1] = color;
	offsetY = v64;
	obj_render_diffuse4[2] = color;
	obj_render_diffuse4[3] = color;
	v65 = v64 - v216 + animParams.offsetx + 14.142136;
	obj_render_pos4[0].x = v65;
	obj_render_pos4[0].y = animParams.offsetz;
	obj_render_pos4[2].y = animParams.offsetz;
	v66 = (long double)animParams.locy * 28.284271;
	obj_render_pos4[3].y = animParams.offsetz;
	*(float *)&shaderId = v66;
	v67 = v66 - v216 + animParams.offsety + 14.142136;
	obj_render_pos4[0].z = v67;
	v68 = v67;
	obj_render_pos4[1].x = v65;
	v69 = v216 + *(float *)&shaderId + animParams.offsety + 14.142136;
	obj_render_pos4[1].z = v69;
	v70 = v216 + offsetY + animParams.offsetx + 14.142136;
	obj_render_pos4[2].x = v70;
	obj_render_pos4[2].z = v69;
	obj_render_pos4[3].x = v70;
	obj_render_pos4[3].z = v68;
	tig_shader_render_3d(
		4,
		obj_render_pos4,
		obj_render_normals4,
		obj_render_diffuse4,
		obj_render_uv4,
		2,
		obj_render_indices6,
		shadow_shader_id);*/
}

objHndl MapObjectRenderer::GiantFrogGetGrappledOpponent(objHndl giantFrog)
{
	// TODO
	throw TempleException("NYI");
}

static class RenderFix : public TempleFix {
public:

	static void obj_render(objHndl handle, int flag, locXY unk, int x);
	static void obj_render_highlight(objHndl handle, int shaderId);

	const char* name() override {
		return "B";
	}

	void apply() override {
		replaceFunction(0x10026560, obj_render);
		replaceFunction(0x10023EC0, obj_render_highlight);

	}

} fix;

void RenderFix::obj_render(objHndl handle, int flag, locXY loc, int x) {
	throw TempleException("Should not be called directly.");
}

void RenderFix::obj_render_highlight(objHndl handle, int shaderId)
{
	auto mdfMaterial{ tig->GetMdfFactory().GetById(shaderId) };
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
