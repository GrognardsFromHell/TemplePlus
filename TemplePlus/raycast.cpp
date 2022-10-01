#include "stdafx.h"
#include "common.h"
#include "raycast.h"
#include "objlist.h"
#include "critter.h"
#include "obj.h"
#include <temple/dll.h>
#include <tig/tig.h>
#include <tig/tig_startup.h>
#include "gameview.h"
#include "gamesystems/gamesystems.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/legacymapsystems.h"
#include <graphics/collision.h>
#include <infrastructure/meshes.h>

#include <DirectXCollision.h>

#include <graphics/device.h>
#include <graphics/camera.h>

struct RaycastAddresses: temple::AddressTable
{
	
	int(__cdecl * Raycast)(RaycastPacket* objIt);
	int(__cdecl * RaycastShortRange)(RaycastPacket* objIt);
	void(__cdecl* RaycastPacketClear)(RaycastPacket * objIt);
	RaycastAddresses()
	{
		rebase(RaycastPacketClear, 0x100BABE0);
		rebase(Raycast, 0x100BACE0);
		rebase(RaycastShortRange, 0x100BC750);
	}
} addresses;

int RaycastPacket::Raycast()
{
	return addresses.Raycast(this);
}

int RaycastPacket::RaycastShortRange()
{
	return addresses.RaycastShortRange(this);
}

void RaycastPacket::RaycastPacketFree()
{
	addresses.RaycastPacketClear(this);
}

RaycastPacket::~RaycastPacket()
{
	addresses.RaycastPacketClear(this);
}

RaycastPointSearchPacket::RaycastPointSearchPacket(const XMFLOAT2& origin, const XMFLOAT2& endPt){
	originAbsX = origin.x;
	originAbsY = origin.y;
	targetAbsX = endPt.x;
	targetAbsY = endPt.y;
	auto deltaX = targetAbsX - originAbsX;
	auto deltaY = targetAbsY - originAbsY;
	this->rangeInch = sqrtf(deltaX*deltaX + deltaY * deltaY);
	auto rangeInverse = 1.0f / rangeInch;
	this->ux = deltaX * rangeInverse;
	this->uy = deltaY * rangeInverse;
	this->absOdotU = ux * originAbsX + uy * originAbsY;
}

bool RaycastPointSearchPacket::IsPointInterceptedBySegment(const XMFLOAT2& v, float objRadiusInch){
	auto radiusAdj = objRadiusInch + this->radius;
	auto radiusAdjSquare = radiusAdj* radiusAdj;
	// find the closest point on the segment by projecting the vector OV onto u
	auto projection = v.x * this->ux + v.y * this->uy - this->absOdotU;
	// There are now 3 cases:
	// 1. projection < 0
	//  means the closest point to v is at the segment origin
	if (projection < 0){
		return false;// for raycasting purposes, the point is considered "behind" the segment. Hence it is not symmetrical with the next case.
	}
	// 2. projection > segment length
	// means the closest point to v is the segment end
	if (projection > this->rangeInch){
		auto deltaX = targetAbsX - v.x;
		auto deltaY = targetAbsY - v.y;
		auto result = deltaX * deltaX + deltaY * deltaY < radiusAdjSquare;
		return result;
	}
	// 3. otherwise:
	// the closest point is O + u * projection
	auto deltaX = projection * ux + originAbsX - v.x;
	auto deltaY = projection * uy + originAbsY - v.y;
	auto result = deltaX * deltaX + deltaY * deltaY < radiusAdjSquare;
	return result;
}

namespace DX = DirectX;

// Originally @ 0x100222E0
static bool IsInSelectionCircle(objHndl handle, const XMFLOAT3 &pos)
{
	using namespace DirectX;

	auto location = objects.GetLocationFull(handle);
	auto center = location.ToInches3D();
	auto radiusSquared = objects.GetRadius(handle);
	radiusSquared *= radiusSquared;
	
	auto distFromCenter = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&pos) - XMLoadFloat3(&center)));

	return distFromCenter <= radiusSquared;
}

// Originally @ 0x10022210
static bool IsPreferredSelectionObject(objHndl selCurrent, objHndl selNew)
{
	if (!selCurrent || !selNew)
	{
		return true;
	}

	auto currentRadius = objects.GetRadius(selCurrent);
	auto newRadius = objects.GetRadius(selNew);

	// Smaller radius is a higher priority selection
	if (newRadius - 0.1f > currentRadius) {
		return false;
	}

	// If both are critters, prefer the one that is alive.
	if (objects.IsCritter(selCurrent) && objects.IsCritter(selNew) 
		&& critterSys.IsDeadOrUnconscious(selNew) 
		&& !critterSys.IsDeadOrUnconscious(selCurrent)) {
		return false;
	}
	return true;
}

