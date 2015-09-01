
#include "particles/particletypes.h"

void ParticleType::Simulate(PartSysEmitter* emitter, float simulTimeSecs) const {
	// TODO Implement
}

void ParticleType::SpawnParticle(PartSysEmitter* emitter, int particleIdx, float timeToSimulate) const {
}

const ParticleTypePoint* ParticleTypePoint::GetInstance() {
	static ParticleTypePoint instance;
	return &instance;
}

void ParticleTypePoint::SpawnParticle(PartSysEmitter* emitter, int particleIdx, float timeToSimulate) const {



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
