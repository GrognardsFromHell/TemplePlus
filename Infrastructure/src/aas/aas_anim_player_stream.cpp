
#include "aas/aas_math.h"

#include "aas_anim_player_stream.h"
#include "aas_skeleton.h"
#include "aas/aas_debugger.h"

namespace aas {

	struct SkelKeyframeData {
		int16_t boneId;
		int16_t scaleX;
		int16_t scaleY;
		int16_t scaleZ;
		int16_t rotationX;
		int16_t rotationY;
		int16_t rotationZ;
		int16_t rotationW;
		int16_t translationX;
		int16_t translationY;
		int16_t translationZ;
	};

	struct SkelAnimStreamData {
		float scaleFactor;
		float translationFactor;
		SkelKeyframeData frameData[1]; // array for initial positions, terminated by boneId = -1; after that the keyframe data begins
	};

	AnimPlayerStream * CreateAnimPlayerStream(Skeleton* skeleton) {
		auto result = (AnimPlayerStream*)malloc(sizeof(AnimPlayerStream) + sizeof(AnimPlayerStreamBone) * skeleton->GetBones().size());
		if (!result)
			return nullptr;

		result->skeleton = skeleton;
		result->animation = nullptr;
		result->boneCount = skeleton->GetBones().size();

		return result;
	}

	void AnimPlayerStream::SetFrame(float frame) {

		//static auto orgMethod = temple::GetPointer<int(AnimPlayerStream*, float frame)>(0x1026b740);
		//orgMethod(this, frame);
		//return;
		if (AasDebugger::IsForcedFrame() && frame < 32766.0) {
			frame = AasDebugger::GetForcedFrame();
			if (frame < -1.f)
				frame = -1.f;
		}

		auto frameRounded = floor(frame);
		if (floor(this->currentFrame) == frameRounded) {
			this->currentFrame = frame;
			return;
		}

		if (frame < this->currentFrame) {
			SetFrame(32766.0); // run to end
			Initialize(this->animation, this->streamIdx);
		}

		// set currentFrame
		this->currentFrame = frame;
		if (frameRounded >= 32767)
			frameRounded = 32766;

		auto sFactor = this->scaleFactor;
		auto tFactor = this->translationFactor;
		const auto rFactor = 1.0f / 32767.0f;

		int16_t* keyframeData = (int16_t*)this->keyframePtr;
		auto keyframeFrame = (*(uint16_t*)keyframeData) / 2;
		
		// advance the frames
		while (keyframeFrame <= frameRounded) {

			keyframeData++;
			auto flags = *(uint16_t*)keyframeData;
			while (flags & 1) {

				auto boneId = flags >> 4;
				auto &tBone = this->bones[boneId];
				keyframeData++;

				if (!(flags & 0xE)) {
					flags = *(uint16_t*)keyframeData;
					continue;
				}

				if (flags & 8) // scale
				{
					tBone.scaleFrame = keyframeFrame;
					tBone.scaleNextFrame = *keyframeData;
					tBone.prevScale = tBone.scale;

					keyframeData++;
					tBone.scale.x = (*keyframeData) * sFactor;
					keyframeData++;
					tBone.scale.y = (*keyframeData) * sFactor;
					keyframeData++;
					tBone.scale.z = (*keyframeData) * sFactor;

					auto one_over_frame_delta = 0.0f;
					if (keyframeFrame < tBone.scaleNextFrame)
						one_over_frame_delta = 1.0f / (tBone.scaleNextFrame - keyframeFrame);
					tBone.prevScale.w = one_over_frame_delta;

					keyframeData++;
				}

				if (flags & 4) // rotation
				{
					tBone.rotationFrame = keyframeFrame;
					tBone.rotationNextFrame = *keyframeData;
					tBone.prevRotation = tBone.rotation;

					keyframeData++;
					tBone.rotation.x = (*keyframeData) * rFactor;
					keyframeData++;
					tBone.rotation.y = (*keyframeData) * rFactor;
					keyframeData++;
					tBone.rotation.z = (*keyframeData) * rFactor;
					keyframeData++;
					tBone.rotation.w = (*keyframeData) * rFactor;

					auto one_over_frame_delta = 0.0f;
					if (keyframeFrame < tBone.rotationNextFrame)
						one_over_frame_delta = 1.0f / (tBone.rotationNextFrame - keyframeFrame);
					tBone.scale.w = one_over_frame_delta;

					keyframeData++;
				}

				if (flags & 2) // translation
				{
					tBone.translationFrame = keyframeFrame;
					tBone.translationNextFrame = *keyframeData;
					tBone.prevTranslation.x = tBone.translation.x;
					tBone.prevTranslation.y = tBone.translation.y;
					tBone.prevTranslation.z = tBone.translation.z;

					keyframeData++;
					int transX = (*keyframeData);
					tBone.translation.x = transX * tFactor;
					keyframeData++;
					int transY = (*keyframeData);
					tBone.translation.y = transY * tFactor;
					keyframeData++;
					int transZ = (*keyframeData);
					tBone.translation.z = transZ * tFactor;

					auto one_over_frame_delta = 0.0f;
					if (keyframeFrame < tBone.translationNextFrame)
						one_over_frame_delta = 1.0f / (tBone.translationNextFrame - keyframeFrame);
					tBone.translation.w = one_over_frame_delta;

					keyframeData++;
				}

				flags = *(uint16_t*)keyframeData;
			}

			this->keyframePtr = keyframeData;
			keyframeFrame = (*(uint16_t*)keyframeData) / 2;
		}

	}

