
#pragma once

#include <stdint.h>

struct PartSysEmitter;

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
};

enum PartSysParamType : uint32_t {
	PSPT_CONSTANT = 0,
	PSPT_RANDOM = 1,
	PSPT_KEYFRAMES = 2,
	PSPT_SPECIAL = 5
};

#pragma pack(push, 1)
class PartSysParam {
public:
	virtual ~PartSysParam() = 0;

	virtual PartSysParamType GetType() const = 0;
	
	virtual float GetValue() const = 0;
};

inline PartSysParam::~PartSysParam() {
}

struct PartSysParamKeyframe {
	float start;
	float value;
	float deltaPerSec;
};

class PartSysParamKeyframes : public PartSysParam {
public:
	PartSysParamKeyframes(const std::vector<PartSysParamKeyframe> &frames) : mFrames(frames) {
	}
	
	PartSysParamType GetType() const override {
		return PSPT_KEYFRAMES;
	}

	float GetValue() const override {
		return 0;
	}

	const std::vector<PartSysParamKeyframe> &GetFrames() const {
		return mFrames;
	}

private:
	std::vector<PartSysParamKeyframe> mFrames;
};

class PartSysParamConstant : public PartSysParam {
public:
	explicit PartSysParamConstant(float value) : mValue(value) {
	}

	float GetValue() const override {
		return mValue;
	}
	
	PartSysParamType GetType() const override {
		return PSPT_CONSTANT;
	}
private:
	float mValue;
};

class PartSysParamRandom : public PartSysParam {
	float base;
	float randomDelta;
};

struct PartSysParamState
{
	PartSysParam *param;
};

struct PartSysParamStateRandom
{
  PartSysParamRandom *param;
  float *particleValues;
};

struct PartSysParamStateKeyframe
{
	PartSysParamKeyframes *param;
	PartSysEmitter *emitter;
};
#pragma pack(pop)
