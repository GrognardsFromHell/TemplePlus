#include <infrastructure/meshes.h>
#include <infrastructure/exception.h>

#include "temple/dll.h"
#include "temple/meshes.h"

namespace temple {

	enum AasStatus : uint32_t {
		AAS_OK
	};

#pragma pack(push, 1)
	struct AasSubmesh {
		int field_0;
		int vertexCount;
		int primCount;
		float* positions;
		float* normals;
		float* uv;
		uint16_t* indices;
	};

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

	/*
	Bitfield that indicates once-per-animation events that happened
	during advancing the animation's time.
*/
	enum AasEventFlag : uint32_t {
		AEF_NONE = 0,
		AEF_ACTION = 1,
		AEF_END = 2
	};

	/*
		Configuration for the overall system
	*/
	typedef AasStatus (*FnAasGetFilename)(int meshId, char* filenameOut);
	typedef AasStatus (*FnAasGetAnimName)(int animid, char* nameOut);
	typedef void (*FnAasRunScript)(const char* script);

	struct AasConfig {
		float scaleX = 28.284271f;
		float scaleY = 28.284271f;
		FnAasGetFilename getSkaFilename = nullptr;
		FnAasGetFilename getSkmFilename = nullptr;
		FnAasGetAnimName getAnimName = nullptr;
		void* mapSpecialAnimId = nullptr; // TODO
		void* field18 = nullptr; // TODO
		FnAasRunScript runScript = nullptr;
	};

#pragma pack(pop)

	static struct AasFunctions : AddressTable {

		int (*Init)(const AasConfig* config);
		int (*Exit)();

		int (__cdecl *FreeSubmesh)(AasSubmesh* submesh);

		AasStatus (__cdecl *CreateModelByIds)(int skmId, int skaId, int idleAnimId, const AasAnimParams* animState, AasHandle* aasHandleOut);

		AasStatus (__cdecl *CreateModelByNames)(const char* skmFilename,
		                                        const char* skaFilename,
		                                        int idleType,
		                                        const AasAnimParams* animState,
		                                        AasHandle* aasHandleOut);

		int (__cdecl *GetSubmesh)(AasHandle aasHandle, AasSubmesh** submeshOut, const AasAnimParams* animState, int submeshIdx);

		int (__cdecl *GetSubmeshForParticles)(AasHandle aasHandle, AasSubmesh** submeshOut, const AasAnimParams* animState, int submeshIdx);

		int (__cdecl *GetSubmeshes)(AasHandle aasHandle, int** submeshMaterials, int* pSubmeshCountOut);

		int (__cdecl *AddAddMesh)(AasHandle aasHandle, const char* addMeshName);

		int (__cdecl *ClearAddMeshes)(AasHandle aasHandle);

		int (__cdecl *Advance)(AasHandle aasHandle,
		                       float deltaTime,
		                       float deltaDistance,
		                       float deltaRotation,
		                       const AasAnimParams* params,
		                       AasEventFlag* eventOut);

		int (__cdecl *Free)(AasHandle aasHandle);

		int (__cdecl *GetAnimId)(AasHandle aasHandle, int* animIdOut);

		int (__cdecl *GetBoneCount)(AasHandle aasHandle);

		const char*(__cdecl *GetBoneNameById)(AasHandle aasHandle, int boneId);

		int (__cdecl *GetBoneParentId)(AasHandle aasHandle, int boneId);

		int (__cdecl *GetBoneWorldMatrixByName)(AasHandle aasHandle,
		                                        const AasAnimParams* params,
		                                        D3DMATRIX* worldMatrixOut,
		                                        const char* boneName);

		int (__cdecl *GetBoneWorldMatrixByNameForChild)(AasHandle parentAasHandle,
		                                                AasHandle childAasHandle,
		                                                const AasAnimParams* params,
		                                                D3DMATRIX* worldMatrixOut,
		                                                const char* boneName);

		float (__cdecl *GetDistPerSec)(AasHandle handle);

		float (__cdecl *GetRotationPerSec)(AasHandle handle);

		/*
		Since this actually sets the anim ID returned by "GetAnimId", this might
		not just check that an anim is supported, but it definetly doesn't "start"
		the animation right away as SetAnimId does.
		*/
		int (__cdecl *HasAnimId)(AasHandle handle, int animId);


