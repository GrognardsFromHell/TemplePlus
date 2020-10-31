
#define _SCL_SECURE_NO_WARNINGS

#include <optional>

#include <infrastructure/logging.h>
#include <infrastructure/exception.h>

#include "aas_anim_player.h"
#include "aas_anim_player_stream.h"
#include "aas_animated_model.h"
#include "aas_skeleton.h"
#include "aas_anim_events.h"

namespace aas {

	struct AnimEvent {
		int frame; // On which frame does the event occur
		AnimEventType type;
		std::string_view args;
	};

	AnimPlayer::AnimPlayer()
	{
		ownerAnim = 0;
		nextRunningAnim = 0;
		prevRunningAnim = 0;
		field_D = 0;
		// Default is fading in over .5 seconds
		weight = 0;
		fadingSpeed = 2.0f;
		eventHandlingDepth = 0;
		streamCount = 0;
		currentTime = 0;
		currentTimeEvents = 0;
		for (int i = 0; i < 4; i++) {
			streams[i] = nullptr;
		}
	}

	AnimPlayer::~AnimPlayer()
	{
		for (int i = 0; i < streamCount; i++) {
			if (streams[i]) {
				free(streams[i]);
			}
		}
		streamCount = 0;

		if (ownerAnim) {
			ownerAnim->RemoveRunningAnim(this);
		}
	}

	void AnimPlayer::GetDistPerSec(float *distPerSec)
	{
		if (animation->driveType == SkelAnimDriver::Distance && fadingSpeed > 0.0) {
			*distPerSec = ownerAnim->scale * distancePerSecond;
		}
	}

	void AnimPlayer::GetRotationPerSec(float * rotationPerSec)
	{
		if (animation->driveType == SkelAnimDriver::Rotation && fadingSpeed > 0.0) {
			*rotationPerSec = distancePerSecond;
		}
	}

	void AnimPlayer::AdvanceEvents(float timeChanged, float distanceChanged, float rotationChanged)
	{

		if (duration <= 0) {
			return;
		}

		// Decide what the effective advancement in the animation stream will be
		float effectiveAdvancement;
		switch (animation->driveType) {
		default:
		case SkelAnimDriver::Time:
			effectiveAdvancement = timeChanged;
			break;
		case SkelAnimDriver::Distance:
			effectiveAdvancement = ownerAnim->scaleInv * distanceChanged;
			break;
		case SkelAnimDriver::Rotation:
			// TODO: Weirdly enough, in the other function it's using the absolute value of it
			// Does this mean that rotation-based animations will not properly trigger events???
			effectiveAdvancement = rotationChanged;
			break;
		}

		for (auto &event : events) {

			// Convert from frame id to "time"
			auto eventTime = std::min(duration, event.frame / frameRate);

			// Check if the increase in time will cause the event to trigger
			// and consider looping animations, but only if the event would occur during the next loop
			if (currentTimeEvents <= eventTime &&  currentTimeEvents + effectiveAdvancement > eventTime
				|| animation->loopable
				&& currentTimeEvents - duration <= eventTime && eventTime < (currentTimeEvents + effectiveAdvancement - duration))
			{
				auto timeAfterEvent = currentTimeEvents + effectiveAdvancement - eventTime;
				eventHandler->HandleEvent(
					event.frame,
					timeAfterEvent,
					event.type,
					event.args
				);
			}
		}

		currentTimeEvents += effectiveAdvancement;

		if (currentTimeEvents > duration) {

			if (animation->loopable) {
				auto extraTime = currentTimeEvents - duration;
				currentTimeEvents = extraTime;
				if (currentTimeEvents > duration) {
					if (extraTime - effectiveAdvancement == 0.0) {
						currentTimeEvents = 0.0;
					} else {
						currentTimeEvents = duration;
					}
				}
			} else {
				currentTimeEvents = duration;
				if (eventHandler) {
					eventHandler->HandleEvent(
						(int)(frameRate * duration),
						0.0f,
						AnimEventType::End,
						""
					);
				}
			}

		}

	}

	void AnimPlayer::FadeInOrOut(float timeChanged)
	{
		// Modify weight according to fadein/fadeout speed
		weight += timeChanged * fadingSpeed;

		// Clamp weight to [0,1]
		if (weight <= 0.0f) {
			weight = 0.0f;
		} else if (weight > 1.0) {
			weight = 1.0f;
		}
	}