// Originally @ 0x10022360
bool PickObjectOnScreen(int x, int y, objHndl * pickedHandle, GameRaycastFlags flags)
{
	if (!flags) {
		flags = GRF_HITTEST_3D;
	}

	auto worldCoord = gameView->ScreenToWorld((float) x, (float) y);

	bool hitTest3d = flags & GRF_HITTEST_3D;
	Ray3d ray;

	objHndl selectedByCircle = objHndl::null;
	objHndl selectedByCylinder = objHndl::null;
	float closestCylinderHit = std::numeric_limits<float>::max();
	objHndl selectedByMesh = objHndl::null;
	float closestMeshHit = std::numeric_limits<float>::max();

	if (hitTest3d) {
		ray = gameView->GetPickRay((float) x, (float) y);
	}

	auto worldLoc = LocAndOffsets::FromInches(worldCoord);

	// Flags for the objects to be considered
	ObjectListFilter objFilter = OLC_ALL;
	if (flags & GRF_ExcludeScenery)
		objFilter &= ~OLC_SCENERY;
	if (flags & GRF_ExcludeItems)
		objFilter &= ~OLC_ITEMS;
	if (flags & GRF_ExcludePortals)
		objFilter &= ~OLC_PORTAL;
	if (flags & GRF_ExcludeContainers)
		objFilter &= ~OLC_CONTAINER;
	if (flags & GRF_ExcludeCritters)
		objFilter &= ~OLC_CRITTERS;

	// Search radius of 1000 is large...
	// Previously it was looking for a tile range which seemed to be calculated incorrectly
	ObjList objList;
	objList.ListRadius(worldLoc, 1000, objFilter);

	for (int i = 0; i < objList.size(); i++) {
		auto handle = objList[i];

		auto location = objects.GetLocationFull(handle);

		if (objects.IsUntargetable(handle)) {
			continue;
		}

		auto type = objects.GetType(handle);
		if (!objects.IsStaticType(type) && (gameSystems->GetMapFogging().IsPosExplored(location) & 1) == 0) {
			continue;
		}

		if (objects.IsCritterType(type)) {
			if (flags & GRF_ExcludeCritters) {
				continue;
			}
			if (flags & GRF_ExcludeDead && critterSys.IsDeadNullDestroyed(handle)) {
				continue;
			}
			if (flags & GRF_ExcludeUnconscious && critterSys.IsDeadOrUnconscious(handle)) {
				continue;
			}
		}

		//auto name = objects.GetDisplayName(handle, handle);

		if (hitTest3d) {
			
			// This seems to be somewhat fishy since the hit-testing doesn't 
			// take this into account elsewhere...
			auto depth = gameSystems->GetHeight().GetDepth(location);
			auto effectiveZ = objects.GetOffsetZ(handle) - depth;

			Cylinder3d cylinder;
			cylinder.baseCenter = location.ToInches3D(effectiveZ);
			cylinder.radius = objects.GetRadius(handle);
			cylinder.height = objects.GetRenderHeight(handle);

			// Interesting. We're doubling the selection circle for certain objects
			// This is probably supposed to make clicking on corpses easier
			bool extendedSelCylinder = (type == obj_t_portal || objects.IsCritterType(type) && critterSys.IsDeadOrUnconscious(handle));
			if (extendedSelCylinder) {
				cylinder.radius *= 2;
			}

			// Always intersect with the selection cylinder of the mesh, even if the actual selection will still
			// require a mesh hittest
			float cylinderHitDist;
			if (!cylinder.HitTest(ray, cylinderHitDist)) {
				continue; // Didn't hit the cylinder
			}

			// The objects that used an extended selection cylinder will only be selected based on a precise mesh hittest
			if ((flags & GRF_HITTEST_CYLINDER) && !extendedSelCylinder) {
				if (cylinderHitDist < closestCylinderHit) {
					closestCylinderHit = cylinderHitDist;
					selectedByCylinder = handle;
				}
			}

			if (flags & GRF_HITTEST_MESH) {
				auto anim = objects.GetAnimHandle(handle);
				if (anim) {
					auto animParams = objects.GetAnimParams(handle);
					float dist;
					if (anim->HitTestRay(animParams, ray, dist) && dist < closestMeshHit) {
						closestMeshHit = dist;
						selectedByMesh = handle;
					}

				}
			}
			
		} else if (flags & GRF_HITTEST_SEL_CIRCLE) {

			// Is the mouse pointer within the circle
			if (IsInSelectionCircle(handle, worldCoord)) {

				// If an object is already selected, decide which has higher priority (if circles overlap)
				if (IsPreferredSelectionObject(selectedByCircle, handle)) {
					selectedByCircle = handle;
				}
			}

		}

	}

	// Return what was selected based on the priority mesh>cylinder>circle (which is by precision)
	// but only consider options that were enabled
	if (flags & GRF_HITTEST_MESH && selectedByMesh) {
		*pickedHandle = selectedByMesh;
		return true;
	} else if (flags & GRF_HITTEST_CYLINDER && selectedByCylinder) {
		*pickedHandle = selectedByCylinder;
		return true;
	} else if (flags & GRF_HITTEST_SEL_CIRCLE && selectedByCircle) {
		*pickedHandle = selectedByCircle;
		return true;
	} else {
		*pickedHandle = objHndl::null;
		return false;
	}

}
