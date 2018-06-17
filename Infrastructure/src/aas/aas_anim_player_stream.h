
#pragma once

#include <gsl/span>

#include "aas/aas_math.h"

namespace aas {

	class Skeleton;
	struct SkelAnim;
	struct SkelBoneState;

	struct AnimPlayerStreamHeader {
		int field0;
		int field1;
		int field2;
		int8_t pad[3];
	};

	struct AnimPlayerStreamBone
	{
		DX::XMFLOAT4 prevScale;
		DX::XMFLOAT4 scale;
		DX::XMFLOAT4 prevRotation;
		DX::XMFLOAT4 rotation;
		DX::XMFLOAT4 prevTranslation;
		DX::XMFLOAT4 translation;
		int16_t scaleFrame;
		int16_t scaleNextFrame;
		int16_t rotationFrame;
		int16_t rotationNextFrame;
		int16_t translationFrame;
		int16_t translationNextFrame;
		int field6C;
	};

	struct AnimPlayerStream {
		const Skeleton* skeleton;
		const SkelAnim* animation;
		int streamIdx;
		float scaleFactor;
		float translationFactor;
		float currentFrame;
		void* keyframePtr;
		int boneCount;
		AnimPlayerStreamBone bones[1]; //set according to skafile bone count
		
		AnimPlayerStream(AnimPlayerStream&) = delete;
		AnimPlayerStream(AnimPlayerStream&&) = delete;
		AnimPlayerStream& operator=(AnimPlayerStream&) = delete;
		AnimPlayerStream& operator=(AnimPlayerStream&&) = delete;


		void SetFrame(float frame);
		void Initialize(const SkelAnim *animation, int streamIdx);
		void GetBoneState(gsl::span<SkelBoneState> boneStateOut);
		float GetCurrentFrame() const;
	};

}