	void AnimPlayerStream::Initialize(const SkelAnim * anim, int streamIdx) {

		this->animation = anim;
		this->streamIdx = streamIdx;

		auto skeleton = this->skeleton;
		auto bones = skeleton->GetBones();
		auto streamBones = this->bones;

		// Initialize this stream with the initial state from the skeleton
		for (auto i = 0; i < bones.size(); i++) {
			auto &sBone = streamBones[i];
			auto &fBone = bones[i];
			sBone.scale = fBone.initialState.scale;
			sBone.prevScale = sBone.scale;

			sBone.rotation = fBone.initialState.rotation;
			sBone.prevRotation = sBone.rotation;

			sBone.translation = fBone.initialState.translation;
			sBone.prevTranslation = sBone.translation;

			sBone.scaleFrame = sBone.scaleNextFrame = sBone.translationFrame = sBone.translationNextFrame = 0;
			sBone.rotationFrame = sBone.rotationNextFrame = 0;
			sBone.field6C = 0;
		}

		auto streamData = anim->GetStreamKeyframes(streamIdx);

		auto sFactor = streamData->scaleFactor;
		auto tFactor = streamData->translationFactor;
		const auto rFactor = 1.0f / 32767.0f;
		this->scaleFactor = sFactor;
		this->translationFactor = tFactor;
		auto frameData = streamData->frameData;

		for (; frameData->boneId >= 0; frameData++) {
			auto boneId = frameData->boneId;
			auto &sBone = streamBones[boneId];
			sBone.scale.x = sFactor * frameData->scaleX;
			sBone.scale.y = sFactor * frameData->scaleY;
			sBone.scale.z = sFactor * frameData->scaleZ;
			sBone.prevScale.x = sBone.scale.x;
			sBone.prevScale.y = sBone.scale.y;
			sBone.prevScale.z = sBone.scale.z;

			sBone.translation.x = tFactor * frameData->translationX;
			sBone.translation.y = tFactor * frameData->translationY;
			sBone.translation.z = tFactor * frameData->translationZ;
			sBone.prevTranslation.x = sBone.translation.x;
			sBone.prevTranslation.y = sBone.translation.y;
			sBone.prevTranslation.z = sBone.translation.z;

			sBone.rotation.x = rFactor * frameData->rotationX;
			sBone.rotation.y = rFactor * frameData->rotationY;
			sBone.rotation.z = rFactor * frameData->rotationZ;
			sBone.rotation.w = rFactor * frameData->rotationW;
			sBone.prevRotation = sBone.rotation;
		}

		this->currentFrame = -1.0;
		this->keyframePtr = ((int16_t*)frameData) + 1;
		SetFrame(0.0);
	}

