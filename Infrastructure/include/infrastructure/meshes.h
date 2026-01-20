#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <DirectXMath.h>
#include <gsl/span>

struct Ray3d;

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
		Unarmed = 0,
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

	enum class BardInstrumentType : int {
		Flute = 0,
		Drum,
		Mandolin,
		Trumpet,
		Harp,
		Lute,
		Pipers,
		Recorder
	};

	enum class NormalAnimType : int {
		Falldown = 0,
		ProneIdle,
		ProneFidget,
		Getup,
		Magichands,
		Picklock,
		PicklockConcentrated,
		Examine,
		Throw,
		Death,
		Death2,
		Death3,
		DeadIdle,
		DeadFidget,
		DeathProneIdle,
		DeathProneFidget,
		AbjurationCasting,
		AbjurationConjuring,
		ConjurationCasting,
		ConjurationConjuring,
		DivinationCasting,
		DivinationConjuring,
		EnchantmentCasting,
		EnchantmentConjuring,
		EvocationCasting,
		EvocationConjuring,
		IllusionCasting,
		IllusionConjuring,
		NecromancyCasting,
		NecromancyConjuring,
		TransmutationCasting,
		TransmutationConjuring,
		Conceal,
		ConcealIdle,
		Unconceal,
		ItemIdle,
		ItemFidget,
		Open,
		Close,
		SkillAnimalEmpathy,
		SkillDisableDevice,
		SkillHeal,
		SkillHealConcentrated,
		SkillHide,
		SkillHideIdle,
		SkillHideFidget,
		SkillUnhide,
		SkillPickpocket,
		SkillSearch,
		SkillSpot,
		FeatTrack,
		Trip,
		Bullrush,
		Flurry,
		Kistrike,
		Tumble,
		Special1,
		Special2,
		Special3,
		Special4,
		Throw2,
		WandAbjurationCasting,
		WandAbjurationConjuring,
		WandConjurationCasting,
		WandConjurationConjuring,
		WandDivinationCasting,
		WandDivinationConjuring,
		WandEnchantmentCasting,
		WandEnchantmentConjuring,
		WandEvocationCasting,
		WandEvocationConjuring,
		WandIllusionCasting,
		WandIllusionConjuring,
		WandNecromancyCasting,
		WandNecromancyConjuring,
		WandTransmutationCasting,
		WandTransmutationConjuring,
		SkillBarbarianRage,
		OpenIdle
	};

	enum class SpellAnimType : int {
		AbjurationCasting = 81,
		AbjurationConjuring,
		ConjurationCasting,
		ConjurationConjuring,
		DivinationCasting,
		DivinationConjuring,
		EnchantmentCasting,
		EnchantmentConjuring,
		EvocationCasting,
		EvocationConjuring,
		IllusionCasting,
		IllusionConjuring,
		NecromancyCasting,
		NecromancyConjuring,
		TransmutationCasting,
		TransmutationConjuring,
		
		WandAbjurationCasting = 126,
		WandAbjurationConjuring,
		WandConjurationCasting,
		WandConjurationConjuring,
		WandDivinationCasting,
		WandDivinationConjuring,
		WandEnchantmentCasting,
		WandEnchantmentConjuring,
		WandEvocationCasting,
		WandEvocationConjuring,
		WandIllusionCasting,
		WandIllusionConjuring,
		WandNecromancyCasting,
		WandNecromancyConjuring,
		WandTransmutationCasting,
		WandTransmutationConjuring,
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

		explicit EncodedAnimId(BardInstrumentType instrumentType) : mId(sBardInstrumentAnimFlag) {
			mId |= (int)instrumentType;
		}

		explicit EncodedAnimId(NormalAnimType animType) : mId((int) animType) {
		}

		explicit EncodedAnimId(SpellAnimType animType) : mId((int)animType) {
		}

		operator int() const {
			return mId;
		}

		bool IsCastingAnimation() const;

		EncodedAnimId ConjurationToCastAnimation();

		bool IsSpecialAnim() const {
			return (mId & (sWeaponAnimFlag | sBardInstrumentAnimFlag)) != 0;
		}

		// Not for weapon/bard anims
		NormalAnimType GetNormalAnimType() const {
			return (NormalAnimType)mId;
		}

		SpellAnimType GetSpellAnimType() const {

			return (SpellAnimType)( (mId & 0xFFFFFFF) + 65 );
		}

		bool IsWeaponAnim() const {
			return (mId & sWeaponAnimFlag) != 0;
		}

		// Only valid for weapon animations
		WeaponAnimType GetWeaponLeftHand() const {
			return (WeaponAnimType)((mId >> 20) & 0x1F);
		}

		// Only valid for weapon animations
		WeaponAnimType GetWeaponRightHand() const {
			return (WeaponAnimType)((mId >> 25) & 0x1F);
		}

		// Only valid for weapon animations
		WeaponAnim GetWeaponAnim() const {
			return (WeaponAnim)(mId & 0xFFFFF);
		}

		bool IsBardInstrumentAnim() const {
			return (mId & sBardInstrumentAnimFlag) != 0;
		}

		// Only valid for bard instrument anim
		BardInstrumentType GetBardInstrumentType() const {
			return (BardInstrumentType)(mId & 7);
		}

		std::string GetName() const;

		bool ToFallback();

	private:
		// Indicates that an animation id uses the encoded format
		static constexpr int sWeaponAnimFlag = 1 << 30;
		static constexpr int sBardInstrumentAnimFlag = 1 << 31;

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
		virtual gsl::span<DirectX::XMFLOAT4> GetPositions() = 0;
		virtual gsl::span<DirectX::XMFLOAT4> GetNormals() = 0;
		virtual gsl::span<DirectX::XMFLOAT2> GetUV() = 0;
		virtual gsl::span<uint16_t> GetIndices() = 0;
	};

	class IRenderState {
	public:
		virtual ~IRenderState() = default;
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

		virtual void SetAnimId(EncodedAnimId animId) = 0;

		// This seems to reset cloth simulation state
		virtual void SetClothFlag() = 0;

		virtual std::vector<int> GetSubmeshes() = 0;

		virtual std::unique_ptr<Submesh> GetSubmesh(const AnimatedModelParams& params, int submeshIdx) = 0;

		virtual std::unique_ptr<Submesh> GetSubmeshForParticles(const AnimatedModelParams& params, int submeshIdx) = 0;

		bool HitTestRay(const AnimatedModelParams& params, const Ray3d &ray, float &hitDistance);

		/**
		 * Find the closest distance that the given point is away from the surface of this mesh.
		 */
		float GetDistanceToMesh(const AnimatedModelParams &params, DirectX::XMFLOAT3 pos);

		/**
			This calculates the effective height in world coordinate units of the model in its current
			state. Scale is the model scale in percent.
		*/
		virtual float GetHeight(int scale = 100) = 0;

		/**
			This calculates the visible radius of the model in its current state.
			The radius is the maximum distance of any vertex on the x,z plane from the models origin. 
			If the model has no vertices, 0 is returned.
			Scale is model scale in percent.
		*/
		virtual float GetRadius(int scale = 100) = 0;

		/**
		 * Sets a custom render state pointer that will be freed when this model is freed.
		 */
		virtual void SetRenderState(std::unique_ptr<IRenderState> renderState) = 0;

		/**
		 * Returns the currently assigned render state or null.
		 */
		virtual IRenderState *GetRenderState() const = 0;

		template<typename T>
		T &GetOrCreateRenderState() {
			auto current = GetRenderState();
			if (!current) {
				auto newState = std::make_unique<T>();
				current = newState.get();
				SetRenderState(std::move(newState));
			}
			return (T&) *current;
		}

	};

	struct AnimatedModelParams { // see: objects.GetAnimParams(handle)
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
		bool rotation3d = false; // Enables use of rotationRoll/rotationPitch/rotationYaw
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

		virtual std::unique_ptr<gfx::AnimatedModel> BorrowByHandle(uint32_t handle) = 0;

		virtual void FreeHandle(uint32_t handle) = 0;

		virtual void FreeAll() = 0;


	};

	class AnimatedModelRenderer {
	public:
		virtual ~AnimatedModelRenderer() {}

		virtual void Render(AnimatedModel *model,
			const AnimatedModelParams& params,
			gsl::span<Light3d> lights,
			const MdfRenderOverrides *materialOverrides = nullptr) = 0;
	};

}
