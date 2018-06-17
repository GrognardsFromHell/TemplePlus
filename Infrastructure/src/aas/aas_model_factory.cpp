
#include <infrastructure/exception.h>
#include <graphics/mdfmaterials.h>

#include "aas/aas_model_factory.h"
#include "aas/aas.h"
#include "aas_animated_model.h"

namespace aas {

	class MaterialResolver : public IMaterialResolver {
	public:

		MaterialResolver(std::function<int(const std::string&)> resolveMaterial) : resolver_(resolveMaterial) {}

		virtual AasMaterial Acquire(std::string_view materialName, std::string_view context) override
		{
			// Handle material replacement slots
			if (materialName == "HEAD") {
				return (AasMaterial) 0x84000000;
			} else if (materialName == "GLOVES") {
				return (AasMaterial) 0x88000000;
			} else if (materialName == "CHEST") {
				return (AasMaterial) 0x90000000;
			} else if (materialName == "BOOTS") {
				return (AasMaterial) 0xA0000000;
			}

			return (AasMaterial) resolver_(std::string(materialName));
		}

		virtual void Release(AasMaterial material, std::string_view context) override
		{
		}

		virtual bool IsMaterialPlaceholder(AasMaterial material) const override;

		virtual gfx::MaterialPlaceholderSlot GetMaterialPlaceholderSlot(AasMaterial material) const override;

	private:
		std::function<int(const std::string&)> resolver_;		
	};

	bool MaterialResolver::IsMaterialPlaceholder(AasMaterial material) const
	{
		uint32_t id = reinterpret_cast<uint32_t>(material);
		return (id & 0x80000000) != 0;
	}

	gfx::MaterialPlaceholderSlot MaterialResolver::GetMaterialPlaceholderSlot(AasMaterial material) const
	{
		Expects(IsMaterialPlaceholder(material));

		uint32_t id = reinterpret_cast<uint32_t>(material) & 0xFF000000;

		switch (id) {
		case 0x84000000:
			return gfx::MaterialPlaceholderSlot::HEAD;
		case 0x88000000:
			return gfx::MaterialPlaceholderSlot::GLOVES;
		case 0xA0000000:
			return gfx::MaterialPlaceholderSlot::BOOTS;
		case 0x90000000:
			return gfx::MaterialPlaceholderSlot::CHEST;
		default:
			throw TempleException("Unknown material placeholder: {:x}", id);
		}
	}

	AnimatedModelFactory::AnimatedModelFactory(std::function<std::string(int)> getSkeletonFilename,
		std::function<std::string(int)> getMeshFilename,
		std::function<void(const std::string&)> runScript,
		std::function<int(const std::string&)> resolveMaterial) {
		
		auto materialResolver = std::make_unique<MaterialResolver>(resolveMaterial);

		aasSystem_ = std::make_unique<AasSystem>(
			getSkeletonFilename,
			getMeshFilename,
			runScript,
			std::move(materialResolver)
		);

	}

	AnimatedModelFactory::~AnimatedModelFactory() = default;

	class AnimatedModelAdapter : public gfx::AnimatedModel {
	public:
		
		AnimatedModelAdapter(AasSystem &aasSystem, AasHandle handle, aas::AnimatedModel &model, bool borrowed = false) 
			: aasSystem_(aasSystem), handle_(handle), model_(model), borrowed_(borrowed) {
		}
		~AnimatedModelAdapter();

		uint32_t GetHandle() const override {
			return handle_;
		}

		bool AddAddMesh(const std::string& filename) override {
			aasSystem_.AddAdditionalMesh(handle_, filename);
			return true;
		}

		bool ClearAddMeshes() override {
			aasSystem_.ClearAddmeshes(handle_);
			return true;
		}

		gfx::AnimatedModelEvents Advance(float deltaTime, float deltaDistance, float deltaRotation, const gfx::AnimatedModelParams& params) override {
			auto aasParams(Convert(params));

			auto events = aasSystem_.Advance(handle_, deltaTime, deltaDistance, deltaRotation, aasParams);

			return {
				events.end,
				events.action
			};
		}

		gfx::EncodedAnimId GetAnimId() const override {
			return aasSystem_.GetAnimId(handle_);
		}

		int GetBoneCount() const override {
			return model_.GetBoneCount();
		}

		std::string GetBoneName(int boneId) override {
			return std::string(model_.GetBoneName(boneId));
		}

		int GetBoneParentId(int boneId) override {
			return model_.GetBoneParentId(boneId);
		}

		bool GetBoneWorldMatrixByName(const gfx::AnimatedModelParams& params, const std::string& boneName, DirectX::XMFLOAT4X4* worldMatrixOut) override {
			auto aasParams(Convert(params));
			return aasSystem_.GetBoneWorldMatrixByName(handle_, aasParams, boneName, worldMatrixOut);
		}

		bool GetBoneWorldMatrixByNameForChild(const gfx::AnimatedModelPtr& child, const gfx::AnimatedModelParams& params, const std::string& boneName, DirectX::XMFLOAT4X4* worldMatrixOut) override {
			auto realChild = std::static_pointer_cast<AnimatedModelAdapter>(child);
			auto aasParams(Convert(params));
			return aasSystem_.GetBoneWorldMatrixByNameForChild(handle_, realChild->handle_, aasParams, boneName, worldMatrixOut);
		}

		float GetDistPerSec() const override {
			return model_.GetDistPerSec();
		}