	void AnimPlayerStream::GetBoneState(gsl::span<SkelBoneState> boneStateOut)
	{
		// static auto orgMethod = temple::GetPointer<void(AnimPlayerStream *self, SkaBoneData *bonesOut)>(0x1026ba10);
		// return orgMethod(this, boneStateOut);

		Expects(boneCount <= boneStateOut.size());

		auto currentFrame = this->currentFrame;
		auto currentFrameCeil = (int)ceil(currentFrame);
		auto currentFrameFloor = (int)floor(currentFrame);

		for (int i = 0; i < boneCount; i++) {
			auto &boneState = bones[i];
			auto &boneOut = boneStateOut[i];
			
			// Handle interpolation of the scale data
			if (currentFrameCeil <= boneState.scaleFrame) {
				boneOut.scale.x = boneState.prevScale.x;
				boneOut.scale.y = boneState.prevScale.y;
				boneOut.scale.z = boneState.prevScale.z;
			} else if (currentFrameFloor >= boneState.scaleNextFrame) {
				boneOut.scale.x = boneState.scale.x;
				boneOut.scale.y = boneState.scale.y;
				boneOut.scale.z = boneState.scale.z;
			} else {
				// Position [0,1] between the two frames.
				auto f = (currentFrame - boneState.scaleFrame) * boneState.scale.w;

				// Interpolate between the scale of the two keyframes
				boneOut.scale.x = (1.0f - f) * boneState.prevScale.x + f * boneState.scale.x;
				boneOut.scale.y = (1.0f - f) * boneState.prevScale.y + f * boneState.scale.y;
				boneOut.scale.z = (1.0f - f) * boneState.prevScale.z + f * boneState.scale.z;
			}

			// Handle interpolation of the rotation data
			if (currentFrameCeil <= boneState.rotationFrame) {
				boneOut.rotation.x = boneState.prevRotation.x;
				boneOut.rotation.y = boneState.prevRotation.y;
				boneOut.rotation.z = boneState.prevRotation.z;
				boneOut.rotation.w = boneState.prevRotation.w;
			}
			else if (currentFrameFloor >= boneState.rotationNextFrame) {
				boneOut.rotation.x = boneState.rotation.x;
				boneOut.rotation.y = boneState.rotation.y;
				boneOut.rotation.z = boneState.rotation.z;
				boneOut.rotation.w = boneState.rotation.w;
			} else {
				// Position [0,1] between the two frames.
				auto f = (currentFrame - boneState.rotationFrame) * boneState.scale.w;
				Quaternion q = slerpQuaternion(boneState.prevRotation, boneState.rotation, f);
				boneOut.rotation = *(DX::XMFLOAT4*)&q;
			}

			// Handle interpolation of the translation data
			if (currentFrameCeil <= boneState.translationFrame) {
				boneOut.translation.x = boneState.prevTranslation.x;
				boneOut.translation.y = boneState.prevTranslation.y;
				boneOut.translation.z = boneState.prevTranslation.z;
			}
			else if (currentFrameFloor >= boneState.translationNextFrame) {
				boneOut.translation.x = boneState.translation.x;
				boneOut.translation.y = boneState.translation.y;
				boneOut.translation.z = boneState.translation.z;
			} else {
				// Position [0,1] between the two frames.
				auto f = (currentFrame - boneState.translationFrame) * boneState.translation.w;
				boneOut.translation.x = f * boneState.translation.x + (1.0f - f) * boneState.prevTranslation.x;
				boneOut.translation.y = f * boneState.translation.y + (1.0f - f) * boneState.prevTranslation.y;
				boneOut.translation.z = f * boneState.translation.z + (1.0f - f) * boneState.prevTranslation.z;
			}

		}

	}

	float AnimPlayerStream::GetCurrentFrame() const
	{
		return currentFrame;
	}

}
