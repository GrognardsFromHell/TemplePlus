
#include "stdafx.h"

#include <util/fixes.h>
#include <infrastructure/meshes.h>

#include "gamesystems/gamesystems.h"
#include "gamesystems/aas_hooks.h"
#include "config/config.h"
#include <temple/meshes.h>
#include <obj.h>

#include <algorithm>
#include <cfloat>
#include <cmath>

using namespace temple;

#pragma pack(push, 1)
struct LegacyAnimParams {
	uint32_t flags;
	uint32_t unknown;
	uint64_t locX;
	uint64_t locY;
	float offsetX;
	float offsetY;
	float offsetZ;
	float rotation;
	float scale;
	float rotationRoll;
	float rotationPitch;
	float rotationYaw;
	uint32_t parentAnim;
	char attachedBoneName[48];
};

struct LegacySubmesh {
	int field_0;
	int vertexCount;
	int primCount;
	DirectX::XMFLOAT4* positions;
	DirectX::XMFLOAT4* normals;
	DirectX::XMFLOAT2* uv;
	uint16_t* indices;
};
#pragma pack(pop)

static gfx::AnimatedModelFactory &aas() {
	return gameSystems->GetAAS();
}

static gfx::AnimatedModelParams Convert(LegacyAnimParams *params) {
	gfx::AnimatedModelParams result;
	result.x = (uint32_t)params->locX;
	result.y = (uint32_t)params->locY;
	result.scale = params->scale;
	result.offsetX = params->offsetX;
	result.offsetY = params->offsetY;
	result.offsetZ = params->offsetZ;
	result.rotation = params->rotation;
	result.rotationYaw = params->rotationYaw;
	result.rotationPitch = params->rotationPitch;
	result.rotationRoll = params->rotationRoll;
	result.attachedBoneName = params->attachedBoneName;
	if (params->flags & 2) {
		result.parentAnim = aas().BorrowByHandle(params->parentAnim);
	}
	else {
		result.parentAnim = nullptr;
	}
	return result;
}

static void FunctionReplaced() {
	throw TempleException("All functions calling this function should have been replaced.");
}