		float GetRotationPerSec() const override {
			return model_.GetRotationPerSec();
		}

		bool HasAnim(gfx::EncodedAnimId animId) const override {
			return aasSystem_.HasAnimId(handle_, animId);
		}

		void SetTime(const gfx::AnimatedModelParams& params, float timeInSecs) override {
			auto aasParams(Convert(params));
			aasSystem_.SetTime(handle_, timeInSecs, aasParams);
		}

		bool HasBone(const std::string& boneName) const override {
			return model_.HasBone(boneName);
		}

		void AddReplacementMaterial(gfx::MaterialPlaceholderSlot slot,
			const gfx::MdfRenderMaterialPtr &material) override {

			uint32_t materialId = material->GetId();

			switch (slot) {
			case gfx::MaterialPlaceholderSlot::HEAD:
				materialId |= 0x84000000;
				break;
			case gfx::MaterialPlaceholderSlot::GLOVES:
				materialId |= 0x88000000;
				break;
			case gfx::MaterialPlaceholderSlot::BOOTS:
				materialId |= 0xA0000000;
				break;
			case gfx::MaterialPlaceholderSlot::CHEST:
				materialId |= 0x90000000;
				break;
			}

			aasSystem_.ReplaceMaterial(handle_, slot, (aas::AasMaterial) materialId);
		}

		void SetAnimId(gfx::EncodedAnimId animId) override {
			aasSystem_.SetAnimById(handle_, animId);
		}

		void SetClothFlag() override {
			model_.SetClothFlagSth();
		}

		std::vector<int> GetSubmeshes() override {
			auto materials = model_.GetSubmeshes();
			std::vector<int> result;
			result.reserve(materials.size());
			for (auto material : materials) {
				result.push_back((int) material);
			}
			return result;
		}

		std::unique_ptr<gfx::Submesh> GetSubmesh(const gfx::AnimatedModelParams& params, int submeshIdx) override {
			auto aasParams(Convert(params));
			aasSystem_.UpdateWorldMatrix(handle_, aasParams);
			return model_.GetSubmesh(submeshIdx);
		}

		std::unique_ptr<gfx::Submesh> GetSubmeshForParticles(const gfx::AnimatedModelParams& params, int submeshIdx) override {
			auto aasParams(Convert(params));
			aasSystem_.UpdateWorldMatrix(handle_, aasParams, true);
			return model_.GetSubmesh(submeshIdx);
		}

		float GetHeight(int scale) override {
			model_.SetScale(scale / 100.0f);
			return model_.GetHeight();
		}

		float GetRadius(int scale) override {
			model_.SetScale(scale / 100.0f);
			return model_.GetRadius();
		}

		virtual void SetRenderState(std::unique_ptr<gfx::IRenderState> renderState) override
		{
			model_.SetRenderState(std::move(renderState));
		}

		virtual gfx::IRenderState * GetRenderState() const override
		{
			return model_.GetRenderState();
		}

		static AasAnimParams Convert(const gfx::AnimatedModelParams& params);

	private:
		AasSystem &aasSystem_;
		AasHandle handle_;
		aas::AnimatedModel &model_;
		bool borrowed_ = false;
	};

	AasAnimParams AnimatedModelAdapter::Convert(const gfx::AnimatedModelParams& params) {
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
		auto parentAnim = std::static_pointer_cast<AnimatedModelAdapter>(params.parentAnim);
		if (parentAnim) {
			result.parentAnim = parentAnim->handle_;
			result.flags = 2;
		}
		else {
			result.parentAnim = 0;
			result.flags = 1;
		}
		return result;
	}

	AnimatedModelAdapter::~AnimatedModelAdapter() {
		if (!borrowed_) {
			aasSystem_.ReleaseModel(handle_);
		}
	}

	gfx::AnimatedModelPtr AnimatedModelFactory::FromIds(int meshId,
		int skeletonId,
		gfx::EncodedAnimId idleAnimId,
		const gfx::AnimatedModelParams& params,
		bool borrow) {

		auto aasParams(AnimatedModelAdapter::Convert(params));
		
		auto handle = aasSystem_->CreateModelFromIds(meshId, skeletonId, idleAnimId, aasParams);

		return std::make_shared<AnimatedModelAdapter>(*aasSystem_, handle, aasSystem_->GetAnimatedModel(handle), borrow);

	}

	gfx::AnimatedModelPtr AnimatedModelFactory::FromFilenames(const std::string& meshFilename,
		const std::string& skeletonFilename,
		gfx::EncodedAnimId idleAnimId,
		const gfx::AnimatedModelParams& params) {

		auto aasParams(AnimatedModelAdapter::Convert(params));
		auto handle = aasSystem_->CreateModel(meshFilename, skeletonFilename, idleAnimId, aasParams);

		return std::make_shared<AnimatedModelAdapter>(*aasSystem_, handle, aasSystem_->GetAnimatedModel(handle));

	}

	std::unique_ptr<gfx::AnimatedModel> AnimatedModelFactory::BorrowByHandle(uint32_t handle)
	{
		auto &model = aasSystem_->GetAnimatedModel(handle);
		return std::make_unique<AnimatedModelAdapter>(*aasSystem_, handle, model, true);
	}

	void AnimatedModelFactory::FreeAll()
	{
		aasSystem_->ReleaseAllModels();
	}

	void AnimatedModelFactory::FreeHandle(uint32_t handle)
	{
		aasSystem_->ReleaseModel(handle);
	}

}