		int (__cdecl *HasBone)(AasHandle aasHandle, const char* boneName);

		int (__cdecl *ReplaceSpecialMaterial)(AasHandle aasHandle, int materialId);

		int (__cdecl *SetAnimId)(AasHandle aasHandle, int animId);

		/*
		Not sure what this does, but it only does something if the animated model has
		cloth bones. Otherwise it's a noop. So presumably, it has something to do 
		with cloth simulation.
	*/
		int (__cdecl *SetClothFlagSth)(AasHandle aasHandle);

		/*
	Used by the particle system model renderer before getting the submesh.
	animTime in this case is the particle's lifetime, for which the model is
	to be rendered.
	*/
		int (__cdecl *SetTime)(AasHandle aasHandle, float time, const AasAnimParams* params);

		AasFunctions() {
			rebase(Init, 0x102640B0);
			rebase(Exit, 0x102645E0);
			rebase(FreeSubmesh, 0x10262500);
			rebase(CreateModelByIds, 0x102641B0);
			rebase(CreateModelByNames, 0x102643A0);
			rebase(GetSubmesh, 0x10263400);
			rebase(GetSubmeshForParticles, 0x102636D0);
			rebase(GetSubmeshes, 0x10263A50);
			rebase(AddAddMesh, 0x10262E30);
			rebase(ClearAddMeshes, 0x10262EC0);
			rebase(Advance, 0x10262C10);
			rebase(Free, 0x10264510);
			rebase(GetAnimId, 0x102627E0);
			rebase(GetBoneCount, 0x10262F40);
			rebase(GetBoneNameById, 0x10262F80);
			rebase(GetBoneParentId, 0x10262F80);
			rebase(GetBoneWorldMatrixByName, 0x10263000);
			rebase(GetBoneWorldMatrixByNameForChild, 0x102631E0);
			rebase(GetDistPerSec, 0x10263DA0);
			rebase(GetRotationPerSec, 0x10263DE0);
			rebase(HasAnimId, 0x10262690);
			rebase(HasBone, 0x10263A10);
			rebase(ReplaceSpecialMaterial, 0x10263AB0);
			rebase(SetAnimId, 0x10262540);
			rebase(SetClothFlagSth, 0x102633C0);
			rebase(SetTime, 0x10263B90);
		}
	} functions;

	class AasSubmeshAdapter : public gfx::Submesh {
	public:
		AasSubmeshAdapter(AasHandle parent, const AasAnimParams& params, int submeshId, bool forParticles);
		~AasSubmeshAdapter();

		int GetVertexCount() override {
			return mSubmesh->vertexCount;
		}

		int GetPrimitiveCount() override {
			return mSubmesh->primCount;
		}

		float* GetPositions() override {
			return mSubmesh->positions;
		}

		float* GetNormals() override {
			return mSubmesh->normals;
		}

		float* GetUV() override {
			return mSubmesh->uv;
		}

		uint16_t* GetIndices() override {
			return mSubmesh->indices;
		}

	private:
		AasSubmesh* mSubmesh = nullptr;
	};

	AasSubmeshAdapter::AasSubmeshAdapter(AasHandle aasHandle, const AasAnimParams& params, int submeshIdx, bool forParticles) {
		if (!forParticles) {
			functions.GetSubmesh(aasHandle, &mSubmesh, &params, submeshIdx);
		} else {
			functions.GetSubmeshForParticles(aasHandle, &mSubmesh, &params, submeshIdx);
		}
	}

	AasSubmeshAdapter::~AasSubmeshAdapter() {
		if (mSubmesh) {
			functions.FreeSubmesh(mSubmesh);
		}
	}
	
	class AasAnimatedModel : public gfx::AnimatedModel {
	public:

		explicit AasAnimatedModel(AasHandle handle) : mHandle(handle) {
		}

		~AasAnimatedModel();