void AasHooks::apply()
{
	// AasAnimatedModelSetAnimId
	replaceFunction(0x10262540, FunctionReplaced);

	// AasAnimatedModelHasAnimId
	replaceFunction(0x10262690, FunctionReplaced);

	// AasAnimatedModelGetAnimId
	replaceFunction<int(uint32_t, gfx::EncodedAnimId*)>(0x102627e0, [](uint32_t handle, gfx::EncodedAnimId *animIdOut) {
		auto model = aas().BorrowByHandle(handle);
		*animIdOut = model->GetAnimId();
		return 0;
	});

	// AasAnimatedModelAdvance
	replaceFunction<int(uint32_t, float, float, float, LegacyAnimParams *, uint32_t *)>(0x10262c10,
		[](uint32_t handle, float deltaTimeInSecs, float deltaDistance, float deltaRotation, LegacyAnimParams *params, uint32_t *eventOut) {

		auto model = aas().BorrowByHandle(handle);
		auto events = model->Advance(deltaTimeInSecs, deltaDistance, deltaRotation, Convert(params));

		if (eventOut) {
			if (events.IsAction()) {
				(*eventOut) |= 1;
			}
			if (events.IsEnd()) {
				(*eventOut) |= 2;
			}
		}

		return 0;
	});

	// AasAnimatedModelAddAddmesh
	replaceFunction(0x10262E30, FunctionReplaced);

	// AasAnimatedModelClearAddmeshes
	replaceFunction(0x10262ec0, FunctionReplaced);

	// AasAnimatedModelGetBoneCount
	replaceFunction(0x10262f40, FunctionReplaced);

	// AasAnimatedModelGetBoneNameById
	replaceFunction(0x10262f80, FunctionReplaced);

	// AasAnimatedModelGetBoneParentId
	replaceFunction(0x10262fc0, FunctionReplaced);

	// AasAnimatedModelGetBoneWorldMatrixByName
	replaceFunction(0x10263000, FunctionReplaced);

	// AasAnimatedModelGetBoneWorldMatrixByNameForChild
	replaceFunction(0x102631e0, FunctionReplaced);

	// AasAnimatedModelSetClothFlagSth
	replaceFunction(0x102633c0, FunctionReplaced);

	// AasAnimatedModelGetSubmesh
	replaceFunction<int(uint32_t, LegacySubmesh**, LegacyAnimParams*, int)>(0x10263400, [](uint32_t handle, LegacySubmesh** submeshOut, LegacyAnimParams *params, int submeshIdx) {

		auto model = aas().BorrowByHandle(handle);

		auto submesh = model->GetSubmesh(Convert(params), submeshIdx);

		auto lgcySubmesh = new LegacySubmesh();
		lgcySubmesh->field_0 = 0;
		lgcySubmesh->indices = submesh->GetIndices().data();
		lgcySubmesh->normals = submesh->GetNormals().data();
		lgcySubmesh->positions = submesh->GetPositions().data();
		lgcySubmesh->uv = submesh->GetUV().data();
		lgcySubmesh->vertexCount = submesh->GetVertexCount();
		lgcySubmesh->primCount = submesh->GetPrimitiveCount();

		*submeshOut = lgcySubmesh;

		return 0;
	});

	// AasAnimatedModelGetSubmeshForParticles
	replaceFunction(0x102636d0, FunctionReplaced);

	// AasAnimatedModelHasBone
	replaceFunction(0x10263a10, FunctionReplaced);

	// AasAnimatedModelGetSubmeshes
	replaceFunction<int(uint32_t, int **, int *)>(0x10263a50, [](uint32_t handle, int **submeshMaterials, int *materialsCountOut) {
		auto model = aas().BorrowByHandle(handle);

		static int materialIds[25];

		auto materials = model->GetSubmeshes();

		auto count = std::min<int>(25, materials.size());
		for (int i = 0; i < count; i++) {
			materialIds[i] = materials[i];
		}
		*submeshMaterials = materialIds;
		*materialsCountOut = count;

		return 0;
	});

	// AasAnimatedModelReplaceSpecialMaterial
	replaceFunction(0x10263ab0, FunctionReplaced);

	// AasAnimatedModelSetTime
	replaceFunction(0x10263b90, FunctionReplaced);

	// AasAnimatedModelGetDistPerSec
	replaceFunction<float(uint32_t)>(0x10263da0, [](uint32_t handle) {
		auto model = aas().BorrowByHandle(handle);
		auto moveSpeed = model->GetDistPerSec();

		if (moveSpeed == 0.0f) {
			return moveSpeed;
		}

		auto mainAnim = model->GetAnimId();

		if (!mainAnim.IsWeaponAnim()) {
			return moveSpeed;
		}

		bool walk = mainAnim.GetWeaponAnim() == gfx::WeaponAnim::Walk;
		bool sneak = mainAnim.GetWeaponAnim() == gfx::WeaponAnim::Sneak;

		if (config.equalizeMoveSpeed) { // *config.speedupFactor; // disregard scaling, equalize across different models
			if (sneak) {
				if (config.fastSneakAnim) {
					moveSpeed = 190;
				}
			}
			else if (!walk) { // should be just running animations now
				if (moveSpeed < 180) {
					moveSpeed = 190; // to equalize with summoned monsters	
				}
			}
		}
		else {
			if (sneak && config.fastSneakAnim) {
				moveSpeed = moveSpeed * 190;
			}
		}

		return moveSpeed;

	});

	// AasAnimatedModelGetRotationPerSec
	replaceFunction<float(uint32_t)>(0x10263de0, [](uint32_t handle) {
		auto model = aas().BorrowByHandle(handle);
		return model->GetRotationPerSec();
	});

	// AasIsConjureAnim
	replaceFunction<int(gfx::SpellAnimType)>(0x10263e20, [](gfx::SpellAnimType animId) {
		gfx::EncodedAnimId sa(animId);
		return sa.IsCastingAnimation() ? 1 : 0;
	});

	// aas_init
	replaceFunction(0x102640b0, FunctionReplaced);

	// AasAnimatedModelFromIds
	replaceFunction(0x102641b0, FunctionReplaced);

	// AasAnimatedModelFromNames
	replaceFunction(0x102643a0, FunctionReplaced);

	// AasAnimatedModelFree
	replaceFunction<int(uint32_t)>(0x10264510, [](uint32_t handle) {
		aas().FreeHandle(handle);
		return 0;
	});

	// aas_exit
	replaceFunction(0x102645e0, FunctionReplaced);

	// AasFreeAllAnimatedModels
	replaceFunction(0x10264650, FunctionReplaced);

	breakRegion(0x10264680, 0x1026BC67);
}

