
#pragma once

#include "../infrastructure/location.h"
#include "collision.h"
#include "math.h"

namespace gfx {

	/**
	 * Defines the projection and position of the camera.
	 */
	class WorldCamera {
	public:

		bool IsBoxOnScreen(XMFLOAT2 point, 
			float left, float top, float right, float bottom) const;
		
		const XMFLOAT4X4& GetViewProj() {
			if (mDirty) {
				Update();
				mDirty = false;
			}
			return mViewProjection;
		}

		const XMFLOAT4X4& GetProj() {
			if (mDirty) {
				Update();
			}
			return mProjection;
		}

		void SetTranslation(float x, float y) {
			mXTranslation = x;
			mYTranslation = y;
			mDirty = true;
		}

		XMFLOAT2 GetTranslation() const {
			return{ mXTranslation, mYTranslation };
		}

		XMFLOAT2 Get2dTranslation() {
			if (mDirty) {
				Update();
			}
			return mCurScreenOffset;
		}

		void SetScreenWidth(float width, float height) {
			mScreenWidth = width;
			mScreenHeight = height;
			mDirty = true;
		}

		float GetScreenWidth() const {
			return mScreenWidth;
		}
		float GetScreenHeight() const {
			return mScreenHeight;
		}

		void SetScale(float scale) {
			mScale = scale;
			mDirty = true;
		}

		void SetCameraAngle(float angleRad) {
			mCameraElevationAngleRad = -angleRad;
			mDirty = true;
		}

		float GetScale() const {
			return mScale;
		}

		void SetIdentityTransform(bool enable) {
			mIdentityTransform = enable;
			mDirty = true;
		}

		/**
		* Transforms a world coordinate into the local coordinate
		* space of the screen (in pixels).
		*/
		XMFLOAT2 WorldToScreen(XMFLOAT3 worldPos);

		/**
		* Transforms a world coordinate into the local coordinate
		* space of the screen (in pixels).
		*/
		XMFLOAT2 WorldToScreenUi(XMFLOAT3 worldPos);

		/**
		 * Transforms a screen coordinate relative to the upper left
		 * corner of the screen into a world position with y = 0.
		 */
		XMFLOAT3 ScreenToWorld(float x, float y);

		/**
		 * Returns a ray that pierces through the screen starting at the mouse position
		 * and goes to zFar, effectively.
		 */
		Ray3d GetPickRay(float x, float y);

		XMFLOAT2 ScreenToTileLegacy(int x, int y);

		LocAndOffsets ScreenToTile(int screenX, int screenY);
		XMFLOAT3 TileToWorld(locXY tilePos);

		void CenterOn(float x, float y, float z);

		const XMFLOAT4X4 &GetUiProjection() {
			if (mDirty) {
				Update();
				mDirty = false;
			}
			return mUiProjection;
		}

	private:
		float mXTranslation = 0.0f;
		float mYTranslation = 0.0f;
		float mScale = 1.0f;
		/*
			The ToEE camera angle is arcsin(0.7) ~= 44.42°.
			The reason here is, that Troika used a 20 by 14 grid
			and 14 / 20 = 0,7. So this ensures that the rotation
			to the isometric perspective makes the height of tiles
			70% of the width.
		*/
		static constexpr float DEFAULT_ANGLE = -0.77539754f;
		float mCameraElevationAngleRad = DEFAULT_ANGLE;
		float mScreenWidth;
		float mScreenHeight;
		bool mDirty = true;
		bool mIdentityTransform = false;

		// This is roughly 64 * PIXEL_PER_TILE (inches per tile to be precise)
		static constexpr float zNear = -1814.2098860142813876543089825124f;
		static constexpr float zFar = 1814.2098860142813876543089825124f;

		// Current x,y offset in screen space relative to a
		// cam position of 0,0
		XMFLOAT2 mCurScreenOffset;

		XMFLOAT4X4 mProjection;
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mViewProjection;
		XMFLOAT4X4 mViewUntranslated;
		XMFLOAT4X4 mInvViewProjection;
		XMFLOAT4X4 mUiProjection;

		void Update();
	};

}
