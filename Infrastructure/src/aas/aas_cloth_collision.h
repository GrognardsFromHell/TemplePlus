
#pragma once

#include <span>

namespace aas {

	struct SkelBone;

	/**
	* Represents a sphere for cloth collision purposes.
	*/
	struct CollisionSphere {
		int boneId;
		float radius;
		Matrix3x4 worldMatrix;
		Matrix3x4 worldMatrixInverse;
		std::unique_ptr<CollisionSphere> next;
	};

	/**
	* Represents a cylinder for cloth collision purposes.
	*/
	struct CollisionCylinder {
		int boneId;
		float radius;
		float height;
		Matrix3x4 worldMatrix;
		Matrix3x4 worldMatrixInverse;
		std::unique_ptr<CollisionCylinder> next;
	};

	struct CollisionGeometry {
		std::unique_ptr<CollisionSphere> firstSphere;
		std::unique_ptr<CollisionCylinder> firstCylinder;
	};

	/**
	 * Extract the cloth collision information from a list of bones.
	 * The information is parsed from the bone names.
	 */
	CollisionGeometry FindCollisionGeometry(std::span<SkelBone> bones);

}
