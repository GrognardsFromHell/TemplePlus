#pragma once

#include "common.h"

// these flags are generated based on picker specs and used inside the raycast function in 10022360
enum GameRaycastFlags : uint32_t {
	GRF_HITTEST_SEL_CIRCLE = 1,
	GRF_HITTEST_CYLINDER = 2, // this is set as a default
	GRF_HITTEST_MESH = 4, // this is set as a default - looks like "Get radius from Aas"
	GRF_HITTEST_3D = 6,
	GRF_ExcludeScenery = 8,
	GRF_ExcludeItems = 16,
	GRF_ExcludePortals = 32,
	GRF_ExcludeContainers = 64,
	GRF_ExcludeCritters = 0x80,
	GRF_ExcludeDead = 0x100,
	GRF_ExcludeUnconscious = 0x200	
};

inline GameRaycastFlags operator|(GameRaycastFlags lhs, GameRaycastFlags rhs) {
	return static_cast<GameRaycastFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

enum RaycastFlags : uint32_t // used in the ObjIterator
{
	HasToBeCleared = 1,
	HasRadius = 2,
	HasSourceObj = 4,
	ExcludeItemObjects = 8, // excludes obj_t_weapon through obj_t_gneric and obj_t_bag
	StopAfterFirstBlockerFound = 0x10, // probably a performance optimization; also probably a source of bugs!
	StopAfterFirstFlyoverFound = 0x20,
	RequireDistToSourceLessThanTargetDist = 0x40, // I think this flag has another more general role, not quite sure yet
	HasRangeLimit = 0x80,
	HasTargetObj = 0x100,
	GetObjIntersection =0x200, // will return the first point along the ray where it intersects with the found object/tile
	IgnoreFlyover = 0x400, // probably used for LOS or archery queries
	ExcludePortals = 0x800, // doors etc.
	FoundCoverProvider = 0x80000000 // subtile marked as Blocker OR (FlyOver AND Cover)
};

enum RaycastResultFlags : int
{
	RaycastResultFlag1 = 1,
	BlockerSubtile = 2,
	FlyoverSubtile = 4,
	RaycastResultFlag8 = 8
};

struct RaycastResultItem
{
	int flags;
	int field4;
	LocAndOffsets loc;
	objHndl obj;
	LocAndOffsets intersectionPoint;
	float intersectionDistance; // dist from origin along the line where the object/tile intersects with the ray
	int field34;
};

struct RaycastPacket
{
	RaycastFlags flags;
	int field4;
	LocAndOffsets origin;
	LocAndOffsets targetLoc;
	float radius;
	int field2C;
	objHndl sourceObj;
	objHndl target;
	float rayRangeInches; // limits the distance from the origin 
	RaycastResultItem * results;
	int resultCount;
	RaycastPacket()
	{
		this->flags = HasToBeCleared;
		this->resultCount = 0;
		this->results = 0;
		this->origin.location.locx = 0;
		this->origin.location.locy = 0;
		this->origin.off_x = 0;
		this->origin.off_y = 0;
		this->targetLoc.location.locx = 0;
		this->targetLoc.location.locy = 0;
		this->targetLoc.off_x = 0;
		this->targetLoc.off_y = 0;
	}

	int Raycast();
	int RaycastShortRange();
	void RaycastPacketFree();
	~RaycastPacket();
};

struct RaycastPointSearchPacket
{
	float originAbsX;
	float originAbsY;
	float targetAbsX;
	float targetAbsY;
	float ux; //normalized direction x component
	float uy;
	float rangeInch;
	float absOdotU; // dot product of the origin point and the direction vector, normalized by the direction vector norm
	float radius;

	RaycastPointSearchPacket(const XMFLOAT2 &origin, const XMFLOAT2 &endPt);
	bool IsPointInterceptedBySegment(const XMFLOAT2 &v, float objRadius);
};

struct PointAlongSegment
{
	float absX;
	float absY;
	float distFromOrigin;
};

bool PickObjectOnScreen(int x, int y, objHndl *pickedHandle, GameRaycastFlags flags);
