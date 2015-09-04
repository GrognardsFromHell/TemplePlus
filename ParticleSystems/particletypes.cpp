#include "particles/particletypes.h"
#include "particles/external.h"
#include "particles/instances.h"

inline float DegToRad(float degrees) {
	return degrees * 0.0174532925f;
}

/*
Converts from polar coordinates to cartesian coordinates.
See Wikipedia for details:
https://en.wikipedia.org/wiki/Spherical_coordinate_system
*/
inline Vec3 SphericalDegToCartesian(float azimuth, float inclination, float radius) {

	azimuth = DegToRad(azimuth);
	inclination = DegToRad(inclination);

	auto tmp = cosf(inclination) * radius;
	return Vec3{
		tmp * sinf(azimuth),
		sinf(inclination) * radius,
		tmp * cosf(azimuth)
	};

}

inline void Rotate2D(float rotation, float& x, float& z) {
	// Rotate the velocity vector according to the current object rotation in the world
	auto newX = -(cosf(rotation) * x) - sinf(rotation) * z;
	auto newZ = sinf(rotation) * x - cosf(rotation) * z;
	x = newX;
	z = newZ;
}

inline void RotateAndMove(float dX, float dY, float dZ, float rotation, float& x, float& y, float& z) {
	if (rotation != 0) {
		Rotate2D(rotation, x, z);
	}
	x += dX;
	y += dY;
	z += dZ;
}

void ParticleType::Simulate(PartSysEmitter* emitter, float simulTimeSecs) const {

}

inline float GetParticleValue(PartSysEmitter* emitter, int particleIdx, PartSysParamId paramId, float atLifetime, float defaultValue = 0.0f) {
	auto state = emitter->GetParamState(paramId);
	return state ? state->GetValue(emitter, particleIdx, atLifetime) : defaultValue;

}

inline void SetParticleParam(PartSysEmitter* emitter, int particleIdx, PartSysParamId id, float atLifetime, ParticleStateField stateField, float defaultValue) {
	auto value = GetParticleValue(emitter, particleIdx, id, atLifetime, defaultValue);
	emitter->GetParticleState().SetState(stateField, particleIdx, value);
}

