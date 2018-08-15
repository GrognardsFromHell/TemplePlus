
#include <chrono>
#include <infrastructure/exception.h>
#include <infrastructure/logging.h>
#include <graphics/mdfmaterials.h>
#include "aas/aas.h"

#include "aas_animated_model.h"
#include "aas_anim_events.h"
#include "aas_mesh.h"
#include "aas_skeleton.h"

namespace aas {

	using std::chrono::system_clock;
		
	struct ActiveModel {
		const AasHandle handle;
		gfx::EncodedAnimId animId = gfx::EncodedAnimId(gfx::WeaponAnim::None);
		float floatconst = 6.3940001f;
		system_clock::time_point timeLoaded = system_clock::now();
		std::unique_ptr<AnimatedModel> model;
		std::unique_ptr<Mesh> mesh;
		std::shared_ptr<Skeleton> skeleton;
		std::vector<std::unique_ptr<Mesh>> additionalMeshes;

		ActiveModel(AasHandle handle) : handle(handle) {}
	};

	struct SkeletonCacheEntry {
		std::weak_ptr<Skeleton> data;
	};

	class EventHandler : public IAnimEventHandler {
	public:

		EventHandler(std::function<void(const std::string&)> scriptInterpreter) : scriptInterpreter_(scriptInterpreter) {}

		virtual void HandleEvent(int frame, float frameTime, AnimEventType type, std::string_view args) override;

		void SetFlagsOut(AnimEvents *flagsOut) {
			flagsOut_ = flagsOut;
		}
		void ClearFlagsOut() {
			flagsOut_ = nullptr;
		}

	private:
		AnimEvents * flagsOut_ = nullptr;
		std::function<void(const std::string&)> scriptInterpreter_;
	};

	AasSystem::AasSystem(std::function<std::string(int)> getSkeletonFilename,
		std::function<std::string(int)> getMeshFilename,
		std::function<void(const std::string&)> runScript,
		std::unique_ptr<IMaterialResolver> materialResolver)
	{
		getMeshFilename_ = getMeshFilename;
		getSkeletonFilename_ = getSkeletonFilename;

		materialResolver_ = std::move(materialResolver);

		eventHandler_ = std::make_unique<EventHandler>(runScript);
	}

	AasSystem::~AasSystem() = default;

	bool AasSystem::IsValidHandle(AasHandle handle) const {
		return handle >= 1 && handle < MaxAnims && activeModels_[handle];
	}

	ActiveModel &AasSystem::GetActiveModel(AasHandle handle) const {
		if (IsValidHandle(handle)) {
			return *activeModels_[handle];
		} else {
			throw TempleException("Invalid animation handle: {}", handle);
		}
	}

	AnimatedModel &AasSystem::GetAnimatedModel(AasHandle handle) const {
		return *GetActiveModel(handle).model;
	}

	void EventHandler::HandleEvent(int frame, float frameTime, AnimEventType type, std::string_view args)
	{
		switch (type) {
		case AnimEventType::Action:
			if (flagsOut_) {
				flagsOut_->action = true;
			}
			break;
		case AnimEventType::End:
			if (flagsOut_) {
				flagsOut_->end = true;
			}
			break;
		case AnimEventType::Script:
			if (scriptInterpreter_) {
				scriptInterpreter_(args.data());
			}
			break;
		}
	}

	Matrix3x4 AasSystem::GetWorldMatrix(const AasAnimParams & params) const
	{
		static constexpr float rotation_offset = 2.3561945f;

		Matrix3x4 scale = scaleMatrix(-1.0f, 1.0f, 1.0f);
		Quaternion q = quaternionAxisAngle(0.0f, 1.0f, 0.0f, params.rotation - rotation_offset);
		auto rotation = rotationMatrix(q);

		float x = worldScaleX_ * (params.locX + 0.5f) + params.offsetX;
		float y = params.offsetZ;
		float z = worldScaleY_ * (params.locY + 0.5f) + params.offsetY;
		auto translation = translationMatrix(x, y, z);

		auto worldMatrix = multiplyMatrix3x3_3x4(multiplyMatrix3x3(scale, rotation), translation);

		if ((params.flags & 2) != 0 && IsValidHandle(params.parentAnim)) {
			auto &parentModel = GetAnimatedModel(params.parentAnim);
			auto mat = parentModel.GetBoneMatrix(params.attachedBoneName, &translation);

			worldMatrix = makeMatrixOrthogonal(*mat);
		}
		return worldMatrix;
	}

