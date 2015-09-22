#pragma once

#include <string>

#include "types.h"

namespace particles {

	/*
Interface for external functionality required by the particle systems.
*/
	class IPartSysExternal {
	public:
		virtual ~IPartSysExternal() = 0;

		/*
		Get the particle system fidelity setting ranging from 0 to 1.
	*/
		virtual float GetParticleFidelity() = 0;

		/*
		Retrieves the current position for the given object in the world.
		Returns true on success.
	*/
		virtual bool GetObjLocation(ObjHndl obj, Vec3& worldPos) = 0;

		/*
		Retrieves the current rotation in radians for the given object in the world.
		Returns true on success.
	*/
		virtual bool GetObjRotation(ObjHndl obj, float& rotation) = 0;

		/*
	Returns the radius of the given object.
	*/
		virtual float GetObjRadius(ObjHndl obj) = 0;

		/*
		Retrieves the world transformation matrix of a given bone for the given object's
		skeleton. This matrix transforms points from the bone space into world space.
	*/
		virtual bool GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) = 0;

		/*
		Returns the number of bones that the skeleton for the given object has.
	*/
		virtual int GetBoneCount(ObjHndl obj) = 0;

		/*
		Gets the position of a bone and it's parent bone.
		Returns the bone idx of the parent or -1 if the bone is not a child bone or is
		a special bone.

		The following bones are ignored by this method:
		Pony
		Footstep
		Origin
		Casting_ref
		EarthElemental_reg
		Casting_ref
		origin
		Bip01
		Bip01 Footsteps
		FootL_ref
		FootR_ref
		Head_ref
		HandL_ref
		HandR_ref
		Chest_ref
		groundParticleRef
		effects_ref
		trap_ref
		And all bones starting with #, which are cloth simulation related.
	*/
		virtual int GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) = 0;

		/*
		Gets the position of the given bone in the skeleton of the object.
		true if the bone position was retrieved
	*/
		virtual bool GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) = 0;

		/*
		Gets the point in screen coordinates for a given woorld coordinate.
	*/
		virtual void WorldToScreen(const Vec3& worldPos, Vec2& screenPos) = 0;

		/*
		Checks if the fog of war at the given point on screen is uncovered.
		TODO: Verify which coordinate system is used for x,y 
	*/
		virtual bool IsPointUnfogged(const Vec2& point) = 0;

		/*
		Checks if the given box (in screen space) is visible or not
	*/
		virtual bool IsBoxVisible(const Box2d& box) = 0;

		static void SetCurrent(IPartSysExternal* external) {
			mCurrent = external;
		}

		static IPartSysExternal* GetCurrent() {
			return mCurrent;
		}

	private:
		static IPartSysExternal* mCurrent;

	};

	inline IPartSysExternal::~IPartSysExternal() {
	}

}
