#include "..\include\particles\instances.h"
#include "..\include\particles\instances.h"
#include "particles/instances.h"
#include "particles/simulation.h"
#include "particles/bones.h"

namespace particles {

	IPartSysExternal* IPartSysExternal::mCurrent = nullptr;

	int PartSys::mIdSequence = 0;

	ParticleState::ParticleState(int particleCount) : mCount(particleCount), mCapacity(particleCount) {
		// Round up to 4 since SSE always processes 4 at a time
		if (mCapacity % 4 != 0) {
			mCapacity = (particleCount / 4 + 1) * 4;
		}

		auto dataSize = PSF_COUNT * mCapacity * sizeof(float);
		mData = (float*)_aligned_malloc(dataSize, 16);
	}

	ParticleState::~ParticleState() {
		if (mData) {
			_aligned_free(mData);
			mData = nullptr;
		}
	}

	PartSysEmitter::PartSysEmitter(const PartSysEmitterSpecPtr& spec) :
		mSpec(spec), mParticleAges(spec->GetMaxParticles(), 0), mParamState(PARTICLE_PARAM_COUNT, nullptr), mParticleState(spec->GetMaxParticles()),
		mWorldPos(0, 0, 0), mObjPos(0, 0, 0), mPrevObjPos(mObjPos), mVelocity(0, 0, 0), mWorldPosVar(0, 0, 0) {

		for (int i = 0; i < PARTICLE_PARAM_COUNT; ++i) {
			auto param = mSpec->GetParam((PartSysParamId)i);
			if (param) {
				mParamState[i] = param->CreateState(mParticleAges.size());
			}
		}

	}

	PartSysEmitter::~PartSysEmitter() {

		for (auto state : mParamState) {
			if (state) {
				state->Free();
			}
		}

	}

	bool PartSysEmitter::IsDead(float lifetimeInSecs) const {
		
		// Emitters with permanent particles will stop emitting at some point, but cannot be dead
		// or otherwise the existing particles would be removed
		if (mSpec->IsPermanentParticles()) {
			if (mEnded && GetActiveCount() == 0) {
				// If it won't emit again, and has no active particles, it is dead anyway
				return true;
			}
			return false;
		}

		// Permanent emitters don't end, unless explicitly ended prematurely using EndPrematurely
		// This is used extensively if an effect is ended, but existing particles should run their course.
		if (mSpec->IsPermanent() && !mEnded) {
			return false;
		}

		// The maximum time the emitter needs to be kept is the sum of how long
		// it'll emit particles and the maximum lifetime of particles it emits.
		float lifespanSum = mSpec->GetLifespan() + mSpec->GetParticleLifespan();

		// Otherwise, it'll end once the emitter's lifespan has elapsed along with the last particle's lifespan
		// We're using the lifetime of the particle system here, because emitter lifetimes
		// are only increased when they are being simulated (so not while being off-screen)
		// This fixes the infamous Elemental Node lag
		return lifetimeInSecs >= lifespanSum;

	}

	void PartSysEmitter::SetAttachedTo(ObjHndl attachedTo) {
		mAttachedTo = attachedTo;

		// Reinitialize the bone state we're tracking for the object
		if (mSpec->GetSpace() == PartSysEmitterSpace::Bones) {
			mBoneState.reset();
			if (attachedTo) {
				mBoneState = std::make_unique<BonesState>(attachedTo);
			}
		}

		UpdatePos(IPartSysExternal::GetCurrent());
	}

	void PartSysEmitter::ApplyAcceleration(PartSysParamId paramId, float timeToSimulateSecs, float& position, float& velocity) {
		auto paramState = mParamState[paramId];
		if (paramState) {
			float accel = GetParamValue(paramState); // Get value from param state
			position += timeToSimulateSecs * timeToSimulateSecs * accel * 0.5f;
			velocity += timeToSimulateSecs * accel;
		}
	}

	void PartSysEmitter::PruneExpiredParticles() {
		// Cull expired particles	
		if (mSpec->IsPermanentParticles()) {
			return;
		}

		auto it = NewIterator();
		auto maxAge = mSpec->GetParticleLifespan();
		while (it.HasNext()) {
			auto particleIdx = it.Next();
			if (mParticleAges[particleIdx] > maxAge) {
				mFirstUsedParticle = particleIdx + 1;
				if (mFirstUsedParticle >= (int) mParticleAges.size()) {
					mFirstUsedParticle = 0;
				}
			}
		}
	}

