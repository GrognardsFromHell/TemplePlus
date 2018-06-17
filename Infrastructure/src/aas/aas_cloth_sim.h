
#pragma once

#include "aas/aas_math.h"

namespace aas {

	struct CollisionCylinder;
	struct CollisionSphere;
	
	/**
	 * Represents an edge (between two vertices) and the distance (squared).
	 */
	struct EdgeDistance {
		int16_t to;
		int16_t from;
		float distSquared;
	};

	struct AasClothStuff {
		uint32_t clothVertexCount = 0;
		DirectX::XMFLOAT4* clothVertexPos1 = nullptr;
		DirectX::XMFLOAT4* clothVertexPos2 = nullptr;
		DirectX::XMFLOAT4* clothVertexPos3 = nullptr;
		uint8_t* bytePerVertex = nullptr;
		int boneDistancesCount = 0;
		int boneDistancesCountDelta = 0;
		EdgeDistance* boneDistances = nullptr;
		int boneDistances2Count = 0;
		int boneDistances2CountDelta = 0;
		EdgeDistance* boneDistances2 = nullptr;
		const CollisionSphere* spheres = nullptr;
		const CollisionCylinder* cylinders = nullptr;
		float field_34_time = 0.0f;

		~AasClothStuff();

		void UpdateBoneDistances();
		void Simulate(float time);

		void SetStuff(
			int clothVertexCount,
			DirectX::XMFLOAT4 *clothVertexPos,
			uint8_t *bytePerClothVertex,
			int edgeCount,
			EdgeDistance *edges,
			int indirectEdgeCount,
			EdgeDistance *indirectEdges
		);

		void SetSpheres(const CollisionSphere *head);
		void SetCylinders(const CollisionCylinder *head);

	private:
		void SimulateStep(float step_time);
		void ApplySprings();
		void HandleCollisions();

	private:
		void SplitEdges(
			EdgeDistance *edges,
			uint8_t *bytePerClothVertex,
			int edgeCount,
			int *boneCountOut,
			int *boneCountOutDelta
		);
	};

}
