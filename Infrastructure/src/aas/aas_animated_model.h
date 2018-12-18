
#pragma once

#include "aas/aas.h"
#include "aas/aas_math.h"

namespace aas {

	namespace DX = DirectX;

	struct AasClothStuff1;
	struct CollisionSphere;
	struct CollisionCylinder;
	class AnimPlayer;
	struct AasSubmeshWithMaterial;
	class IAnimEventHandler;

	struct VariationSelector {
		int variationId;
		float factor;
	};

	class AnimatedModel {
	public:
		AnimPlayer * runningAnimsHead = nullptr;
		AnimPlayer* newestRunningAnim = nullptr;
		bool submeshesValid = false;
		float scale;
		float scaleInv;
		std::shared_ptr<Skeleton> skeleton;
		int variationCount = 0;
		VariationSelector variations[8];
		bool hasClothBones = false;
		int clothBoneId = 0;
		int cloth_stuff1_count = 0;
		AasClothStuff1* cloth_stuff1 = nullptr;
		std::unique_ptr<CollisionSphere> collisionSpheresHead;
		std::unique_ptr<CollisionCylinder> collisionCylindersHead;
		IAnimEventHandler* eventHandler = nullptr;
		std::vector<AasSubmeshWithMaterial> submeshes;
		float drivenTime = 0.0f;
		float timeForClothSim = 0.0f;
		float drivenDistance = 0.0f;
		float drivenRotation = 0.0f;
		Matrix3x4 currentWorldMatrix; // Current world matrix
		Matrix3x4 worldMatrix = Matrix3x4::identity();
		std::unique_ptr<Matrix3x4[]> boneMatrices;
		uint32_t field_F8;
		uint32_t field_FC;

		AnimatedModel();
		~AnimatedModel();

		void SetSkeleton(std::shared_ptr<Skeleton> skeleton);
		void SetScale(float scale);
		int GetBoneCount() const;

		std::string_view GetBoneName(int boneIdx) const;
		int GetBoneParentId(int boneIdx) const;
		void AddMesh(Mesh* mesh, IMaterialResolver* matResolver);
		void RemoveMesh(Mesh* mesh);
		void SetEventHandler(IAnimEventHandler* eventHandler) {
			this->eventHandler = eventHandler;
		}
		void ResetSubmeshes();
		void Method11();
		void SetClothFlagSth();

		std::vector<AasMaterial> GetSubmeshes() const;

		int Method14();
		bool HasAnimation(std::string_view animName) const;
		void PlayAnim(int animIdx);
		void Advance(const Matrix3x4 &worldMatrix, float deltaTime, float deltaDistance, float deltaRotation);
		void SetWorldMatrix(const Matrix3x4& worldMatrix);
		void Method19(); // SetBoneMatrices maybe
		void SetTime(float time, const Matrix3x4 &worldMatrix);
		float GetCurrentFrame();
		std::unique_ptr<gfx::Submesh> GetSubmesh(int submeshIdx);
		void AddRunningAnim(AnimPlayer* player);
		void RemoveRunningAnim(AnimPlayer* player);
		int GetAnimCount();
		std::string_view GetAnimName(int animIdx) const;
		bool HasBone(std::string_view boneName) const;
		void ReplaceMaterial(AasMaterial oldMaterial, AasMaterial newMaterial);
		float GetDistPerSec();
		float GetRotationPerSec();

		Matrix3x4 *GetBoneMatrix(std::string_view boneName, Matrix3x4 *matrixOut);

		float GetHeight();
		float GetRadius();

		void DeleteSubmesh(int submeshIdx);
		bool SetAnimByName(std::string_view name);

		void SetSpecialMaterial(gfx::MaterialPlaceholderSlot slot, AasMaterial material);

		void SetRenderState(std::unique_ptr<gfx::IRenderState> renderState) {
			renderState_ = std::move(renderState);
		}

		gfx::IRenderState *GetRenderState() const {
			return renderState_.get();
		}

	private:
		AasSubmeshWithMaterial * GetOrAddSubmesh(AasMaterial material, IMaterialResolver *materialResolver);

		std::unique_ptr<gfx::IRenderState> renderState_;

		void CleanupAnimations(AnimPlayer *player);
	};

}
