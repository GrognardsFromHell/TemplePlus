#include "graphics/camera.h"

using namespace DirectX;

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

	Ray3d WorldCamera::GetPickRay(float x, float y) {
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

		// calculate the position on the ground (y=0) and then move
		// along the screen vector 250 units below ground and 500 above ground
		// to build the ray. This is not really correct.
		auto onGround = worldFar - worldRay * dist;
		worldRay = XMVector3Normalize(worldRay);
		auto belowGround = onGround - worldRay * 250.0f;
		auto aboveGround = onGround + worldRay * 500.0f;

		XMFLOAT3 from, dir;
		XMStoreFloat3(&from, aboveGround);
		XMStoreFloat3(&dir, belowGround - aboveGround);

		return Ray3d{ from, dir };
	}

	// TODO: See if this can just be replaced by the proper version used below
	// This is equivalent to 10029570
	XMFLOAT2 WorldCamera::ScreenToTileLegacy(int x, int y) {
		
		auto tmpX = (x - mXTranslation) * 20 / INCH_PER_TILE; // * 0.70710677
		auto tmpY = (y - mYTranslation) / 0.7f * 20 / INCH_PER_TILE ; // * 1.0101526 originally

		return {
			tmpY - tmpX - INCH_PER_HALFTILE,
			tmpY + tmpX - INCH_PER_HALFTILE
		};

	}

	LocAndOffsets WorldCamera::ScreenToTile(int screenX, int screenY) {

		auto tmpX = (int)((screenX - mXTranslation) / 2);
		auto tmpY = (int)(((screenY - mYTranslation) / 2) / 0.7f);
		
		auto unrotatedX = tmpY - tmpX;
		auto unrotatedY = tmpY + tmpX;
		
		// Convert to tiles
		LocAndOffsets result;
		result.location.locx = unrotatedX / 20;
		result.location.locy = unrotatedY / 20;

		// Convert to offset within tile
		result.off_x = ((unrotatedX % 20) / 20.0f - 0.5f) * INCH_PER_TILE;
		result.off_y = ((unrotatedY % 20) / 20.0f - 0.5f) * INCH_PER_TILE;
		
		return result;
	}

	// replaces 10028EC0
	XMFLOAT3 WorldCamera::TileToWorld(locXY tilePos)
	{
		auto result = XMFLOAT3();
		result.x = (tilePos.locy - tilePos.locx - 1) * 20;
		result.y = (tilePos.locy + tilePos.locx ) * 14;
		result.z = 0;

		return result;
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

		auto view = XMMatrixRotationX(mCameraElevationAngleRad);

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

