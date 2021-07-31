
#include <gsl/gsl>
#include <memory>
#include <aas/aas_math.h>

#include "aas_cloth_collision.h"
#include "aas_skeleton.h"

namespace aas {

	CollisionGeometry FindCollisionGeometry(std::span<SkelBone> bones)
	{
		CollisionGeometry result;

		// Tails of the linked lists		
		CollisionSphere* spheresTail = nullptr;
		CollisionCylinder* cylindersTail = nullptr;

		// Parse cloth bone state from Skeleton
		for (auto i = 0u; i < bones.size(); i++) {

			auto namePos = bones[i].name;
			if (!_strnicmp("#Sphere", namePos, 7)) {
				while (*namePos) {
					if (*namePos == '{')
						break;
					namePos++;
				}
				auto radius = 0.0f;
				if (*namePos == '{') {
					radius = (float)atof(namePos + 1);
				}

				auto newClothSphere = std::make_unique<CollisionSphere>();
				newClothSphere->radius = radius;
				newClothSphere->next = nullptr;
				newClothSphere->boneId = i;

				// Append to the linked list structure
				if (!result.firstSphere) {
					result.firstSphere = std::move(newClothSphere);
					spheresTail = result.firstSphere.get();
				} else {
					spheresTail->next = std::move(newClothSphere);
					spheresTail = spheresTail->next.get();
				}
				continue;
			}

			if (!_strnicmp("#Cylinder", namePos, 9)) {
				while (*namePos) {
					if (*namePos == '{')
						break;
					namePos++;
				}
				auto radius = 0.0f;
				auto height = 0.0f;
				if (*namePos == '{') {
					radius = (float)atof(namePos + 1);
					while (*namePos) {
						if (*namePos == ',')
							break;
						namePos++;
					}
					if (*namePos == ',')
						height = (float)atof(namePos + 1);
				}

				auto newClothCyl = std::make_unique<CollisionCylinder>();
				newClothCyl->radius = radius;
				newClothCyl->height = height;
				newClothCyl->next = nullptr;
				newClothCyl->boneId = i;

				if (!result.firstCylinder) {
					result.firstCylinder = std::move(newClothCyl);
					cylindersTail = result.firstCylinder.get();
				}
				else {
					cylindersTail->next = std::move(newClothCyl);
					cylindersTail = cylindersTail->next.get();
				}
				continue;
			}
		}

		return result;

	}

}
