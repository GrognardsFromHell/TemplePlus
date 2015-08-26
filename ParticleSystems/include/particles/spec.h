#pragma once

#include <string>
#include <memory>
#include <vector>

#include <materials.h>

class PartSysSpec;
class PartSysEmitterSpec;

typedef std::shared_ptr<PartSysSpec> PartSysSpecPtr;
typedef std::shared_ptr<PartSysEmitterSpec> PartSysEmitterSpecPtr;

class PartSysSpec {
public:
	explicit PartSysSpec(const std::string& name);

	/// The name of the particle system
	const std::string& GetName() const {
		return mName;
	}

	/// The ELF32 Hash of the name
	uint32_t GetNameHash() const {
		return mNameHash;
	}

	/// All declared emitters in this particle system spec
	const std::vector<PartSysEmitterSpecPtr>& GetEmitters() const {
		return mEmitters;
	}

	PartSysEmitterSpecPtr CreateEmitter(const std::string& name);

private:
	std::string mName;
	uint32_t mNameHash; // Cached ELF-hash of the name
	std::vector<PartSysEmitterSpecPtr> mEmitters;
};

enum class PartSysEmitterSpace {
	World,
	ObjectPos,
	ObjectYpr,
	NodePos,
	NodeYpr,
	Bones
};

enum class PartSysCoordSys {
	Cartesian,
	Polar
};

enum class PartSysParticleType {
	Point,
	Sprite,
	Disc,
	Billboard,
	Model
};

enum class PartSysBlendMode {
	Add,
	Subtract,
	Blend,
	Multiply
};

enum class PartSysParticleSpace {
	World,
	SameAsEmitter,
	EmitterYpr
};

class PartSysEmitterSpec {
	friend class PartSysSpec;
public:
	PartSysEmitterSpec(const PartSysSpec& parent, const std::string& name) : mParent(parent), mName(name) {
	}

	const std::string& GetName() const {
		return mName;
	}

	const PartSysSpec& GetParent() const {
		return mParent;
	}

	float GetDelay() const {
		return mDelay;
	}

	void SetDelay(float delay) {
		mDelay = delay;
	}

	bool IsPermanent() const {
		return mPermanent;
	}

	void SetPermanent(bool enable) {
		mPermanent = enable;
	}

	float GetLifespan() const {
		return mLifespan;
	}

	void SetLifespan(float lifespan) {
		mLifespan = lifespan;
	}

	bool IsPermanentParticles() const {
		return mPermanentParticles;
	}

	void SetPermanentParticles(bool permanentParticles) {
		mPermanentParticles = permanentParticles;
	}

	float GetParticleLifespan() const {
		return mParticleLifespan;
	}

	void SetParticleLifespan(float particleLifespan) {
		mParticleLifespan = particleLifespan;
	}

	int GetMaxParticles() const {
		return mMaxParticles;
	}

	void SetMaxParticles(int maxParticles) {
		mMaxParticles = maxParticles;
	}

	float GetParticleRate() const {
		return mParticleRate;
	}

	void SetParticleRate(float particleRate) {
		mParticleRate = particleRate;
	}

	float GetParticleRateSecondary() const {
		return mParticleRateSecondary;
	}

	void SetParticleRateSecondary(float particleRateSecondary) {
		mParticleRateSecondary = particleRateSecondary;
	}

	bool IsInstant() const {
		return mInstant;
	}

	void SetInstant(bool instant) {
		mInstant = instant;
	}

	PartSysEmitterSpace GetSpace() const {
		return mSpace;
	}

	void SetSpace(PartSysEmitterSpace space) {
		mSpace = space;
	}

	const std::string& GetNodeName() const {
		return mNodeName;
	}

	void SetNodeName(const std::string& nodeName) {
		mNodeName = nodeName;
	}

	PartSysCoordSys GetCoordSys() const {
		return mCoordSys;
	}

	void SetCoordSys(PartSysCoordSys coordSys) {
		mCoordSys = coordSys;
	}

	PartSysCoordSys GetOffsetCoordSys() const {
		return mOffsetCoordSys;
	}

	void SetOffsetCoordSys(PartSysCoordSys offsetCoordSys) {
		mOffsetCoordSys = offsetCoordSys;
	}

	PartSysParticleType GetParticleType() const {
		return mParticleType;
	}

	void SetParticleType(PartSysParticleType particleType) {
		mParticleType = particleType;
	}

	PartSysBlendMode GetBlendMode() const {
		return mBlendMode;
	}

	void SetBlendMode(PartSysBlendMode blendMode) {
		mBlendMode = blendMode;
	}

	const MaterialRef& GetMaterial() const {
		return mMaterial;
	}

	void SetMaterial(const MaterialRef& material) {
		mMaterial = material;
	}

	PartSysCoordSys GetParticlePosCoordSys() const {
		return mParticlePosCoordSys;
	}

	void SetParticlePosCoordSys(PartSysCoordSys particlePosCoordSys) {
		mParticlePosCoordSys = particlePosCoordSys;
	}

	PartSysCoordSys GetParticleVelocityCoordSys() const {
		return mParticleVelocityCoordSys;
	}

	void SetParticleVelocityCoordSys(PartSysCoordSys particleVelocityCoordSys) {
		mParticleVelocityCoordSys = particleVelocityCoordSys;
	}

	PartSysParticleSpace GetParticleSpace() const {
		return mParticleSpace;
	}

	void SetParticleSpace(PartSysParticleSpace particleSpace) {
		mParticleSpace = particleSpace;
	}

private:
	const PartSysSpec& mParent;
	std::string mName;
	bool mInstant = false;
	bool mPermanent = false;
	bool mPermanentParticles = false;
	float mLifespan = 1.0f;
	float mParticleLifespan = 1.0f;
	int mMaxParticles = 1;
	float mParticleRate = 1.0f;
	float mParticleRateSecondary = 0.0f;
	PartSysEmitterSpace mSpace = PartSysEmitterSpace::World;
	std::string mNodeName;
	PartSysCoordSys mCoordSys = PartSysCoordSys::Cartesian;
	PartSysCoordSys mOffsetCoordSys = PartSysCoordSys::Cartesian;
	PartSysParticleType mParticleType = PartSysParticleType::Point;
	PartSysBlendMode mBlendMode = PartSysBlendMode::Add;
	MaterialRef mMaterial;
	PartSysCoordSys mParticlePosCoordSys = PartSysCoordSys::Cartesian;
	PartSysCoordSys mParticleVelocityCoordSys = PartSysCoordSys::Cartesian;
	PartSysParticleSpace mParticleSpace = PartSysParticleSpace::World;

	float mDelay = 0.0f;
};
