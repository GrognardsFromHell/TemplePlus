
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <infrastructure/meshes.h>

#include "aas_math.h"

namespace aas {

	typedef void* AasMaterial;

	class Skeleton;
	class Mesh;
	struct ActiveModel;
	class AnimatedModel;
	struct SkeletonCacheEntry;

	using AasHandle = uint32_t;

	struct AasAnimParams {
		uint32_t flags;
		uint32_t unknown;
		uint64_t locX;
		uint64_t locY;
		float offsetX;
		float offsetY;
		float offsetZ;
		float rotation;
		float scale;
		float rotationRoll;
		float rotationPitch;
		float rotationYaw;
		AasHandle parentAnim;
		char attachedBoneName[48];
	};

	struct AnimEvents {
		/**
		 * Indicates the animation has ended.
		 */
		bool end : 1;
		/**
		 * Indicates that the frame on which an action should connect (weapon swings, etc.)
		 * has occurred.
		 */
		bool action : 1;
		AnimEvents() : end(false), action(false) {}
	};

	class IMaterialResolver {
	public:
		virtual AasMaterial Acquire(std::string_view materialName, std::string_view context) = 0;
		virtual void Release(AasMaterial material, std::string_view context) = 0;
		virtual ~IMaterialResolver() = default;

		virtual bool IsMaterialPlaceholder(AasMaterial material) const = 0;
		virtual gfx::MaterialPlaceholderSlot GetMaterialPlaceholderSlot(AasMaterial material) const = 0;
	};
	
	class AasSystem {
	public:
		AasSystem(std::function<std::string(int)> getSkeletonFilename,
			std::function<std::string(int)> getMeshFilename,
			std::function<void(const std::string&)> runScript,
			std::unique_ptr<IMaterialResolver> materialResolver);
		~AasSystem();
		
		AasHandle CreateModelFromIds(
			int meshId,
			int skeletonId,
			gfx::EncodedAnimId idleAnimId,
			const AasAnimParams& params
		);

		AasHandle CreateModel(
			std::string_view meshFilename,
			std::string_view skeletonFilename,
			gfx::EncodedAnimId idleAnimId,
			const AasAnimParams& params
		);

		bool SetAnimById(AasHandle handle, gfx::EncodedAnimId animId);
		bool HasAnimId(AasHandle handle, gfx::EncodedAnimId animId) const;
		gfx::EncodedAnimId GetAnimId(AasHandle handle) const;

		AnimEvents Advance(AasHandle handle,
			float deltaTimeInSecs,
			float deltaDistance,
			float deltaRotation,
			const AasAnimParams &params);

		void UpdateWorldMatrix(AasHandle handle, const AasAnimParams& params, bool forParticles = false);

		std::unique_ptr<gfx::Submesh> GetSubmesh(AasHandle handle,
			const AasAnimParams& params,
			int submeshIdx);

		std::unique_ptr<gfx::Submesh> GetSubmeshForParticles(AasHandle handle,
			const AasAnimParams& params,
			int submeshIdx);

		void ReleaseAllModels();
		void ReleaseModel(AasHandle handle);

		bool IsValidHandle(AasHandle handle) const;
		AnimatedModel &GetAnimatedModel(AasHandle handle) const;

		void AddAdditionalMesh(AasHandle handle, std::string_view filename);
		void ClearAddmeshes(AasHandle handle);

		bool GetBoneWorldMatrixByName(AasHandle handle,
			const AasAnimParams &params,
			std::string_view boneName,
			DX::XMFLOAT4X4 *worldMatrixOut);

		bool GetBoneWorldMatrixByNameForChild(AasHandle parentHandle,
			AasHandle handle,
			const AasAnimParams &params,
			std::string_view boneName,
			DX::XMFLOAT4X4 *worldMatrixOut);

		void SetTime(AasHandle handle, float time, const AasAnimParams &params);

		void ReplaceMaterial(AasHandle handle, gfx::MaterialPlaceholderSlot slot, AasMaterial material);

	private:

		static constexpr size_t MaxAnims = 5000;

		std::function<std::string(int)> getSkeletonFilename_;
		std::function<std::string(int)> getMeshFilename_;
		std::unique_ptr<IMaterialResolver> materialResolver_;

		std::unordered_map<std::string, SkeletonCacheEntry> skeletonCache_;

		std::unique_ptr<ActiveModel> activeModels_[MaxAnims];

		float worldScaleX_ = 28.284271f;
		float worldScaleY_ = 28.284271f;

		std::unique_ptr<class EventHandler> eventHandler_;

		ActiveModel &GetActiveModel(AasHandle handle) const;
		AasHandle FindFreeHandle() const;

		Matrix3x4 GetWorldMatrix(const AasAnimParams &params) const;
		Matrix3x4 GetWorldMatrixForParticles(const AasAnimParams &params) const;


	};

}
