
#pragma once

#include <span>

#include "aas/aas_math.h"

namespace aas {

	struct AnimPlayerStream;
	class AnimatedModel;
	struct SkelAnim;
	struct SkelAnimEvent;
	struct SkelBoneState;
	class IAnimEventHandler;
	class Skeleton;
	struct AnimEvent;

	class AnimPlayer {
	public:
		AnimatedModel * ownerAnim;
		char field_D;
		char field_E;
		char field_F;
		float weight; // 1.0 = Fully weighted, otherwise weighted with primary anims
		float fadingSpeed; // Velocity with which weight is being changed
		int eventHandlingDepth;
		AnimPlayer * nextRunningAnim;
		AnimPlayer * prevRunningAnim;
		const SkelAnim * animation;
		int streamCount;
		AnimPlayerStream * streams[4];
		float streamFps[4]; // "frames per drive unit"
		int8_t streamVariationIndices[4]; // indices into streamVariationIds
		int8_t streamVariationIds[4];
		int8_t variationId[4];
		float currentTimeEvents;
		float currentTime;
		float duration;
		float frameRate;
		float distancePerSecond;
		std::vector<AnimEvent> events;
		IAnimEventHandler* eventHandler;
	public:

		AnimPlayer();
		~AnimPlayer();

		void GetDistPerSec(float* distPerSec);
		void GetRotationPerSec(float* rotationPerSec);
		void AdvanceEvents(float timeChanged, float distanceChanged, float rotationChanged);
		void FadeInOrOut(float timeChanged);
		void method6(std::span<SkelBoneState> boneStateOut, float timeChanged, float distanceChanged, float rotationChanged);
		void SetTime(float time);
		float GetCurrentFrame();
		float method9();
		float method10();
		void EnterEventHandling();
		void LeaveEventHandling();
		int GetEventHandlingDepth();

		void Attach(AnimatedModel *owner, int animIdx, IAnimEventHandler *eventHandler);
		void Setup2(float a);

	private:
		void SetEvents(const AnimatedModel *owner, const SkelAnim *anim);

	};

	AnimPlayerStream * CreateAnimPlayerStream(Skeleton* skeleton);

}
