
#include <infrastructure/vfs.h>

#include "aas_skeleton.h"

namespace aas {

	struct SkelFileHeader {
		int boneCount;
		int boneDataStart;
		int variationCount; // seems to be always 0 as far as I can tell
		int variationDataStart;
		int animationCount;
		int animationDataStart;
		int unks[19];
	};

	Skeleton::Skeleton(std::string filename, std::vector<uint8_t> data)
		: filename_(std::move(filename)), data_(std::move(data))
	{
		auto header = *reinterpret_cast<SkelFileHeader*>(data_.data());

		bones_ = std::span(reinterpret_cast<SkelBone*>(&data_[header.boneDataStart]), header.boneCount);
		animations_ = std::span(reinterpret_cast<SkelAnim*>(&data_[header.animationDataStart]), header.animationCount);
	}

	const SkelBone * Skeleton::FindBoneByName(std::string_view name) const
	{
		for (auto &bone : bones_) {
			if (!_stricmp(bone.name, name.data())) {
				return &bone;
			}
		}

		return nullptr;
	}

	int Skeleton::FindBoneIdxByName(std::string_view name) const
	{
		for (int i = 0; i < bones_.size(); i++) {
			if (!_stricmp(bones_[i].name, name.data())) {
				return i;
			}
		}

		return -1;
	}

	int Skeleton::FindAnimIdxByName(std::string_view name) const
	{
		for (int i = 0; i < animations_.size(); i++) {
			if (!_stricmp(animations_[i].name, name.data())) {
				return i;
			}
		}

		return -1;
	}

	std::unique_ptr<Skeleton> LoadSkeletonFile(std::string_view filename)
	{
		auto buffer = vfs->ReadAsBinary(filename);

		return std::make_unique<Skeleton>(std::string(filename), std::move(buffer));
	}

	void SkelBoneState::Lerp(std::span<SkelBoneState> out,
		const std::span<SkelBoneState> from,
		const std::span<SkelBoneState> to,
		int boneCount,
		float fraction) {

		assert(boneCount<= from.size());
		assert(boneCount<= to.size());
		assert(boneCount<= out.size());
		
		//static auto orgMethod = temple::GetPointer<void(SkaBoneData *out, const SkaBoneData *from, const SkaBoneData *to, int boneCount, float fraction)>(0x10269ee0);
		//orgMethod(out, from, to, boneCount, fraction);

		// Handle simple cases where no interpolation needs to be performed
		if (fraction < 0.001) {
			if (out.begin() != from.begin()) {
				std::copy_n(from.begin(), boneCount, out.begin());
			}
			return;
		}
		else if (fraction > 0.999) {
			if (out.begin() != to.begin()) {
				std::copy_n(to.begin(), boneCount, out.begin());
			}
			return;
		}

		for (int i = 0; i < boneCount; i++) {
			auto scaleFrom = DX::XMLoadFloat4(&from[i].scale);
			auto scaleTo = DX::XMLoadFloat4(&to[i].scale);
			auto scale = DX::XMVectorLerp(scaleFrom, scaleTo, fraction);
			DX::XMStoreFloat4(&out[i].scale, scale);

			auto translationFrom = DX::XMLoadFloat4(&from[i].translation);
			auto translationTo = DX::XMLoadFloat4(&to[i].translation);
			auto translation = DX::XMVectorLerp(translationFrom, translationTo, fraction);
			DX::XMStoreFloat4(&out[i].translation, translation);

			auto rotationFrom = DX::XMLoadFloat4(&from[i].rotation);
			auto rotationTo = DX::XMLoadFloat4(&to[i].rotation);
			auto rotation = DX::XMQuaternionSlerp(rotationFrom, rotationTo, fraction);
			DX::XMStoreFloat4(&out[i].rotation, rotation);
		}

	}

}
