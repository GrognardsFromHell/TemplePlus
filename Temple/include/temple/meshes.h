#pragma once

#include <string>
#include <infrastructure/meshes.h>

namespace temple {

	class AasAnimatedModelFactory : public gfx::AnimatedModelFactory {
	public:
		AasAnimatedModelFactory();
		~AasAnimatedModelFactory();

		gfx::AnimatedModelPtr FromIds(
			int meshId,
			int skeletonId,
			gfx::EncodedAnimId idleAnimId,
			const gfx::AnimatedModelParams& params) override;

		gfx::AnimatedModelPtr FromFilenames(
			const std::string& meshFilename,
			const std::string& skeletonFilename,
			gfx::EncodedAnimId idleAnimId,
			const gfx::AnimatedModelParams& params) override;
	};

}
