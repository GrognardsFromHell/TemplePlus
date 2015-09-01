
#pragma once

#include <vector>

#include "types.h"
#include "spec.h"
#include "external.h"

enum PartSysEmitterRunFlags : uint32_t
{
	PSERF_ENDED = 0x1,
};

#pragma pack(push, 1)
struct PartSysParticleValArray
{
	int count;
	float *val;
};

class PartSysEmitterSpec;
struct PartSysEmitterBones;
class PartSysParamState;

enum ParticleStateField : uint32_t {
	PSF_X = 0,
	PSF_Y,
	PSF_Z,
	PSF_POS_VAR_X,
	PSF_POS_VAR_Y,
	PSF_POS_VAR_Z,
	PSF_VEL_X,
	PSF_VEL_Y,
	PSF_VEL_Z,
	PSF_RED,
	PSF_GREEN,
	PSF_BLUE,
	PSF_ALPHA,
	PSF_UNK1,
	PSF_UNK2,
	PSF_UNK3,
	PSF_UNK4,
	PSF_COUNT
};

class ParticleState {
public:
	explicit ParticleState(int particleCount);
	~ParticleState();

	int GetCount() const {
		return mCount;
	}
	int GetCapacity() const {
		return mCapacity;
	}
	float *GetState(ParticleStateField field) {
		return mData + (mCapacity * (uint32_t) field);
	}

private:
	ParticleState& operator=(const ParticleState&) = delete;
	ParticleState(const ParticleState&) = delete;

	int mCapacity;
	int mCount;
	float *mData = nullptr;
};

class PartSysEmitter
{
public:
	explicit PartSysEmitter(const PartSysEmitterSpecPtr &spec);
	~PartSysEmitter();
	
	const PartSysEmitterSpecPtr &GetSpec() const {
		return mSpec;
	}

	const std::vector<float> &GetParticles() {
		return mParticles;
	}

	const std::vector<PartSysParamState*> &GetParamState() const {
		return mParamState;
	}

	float GetAliveInSecs() const {
		return mAliveInSecs;
	}

	float GetOutstandingSimulation() const {
		return mOutstandingSimulation;
	}

	ObjHndl GetAttachedTo() const {
		return mAttachedTo;
	}

	const Vec3 &GetWorldPos() const {
		return mWorldPos;
	}

	void SimulateEmitterMovement(float timeToSimulateSecs);
	int ReserveParticle(float spawnedAt);
	void RefreshRandomness(int particleIdx);
	void Simulate(float timeToSimulateSecs, IPartSysExternal *external);

private:
	PartSysEmitterSpecPtr mSpec;
	std::vector<float> mParticles;
	std::vector<PartSysParamState*> mParamState;
	ParticleState mParticleState;
	float mAliveInSecs = 0;
	float mOutstandingSimulation = 0;
	ObjHndl mAttachedTo = 0;
	Vec3 mWorldPos;
	bool mEnded = false; // Indicates that emission has ended but particles may still be around

	Vec3 mObjPos; // Current known pos of mAttachedTo
	Vec3 mPrevObjPos; // Prev. known pos of mAttachedTo
	float mObjRotation = 0; // Current known rotation of mAttachedTo
	float mPrevObjRotation = 0; // Prev. known rotation of mAttachedTo

	Vec3 mVelocity; // Current velocity of this emitter based on previous acceleration

	Vec3 mWorldPosVar;

	int mFirstUsedParticle = 0; // not sure what it is *exactly* yet
	int mNextFreeParticle = 0; // not sure what it is *exactly* yet

	float GetParamValue(PartSysParamState *state, int particleIdx = 0);
	void SimulateParticles(float timeToSimulateSecs, IPartSysExternal *external);
	void UpdatePos(IPartSysExternal *external);
	void UpdateBonePos(IPartSysExternal *external);
	void ApplyAcceleration(PartSysParamId paramId,
		float timeToSimulateSecs,
		float &position,
		float &velocity);
};

struct PartSysEmitterBones
{
	int childBoneCount;
	int boneCount;
	float *distFromParent;
	float distFromParentSum;
	int *boneIds;
	int *boneParentIds;
	float *x;
	float *y;
	float *z;
	float *prevX;
	float *prevY;
	float *prevZ;
};

class PartSys {
public:
	explicit PartSys(const PartSysSpecPtr &spec);
	
	float GetAliveInSecs() const {
		return mAliveInSecs;
	}

	void SetAliveInSecs(float aliveInSecs) {
		mAliveInSecs = aliveInSecs;
	}

	float GetLastSimulated() const {
		return mLastSimulated;
	}

	void SetLastSimulated(float lastSimulated) {
		mLastSimulated = lastSimulated;
	}
		
	void Simulate(float elapsedSecs);

	ObjHndl GetAttachedTo() const {
		return mAttachedTo;
	}

	void SetAttachedTo(ObjHndl attachedTo) {
		mAttachedTo = attachedTo;
	}

private:
	static int mIdSequence;
	int mId = mIdSequence++;
	ObjHndl mAttachedTo = 0;
	float mAliveInSecs = 0.0f;
	float mLastSimulated = 0.0f; // Also in Secs since creation
	PartSysSpecPtr mSpec;
	std::vector<std::unique_ptr<PartSysEmitter>> mEmitters;
	Box2d mScreenBounds;
	
	/*
		Does this particle system's bounding box intersect 
		the screen?
	*/
	bool IsOnScreen(IPartSysExternal *external) const;

	/*
		Update this particle system's on screen bounding box according
		to it's position in the world.
	*/
	void UpdateBoundingBox(IPartSysExternal *external);
};

typedef std::shared_ptr<PartSys> PartSysPtr;

#pragma pack(pop)
