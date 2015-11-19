#pragma once

#include <memory>
#include <vector>

#include <graphics/mdfmaterials.h>
#include <infrastructure/meshes.h>
#include <infrastructure/exception.h>
#include <infrastructure/format.h>

#include "params.h"

namespace particles {

class PartSysSpec;
class PartSysEmitterSpec;

using PartSysSpecPtr = std::shared_ptr<PartSysSpec>;
using PartSysEmitterSpecPtr = std::shared_ptr<PartSysEmitterSpec>;

class PartSysSpec {
public:
	explicit PartSysSpec(const std::string &name);

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
	PartSysEmitterSpec(const PartSysSpec& parent, const std::string& name) 
		: mParent(parent), 
		  mName(name), 
		  mParams((int)part_attractorBlend + 1),
          mParticleType(PartSysParticleType::Point) {
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

	PartSysBlendMode GetBlendMode() const {
		return mBlendMode;
	}

	void SetBlendMode(PartSysBlendMode blendMode) {
		mBlendMode = blendMode;
	}

	const std::string& GetTextureName() const {
		return mTextureName;
	}

	void SetTextureName(const std::string& texture) {
		mTextureName = texture;
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

	const std::string& GetMeshName() const {
		return mMeshName;
	}

	void SetMeshName(const std::string& meshName) {
		mMeshName = meshName;
	}

	float GetBoxLeft() const {
		return mBoxLeft;
	}

	void SetBoxLeft(float boxLeft) {
		mBoxLeft = boxLeft;
	}

	float GetBoxTop() const {
		return mBoxTop;
	}

	void SetBoxTop(float boxTop) {
		mBoxTop = boxTop;
	}

	float GetBoxRight() const {
		return mBoxRight;
	}

	void SetBoxRight(float boxRight) {
		mBoxRight = boxRight;
	}

	float GetBoxBottom() const {
		return mBoxBottom;
	}

	void SetBoxBottom(float boxBottom) {
		mBoxBottom = boxBottom;
	}

	const PartSysParam* GetParam(PartSysParamId id) const {
		if (id < mParams.size()) {
			return mParams[id].get();
		} else {
			return nullptr;
		}
	}

	const PartSysParticleType& GetParticleType() const {
		return mParticleType;
	}

	void SetParticleType(PartSysParticleType particleType) {
		mParticleType = particleType;
	}

	void SetParam(PartSysParamId id, std::unique_ptr<PartSysParam>& param) {
		if (id < mParams.size()) {
			mParams[id].swap(param);
		} else {
			throw TempleException("Parameter index out of range: {}", id);
		}
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
	PartSysBlendMode mBlendMode = PartSysBlendMode::Add;
	std::string mTextureName;
	PartSysCoordSys mParticlePosCoordSys = PartSysCoordSys::Cartesian;
	PartSysCoordSys mParticleVelocityCoordSys = PartSysCoordSys::Cartesian;
	PartSysParticleSpace mParticleSpace = PartSysParticleSpace::World;
	std::string mMeshName;
	float mBoxLeft = -399.0f;
	float mBoxTop = -299.0f;
	float mBoxRight = 399.0f;
	float mBoxBottom = 299.0f;
	std::vector<std::unique_ptr<PartSysParam>> mParams;
	PartSysParticleType mParticleType;

	float mDelay = 0.0f;
};

}
