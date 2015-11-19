
#include "graphics/camera.h"

XMFLOAT2 WorldCamera::WorldToScreen(XMFLOAT3 worldPos)
{
	auto viewMatrix(XMLoadFloat4x4(&mView));

	auto pos(XMVector3TransformCoord(XMLoadFloat3(&worldPos), viewMatrix));

	XMFLOAT2 screenPos;
	XMStoreFloat2(&screenPos, pos);

	return screenPos;
}

XMFLOAT3 WorldCamera::ScreenToWorld(float x, float y)
{
	auto screenVecFar(XMVectorSet(x, y, 0, 1));
	auto worldFar(XMVector3Unproject(
			screenVecFar,
			0,
			0,
			mScreenWidth,
			mScreenWidth,
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
		mScreenWidth,
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

void WorldCamera::CenterOn(float x, float y, float z) {

	if (mDirty) {
		Update();
	}

	auto targetScreenPos(WorldToScreen(XMFLOAT3(x, y, z)));

	// Y is flipped
	targetScreenPos.y *= -1;
	
	mXTranslation = - targetScreenPos.x;
	mYTranslation = - targetScreenPos.y;
	mDirty = true;
}

void WorldCamera::Update()
{

	if (mIdentityTransform) {
		XMStoreFloat4x4(&mViewProjection, XMMatrixIdentity());
		return;
	}

	auto projection(XMMatrixOrthographicLH(mScreenWidth, mScreenHeight, zNear, zFar));

	XMStoreFloat4x4(&mProjection, projection);
		
	XMMATRIX view;
	if (mScale != 1) {
		view = XMMatrixScaling(mScale, mScale, mScale);
	} else {
		view = XMMatrixIdentity();
	}
	
	/*
		This is x for sin(x) = 0.7, so x is roughly 44.42°.
		The reason here is, that Troika used a 20 by 14 grid
		and 14 / 20 = 0,7. So this ensures that the rotation
		to the isometric perspective makes the height of tiles
		70% of the width.
	*/
	view = XMMatrixRotationX(-0.77539754f) * view; 	// Roughly 45° (but not exact)
	
	/*
		We apply the translation before rotating the camera down,
		because otherwise the Z value is broken.
	*/
	auto dx = mXTranslation - mScreenWidth * 0.5f;
	auto dy = mYTranslation - mScreenHeight * 0.5f;
	dy = dy * 20.f / 14.f;
	view = XMMatrixTranslation(dx, 0, -dy) * view;

	view = XMMatrixRotationY(2.3561945f) * view;  // 135°

	XMStoreFloat4x4(&mView, view);

	XMStoreFloat4x4(&mViewProjection, view * projection);

	XMStoreFloat4x4(&mInvViewProjection, XMMatrixInverse(nullptr, projection));
	
	// Build a projection matrix that maps [0,w] and [0,h] to the screen.
	// To be used for UI drawing
	XMStoreFloat4x4(
		&mUiProjection,
		XMMatrixOrthographicOffCenterLH(0, mScreenWidth, mScreenHeight, 0, 1, 0)
	);

}
