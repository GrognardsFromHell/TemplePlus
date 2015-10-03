
#pragma once

#include <stdint.h>
#include <obj.h>

namespace legacypartsys {

	void DumpPartSysProps();

	struct PartSysEmitterSpec;	
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
	struct PartSysParam {
		PartSysParamType type;
	};

	struct PartSysParamKeyframe : public PartSysParam {
		int frameCount;
		float *startTimes;
		float *values;
		float *deltaPerTick;
		// Variable length follows (frameCount * 12 bytes)

		PartSysParamKeyframe() {
			type = PSPT_KEYFRAMES;
		}
	};

	struct PartSysParamRandom : public PartSysParam {
		float base;
		float randomDelta;

		PartSysParamRandom() {
			type = PSPT_RANDOM;
		}
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
		PartSysParamKeyframe *param;
		PartSysEmitter *emitter;
	};

	struct PartSysSpec
	{
		char *name;
		int emitterCount;
		PartSysEmitterSpec **emitters;
	};

	enum PartSysEmitterFlags : uint32_t
	{
		PSEF_PERM = 0x1,
		PSEF_INSTANT = 0x2,
		PSEF_UNKNOWN = 0x8,
	};

	enum PartSysEmitterSpace : uint32_t
	{
		PSES_WORLD = 0x0,
		PSES_OBJECT_POS = 0x1,
		PSES_OBJECT_YPR = 0x2,
		PSES_NODE_POS = 0x3,
		PSES_NODE_YPR = 0x4,
		PSES_BONES = 0x5,
	};

	enum PartSysParticleCoordSpace : uint32_t
	{
		PSPCS_WORLD = 0x0,
		PSPCS_EMITTER = 0x1,
		PSPCS_EMITTER_YPR = 0x2,
	};

	struct PartSysParticleType;

	struct PartSysEmitterSpec
	{
		float bb0;
		float bb1;
		float bb2;
		float bb3;
		int emit_type;
		PartSysParticleType *renderType;
		int maxActiveParticles;
		float particleLifespan;
		PartSysEmitterFlags flags;
		float emitterLifespan;
		float particlesPerSecSecondary;
		float particlesPerSec;
		float delay;
		PartSysEmitterSpace emitter_space;
		int emitter_coord_sys;
		int emitter_offset_coord_sys;
		char *node_name;
		int particleType;
		int material_shader_id;
		int blend_mode;
		int particle_pos_coord_sys;
		int particle_velocity_coord_sys;
		PartSysParticleCoordSpace particle_coord_space;
		float field_5C;
		float field_60;
		int model_id;
		int field_68;
		PartSysParam *params[45];
	};

	struct PartSysEmitter;

	struct PartSys {
		uint32_t id;
		PartSysSpec *spec;
		float aliveInSecs;
		float lastAliveWhenUpdated;
		float screenBBLeft;
		float screenBBTop;
		float screenBBRight;
		float screenBBBottom;
		float x;
		float y;
		float z;
		int field2c;
		objHndl obj;
		PartSysEmitter** emitters;
		PartSys *next;
	};

	struct PartSysEmitter {
		void *particleParams;
		void *particles;
		void *field8;
		void *fieldC;
		PartSysEmitterSpec *spec;
		uint32_t flags;
		objHndl obj;
		float aliveInSecs;
		/* There's more here that is currently irrelevant */
	};

#pragma pack(pop)

}
