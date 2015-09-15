#pragma once

#include <vector>

#include "types.h"
#include "spec.h"
#include "external.h"

enum PartSysEmitterRunFlags : uint32_t {
	PSERF_ENDED = 0x1,
};

#pragma pack(push, 1)
struct PartSysParticleValArray {
	int count;
	float* val;
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
	// These seem to be used for polar coordinate based positioning
	PSF_POS_INCLINATION,
	PSF_POS_AZIMUTH,
	PSF_POS_RADIUS,
	PSF_ROTATION,
	PSF_COUNT
};

class ParticleRange {
public:
	ParticleRange(int start, int end) : mStart(start), mEnd(end) {
	}

	int GetStart() const {
		return mStart;
	}

	int GetEnd() const {
		return mEnd;
	}

private:
	const int mStart;
	const int mEnd; // Exclusive
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

	void SetState(ParticleStateField field, int particleIdx, float value) {
		*GetStatePtr(field, particleIdx) = value;
	}

	float GetState(ParticleStateField field, int particleIdx) const {
		return *GetStatePtr(field, particleIdx);
	}

	float* GetStatePtr(ParticleStateField field, int particleIdx) {
		return mData + (mCapacity * (uint32_t)field) + particleIdx;
	}

	float* GetStatePtr(ParticleStateField field, int particleIdx) const {
		return mData + (mCapacity * (uint32_t)field) + particleIdx;
	}

private:
	int mCapacity;
	int mCount;
	float* mData = nullptr;

	ParticleState& operator=(const ParticleState&) = delete;
	ParticleState(const ParticleState&) = delete;

};

class ParticleIterator {
public:
	explicit ParticleIterator(int start, int end, int size)
		: mIndex(start), mEnd(end), mSize(size) {
	}

	bool HasNext() const {
		return mIndex != mEnd;
	}

	int Next() {
		auto result = mIndex++;
		if (mIndex == mSize) {
			mIndex = 0;
		}
		return result;
	}

private:
	int mIndex;
	const int mEnd;
	const int mSize;
};

class PartSysEmitter {
	friend class ParticleIterator;
public:
	explicit PartSysEmitter(const PartSysEmitterSpecPtr& spec);
	~PartSysEmitter();

	const PartSysEmitterSpecPtr& GetSpec() const {
		return mSpec;
	}

	bool IsDead() const;

	int GetActiveCount() const {
		if (mNextFreeParticle < mFirstUsedParticle) {
			return (mParticleAges.size() - mFirstUsedParticle) + mNextFreeParticle;
		} else {
			return mNextFreeParticle - mFirstUsedParticle;
		}
	}

	ParticleRange GetActiveRange() const {
		return ParticleRange(mFirstUsedParticle, mNextFreeParticle);
	}

	const std::vector<float>& GetParticles() const {
		return mParticleAges;
	}
	
	std::vector<float>& GetParticles() {
		return mParticleAges;
	}

	const std::vector<PartSysParamState*>& GetParamState() const {
		return mParamState;
	}

	PartSysParamState* GetParamState(PartSysParamId paramId) const {
		return mParamState[paramId];
	}

	ParticleIterator NewIterator() const {
		return ParticleIterator(mFirstUsedParticle, mNextFreeParticle, mParticleAges.size());
	}

	float GetParamValue(PartSysParamId paramId, int particleIdx, float lifetime, float defaultValue = 0.0f) const {
		auto state = GetParamState(paramId);
		if (state) {
			return state->GetValue(this, particleIdx, lifetime);
		} else {
			return defaultValue;
		}
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

	const Vec3& GetWorldPos() const {
		return mWorldPos;
	}

	const Vec3& GetWorldPosVar() const {
		return mWorldPosVar;
	}

	const Vec3& GetObjPos() const {
		return mObjPos;
	}

	const Vec3& GetPrevObjPos() const {
		return mPrevObjPos;
	}

	float GetObjRotation() const {
		return mObjRotation;
	}

	float GetPrevObjRotation() const {
		return mPrevObjRotation;
	}

	float GetParticleAge(int particleIdx) const {
		return mParticleAges[particleIdx];
	}

	float GetParticleSpawnTime(int particleIdx) const {
		return mAliveInSecs - GetParticleAge(particleIdx);
	}

	ParticleState& GetParticleState() {
		return mParticleState;
	}

	const ParticleState& GetParticleState() const {
		return mParticleState;
	}

	void SetWorldPos(const Vec3 &worldPos) {
		mWorldPos = worldPos;
	}

	void PruneExpiredParticles();

	void Reset();
	
	void SimulateEmitterMovement(float timeToSimulateSecs);
	int ReserveParticle(float spawnedAt);
	void RefreshRandomness(int particleIdx);
	void Simulate(float timeToSimulateSecs, IPartSysExternal* external);	

private:
	PartSysEmitterSpecPtr mSpec;
	std::vector<float> mParticleAges;
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

	float GetParamValue(PartSysParamState* state, int particleIdx = 0);
	void UpdatePos(IPartSysExternal* external);
	void UpdateBonePos(IPartSysExternal* external);
	void ApplyAcceleration(PartSysParamId paramId,
	                       float timeToSimulateSecs,
	                       float& position,
	                       float& velocity);
};

struct PartSysEmitterBones {
	int childBoneCount;
	int boneCount;
	float* distFromParent;
	float distFromParentSum;
	int* boneIds;
	int* boneParentIds;
	float* x;
	float* y;
	float* z;
	float* prevX;
	float* prevY;
	float* prevZ;
};

class PartSys {
public:
	typedef std::vector<std::unique_ptr<PartSysEmitter>> EmitterList;

	explicit PartSys(const PartSysSpecPtr& spec);

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

	bool IsDead() const;

	void Simulate(float elapsedSecs);

	ObjHndl GetAttachedTo() const {
		return mAttachedTo;
	}

	void SetAttachedTo(ObjHndl attachedTo) {
		mAttachedTo = attachedTo;
	}

	const PartSysSpecPtr& GetSpec() const {
		return mSpec;
	}

	void SetWorldPos(float x, float y, float z) {
		Vec3 worldPos(x, y, z);
		for (auto &emitter : mEmitters) {
			if (emitter->GetSpec()->GetSpace() == PartSysEmitterSpace::World) {
				emitter->SetWorldPos(worldPos);
			}
		}

		mAttachedTo = 0;
		// TODO: Update the screen bounding box here, see  PartSysUpdateWorldBoundingBox
	}

	const PartSysEmitter* GetEmitter(int emitterIdx) const {
		return mEmitters.at(emitterIdx).get();
	}

	int GetEmitterCount() const {
		return mEmitters.size();
	}

	EmitterList::const_iterator begin() const {
		return mEmitters.cbegin();
	}

	EmitterList::const_iterator end() const {
		return mEmitters.cend();
	}

	const Box2d &GetScreenBounds() const {
		return mScreenBounds;
	}

	void Reset();

private:

	static int mIdSequence;
	int mId = mIdSequence++;
	ObjHndl mAttachedTo = 0;
	float mAliveInSecs = 0.0f;
	float mLastSimulated = 0.0f; // Also in Secs since creation
	PartSysSpecPtr mSpec;
	EmitterList mEmitters;
	Box2d mScreenBounds;

	/*
		Does this particle system's bounding box intersect 
		the screen?
	*/
	bool IsOnScreen(IPartSysExternal* external) const;

	/*
		Update this particle system's on screen bounding box according
		to it's position in the world.
	*/
	void UpdateBoundingBox(IPartSysExternal* external);
};

typedef std::shared_ptr<PartSys> PartSysPtr;

#pragma pack(pop)
