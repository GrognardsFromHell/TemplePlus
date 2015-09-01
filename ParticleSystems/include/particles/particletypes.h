
#pragma once

#include <memory>

class PartSysEmitter;

class ParticleType {
public:
	virtual ~ParticleType() {}

	void Simulate(PartSysEmitter *emitter, float simulTimeSecs) const;

	virtual void SpawnParticle(PartSysEmitter *emitter, int particleIdx, float timeToSimulate) const;
	
	virtual bool HasModel() const {
		return false;
	}

};

class ParticleTypePoint : public ParticleType {
public:

	static const ParticleTypePoint *GetInstance();

	void SpawnParticle(PartSysEmitter* emitter, int particleIdx, float timeToSimulate) const override;

};

class ParticleTypeSprite : public ParticleType {
public:

	static const ParticleTypeSprite *GetInstance();

};

class ParticleTypeDisc : public ParticleType {
public:

	static const ParticleTypeDisc *GetInstance();
	
};

class ParticleTypeBillboard : public ParticleType {
public:

	static const ParticleTypeBillboard *GetInstance();
	
};

class ParticleTypeModel : public ParticleType {
public:

	static const ParticleTypeModel *GetInstance();

};