	void PartSysEmitter::Reset() {
		mAliveInSecs = 0;
		mVelocity.x = 0;
		mVelocity.y = 0;
		mVelocity.z = 0;
		mPrevObjPos = mObjPos;
		mPrevObjRotation = mObjRotation;
		mEnded = false;
		mOutstandingSimulation = 0;
		mFirstUsedParticle = 0;
		mNextFreeParticle = 0;
	}

	void PartSysEmitter::SimulateEmitterMovement(float timeToSimulateSecs) {

		// This really doesn't seem necessary
		if (timeToSimulateSecs == 0) {
			return;
		}

		// Move the emitter according to its velocity accumulated from previous acceleration
		mWorldPos.x += timeToSimulateSecs * mVelocity.x;
		mWorldPos.y += timeToSimulateSecs * mVelocity.y;
		mWorldPos.z += timeToSimulateSecs * mVelocity.z;

		// Apply the acceleration to both the position and velocity
		ApplyAcceleration(emit_accel_X, timeToSimulateSecs, mWorldPos.x, mVelocity.x);
		ApplyAcceleration(emit_accel_Y, timeToSimulateSecs, mWorldPos.y, mVelocity.y);
		ApplyAcceleration(emit_accel_Z, timeToSimulateSecs, mWorldPos.z, mVelocity.z);

		/*
		This seems to be for constant or keyframe based velocity
	*/
		PartSysParamState* param;
		param = mParamState[emit_velVariation_X];
		if (param) {
			mWorldPos.x += GetParamValue(param) * timeToSimulateSecs;
		}
		param = mParamState[emit_velVariation_Y];
		if (param) {
			mWorldPos.y += GetParamValue(param) * timeToSimulateSecs;
		}
		param = mParamState[emit_velVariation_Z];
		if (param) {
			mWorldPos.z += GetParamValue(param) * timeToSimulateSecs;
		}

		/*
		Not sure yet, what pos variation is used for yet
	*/
		mWorldPosVar = mWorldPos;

		param = mParamState[emit_posVariation_X];
		if (param) {
			mWorldPosVar.x += GetParamValue(param);
		}
		param = mParamState[emit_posVariation_Y];
		if (param) {
			mWorldPosVar.y += GetParamValue(param);
		}
		param = mParamState[emit_posVariation_Z];
		if (param) {
			mWorldPosVar.z += GetParamValue(param);
		}
	}

	int PartSysEmitter::ReserveParticle(float particleAge) {

		auto result = mNextFreeParticle++;

		mParticleAges[result] = particleAge;

		// TODO UNKNOWN
		// if (a1->particleParams->flags & 8)
		//	a1->particles[v2] = (long double)(unsigned int)v2 * 0.12327 + a1->particles[v2];	

		if (mNextFreeParticle == mSpec->GetMaxParticles())
			mNextFreeParticle = 0;

		if (mFirstUsedParticle == mNextFreeParticle) {
			// The following effectively frees up an existing particle
			mFirstUsedParticle++;
			if (mFirstUsedParticle == mSpec->GetMaxParticles())
				mFirstUsedParticle = 0;
		}

		return result;

	}

	// Regenerate a random number for the new particle for each random parameter
	// ONLY if this emitter is permanent. Probable reason: For permanent emitter's
	// particle slots may be reused, while for non-permanent emitters they are
	// not reused.
	void PartSysEmitter::RefreshRandomness(int particleIdx) {
		if (mSpec->IsPermanent()) {
			for (auto state : mParamState) {
				if (state) {
					state->InitParticle(particleIdx);
				}
			}
		}
	}

