#include "graphics/camera.h"

namespace gfx {

	constexpr float INCH_PER_TILE = 28.284271247461900976033774484194f;
	constexpr float INCH_PER_HALFTILE = (INCH_PER_TILE / 2.0f);

	bool WorldCamera::IsBoxOnScreen(XMFLOAT2 screenCenter, float left, float top, float right, float bottom) const {
		auto dx = mCurScreenOffset.x;
		auto dy = mCurScreenOffset.y;

		auto screenX = dx - screenCenter.x;
		auto screenY = dy - screenCenter.y;

		// calculate viewport
		float viewportRight = mScreenWidth * 0.5f / mScale;
		float viewportLeft = -viewportRight;
		float viewportBottom = mScreenHeight * 0.5f / mScale;
		float viewportTop = -viewportBottom;

		if (screenX + left >= viewportRight)
			return false;

		if (screenX + right <= viewportLeft)
			return false;

		if (screenY + top >= viewportBottom)
			return false;

		if (screenY + bottom <= viewportTop)
			return false;

		return true;
	}

	XMFLOAT2 WorldCamera::WorldToScreen(XMFLOAT3 worldPos) {
		if (mDirty) {
			Update();
		}

		auto viewMatrix(XMLoadFloat4x4(&mView));

		auto pos(XMVector3TransformCoord(XMLoadFloat3(&worldPos), viewMatrix));

		XMFLOAT2 screenPos;
		XMStoreFloat2(&screenPos, pos);
		screenPos.x *= -1;

		return screenPos;
	}

	XMFLOAT2 WorldCamera::WorldToScreenUi(XMFLOAT3 worldPos) {
		auto result(WorldToScreen(worldPos));
		result.x *= -1;
		result.y *= -1;

		// Move the origin to the top left instead of the center of the screen
		result.x += mScreenWidth / 2;
		result.y += mScreenHeight / 2;

		return result;
	}

	XMFLOAT3 WorldCamera::ScreenToWorld(float x, float y) {

		if (mDirty) {
			Update();
		}

		auto screenVecFar(XMVectorSet(x, y, 0, 1));
		auto worldFar(XMVector3Unproject(
			screenVecFar,
			0,
			0,
			mScreenWidth,
			mScreenHeight,
			zNear,
			zFar,
			XMLoadFloat4x4(&mProjection),
			XMLoadFloat4x4(&mView),
			XMMatrixIdentity()
		));

		auto screenVecClose(XMVectorSet(x, y, 1, 1));
		auto worldClose(XMVector3Unproject(
			screenVecClose,
			0,
			0,
			mScreenWidth,
			mScreenHeight,
			zNear,
			zFar,
			XMLoadFloat4x4(&mProjection),
			XMLoadFloat4x4(&mView),
			XMMatrixIdentity()
		));

		auto worldRay(worldFar - worldClose);
		auto dist = XMVectorGetY(worldFar) / XMVectorGetY(worldRay);

		XMFLOAT3 result;
		XMStoreFloat3(&result, worldFar - worldRay * dist);

		return result;
	}

	// This is equivalent to 10029570
	XMFLOAT2 WorldCamera::ScreenToTileLegacy(int x, int y) {

		auto tmpX = (x - mXTranslation) * 20 / INCH_PER_TILE;
		auto tmpY = (y - mYTranslation) * INCH_PER_TILE / 14 * 0.5f;

		return {
			tmpY - tmpX - INCH_PER_HALFTILE,
			tmpY + tmpX - INCH_PER_HALFTILE
		};

	}

	void WorldCamera::CenterOn(float x, float y, float z) {

		mDirty = true;
		mXTranslation = 0;
		mYTranslation = 0;
		Update();

		auto targetScreenPos(WorldToScreen(XMFLOAT3(x, y, z)));

		mXTranslation = targetScreenPos.x;
		mYTranslation = targetScreenPos.y;
		mDirty = true;
	}

	void WorldCamera::Update() {

		if (mIdentityTransform) {
			XMStoreFloat4x4(&mViewProjection, XMMatrixIdentity());
			return;
		}

		auto projection(XMMatrixOrthographicLH(mScreenWidth / mScale,
		                                       mScreenHeight / mScale,
		                                       zNear,
		                                       zFar));

		XMStoreFloat4x4(&mProjection, projection);

		/*
			This is x for sin(x) = 0.7, so x is roughly 44.42°.
			The reason here is, that Troika used a 20 by 14 grid
			and 14 / 20 = 0,7. So this ensures that the rotation
			to the isometric perspective makes the height of tiles
			70% of the width.
		*/
		auto view = XMMatrixRotationX(-0.77539754f); // Roughly 45° (but not exact)

		auto dxOrigin = - mScreenWidth * 0.5f;
		auto dyOrigin = - mScreenHeight * 0.5f;
		dyOrigin = dyOrigin * 20.f / 14.f;
		XMFLOAT2 transformOrigin;
		XMStoreFloat2(&transformOrigin,
		              XMVector3TransformCoord(
			              XMVectorZero(),
			              XMMatrixTranslation(dxOrigin, 0, -dyOrigin) * view
		              )
		);

		/*
			We apply the translation before rotating the camera down,
			because otherwise the Z value is broken.
		*/
		auto dx = mXTranslation - mScreenWidth * 0.5f;
		auto dy = mYTranslation - mScreenHeight * 0.5f;
		dy = dy * 20.f / 14.f;
		view = XMMatrixTranslation(dx, 0, -dy) * view;

		// Calculate how much the screen has moved in x,y coordinates
		// using the current camera position
		XMStoreFloat2(&mCurScreenOffset, XMVector3TransformCoord(XMVectorZero(), view));
		mCurScreenOffset.x -= transformOrigin.x;
		// mCurScreenOffset.x *= -1;
		mCurScreenOffset.y -= transformOrigin.y;
		mCurScreenOffset.y *= -1;

		view = XMMatrixRotationY(2.3561945f) * view; // 135°

		XMStoreFloat4x4(&mView, view);

		XMStoreFloat4x4(&mViewProjection, view * projection);

		XMStoreFloat4x4(&mInvViewProjection, XMMatrixInverse(nullptr, projection));

		// Build a projection matrix that maps [0,w] and [0,h] to the screen.
		// To be used for UI drawing
		XMStoreFloat4x4(
			&mUiProjection,
			XMMatrixOrthographicOffCenterLH(0, mScreenWidth, mScreenHeight, 0, 1, 0)
		);

		mDirty = false;

	}

}

