
#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

class IPartSysExternal;
class PartSysParamState;
class PartSysEmitter;

enum PartSysParamId : uint32_t
{
	emit_accel_X = 0x0,
	emit_accel_Y = 0x1,
	emit_accel_Z = 0x2,
	emit_velVariation_X = 0x3,
	emit_velVariation_Y = 0x4,
	emit_velVariation_Z = 0x5,
	emit_posVariation_X = 0x6,
	emit_posVariation_Y = 0x7,
	emit_posVariation_Z = 0x8,
	emit_yaw = 0x9,
	emit_pitch = 0xA,
	emit_roll = 0xB,
	emit_scale_X = 0xC,
	emit_scale_Y = 0xD,
	emit_scale_Z = 0xE,
	emit_offset_X = 0xF,
	emit_offset_Y = 0x10,
	emit_offset_Z = 0x11,
	emit_init_vel_X = 0x12,
	emit_init_vel_Y = 0x13,
	emit_init_vel_Z = 0x14,
	emit_init_alpha = 0x15,
	emit_init_red = 0x16,
	emit_init_green = 0x17,
	emit_init_blue = 0x18,
	part_accel_X = 0x19,
	part_accel_Y = 0x1A,
	part_accel_Z = 0x1B,
	part_velVariation_X = 0x1C,
	part_velVariation_Y = 0x1D,
	part_velVariation_Z = 0x1E,
	part_posVariation_X = 0x1F,
	part_posVariation_Y = 0x20,
	part_posVariation_Z = 0x21,
	part_scale_X = 0x22,
	part_scale_Y = 0x23,
	part_scale_Z = 0x24,
	part_yaw = 0x25,
	part_pitch = 0x26,
	part_roll = 0x27,
	part_alpha = 0x28,
	part_red = 0x29,
	part_green = 0x2A,
	part_blue = 0x2B,
	part_attractorBlend = 0x2C,
	PARTICLE_PARAM_COUNT
};

enum PartSysParamType : uint32_t {
	PSPT_CONSTANT = 0,
	PSPT_RANDOM = 1,
	PSPT_KEYFRAMES = 2,
	PSPT_SPECIAL = 5
};

/*
Abstract base class that encapsulates the runtime state
required to model a parameter.
*/
class PartSysEmitter;
class PartSysParamState
{
public:
	virtual void Free() {
		delete this;
	}

	virtual float GetValue(const PartSysEmitter * emitter, int particleIdx, float lifetimeSec) = 0;

	virtual void InitParticle(int particleIdx) {
	}

protected:
	virtual ~PartSysParamState() {}
};

class PartSysParam {
public:
	virtual ~PartSysParam() = 0;

	virtual PartSysParamType GetType() const = 0;
	
	virtual PartSysParamState* CreateState(size_t particleCount) const = 0;

	/*
		Returns the value a parameter shall have if it
		is undefined
	*/
	static float GetDefaultValue(PartSysParamId id);
};

inline PartSysParam::~PartSysParam() {
}

struct PartSysParamKeyframe {
	float start;
	float value;
	float deltaPerSec;
};

class PartSysParamKeyframes : public PartSysParam, public PartSysParamState {
public:
	PartSysParamKeyframes(const std::vector<PartSysParamKeyframe> &frames) : mFrames(frames) {
	}
	
	PartSysParamType GetType() const override {
		return PSPT_KEYFRAMES;
	}

	const std::vector<PartSysParamKeyframe> &GetFrames() const {
		return mFrames;
	}

	PartSysParamState* CreateState(size_t /*particleCount*/) const override {
		return const_cast<PartSysParamKeyframes*>(this);
	}

	float GetValue(const PartSysEmitter*, int /*particleIdx*/, float /*lifetimeSec*/) override {
		// TODO: Implement
		return 0;
	}

	void Free() override {
		// Do nothing since we're owned by the particle system spec instead
	}

private:
	std::vector<PartSysParamKeyframe> mFrames;
};

class PartSysParamConstant : public PartSysParam, public PartSysParamState {
public:
	explicit PartSysParamConstant(float value) : mValue(value) {
	}

	float GetValue() const {
		return mValue;
	}

	PartSysParamType GetType() const override {
		return PSPT_CONSTANT;
	}

	PartSysParamState* CreateState(size_t /*particleCount*/) const override {
		return const_cast<PartSysParamConstant*>(this);
	}
	
	float GetValue(const PartSysEmitter*, int /*particleIdx*/, float /*lifetimeSec*/) override {
		return mValue;
	}

	void Free() override {
		// Do nothing since we're owned by the particle system spec instead
	}

private:
	float mValue;
};

class PartSysRandomGen {
public:
	
	static constexpr uint16_t MAX_VALUE = 0x7FFF;

	static constexpr float MAX_VALUE_FACTOR = 1.0f / MAX_VALUE;

	static uint16_t NextValue() {
		mState = 0x19660D * mState + 0x3C6EF35F;
		return (mState >> 8) & 0x7FFF;
	}

	static float NextValue(float rangeInclusive) {
		return NextValue() * rangeInclusive / MAX_VALUE_FACTOR;
	}
	
private:
	static uint32_t mState;

};

class PartSysParamStateRandom : public PartSysParamState {
public:
	
	explicit PartSysParamStateRandom(int particleCount, float base, float variance)
		: mParticles(particleCount), mBase(base), mVariance(variance) {
		// Create a value for each parameter
		for (int i = 0; i < particleCount; ++i) {
			PartSysParamStateRandom::InitParticle(i);
		}
	}

	float GetValue(const PartSysEmitter* /*emitter*/, int particleIdx, float /*lifetimeSec*/) override {
		return mParticles[particleIdx];
	}
	
	void InitParticle(int particleIdx) override {
		mParticles[particleIdx] = mBase + PartSysRandomGen::NextValue(mVariance);
	}

private:
	std::vector<float> mParticles;
	float mBase;
	float mVariance;
};

class PartSysParamRandom : public PartSysParam {
public:
	PartSysParamRandom(float base, float variance) : mBase(base), mVariance(variance) {
	}

	PartSysParamType GetType() const override {
		return PSPT_RANDOM;
	}

	float GetBase() const {
		return mBase;
	}

	float GetVariance() const {
		return mVariance;
	}

	PartSysParamState* CreateState(size_t particleCount) const override {
		return new PartSysParamStateRandom(particleCount, mBase, mVariance);
	}

private:
	float mBase;
	float mVariance;
};

enum PartSysParamSpecialType {
	PSPST_RADIUS
};

class PartSysParamSpecial : public PartSysParam, public PartSysParamState {
public:
	explicit PartSysParamSpecial(PartSysParamSpecialType specialType)
		: mSpecialType(specialType) {
	}

	PartSysParamType GetType() const override {
		return PSPT_SPECIAL;
	}

	PartSysParamSpecialType GetSpecialType() const {
		return mSpecialType;
	}

	PartSysParamState* CreateState(size_t /*particleCount*/) const override {
		return const_cast<PartSysParamSpecial*>(this);
	}
	
	float GetValue(const PartSysEmitter* emitter, int particleIdx, float lifetimeSec) override;

	void Free() override {
		// Do nothing since we're owned by the particle system spec instead
	}
private:
	PartSysParamSpecialType mSpecialType;
};