	void AnimPlayer::method6(gsl::span<SkelBoneState> boneStateOut, float timeChanged, float distanceChanged, float rotationChanged)
	{

		//static auto AasRunningAnim_method6 = temple::GetPointer<void __fastcall(AnimPlayer *self, int dummy, SkelBoneState *bones, float timeChanged, float distanceChanged, float rotationChanged)>(0x1026add0);
		//return AasRunningAnim_method6(this, 0, boneStateOut, timeChanged, distanceChanged, rotationChanged);

		// In ToEE, boneIdx is never set to anything other than -1

		if (duration > 0) {
			// Decide what the effective advancement in the animation stream will be
			float effectiveAdvancement;
			switch (animation->driveType) {
			case SkelAnimDriver::Time:
				effectiveAdvancement = timeChanged;
				break;
			case SkelAnimDriver::Distance:
				effectiveAdvancement = ownerAnim->scaleInv * distanceChanged;
				break;
			case SkelAnimDriver::Rotation:
				// TODO: Weirdly enough, in the other function it's using the absolute value of it
				// Does this mean that rotation-based animations will not properly trigger events???
				effectiveAdvancement = fabs(rotationChanged);
				break;
			default:
				throw TempleException("Unknown animation drive type: {}", (int)animation->driveType);
			}

			// Same logic as in AddTime for events, just different data fields and no event handling
			currentTime += effectiveAdvancement;
			if (currentTime > duration) {
				if (animation->loopable) {
					auto extraTime = currentTime - duration;
					currentTime = extraTime;
					if (currentTime > duration) {
						if (extraTime - effectiveAdvancement == 0.0) {
							currentTime = 0.0;
						} else {
							currentTime = duration;
						}
					}
				} else {
					currentTime = duration;
				}
			}

			// Propagate the frame index derived from the current time to all streams
			for (int i = 0; i < streamCount; i++) {
				auto frame = streamFps[i] * currentTime;
				streams[i]->SetFrame(frame);
			}

		}

		if (streamCount != 1 || ownerAnim->variationCount != 1 || weight != 1.0f) {

			// Get the bone data for each stream
			SkelBoneState boneData[4 * 1024];			 // 4 streams with at most 1024 bones each
			for (int i = 0; i < streamCount; i++) {
				streams[i]->GetBoneState(gsl::span(&boneData[i * 1024], 1024));
			}

			auto boneCount = ownerAnim->skeleton->GetBones().size();
			Expects(boneCount <= 1024);

			SkelBoneState boneDataTemp[1024];
			gsl::span<SkelBoneState> boneDataBuf = boneDataTemp;
			if (weight == 1.0f) {
				boneDataBuf = boneStateOut;
			}

			// Copy over the first stream's bone data
			std::copy_n(&boneData[1024 * streamVariationIndices[0]], boneDataBuf.size(), boneDataBuf.begin());

			// LERP the rest
			for (auto i = 1; i < ownerAnim->variationCount; i++) {
				if (streamVariationIndices[i] < 4) {
					auto factor = ownerAnim->variations[i].factor;
					SkelBoneState::Lerp(boneDataBuf, boneDataBuf, gsl::span(&boneData[1024 * streamVariationIndices[i]], 1024), boneCount, factor);
				}
			}

			SkelBoneState::Lerp(boneStateOut, boneStateOut, boneDataBuf, boneCount, weight);
		} else {
			streams[0]->GetBoneState(boneStateOut);
		}

	}

	void AnimPlayer::SetTime(float time) {
		currentTime = time;
		while (currentTime > duration) {
			currentTime -= duration;
		}

		auto frameIndex = frameRate * currentTime;
		for (int i = 0; i < streamCount; i++) {
			auto stream = streams[i];
			stream->SetFrame(frameIndex);
		}
	}

	float AnimPlayer::GetCurrentFrame()
	{
		if (streamCount < 1) {
			return 0.0f;
		}
		return streams[0]->GetCurrentFrame();
	}

	float AnimPlayer::method9()
	{
		return 0.5f;
	}

	float AnimPlayer::method10()
	{
		return 0.5f;
	}

	void AnimPlayer::EnterEventHandling()
	{
		eventHandlingDepth++;
	}

	void AnimPlayer::LeaveEventHandling()
	{
		eventHandlingDepth--;
	}

	int AnimPlayer::GetEventHandlingDepth()
	{
		return eventHandlingDepth;
	}