	Matrix3x4 AasSystem::GetWorldMatrixForParticles(const AasAnimParams & params) const
	{
		Matrix3x4 scale = scaleMatrix(-1.0f, 1.0f, 1.0f);
		auto rotationYaw = rotationMatrix(quaternionAxisAngle(0.0f, 0.0f, 1.0f, params.rotationYaw));
		auto rotationPitch = rotationMatrix(quaternionAxisAngle(1.0f, 0.0f, 0.0f, params.rotationPitch));
		auto rotationRoll = rotationMatrix(quaternionAxisAngle(0.0f, 1.0f, 0.0f, params.rotationRoll));

		float x = worldScaleX_ * (params.locX + 0.5f) + params.offsetX;
		float y = params.offsetZ;
		float z = worldScaleY_ * (params.locY + 0.5f) + params.offsetY;
		auto translation = translationMatrix(x, y, z);

		auto worldMatrix = multiplyMatrix3x3_3x4(multiplyMatrix3x3(multiplyMatrix3x3(multiplyMatrix3x3(scale, rotationRoll), rotationPitch), rotationYaw), translation);

		if ((params.flags & 2) != 0 && IsValidHandle(params.parentAnim)) {
			auto &parentModel = GetAnimatedModel(params.parentAnim);
			auto mat = parentModel.GetBoneMatrix(params.attachedBoneName, &translation);

			worldMatrix = makeMatrixOrthogonal(*mat);
		}

		return worldMatrix;
	}

	// Originally @ 0x102641B0
	AasHandle AasSystem::CreateModelFromIds(int meshId, int skeletonId, gfx::EncodedAnimId idleAnimId, const AasAnimParams& params)
	{
		auto skeletonFilename = getSkeletonFilename_(skeletonId);
		if (skeletonFilename.empty()) {
			throw TempleException("Could not resolve the filename for skeleton id {}", skeletonId);
		}

		auto meshFilename = getMeshFilename_(meshId);
		if (meshFilename.empty()) {
			throw TempleException("Could not resolve the filename for mesh id {}", meshId);
		}

		return CreateModel(meshFilename, skeletonFilename, idleAnimId, params);
	}


	AasHandle AasSystem::CreateModel(std::string_view meshFilename, std::string_view skeletonFilename, gfx::EncodedAnimId idleAnimId, const AasAnimParams & params)
	{
		auto animHandle = FindFreeHandle();

		auto activeModel = std::make_unique<ActiveModel>(animHandle);

		activeModel->skeleton = LoadSkeletonFile(skeletonFilename);

		// Get SKM data
		activeModel->mesh = LoadMeshFile(meshFilename);

		activeModel->model = std::make_unique<AnimatedModel>();
		activeModel->model->SetSkeleton(activeModel->skeleton);
		activeModel->model->AddMesh(activeModel->mesh.get(), materialResolver_.get());
		activeModel->model->SetEventHandler(eventHandler_.get());

		activeModels_[animHandle] = std::move(activeModel);

		try {
			SetAnimById(animHandle, idleAnimId);
			Advance(animHandle, 0, 0, 0, params);
		} catch (...) {
			ReleaseModel(animHandle);
			throw;
		}

		return animHandle;
	}


	// Originally @ 10262540
	bool AasSystem::SetAnimById(AasHandle handle, gfx::EncodedAnimId animId)
	{
		/*if (functions.SetAnimId(handle, animId) != AAS_OK) {
		throw AasException("Unable to set anim id {} for handle {}", animId, handle);
		}*/

		auto &anim = GetActiveModel(handle);

		anim.animId = animId;

		int fallbackState = 0;
		while (true)
		{
			if (anim.model->SetAnimByName(animId.GetName().c_str())) {
				return true;
			}

			if (animId.ToFallback()) {
				continue;
			}

			if (fallbackState == 0) {
				fallbackState = 1;
				if (animId.IsWeaponAnim()) {
					// Retry with unarmed version
					animId = gfx::EncodedAnimId(animId.GetWeaponAnim());
					continue;
				}
			}

			if (fallbackState == 1) {
				fallbackState = 2;
				animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
				continue;
			}

			if (fallbackState == 2) {
				break;
			}
		}

		logger->error("aas_anim_set: ERROR: could not fallback anim {:x} ({})", (int)animId, animId.GetName());
		logger->error("            : Anim File: '{}', Mesh File: '{}'", anim.skeleton->GetFilename(), anim.mesh->GetFilename());
		return false;

	}

	// Originally @ 10262690
	bool AasSystem::HasAnimId(AasHandle handle, gfx::EncodedAnimId animId) const
	{
		auto &anim = GetActiveModel(handle);

		int fallbackState = 0;
		while (true)
		{
			if (anim.model->HasAnimation(animId.GetName().c_str())) {
				return true;
			}

			if (animId.ToFallback()) {
				continue;
			}

			if (fallbackState == 0) {
				fallbackState = 1;
				if (animId.IsWeaponAnim()) {
					// Retry with unarmed version
					animId = gfx::EncodedAnimId(animId.GetWeaponAnim());
					continue;
				}
			}

			if (fallbackState == 1) {
				fallbackState = 2;
				animId = gfx::EncodedAnimId(gfx::NormalAnimType::ItemIdle);
				continue;
			}

			if (fallbackState == 2) {
				break;
			}
		}

		logger->error("aas_anim_set: ERROR: could not fallback anim {:x} ({})", (int)animId, animId.GetName());
		logger->error("            : Anim File: '{}', Mesh File: '{}'", anim.skeleton->GetFilename(), anim.mesh->GetFilename());
		return false;
	}