	void PartSysEmitter::Simulate(float timeToSimulateSecs, IPartSysExternal* external) {

		UpdatePos(external);

		particles::SimulateParticleAging(this, timeToSimulateSecs);
		particles::SimulateParticleMovement(this, timeToSimulateSecs);

		// Emitter already dead or lifetime expired?
		if (mEnded || (!mSpec->IsPermanent() && mAliveInSecs > mSpec->GetLifespan())) {
			mAliveInSecs += timeToSimulateSecs;
			return;
		}

		// Particle spawning logic
		if (mSpec->IsInstant()) {
			// The secondary rate seem to be the "minimum" particles that circumvent the fidelity setting?
			// Also note how the "max particles" count is used here instead of the rate as it is below
			int scaledMaxParts = (int)(mSpec->GetParticleRateMin() +
				(mSpec->GetMaxParticles() - mSpec->GetParticleRateMin()) * external->GetParticleFidelity());
		
			if (scaledMaxParts > 0) {
				// The time here is probably only the smallest greater than 0 since there's a 
				// check in there that skips simulation if the time is zero
				SimulateEmitterMovement(0.0001f);

				// We fake spreading out the spawning over 1second equally for all particles
				// If there's just one particle, this is "NaN", but it doesnt matter
				mAliveInSecs = 0;
				auto timeStep = 1.0f / (float)(scaledMaxParts - 1);
				int remaining = scaledMaxParts - 1;
				if (remaining > 0) {
					do {
						mAliveInSecs += timeStep;

						auto particleIdx = ReserveParticle(0.0f);

						RefreshRandomness(particleIdx);

						particles::SimulateParticleSpawn(this, particleIdx, timeToSimulateSecs);
						--remaining;
					} while (remaining);
				}
				mAliveInSecs = 0;
				mAliveInSecs = timeToSimulateSecs;
				return;
			}

			// Set the emitter past it's lifespan to prevent this logic from being active again
			mAliveInSecs = mSpec->GetLifespan() + 1.0f;
			return;
		}

		// Scale the particle rate according to the fidelity setting
		auto partsPerSec = mSpec->GetEffectiveParticleRate(external->GetParticleFidelity());

		// If this emitter will not emit anything in 1000 seconds, 
		// because of the fidelity setting, simply set it to end
		if (partsPerSec <= 0.001f) {
			mAliveInSecs = mSpec->GetLifespan() + 1.0f;
			return;
		}

		mOutstandingSimulation += timeToSimulateSecs;
		mAliveInSecs += timeToSimulateSecs;

		// Calculate how many seconds go by until the emitter spawns
		// another particle
		auto secsPerPart = 1.0f / partsPerSec;

		// It is pointless to simulate more time than the lifetime of a single particle,
		// because we'll spawn more than could actually be active at the same time
		mOutstandingSimulation = std::min(mSpec->GetParticleLifespan(), mOutstandingSimulation);

		while (mOutstandingSimulation >= secsPerPart) {
			mOutstandingSimulation -= secsPerPart;

			// Simulate emitter movement just for the interval between two particle spawns
			SimulateEmitterMovement(secsPerPart);

			auto particleIdx = ReserveParticle(mOutstandingSimulation);

			RefreshRandomness(particleIdx);

			particles::SimulateParticleSpawn(this, particleIdx, timeToSimulateSecs);
		}

	}

	void PartSysEmitter::EndPrematurely()
	{
		mEnded = true;
	}

	static ObjHndl PartSysCurObj; // This is stupid, this is only used for radius determination as far as i can tell

	float PartSysEmitter::GetParamValue(PartSysParamState* state, int particleIdx) {
		return state->GetValue(this, particleIdx, mAliveInSecs);
	}

	void PartSysEmitter::UpdatePos(IPartSysExternal* external) {

		PartSysCurObj = mAttachedTo;

		// The position only needs to be updated if we're attached to an object
		if (!mAttachedTo) {
			return;
		}

		XMFLOAT4X4 boneMatrix;

		switch (mSpec->GetSpace()) {
			// We are attached to one of the bones of the object
		case PartSysEmitterSpace::NodePos:
		case PartSysEmitterSpace::NodeYpr:
			mPrevObjPos = mObjPos;
			mPrevObjRotation = mObjRotation;
			external->GetObjRotation(mAttachedTo, mObjRotation);

			if (external->GetBoneWorldMatrix(mAttachedTo, mSpec->GetNodeName(), boneMatrix)) {
				// The last row of the bone matrix is the translation part
				mObjPos.x = boneMatrix._41;
				mObjPos.y = boneMatrix._42;
				mObjPos.z = boneMatrix._43;
			} else {
				// As a fallback we use the object's location			
				external->GetObjLocation(mAttachedTo, mObjPos);
			}
			break;

			// We're attached to the world position of the object
		case PartSysEmitterSpace::ObjectPos:
		case PartSysEmitterSpace::ObjectYpr:
			mPrevObjPos = mObjPos;
			external->GetObjLocation(mAttachedTo, mObjPos);
			// The rotation is only relevant when we're in OBJECT_YPR space
			if (mSpec->GetSpace() == PartSysEmitterSpace::ObjectYpr) {
				mPrevObjRotation = mObjRotation;
				external->GetObjRotation(mAttachedTo, mObjRotation);
			}
			break;

		case PartSysEmitterSpace::Bones:
			if (!mBoneState && mAttachedTo) {
				mBoneState = std::make_unique<BonesState>(mAttachedTo);
			}
			if (mBoneState) {
				mBoneState->UpdatePos();
			}
			break;
		default:
			break;
		}

	}

