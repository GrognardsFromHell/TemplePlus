#pragma once

#include <string>
#include <functional>
#include <infrastructure/meshes.h>

namespace temple {

	using AasHandle = uint32_t;

	struct AasConfig {
		float scaleX = 28.284271f;
		float scaleY = 28.284271f;
		std::function<std::string(int)> resolveSkaFile;
		std::function<std::string(int)> resolveSkmFile;
		std::function<void(const std::string&)> runScript;
		std::function<int(const std::string&)> resolveMaterial;
	};

	using AasFreeListener = std::function<void(AasHandle)>;
	using AasFreeListenerHandle = std::list<AasFreeListener>::iterator;
	
	class AasAnimatedModelFactory : public gfx::AnimatedModelFactory {
	public:
		explicit AasAnimatedModelFactory(const AasConfig &config);
		~AasAnimatedModelFactory();

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
		std::unique_ptr<gfx::AnimatedModel> BorrowByHandle(AasHandle handle);

		AasFreeListenerHandle AddFreeListener(AasFreeListener listener);
		void RemoveFreeListener(AasFreeListenerHandle handle);

		void InvalidateBuffers(AasHandle handle);

		void FreeAll();

	private:
		AasConfig mConfig;
		using FnAasModelFree = int(temple::AasHandle);
		FnAasModelFree* mOrgModelFree;
		static AasAnimatedModelFactory *sInstance;
		std::list<AasFreeListener> mListeners;

		// This is the mapping loaded from meshes.mes
		std::unordered_map<int, std::string> mMapping;

		static int __stdcall AasResolveMaterial(const char *filename, int, int);
		static int AasFreeModel(temple::AasHandle handle);

	};

#pragma pack(push, 1)
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
#pragma pack(pop)

}