	gfx::EncodedAnimId AasSystem::GetAnimId(AasHandle handle) const
	{
		return GetActiveModel(handle).animId;
	}

	AnimEvents AasSystem::Advance(AasHandle handle, float deltaTimeInSecs, float deltaDistance, float deltaRotation, const AasAnimParams & params)
	{
		auto worldMatrix = GetWorldMatrix(params);

		AnimEvents events;

		auto &model = GetAnimatedModel(handle);
	
		eventHandler_->SetFlagsOut(&events);
		model.SetScale(params.scale);
		model.Advance(
			worldMatrix,
			deltaTimeInSecs,
			deltaDistance,
			deltaRotation);
		eventHandler_->ClearFlagsOut();

		return events;
	}

	void AasSystem::UpdateWorldMatrix(AasHandle handle, const AasAnimParams & params, bool forParticles)
	{
		auto &model = GetAnimatedModel(handle);

		if (forParticles) {
			model.SetWorldMatrix(GetWorldMatrixForParticles(params));
		} else {
			model.SetWorldMatrix(GetWorldMatrix(params));
		}
		model.SetScale(params.scale);

	}

	std::unique_ptr<gfx::Submesh> AasSystem::GetSubmesh(AasHandle handle, const AasAnimParams &params, int submeshIdx)
	{
		auto &model = GetAnimatedModel(handle);
		UpdateWorldMatrix(handle, params);
		return model.GetSubmesh(submeshIdx);
	}

	std::unique_ptr<gfx::Submesh> AasSystem::GetSubmeshForParticles(AasHandle handle, const AasAnimParams & params, int submeshIdx)
	{
		auto &model = GetAnimatedModel(handle);
		UpdateWorldMatrix(handle, params, true);
		return model.GetSubmesh(submeshIdx);

	}

	void AasSystem::ReleaseModel(AasHandle handle)
	{
		if (IsValidHandle(handle)) {
			activeModels_[handle].reset();
		}
	}

	void AasSystem::ReleaseAllModels()
	{
		for (auto &activeModel : activeModels_) {
			activeModel.reset();
		}
	}

	void AasSystem::AddAdditionalMesh(AasHandle handle, std::string_view filename)
	{
		auto &anim = GetActiveModel(handle);

		// Should actually check if it's already loaded
		auto mesh = anim.additionalMeshes.emplace_back(LoadMeshFile(filename)).get();

		anim.model->AddMesh(mesh, materialResolver_.get());
	}

	void AasSystem::ClearAddmeshes(AasHandle handle)
	{
		auto &anim = GetActiveModel(handle);

		for (auto &mesh : anim.additionalMeshes) {
			anim.model->RemoveMesh(mesh.get());
		}

		anim.additionalMeshes.clear();
	}

	bool AasSystem::GetBoneWorldMatrixByName(AasHandle handle, const AasAnimParams & params, std::string_view boneName, DX::XMFLOAT4X4 * worldMatrixOut)
	{
		auto &model = GetAnimatedModel(handle);
		
		auto worldMatrix = GetWorldMatrix(params);

		model.GetBoneMatrix(boneName, &worldMatrix);
		*worldMatrixOut = worldMatrix.ToFloat4x4();
		return true;
	}

	bool AasSystem::GetBoneWorldMatrixByNameForChild(AasHandle parentHandle, AasHandle handle, const AasAnimParams & params, std::string_view boneName, DX::XMFLOAT4X4 * worldMatrixOut)
	{
		// TODO: This function just seems pointless.....
		// If it ever succeeds, it's using the children's bone matrix, ignoring the parent completely

		auto &model = GetAnimatedModel(handle);

		auto worldMatrix = GetWorldMatrix(params);

		if (parentHandle) {
			auto &parentModel = GetAnimatedModel(parentHandle);
			model.GetBoneMatrix(params.attachedBoneName, &worldMatrix);
		}

		model.GetBoneMatrix(boneName, &worldMatrix);
		*worldMatrixOut = worldMatrix.ToFloat4x4();
		return true;
	}

	void AasSystem::SetTime(AasHandle handle, float time, const AasAnimParams &params)
	{
		auto &model = GetAnimatedModel(handle);

		auto worldMatrix = GetWorldMatrixForParticles(params);

		model.SetTime(time, worldMatrix);

	}

	void AasSystem::ReplaceMaterial(AasHandle handle, gfx::MaterialPlaceholderSlot slot, AasMaterial material)
	{
		auto &model = GetAnimatedModel(handle);
		model.SetSpecialMaterial(slot, material);
	}

	AasHandle AasSystem::FindFreeHandle() const
	{
		for (auto i = 1; i < MaxAnims; i++) {
			if (!activeModels_[i]) {
				return i;
			}
		}

		throw TempleException("Maximum number of active animated models has been exceded.");
	}

}