	PartSys::PartSys(const PartSysSpecPtr& spec) : mId(0), mSpec(spec), mEmitters(spec->GetEmitters().size()) {
		// Instantiate the emitters
		for (size_t i = 0; i < mSpec->GetEmitters().size(); ++i) {
			auto emitterSpec = spec->GetEmitters()[i];
			mEmitters[i] = std::make_unique<PartSysEmitter>(emitterSpec);

			// Update the screen bounds
			mScreenBounds.left = std::min<float>(mScreenBounds.left, emitterSpec->GetBoxLeft());
			mScreenBounds.top = std::min<float>(mScreenBounds.top, emitterSpec->GetBoxTop());
			mScreenBounds.right = std::max<float>(mScreenBounds.right, emitterSpec->GetBoxRight());
			mScreenBounds.bottom = std::max<float>(mScreenBounds.bottom, emitterSpec->GetBoxBottom());
		}
	}

	bool PartSys::IsDead() const {
		for (auto& emitter : mEmitters) {
			if (!emitter->IsDead(mAliveInSecs)) {
				return false;
			}
		}
		return true;
	}

	void PartSys::Simulate(float elapsedSecs) {

		// This ensures that the time for the particle system advances even if we don't
		// simulate because it is off screen
		mAliveInSecs += elapsedSecs;

		// If for whatever reason this method is called multiple times in succession,
		// don't act on it
		if (mAliveInSecs == mLastSimulated) {
			return;
		}

		auto external = IPartSysExternal::GetCurrent();

		UpdateObjBoundingBox(external);

		if (!IsOnScreen(external) || mEmitters.empty()) {
			return; // Don't simulate while off-screen or having no emitters
		}

		// This is a pretty poor method if heuristically determining 
		// whether this particle system is permanent
		bool permanent = mEmitters[0]->GetSpec()->IsPermanent();

		// Previously, permanent systems were not being simulated while 
		// being offscreen, for which i dont really see a reason since 
		// there i an off-screen check above.

		// Even if the particle system is offscreen, we advance "mAliveInSecs"
		// so this ensures that if the particle system has been off screen for 2 
		// seconds, those two seconds will be simulated once it is on screen again
		float secsToSimulate = mAliveInSecs - mLastSimulated;
		mLastSimulated = mAliveInSecs;
		
		for (auto& emitter : mEmitters) {
			float simForEmitter = secsToSimulate;
			if (emitter->GetSpec()->IsPermanent()) {
				// If the emitter is permanent, simulate at most 0.5s
				simForEmitter = std::min<float>(0.5f, simForEmitter);
			}

			// Emitters with a delay are not simulated until the particle system
			// has been alive longer than the delay
			if (emitter->GetSpec()->GetDelay() > mAliveInSecs) {
				continue;
			}

			emitter->Simulate(simForEmitter, external);
		}

	}

	void PartSys::SetAttachedTo(ObjHndl attachedTo) {
		mAttachedTo = attachedTo;

		for (auto& emitter : mEmitters) {
			emitter->SetAttachedTo(attachedTo);
		}
	}

	void PartSys::SetWorldPos(IPartSysExternal* external, float x, float y, float z) {
		Vec3 worldPos(x, y, z);
		for (auto& emitter : mEmitters) {
			if (emitter->GetSpec()->GetSpace() == PartSysEmitterSpace::World) {
				emitter->SetWorldPos(worldPos);
			}
		}

		mAttachedTo = 0;
		UpdateScreenBoundingBox(external, worldPos);
	}

	void PartSys::EndPrematurely() {
		for (auto& emitter : mEmitters) {
			emitter->EndPrematurely();
		}
	}

	void PartSys::Reset() {
		mAliveInSecs = 0.0f;
		mLastSimulated = 0.0f;

		for (auto& emitter : mEmitters) {
			emitter->Reset();
		}
	}

	bool PartSys::IsOnScreen(IPartSysExternal* external) const {
		return external->IsBoxVisible(mScreenPosAbs, mScreenBounds);
	}

	// Updates the bounding box of the particle system if it's attached to an object
	void PartSys::UpdateObjBoundingBox(IPartSysExternal* external) {

		Vec3 pos;
		if (mAttachedTo && external->GetObjLocation(mAttachedTo, pos)) {
			UpdateScreenBoundingBox(external, pos);
		}

	}

	void PartSys::UpdateScreenBoundingBox(IPartSysExternal* external, const Vec3& worldPos) {

		// Where is the obj center in screen coords?
		external->WorldToScreen(worldPos, mScreenPosAbs);

	}

}
