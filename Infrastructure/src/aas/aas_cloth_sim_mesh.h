
#pragma once

#include <cstdint>

#include "aas/aas_math.h"

namespace aas {

	struct AasClothStuff;
	class Mesh;

	struct AasClothStuff1 {
		const Mesh* mesh;
		int clothVertexCount; // Count of vertices found in skm_vertex_idx
		int16_t* vertexIdxForClothVertexIdx; // List of the original vertex indices from SKM
		uint8_t* bytePerClothVertex;
		uint8_t* bytePerClothVertex2;
		AasClothStuff* clothStuff;
		int8_t field_18;

		void Init(const Mesh &skmFile,
			int clothBoneId,
			const CollisionSphere *collilsionSpheresHead,
			const CollisionCylinder *collisionCylindersHead);
	};

}