		bool AddAddMesh(const ::std::string& filename) override;
		bool ClearAddMeshes() override;
		::gfx::AnimatedModelEvents Advance(float deltaTime, float deltaDistance, float deltaRotation, const ::gfx::AnimatedModelParams& params) override;
		::gfx::EncodedAnimId GetAnimId() const override;
		int GetBoneCount() const override;
		::std::string GetBoneName(int boneId) override;
		int GetBoneParentId(int boneId) override;
		bool GetBoneWorldMatrixByName(const ::gfx::AnimatedModelParams& params, const ::std::string& boneName, ::D3DMATRIX* worldMatrixOut) override;
		bool GetBoneWorldMatrixByNameForChild(const gfx::AnimatedModelPtr& child, const gfx::AnimatedModelParams& params, const std::string& boneName, D3DMATRIX* worldMatrixOut) override;
		float GetDistPerSec() const override;
		float GetRotationPerSec() const override;
		bool HasAnim(::gfx::EncodedAnimId animId) const override;
		void SetTime(const ::gfx::AnimatedModelParams& params, float timeInSecs) override;
		bool HasBone(const ::std::string& boneName) const override;
		void AddReplacementMaterial(int materialId) override;
		void SetAnimId(int animId) override;
		void SetClothFlag() override;
		::std::vector<int> GetSubmeshes() override;
		::std::unique_ptr<::gfx::Submesh> GetSubmesh(const ::gfx::AnimatedModelParams& params, int submeshIdx) override;
		::std::unique_ptr<::gfx::Submesh> GetSubmeshForParticles(const ::gfx::AnimatedModelParams& params, int submeshIdx) override;

		static AasAnimParams Convert(const gfx::AnimatedModelParams& params);

	private:
		AasHandle mHandle;
	};

	AasAnimParams AasAnimatedModel::Convert(const gfx::AnimatedModelParams& params) {
		AasAnimParams result;
		result.flags = 0;
		result.locX = params.x;
		result.locY = params.y;
		result.scale = params.scale;
		result.offsetX = params.offsetX;
		result.offsetY = params.offsetY;
		result.offsetZ = params.offsetZ;
		result.rotation = params.rotation;
		result.rotationYaw = params.rotationYaw;
		result.rotationPitch = params.rotationPitch;
		result.rotationRoll = params.rotationRoll;
		strncpy_s(result.attachedBoneName, params.attachedBoneName.c_str(), sizeof(result.attachedBoneName) - 1);
		result.attachedBoneName[sizeof(result.attachedBoneName) - 1] = '\0';
		result.unknown = 0;
		auto parentAnim = std::static_pointer_cast<AasAnimatedModel>(params.parentAnim);
		if (parentAnim) {
			result.parentAnim = parentAnim->mHandle;
		} else {
			result.parentAnim = 0;
		}
		return result;
	}

	AasAnimatedModel::~AasAnimatedModel() {
		functions.Free(mHandle);
	}

	bool AasAnimatedModel::AddAddMesh(const ::std::string& filename) {
		return functions.AddAddMesh(mHandle, filename.c_str()) == AAS_OK;
	}

	bool AasAnimatedModel::ClearAddMeshes() {
		return functions.ClearAddMeshes(mHandle) == AAS_OK;
	}

	::gfx::AnimatedModelEvents AasAnimatedModel::Advance(float deltaTime, float deltaDistance, float deltaRotation, const ::gfx::AnimatedModelParams& params) {
		AasEventFlag eventsTriggered;
		auto aasParams(Convert(params));
		functions.Advance(mHandle, deltaTime, deltaDistance, deltaRotation, &aasParams, &eventsTriggered);

		return{
			(eventsTriggered & AEF_END) != 0,
			(eventsTriggered & AEF_ACTION) != 0
		};
	}

	::gfx::EncodedAnimId AasAnimatedModel::GetAnimId() const {
		int animId;
		functions.GetAnimId(mHandle, &animId);
		return gfx::EncodedAnimId(animId);
	}

	int AasAnimatedModel::GetBoneCount() const {
		return functions.GetBoneCount(mHandle);
	}

	::std::string AasAnimatedModel::GetBoneName(int boneId) {
		return functions.GetBoneNameById(mHandle, boneId);
	}

	int AasAnimatedModel::GetBoneParentId(int boneId) {
		return functions.GetBoneParentId(mHandle, boneId);
	}

	bool AasAnimatedModel::GetBoneWorldMatrixByName(const ::gfx::AnimatedModelParams& params, const ::std::string& boneName, ::D3DMATRIX* worldMatrixOut) {
		auto aasParams(Convert(params));
		return functions.GetBoneWorldMatrixByName(mHandle, &aasParams, worldMatrixOut, boneName.c_str()) == AAS_OK;
	}

