#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <DirectXMath.h>
#include <gsl/array_view.h>

enum AasEventFlag;

namespace gfx {

	struct Light3d;
	struct MdfRenderOverrides;
	enum class MaterialPlaceholderSlot;

	struct AnimatedModelParams;
	using AnimatedModelPtr = std::shared_ptr<class AnimatedModel>;
	using MdfRenderMaterialPtr = std::shared_ptr<class MdfRenderMaterial>;

	enum class WeaponAnim : int {
		None = 0,
		RightAttack,
		RightAttack2,
		RightAttack3,
		LeftAttack,
		LeftAttack2,
		LeftAttack3,
		Walk,
		Run,
		Idle,
		FrontHit,
		FrontHit2,
		FrontHit3,
		LeftHit,
		LeftHit2,
		LeftHit3,
		RightHit,
		RightHit2,
		RightHit3,
		BackHit,
		BackHit2,
		BackHit3,
		RightCriticalSwing,
		LeftCriticalSwing,
		Fidget,
		Fidget2,
		Fidget3,
		Sneak,
		Panic,
		RightCombatStart,
		LeftCombatStart,
		CombatIdle,
		CombatFidget,
		Special1,
		Special2,
		Special3,
		FrontDodge,
		RightDodge,
		LeftDodge,
		BackDodge,
		RightThrow,
		LeftThrow,
		LeftSnatch,
		RightSnatch,
		LeftTurn,
		RightTurn
	};

	enum class WeaponAnimType : int {
		Unarmed,
		Dagger,
		Sword,
		Mace,
		Hammer,
		Axe,
		Club,
		Battleaxe,
		Greatsword,
		Greataxe,
		Greathammer,
		Spear,
		Staff,
		Polearm,
		Bow,
		Crossbow,
		Sling,
		Shield,
		Flail,
		Chain,
		TwoHandedFlail,
		Shuriken,
		Monk
	};

	/*
	Represents an encoded animation id.
	*/
	class EncodedAnimId {
	public:
		explicit EncodedAnimId(int id) : mId(id) {
		}

		explicit EncodedAnimId(WeaponAnim anim,
		                       WeaponAnimType leftHand = WeaponAnimType::Unarmed,
		                       WeaponAnimType rightHand = WeaponAnimType::Unarmed) : mId(sWeaponAnimFlag) {
			auto animId = (int)anim;
			auto leftHandId = (int)leftHand;
			auto rightHandId = (int)rightHand;

			mId |= animId & 0xFFFFF;
			mId |= leftHandId << 20;
			mId |= rightHandId << 25;
		}

		operator int() const {
			return mId;
		}

	private:
		// Indicates that an animation id uses the encoded format
		static constexpr int sWeaponAnimFlag = 1 << 30;

		int mId;
	};

	/*
		Represents the events that can trigger when the animation
		of an animated model is advanced.
	*/
	class AnimatedModelEvents {
	public:
		AnimatedModelEvents(bool isEnd, bool isAction)
			: mIsEnd(isEnd),
			  mIsAction(isAction) {
		}

		bool IsEnd() const {
			return mIsEnd;
		}

		bool IsAction() const {
			return mIsAction;
		}

	private:
		const bool mIsEnd : 1;
		const bool mIsAction : 1;
	};

	class Submesh {
	public:
		virtual ~Submesh() {
		}

		virtual int GetVertexCount() = 0;
		virtual int GetPrimitiveCount() = 0;
		virtual gsl::array_view<DirectX::XMFLOAT4> GetPositions() = 0;
		virtual gsl::array_view<DirectX::XMFLOAT4> GetNormals() = 0;
		virtual gsl::array_view<DirectX::XMFLOAT2> GetUV() = 0;
		virtual gsl::array_view<uint16_t> GetIndices() = 0;
	};

	class AnimatedModel {
	public:
		virtual ~AnimatedModel() {
		}

		virtual uint32_t GetHandle() const = 0;

		virtual bool AddAddMesh(const std::string& filename) = 0;

		virtual bool ClearAddMeshes() = 0;

		virtual AnimatedModelEvents Advance(float deltaTime,
		                                    float deltaDistance,
		                                    float deltaRotation,
		                                    const AnimatedModelParams& params) = 0;

		virtual EncodedAnimId GetAnimId() const = 0;

		virtual int GetBoneCount() const = 0;

		virtual std::string GetBoneName(int boneId) = 0;

		virtual int GetBoneParentId(int boneId) = 0;

		virtual bool GetBoneWorldMatrixByName(
			const AnimatedModelParams& params,
			const std::string& boneName,
			DirectX::XMFLOAT4X4* worldMatrixOut) = 0;

		virtual bool GetBoneWorldMatrixByNameForChild(const AnimatedModelPtr& child,
		                                              const AnimatedModelParams& params,
		                                              const std::string& boneName,
				DirectX::XMFLOAT4X4* worldMatrixOut) = 0;


		virtual float GetDistPerSec() const = 0;

		virtual float GetRotationPerSec() const = 0;

		virtual bool HasAnim(EncodedAnimId animId) const = 0;

		virtual void SetTime(const AnimatedModelParams& params, float timeInSecs) = 0;

		virtual bool HasBone(const std::string& boneName) const = 0;

		virtual void AddReplacementMaterial(gfx::MaterialPlaceholderSlot slot,
			const gfx::MdfRenderMaterialPtr &material) = 0;

		virtual void SetAnimId(int animId) = 0;

		virtual void SetClothFlag() = 0;

		virtual std::vector<int> GetSubmeshes() = 0;

		virtual std::unique_ptr<Submesh> GetSubmesh(const AnimatedModelParams& params, int submeshIdx) = 0;

		virtual std::unique_ptr<Submesh> GetSubmeshForParticles(const AnimatedModelParams& params, int submeshIdx) = 0;

	};

	struct AnimatedModelParams {
		uint32_t x = 0;
		uint32_t y = 0;
		float offsetX = 0;
		float offsetY = 0;
		float offsetZ = 0;
		float rotation = 0;
		float scale = 1;
		float rotationRoll = 0;
		float rotationPitch = 0;
		float rotationYaw = 0;
		AnimatedModelPtr parentAnim;
		std::string attachedBoneName;
	};

	class AnimatedModelFactory {
	public:
		virtual ~AnimatedModelFactory() {
		}

		virtual AnimatedModelPtr FromIds(
			int meshId,
			int skeletonId,
			EncodedAnimId idleAnimId,
			const AnimatedModelParams& params,
			bool borrow = false) = 0;

		virtual AnimatedModelPtr FromFilenames(
			const std::string& meshFilename,
			const std::string& skeletonFilename,
			EncodedAnimId idleAnimId,
			const AnimatedModelParams& params) = 0;

	};

	class AnimatedModelRenderer {
	public:
		virtual ~AnimatedModelRenderer() {}

		virtual void Render(AnimatedModel *model,
			const AnimatedModelParams& params,
			gsl::array_view<Light3d> lights,
			const MdfRenderOverrides *materialOverrides = nullptr) = 0;
	};

}