void ParticleType::SpawnParticle(PartSysEmitter* emitter, int particleIdx, float timeToSimulate) const {

	const auto& spec = emitter->GetSpec();
	auto partSpawnTime = emitter->GetParticleSpawnTime(particleIdx);
	
	auto& worldPosVar = emitter->GetWorldPosVar();
	auto particleX = worldPosVar.x;
	auto particleY = worldPosVar.y;
	auto particleZ = worldPosVar.z;

	auto emitOffsetX = GetParticleValue(emitter, particleIdx, emit_offset_X, partSpawnTime);
	auto emitOffsetY = GetParticleValue(emitter, particleIdx, emit_offset_Y, partSpawnTime);
	auto emitOffsetZ = GetParticleValue(emitter, particleIdx, emit_offset_Z, partSpawnTime);

	if (spec->GetOffsetCoordSys() == PartSysCoordSys::Polar) {
		auto coords = SphericalDegToCartesian(emitOffsetX, emitOffsetY, emitOffsetZ);
		particleX += coords.x;
		particleY += coords.y;
		particleZ += coords.z;
	} else {
		particleX += emitOffsetX;
		particleY += emitOffsetY;
		particleZ += emitOffsetZ;
	}

	switch (spec->GetSpace()) {
	case PartSysEmitterSpace::ObjectPos:
	case PartSysEmitterSpace::ObjectYpr: {
		if (spec->GetParticleSpace() != PartSysParticleSpace::SameAsEmitter) {
			// TODO: Figure out this formula...
			auto scale = 1.0f - emitter->GetParticleAge(particleIdx) / timeToSimulate;
			auto prevObjPos = emitter->GetPrevObjPos();
			auto objPos = emitter->GetObjPos();
			auto dX = prevObjPos.x + (objPos.x - prevObjPos.x) * scale;
			auto dY = prevObjPos.y + (objPos.y - prevObjPos.y) * scale;
			auto dZ = prevObjPos.z + (objPos.z - prevObjPos.z) * scale;
			auto rotation = 0.0f;
			if (spec->GetSpace() == PartSysEmitterSpace::ObjectYpr) {
				rotation = emitter->GetPrevObjRotation() + (emitter->GetObjRotation() - emitter->GetPrevObjRotation()) * scale;
			}
			RotateAndMove(dX, dY, dZ, rotation, particleX, particleY, particleZ);
		}
	}
		break;

	case PartSysEmitterSpace::Bones: {
		auto scale = 1.0 - emitter->GetParticleAge(particleIdx) / timeToSimulate;
		// TODO This may apply the position of a random bone
		// TODO sub_101F4C50(emitter, scale, (int)&particleX, (int)&particleY, (int)&particleZ);
	}
		break;

	case PartSysEmitterSpace::NodePos:
	case PartSysEmitterSpace::NodeYpr: {
		if (spec->GetParticleSpace() != PartSysParticleSpace::SameAsEmitter) {
			auto scale = 1.0f - emitter->GetParticleAge(particleIdx) / timeToSimulate;
			if (spec->GetSpace() == PartSysEmitterSpace::NodeYpr) {
				auto external = IPartSysExternal::GetCurrent();
				Matrix4x4 matrix;
				XMStoreFloat4x4(&matrix, DirectX::XMMatrixIdentity());
				external->GetBoneWorldMatrix(emitter->GetAttachedTo(), spec->GetNodeName(), matrix);
				auto boneM = DirectX::XMLoadFloat4x4(&matrix);
				auto ppos = DirectX::XMVectorSet(particleX, particleY, particleZ, 1);
				auto newpos = DirectX::XMVector3TransformCoord(ppos, boneM);
				particleX = DirectX::XMVectorGetX(newpos);
				particleY = DirectX::XMVectorGetY(newpos);
				particleZ = DirectX::XMVectorGetZ(newpos);
			} else {
				auto prevObjPos = emitter->GetPrevObjPos();
				auto objPos = emitter->GetObjPos();
				auto dX = prevObjPos.x + (objPos.x - prevObjPos.x) * scale;
				auto dY = prevObjPos.y + (objPos.y - prevObjPos.y) * scale;
				auto dZ = prevObjPos.z + (objPos.z - prevObjPos.z) * scale;
				RotateAndMove(dX, dY, dZ, 0.0f, particleX, particleY, particleZ);
			}
		}
	}
		break;

	default:
		break;
	}

	auto& state = emitter->GetParticleState();
	state.SetState(PSF_X, particleIdx, particleX);
	state.SetState(PSF_Y, particleIdx, particleY);
	state.SetState(PSF_Z, particleIdx, particleZ);

	// Initialize particle color
	SetParticleParam(emitter, particleIdx, emit_init_red, partSpawnTime, PSF_RED, 255.0f);
	SetParticleParam(emitter, particleIdx, emit_init_green, partSpawnTime, PSF_GREEN, 255.0f);
	SetParticleParam(emitter, particleIdx, emit_init_blue, partSpawnTime, PSF_BLUE, 255.0f);
	SetParticleParam(emitter, particleIdx, emit_init_alpha, partSpawnTime, PSF_ALPHA, 255.0f);

	// Initialize particle velocity
	auto partVelX = GetParticleValue(emitter, particleIdx, emit_init_vel_X, partSpawnTime);
	auto partVelY = GetParticleValue(emitter, particleIdx, emit_init_vel_Y, partSpawnTime);
	auto partVelZ = GetParticleValue(emitter, particleIdx, emit_init_vel_Z, partSpawnTime);

	if (spec->GetSpace() == PartSysEmitterSpace::ObjectYpr) {
		if (spec->GetParticleSpace() != PartSysParticleSpace::SameAsEmitter) {
			auto scale = 1.0f - emitter->GetParticleAge(particleIdx) / timeToSimulate;

			auto rotation = 0.0f;
			if (spec->GetSpace() == PartSysEmitterSpace::ObjectYpr)
				rotation = (emitter->GetObjRotation() - emitter->GetPrevObjRotation()) * scale + emitter->GetPrevObjRotation();

			// Rotate the velocity vector according to the current object rotation in the world
			// TODO: Even for rotation == 0, this will flip the velocity vector
			Rotate2D(rotation, partVelX, partVelZ);
		}

	} else if (spec->GetSpace() == PartSysEmitterSpace::NodeYpr) {

		if (spec->GetParticleSpace() != PartSysParticleSpace::SameAsEmitter) {
			auto external = IPartSysExternal::GetCurrent();
			auto objId = emitter->GetAttachedTo();

			Matrix4x4 boneMatrix;
			XMStoreFloat4x4(&boneMatrix, DirectX::XMMatrixIdentity());

			external->GetBoneWorldMatrix(objId, spec->GetNodeName(), boneMatrix);

			// Construct a directional vector (not a positional one, w=0 here) for the velocity
			auto dirVec = DirectX::XMVectorSet(partVelX, partVelY, partVelZ, 0);
			auto transMat = DirectX::XMLoadFloat4x4(&boneMatrix);
			dirVec = DirectX::XMVector3TransformNormal(dirVec, transMat);

			partVelX = DirectX::XMVectorGetX(dirVec);
			partVelY = DirectX::XMVectorGetY(dirVec);
			partVelZ = DirectX::XMVectorGetZ(dirVec);
		}
	}

	// Are particle coordinates defined as polar coordinates? Convert them to cartesian here
	if (spec->GetParticleVelocityCoordSys() == PartSysCoordSys::Polar) {
		auto cartesianVel = SphericalDegToCartesian(partVelX, partVelY, partVelZ);
		partVelX = cartesianVel.x;
		partVelY = cartesianVel.y;
		partVelZ = cartesianVel.z;
	}

	state.SetState(PSF_X, particleIdx, partVelX);
	state.SetState(PSF_Y, particleIdx, partVelY);
	state.SetState(PSF_Z, particleIdx, partVelZ);

	// I don't know why it's taken at lifetime 0.
	// TODO: Figure out if this actually *never* changes?
	auto posVarX = GetParticleValue(emitter, particleIdx, part_posVariation_X, 0);
	auto posVarY = GetParticleValue(emitter, particleIdx, part_posVariation_Y, 0);
	auto posVarZ = GetParticleValue(emitter, particleIdx, part_posVariation_Z, 0);

	// For a polar system, convert these positions to cartesian to apply them to the
	// rendering position
	if (spec->GetParticlePosCoordSys() == PartSysCoordSys::Polar) {
		state.SetState(PSF_POS_AZIMUTH, particleIdx, 0);
		state.SetState(PSF_POS_INCLINATION, particleIdx, 0);
		state.SetState(PSF_POS_RADIUS, particleIdx, 0);

		// Convert to cartesian and add to the actual current particle position
		// As usual, x, y, z here are (azimuth, inclination, radius)
		auto cartesianPos = SphericalDegToCartesian(posVarX, posVarY, posVarZ);
		posVarX = cartesianPos.x;
		posVarY = cartesianPos.y;
		posVarZ = cartesianPos.z;
	}

	// Apply the position variation to the initial position and store it
	posVarX += particleX;
	posVarY += particleY;
	posVarZ += particleZ;
	state.SetState(PSF_POS_VAR_X, particleIdx, posVarX);
	state.SetState(PSF_POS_VAR_Y, particleIdx, posVarY);
	state.SetState(PSF_POS_VAR_Z, particleIdx, posVarZ);

	/*
		The following code will apply particle movement after 
		spawning a particle retroactively. This should only happen
		for high frequency particle systems that spawn multiple particles
		per frame.
		Also note how particle age instead of particle lifetime is used here
		to access parameters of the emitter.
	*/
	auto partAge = emitter->GetParticleAge(particleIdx);
	if (partAge != 0) {
		auto partAgeSquared = partAge * partAge;

		particleX += partVelX * partAge;
		particleY += partVelY * partAge;
		particleZ += partVelZ * partAge;

		auto param = emitter->GetParamState(part_accel_X);
		if (param) {
			auto accelX = param->GetValue(emitter, particleIdx, partAge);
			particleX += accelX * partAgeSquared * 0.5f;
			partVelX += accelX * partAge;
		}

		param = emitter->GetParamState(part_accel_Y);
		if (param) {
			auto accelY = param->GetValue(emitter, particleIdx, partAge);
			particleY += accelY * partAgeSquared * 0.5f;
			partVelY += accelY * partAge;
		}

		param = emitter->GetParamState(part_accel_Z);
		if (param) {
			auto accelZ = param->GetValue(emitter, particleIdx, partAge);
			particleZ += accelZ * partAgeSquared * 0.5f;
			partVelZ += accelZ * partAge;
		}

		state.SetState(PSF_VEL_X, particleIdx, partVelX);
		state.SetState(PSF_VEL_Y, particleIdx, partVelY);
		state.SetState(PSF_VEL_Z, particleIdx, partVelZ);

		param = emitter->GetParamState(part_velVariation_X);
		if (param) {
			particleX += param->GetValue(emitter, particleIdx, partAge) * partAge;
		}

		param = emitter->GetParamState(part_velVariation_Y);
		if (param) {
			particleY += param->GetValue(emitter, particleIdx, partAge) * partAge;
		}

		param = emitter->GetParamState(part_velVariation_Z);
		if (param) {
			particleZ += param->GetValue(emitter, particleIdx, partAge) * partAge;
		}

		state.SetState(PSF_X, particleIdx, particleX);
		state.SetState(PSF_Y, particleIdx, particleY);
		state.SetState(PSF_Z, particleIdx, particleZ);
	}

}

const ParticleTypePoint* ParticleTypePoint::GetInstance() {
	static ParticleTypePoint instance;
	return &instance;
}

void ParticleTypeComplex::SpawnParticle(PartSysEmitter* emitter, int particleIdx, float timeToSimulate) const {
	ParticleType::SpawnParticle(emitter, particleIdx, timeToSimulate);

	auto particleAge = emitter->GetParticleSpawnTime(particleIdx);
	auto rotation = emitter->GetParamValue(emit_yaw, particleIdx, particleAge, 0.0f);
	emitter->GetParticleState().SetState(PSF_ROTATION, particleIdx, rotation);
}

const ParticleTypeSprite* ParticleTypeSprite::GetInstance() {
	static ParticleTypeSprite instance;
	return &instance;
}

const ParticleTypeDisc* ParticleTypeDisc::GetInstance() {
	static ParticleTypeDisc instance;
	return &instance;
}

const ParticleTypeBillboard* ParticleTypeBillboard::GetInstance() {
	static ParticleTypeBillboard instance;
	return &instance;
}

const ParticleTypeModel* ParticleTypeModel::GetInstance() {
	static ParticleTypeModel instance;
	return &instance;
}
