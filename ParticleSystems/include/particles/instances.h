
#pragma once

#include <stdint.h>

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

struct PartSysEmitterSpec;
struct PartSysEmitterBones;
struct PartSysParamState;
typedef uint64_t ObjHndl;

struct PartSysEmitter
{
	int particleParams;
	float *particles;
	int field_8;
	int field_C;
	PartSysEmitterSpec *emitterSpec;
	PartSysEmitterRunFlags flags;
	ObjHndl obj;
	float aliveInSecs;
	float accumSpawnTimeMaybe;
	float objX;
	float objY;
	float objZ;
	float objRot;
	float prevObjX;
	float prevObjY;
	float prevObjZ;
	float prevObjRot;
	float worldX;
	float worldY;
	float worldZ;
	int field_54;
	int field_58;
	int field_5C;
	float field_60;
	float field_64;
	float field_68;
	int field_6C;
	int field_70;
	int field_74;
	int field_78;
	PartSysEmitterBones *bones;
	PartSysParticleValArray particlesX;
	PartSysParticleValArray particlesY;
	PartSysParticleValArray particlesZ;
	PartSysParticleValArray particlesPosVarX;
	PartSysParticleValArray particlesPosVary;
	PartSysParticleValArray particlesPosVarZ;
	PartSysParticleValArray particlesVelX;
	PartSysParticleValArray particlesVelY;
	PartSysParticleValArray particlesVelZ;
	PartSysParticleValArray particlesRed;
	PartSysParticleValArray particlesGreen;
	PartSysParticleValArray particlesBlue;
	PartSysParticleValArray particlesAlpha;
	PartSysParticleValArray field_E8;
	PartSysParticleValArray field_F0;
	PartSysParticleValArray field_F8;
	PartSysParticleValArray field_100;
	int field_108;
	int field_10C;
	PartSysParamState *params[45];
	int field_1C4;
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

#pragma pack(pop)