// Guards for the vanilla (temple.dll) cloth simulation, used when newAnimSystem is off.
// See AasClothStuff::Simulate / SanitizeVertices in Infrastructure/src/aas/aas_cloth_sim.cpp for the same logic
// in the new animation system. Memory layout of the vanilla cloth state (matches aas::AasClothStuff):
namespace {
	struct LegacyClothState {
		uint32_t count;
		DirectX::XMFLOAT4* pos1;
		DirectX::XMFLOAT4* pos2; // current
		DirectX::XMFLOAT4* pos3; // previous
		uint8_t* bytePerVertex;  // != 0: vertex is driven by the skeleton
		int edgeCount, edgeCountDelta; void* edges;
		int edges2Count, edges2CountDelta; void* edges2;
		void* spheres; void* cylinders;
		float prevStepTime;
	};
	static_assert(sizeof(LegacyClothState) == 0x38, "vanilla cloth state layout");

	void(__fastcall* sOrgClothSimulate)(LegacyClothState*, void*, float) = nullptr;
	int(__cdecl* sOrgGetSubmesh)(AasHandle, void**, AasAnimParams*, int) = nullptr;
	int(__cdecl* sOrgGetSubmeshForParticles)(AasHandle, void**, AasAnimParams*, int) = nullptr;
	AasHandle sClothSkinHandle = 0; // model currently being skinned by GetSubmesh (the cloth sim runs inside it)

	inline bool IsFinite(const DirectX::XMFLOAT4& p) {
		return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
	}