	void AnimPlayer::Attach(AnimatedModel *owner, int animIdx, IAnimEventHandler *eventHandler)
	{
		//static auto orgMethod = temple::GetPointer<int __fastcall(AnimPlayer*, void*, AnimatedModel*, int boneIdx, int animIdx, IAnimEventHandler*)>(0x1026a680);
		//return orgMethod(this, 0, owner, boneIdx, animIdx, eventHandler);
		Expects(!streamCount && owner && !ownerAnim);
		auto skeleton = owner->skeleton;
		Expects(skeleton);

		auto anims = skeleton->GetAnimations();
		Expects(animIdx >= 0 && animIdx < anims.size());

		this->animation = &anims[animIdx];
		this->distancePerSecond = animation->streams[0].dps;
		this->frameRate = animation->streams[0].frameRate;
		this->eventHandler = eventHandler;
		SetEvents(owner, animation);
		this->streamCount = 0;

		// TODO: The entire variation stuff is unused I think
		int skaStreamIdxMap[4]; // Maps this player's streams to their respective idx in the SKA anim
		for (int i = 0; i < owner->variationCount; i++) {
			auto &variation = owner->variations[i];

			// Find a stream in the animation suitable for the variation that is requested
			int skaStreamIdx = -1;
			for (int j = 0; j < animation->streamCount; j++) {
				int streamVariation = animation->streams[j].variationId;
				if (streamVariation = variation.variationId || skaStreamIdx == -1 && streamVariation == -1) {
					skaStreamIdx = j;
				}
			}

			streamVariationIds[i] = 4; // This effectively means no stream for this variation
			if (skaStreamIdx != -1) {
				// Do we already use that stream???
				int j;
				for (j = 0; j < streamCount; j++) {
					if (skaStreamIdxMap[j] == skaStreamIdx) {
						break; // Found it
					}
				}

				// Remember which stream in this player is used 
				// for the variation found in the parent model
				streamVariationIndices[i] = j;

				// Do we have to create the new stream?
				if (j == streamCount && j < 4) {
					skaStreamIdxMap[j] = skaStreamIdx;

					streams[j] = CreateAnimPlayerStream(skeleton.get());
					streams[j]->Initialize(animation, skaStreamIdx);
					streamFps[j] = 0;
					// NOTE: The following line used the incorrect index into animation.streams
					variationId[j] = (uint8_t) animation->streams[skaStreamIdx].variationId;
					streamCount++;
				}

			}
		}

		// Calculate the duration based on the individual streams (weighted together)
		// NOTE: I don't think this makes a terrible amount of sense since as far as I understand
		// the variation factor it only affects bone blending, not length
		duration = 0;
		for (int i = 0; i < owner->variationCount; i++) {
			auto factor = owner->variations[i].factor;

			auto streamIdx = streamVariationIndices[i];
			if (streamIdx < 4) {
				auto &skaStream = animation->streams[skaStreamIdxMap[i]];
				if (skaStream.frames > 1 && skaStream.frameRate > 0) {
					duration = (skaStream.frames - 1) / skaStream.frameRate * factor 
						+ (1.0f - factor) * duration;
				}
			}
		}

		// Recalc the stream FPS taking into account the variation weighting applied above
		if (duration > 0) {
			for (int i = 0; i < streamCount; i++) {
				auto &skaStream = animation->streams[skaStreamIdxMap[i]];
				if (skaStream.frames > 1) {
					streamFps[i] = (skaStream.frames - 1) / duration;
				}
			}
		}

		owner->AddRunningAnim(this);
		field_D = 1;

	}

	void AnimPlayer::Setup2(float fadeInTimeSecs)
	{
		// Always 0.5s
		if (fadeInTimeSecs <= 0.0f) {
			fadingSpeed = 1.0f;
			weight = 1.0f;
		} else {
			auto fadeInSpeed = 1.0f / fadeInTimeSecs;
			if (fadeInSpeed <= fadingSpeed) {
				weight = 0.0001f;
			}
			else {
				fadingSpeed = fadeInSpeed;
				weight = 0.0001f;
			}
		}
	}

	static std::optional<AnimEventType> GetEventType(std::string_view type) {
		if (!_stricmp(type.data(), "script")) {
			return AnimEventType::Script;
		} else if (!_stricmp(type.data(), "end")) {
			return AnimEventType::End;
		} else if (!_stricmp(type.data(), "action")) {
			return AnimEventType::Action;
		} else {
			return {};
		}
	}

	void AnimPlayer::SetEvents(const AnimatedModel *owner, const SkelAnim *anim)
	{
		auto skelEvents = anim->GetEventData();

		events.resize(events.size());
		for (auto &skelEvent : skelEvents) {
			auto type = GetEventType(skelEvent.type);
			if (!type) {
				logger->warn("Unknown animation type '{}' in {}", anim->name, owner->skeleton->GetFilename());
				continue;
			}

			AnimEvent event;
			event.frame = skelEvent.frame;
			event.type = *type;
			event.args = skelEvent.action;
			events.push_back(event);
		}
	}

}
