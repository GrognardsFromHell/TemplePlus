
#pragma once

#include <string>
#include <string_view>
#include <span>
#include <vector>
#include <memory>

#include "aas/aas_math.h"

namespace aas {

	/**
	 * Animations can either be advanced by time (automatic), distance moved, or object rotation.
	 */
	enum class SkelAnimDriver : uint8_t {
		Time = 0,
		Distance,
		Rotation
	};

	struct SkelAnimStream {
		uint16_t frames;
		int16_t variationId; // This might actually be a uint8_t
		float frameRate;
		float dps;
		// Offset to the start of the key-frame stream in relation to SkaAnimation start
		uint32_t dataOffset;
	};

	struct SkelAnimStreamData;

	struct SkelAnimEvent {
		int16_t frame;
		char type[48];
		char action[128];
	};

	struct SkelAnim {
		char name[64];
		SkelAnimDriver driveType;
		uint8_t loopable;
		uint16_t eventCount; // seems to be uint8_t in reality...
		uint32_t eventOffset;
		uint16_t streamCount;
		uint16_t unk;
		SkelAnimStream streams[10];

		const std::span<SkelAnimEvent> GetEventData() const {
			if (eventCount == 0) {
				return {};
			} else {
				auto data = (SkelAnimEvent*)(((char*)this) + eventOffset);
				return std::span(data, eventCount);
			}
		}

		const SkelAnimStreamData* GetStreamKeyframes(int streamIdx = 0) const {
			return (SkelAnimStreamData*)(((char*)this) + streams[streamIdx].dataOffset);
		}
	};

	struct SkelBoneState {
		DX::XMFLOAT4 scale;
		DX::XMFLOAT4 rotation;
		DX::XMFLOAT4 translation;

		static void Lerp(std::span<SkelBoneState> out,
			const std::span<SkelBoneState> from,
			const std::span<SkelBoneState> to,
			int count,
			float fraction);
	};

	struct SkelBone {
		int16_t flags;
		int16_t parentId;
		char name[40];
		int field_2C;
		float field_30;
		SkelBoneState initialState;
	};

	class Skeleton {
	public:
		Skeleton(std::string filename, std::vector<uint8_t> data);

		std::span<SkelBone> GetBones() const {
			return bones_;
		}

		std::span<SkelAnim> GetAnimations() const {
			return animations_;
		}

		/**
		 * Searches for a bone in this skeleton by name (case-insensitive).
		 * Returns nullptr if no bone is found.
		 */
		const SkelBone* FindBoneByName(std::string_view name) const;

		/**
		* Searches for the index of a bone in this skeleton by name (case-insensitive).
		* Returns -1 if no bone is found.
		*/
		int FindBoneIdxByName(std::string_view name) const;

		/**
		 * Searches for an animation in this skeleton by it's name (case insensitive)
		 * and returns the index of the animation in the skeleton.
		 * If no animation is found, returns -1.
		 */
		int FindAnimIdxByName(std::string_view name) const;

		const std::string &GetFilename() const {
			return filename_;
		}

	private:
		std::string filename_;
		std::vector<uint8_t> data_;

		// These are views into the data buffer
		std::span<SkelBone> bones_;
		std::span<SkelAnim> animations_;
	};

	std::unique_ptr<Skeleton> LoadSkeletonFile(std::string_view filename);

}