	bool AasAnimatedModel::GetBoneWorldMatrixByNameForChild(const gfx::AnimatedModelPtr& child, const gfx::AnimatedModelParams& params, const std::string& boneName, D3DMATRIX* worldMatrixOut) {
		auto realChild = std::static_pointer_cast<AasAnimatedModel>(child);
		auto aasParams(Convert(params));
		return functions.GetBoneWorldMatrixByNameForChild(mHandle, realChild->mHandle, &aasParams, worldMatrixOut, boneName.c_str()) == AAS_OK;
	}

	float AasAnimatedModel::GetDistPerSec() const {
		return functions.GetDistPerSec(mHandle);
	}

	float AasAnimatedModel::GetRotationPerSec() const {
		return functions.GetRotationPerSec(mHandle);
	}

	bool AasAnimatedModel::HasAnim(::gfx::EncodedAnimId animId) const {
		return functions.HasAnimId(mHandle, animId) == AAS_OK;
	}

	void AasAnimatedModel::SetTime(const ::gfx::AnimatedModelParams& params, float timeInSecs) {
		auto aasParams(Convert(params));
		functions.SetTime(mHandle, timeInSecs, &aasParams);
	}

	bool AasAnimatedModel::HasBone(const ::std::string& boneName) const {
		return functions.HasBone(mHandle, boneName.c_str()) == AAS_OK;
	}

	void AasAnimatedModel::AddReplacementMaterial(int materialId) {
		functions.ReplaceSpecialMaterial(mHandle, materialId);
	}

	void AasAnimatedModel::SetAnimId(int animId) {
		functions.SetAnimId(mHandle, animId);
	}

	void AasAnimatedModel::SetClothFlag() {
		functions.SetClothFlagSth(mHandle);
	}

	::std::vector<int> AasAnimatedModel::GetSubmeshes() {
		int* materialIds;
		int submeshCount;
		if (functions.GetSubmeshes(mHandle, &materialIds, &submeshCount) != AAS_OK) {
			return{};
		}

		return ::std::vector<int>(materialIds, materialIds + submeshCount);
	}

	::std::unique_ptr<::gfx::Submesh> AasAnimatedModel::GetSubmesh(const ::gfx::AnimatedModelParams& params, int submeshIdx) {
		auto aasParams(Convert(params));
		return std::make_unique<AasSubmeshAdapter>(mHandle, aasParams, submeshIdx, false);
	}

	::std::unique_ptr<::gfx::Submesh> AasAnimatedModel::GetSubmeshForParticles(const ::gfx::AnimatedModelParams& params, int submeshIdx) {
		auto aasParams(Convert(params));
		return std::make_unique<AasSubmeshAdapter>(mHandle, aasParams, submeshIdx, true);
	}

	AasAnimatedModelFactory::AasAnimatedModelFactory() {

		AasConfig config;
		if (functions.Init(&config)) {
			throw TempleException("Unable to initialize the animation system.");
		}

	}

	AasAnimatedModelFactory::~AasAnimatedModelFactory() {
		functions.Exit();
	}

	gfx::AnimatedModelPtr AasAnimatedModelFactory::FromIds(int meshId,
	                                                       int skeletonId,
	                                                       gfx::EncodedAnimId idleAnimId,
	                                                       const gfx::AnimatedModelParams& params) {

		AasHandle handle;
		auto aasParams(AasAnimatedModel::Convert(params));
		if (functions.CreateModelByIds(meshId, skeletonId, idleAnimId, &aasParams, &handle) != AAS_OK) {
			throw TempleException("Could not load model {} with skeleton {}.", meshId, skeletonId);
		}

		return std::make_shared<AasAnimatedModel>(handle);

	}

	gfx::AnimatedModelPtr AasAnimatedModelFactory::FromFilenames(const std::string& meshFilename,
	                                                             const std::string& skeletonFilename,
	                                                             gfx::EncodedAnimId idleAnimId,
	                                                             const gfx::AnimatedModelParams& params) {

		AasHandle handle;
		auto aasParams(AasAnimatedModel::Convert(params));
		if (functions.CreateModelByNames(meshFilename.c_str(), skeletonFilename.c_str(), idleAnimId, &aasParams, &handle) != AAS_OK) {
			throw new TempleException("Could not load model {} with skeleton {}.", meshFilename, skeletonFilename);
		}

		return std::make_shared<AasAnimatedModel>(handle);

	}

}