	void __fastcall ClothSimulateGuarded(LegacyClothState* cs, void* edx, float time) {
		if (!(time > 0.0f) || !std::isfinite(time)) {
			return;
		}
		// Limit the Verlet time-correction ratio (vanilla extrapolates with 1 + 2^-dt * dt / prevDt, unbounded)
		int steps = (int)ceil(time * 200.0);
		float stepTime = steps <= 5 ? time / steps : 0.005f;
		if (!std::isfinite(cs->prevStepTime) || cs->prevStepTime < 0.0f) {
			cs->prevStepTime = 0.0f;
		}
		if (cs->prevStepTime > 0.0f && stepTime > 2.0f * cs->prevStepTime) {
			static int sLogged = 0;
			if (sLogged++ < 20) {
				logger->debug("Cloth sim: limited Verlet step ratio {:.1f} (step {} s, previous step {} s)",
					stepTime / cs->prevStepTime, stepTime, cs->prevStepTime);
			}
			cs->prevStepTime = 0.5f * stepTime;
		}

		sOrgClothSimulate(cs, edx, time);

		if (!cs->count) {
			return;
		}
		// Detect non-finite (0/0 in the vanilla spring/collision code) or runaway free vertices
		float mn[3] = { FLT_MAX, FLT_MAX, FLT_MAX }, mx[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (uint32_t i = 0; i < cs->count; i++) {
			auto& p = cs->pos2[i];
			if (!cs->bytePerVertex[i] || !IsFinite(p)) continue;
			mn[0] = std::min(mn[0], p.x); mx[0] = std::max(mx[0], p.x);
			mn[1] = std::min(mn[1], p.y); mx[1] = std::max(mx[1], p.y);
			mn[2] = std::min(mn[2], p.z); mx[2] = std::max(mx[2], p.z);
		}
		float cx = 0, cy = 0, cz = 0, limit = 1000.0f;
		if (mn[0] <= mx[0]) {
			cx = 0.5f * (mn[0] + mx[0]); cy = 0.5f * (mn[1] + mx[1]); cz = 0.5f * (mn[2] + mx[2]);
			float dx = mx[0] - mn[0], dy = mx[1] - mn[1], dz = mx[2] - mn[2];
			limit = 4.0f * sqrtf(dx * dx + dy * dy + dz * dz) + 100.0f;
		}
		auto inBounds = [&](const DirectX::XMFLOAT4& p) {
			if (!IsFinite(p)) return false;
			float dx = p.x - cx, dy = p.y - cy, dz = p.z - cz;
			return dx * dx + dy * dy + dz * dz <= limit * limit;
		};
		int bad = 0;
		for (uint32_t i = 0; i < cs->count; i++) {
			if (cs->bytePerVertex[i]) continue;
			auto& p = cs->pos2[i];
			if (!inBounds(p)) {
				bad++;
				auto& q = cs->pos3[i];
				if (inBounds(q)) p = q; else { p.x = cx; p.y = cy; p.z = cz; }
				cs->pos3[i] = p;
			}
		}
		if (bad) {
			static int sLogged = 0;
			if (sLogged++ < 20) {
				logger->warn("Cloth simulation of AAS model {} produced {} invalid vertices, resetting cloth.", sClothSkinHandle, bad);
			}
			// Re-poses the free cloth vertices from the skinned mesh later in this same GetSubmesh call
			if (sClothSkinHandle) {
				temple::GetRef<int(__cdecl)(AasHandle)>(0x102633C0)(sClothSkinHandle);
			}
		}
	}

	int __cdecl GetSubmeshTrackHandle(AasHandle handle, void** submeshOut, AasAnimParams* params, int submeshIdx) {
		auto prev = sClothSkinHandle;
		sClothSkinHandle = handle;
		auto result = sOrgGetSubmesh(handle, submeshOut, params, submeshIdx);
		sClothSkinHandle = prev;
		return result;
	}

	int __cdecl GetSubmeshForParticlesTrackHandle(AasHandle handle, void** submeshOut, AasAnimParams* params, int submeshIdx) {
		auto prev = sClothSkinHandle;
		sClothSkinHandle = handle;
		auto result = sOrgGetSubmeshForParticles(handle, submeshOut, params, submeshIdx);
		sClothSkinHandle = prev;
		return result;
	}
}

void AasDebugHooks::apply() {
	// Cloth simulation guards (vanilla AasClothStuff::Simulate @ 0x10269d50, called from GetSubmesh's skinning)
	if (config.clothSimGuards) {
		logger->info("Cloth simulation guards enabled (clothSimGuards).");
		MH_CreateHook(temple::GetPointer<void*>(0x10269d50), (void*)&ClothSimulateGuarded, (void**)&sOrgClothSimulate);
		MH_CreateHook(temple::GetPointer<void*>(0x10263400), (void*)&GetSubmeshTrackHandle, (void**)&sOrgGetSubmesh);
		MH_CreateHook(temple::GetPointer<void*>(0x102636D0), (void*)&GetSubmeshForParticlesTrackHandle, (void**)&sOrgGetSubmeshForParticles);
	} else {
		logger->info("Cloth simulation guards disabled (clothSimGuards = false).");
	}

	static int(__cdecl * orgCreateFromId)(int, int, int, AasAnimParams*, AasHandle*) =
		replaceFunction<int(__cdecl)(int, int, int, AasAnimParams*, AasHandle*)>(0x102641B0, [](int skmId, int skaId, int idleAnimId, AasAnimParams* animState, AasHandle* handleOut)->int {
		auto result = orgCreateFromId(skmId, skaId, idleAnimId, animState, handleOut);

		auto model = gameSystems->GetAAS().BorrowByHandle(*handleOut);
		auto& submeshes = model->GetSubmeshes();

		gfx::AnimatedModelParams animParams;
		memset(&animParams, 0, sizeof(animParams));
		animParams.scale = 1.0f;

		model->Advance(0.0f, 0.0f, 0.1f, animParams); // this fixes bad radius/height errors by forcing an update inside 0x102682a0 (itself called by GetSubmesh) 

		auto maxRadiusSquared = -10000.0f;
		for (uint32_t i = 0; i < submeshes.size(); i++) {
			auto submesh = model->GetSubmesh(animParams, i); // GetSubmesh calls 0x102682a0 which checks for changes in deltaTime (or other related animParams). Only when there's a change will it actually update the position values (according to the animation)
			auto positions = submesh->GetPositions();

			for (auto j = 0; j < submesh->GetVertexCount(); j++) {
				auto& pos = positions[j];

				// Distance from model origin (squared)
				auto distSq = pos.x * pos.x + pos.z * pos.z;

				if (distSq > maxRadiusSquared) {
					maxRadiusSquared = distSq;
				}
			}
		}
		
		// in case it still fails - some debug...
		if (maxRadiusSquared <= 0 || maxRadiusSquared > 4000000.0f) {
			logger->error("Bad radius calculated: {} for skmId {}", maxRadiusSquared, skmId);
			for (uint32_t i = 0; i < submeshes.size(); i++) {
				auto submesh = model->GetSubmesh(animParams, i);
				auto positions = submesh->GetPositions();

				for (auto j = 0; j < submesh->GetVertexCount(); j++) {
					auto& pos = positions[j];

					// Distance from model origin (squared)
					auto distSq = pos.x * pos.x + pos.z * pos.z;
					logger->debug("vertex {}: x,y,z= {},{},{}", j, pos.x, pos.y, pos.z);
					if (distSq > maxRadiusSquared) {
						maxRadiusSquared = distSq;
					}
				}
			}
		}
		return result;
			});
}