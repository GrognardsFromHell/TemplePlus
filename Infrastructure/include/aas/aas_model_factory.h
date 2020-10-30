
#pragma once

#include <functional>
#include <infrastructure/meshes.h>

namespace aas {

	class AasSystem;

	class AnimatedModelFactory : public gfx::AnimatedModelFactory {
	public:
		explicit AnimatedModelFactory(std::function<std::string(int)> getSkeletonFilename,
			std::function<std::string(int)> getMeshFilename,
			std::function<void(const std::string&)> runScript,
			std::function<int(const std::string&)> resolveMaterial);
		~AnimatedModelFactory();

		gfx::AnimatedModelPtr FromIds(
			int meshId,
			int skeletonId,
			gfx::EncodedAnimId idleAnimId,
			const gfx::AnimatedModelParams& params,
			bool borrow = false) override;

		gfx::AnimatedModelPtr FromFilenames(
			const std::string& meshFilename,
			const std::string& skeletonFilename,
			gfx::EncodedAnimId idleAnimId,
			const gfx::AnimatedModelParams& params) override;

		/*
		Gets an existing animated model by its handle.
		The returned model will not free the actual animation when it is
		destroyed.
		*/
		std::unique_ptr<gfx::AnimatedModel> BorrowByHandle(uint32_t handle) override;

		void FreeAll() override;

		void FreeHandle(uint32_t handle) override;

		void ForceFrame(bool en, float frame) override;
		void ForceWorldMatrix(bool en, DirectX::XMFLOAT4X4& worldMat) override;
		void GetWorldMatrix(gfx::AnimatedModelParams &aasParams, DirectX::XMFLOAT4X4& worldMat) override;

	private:
		// This is the mapping loaded from meshes.mes
		std::unordered_map<int, std::string> fileIdMapping_;

		std::unique_ptr<AasSystem> aasSystem_;
	};

}
