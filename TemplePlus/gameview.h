
#pragma once

#include <graphics/device.h>
#include "mainwindow.h"

class GameView {
public:
	GameView(MainWindow &mainWindow, gfx::RenderingDevice &device, int width, int height);
	~GameView();

	int GetWidth() const {
		return mWidth;
	}
	int GetHeight() const {
		return mHeight;
	}
	float GetZoom() const {
		return mCamera->GetScale();
	}
	void SetZoom(float zoom) const {
		mCamera->SetScale(zoom);
	}

	XMFLOAT4 GetSceneRect() const {
		return mSceneRect;
	}

	void SetTranslation(float x, float y) {
		mCamera->SetTranslation(x, y);
	}

	XMFLOAT2 GetTranslation() const {
		return mCamera->GetTranslation();
	}

	/**
	 * Maps from screen coordinates into the scene coordinate system.
	 */
	XMINT2 MapToScene(int x, int y) const;

	/*
	 * maps from scene coordinates to window coordinates
	 */
	XMINT2 MapFromScene(int x, int y) const;

	/*
	 * returns the camera used to render the scene
	 */
	const gfx::WorldCameraPtr &GetCamera() {
		return mCamera;
	}

	/**
	 * Gets the world coordinate that is at the center of the screen.
	 */
	XMFLOAT3 GetScreenCenterInWorld3d();

	XMFLOAT3 ScreenToWorld(float x, float y) {
		return mCamera->ScreenToWorld(x, y);
	}

	XMFLOAT2 WorldToScreenUi(XMFLOAT3 worldPos) {
		return mCamera->WorldToScreenUi(worldPos);
	}

	Ray3d GetPickRay(float x, float y) {
		return mCamera->GetPickRay(x, y);
	}

	// Note: not to be used with WorldToScreenUi. Use tilePos.ToInches3D() instead
	XMFLOAT3 TileToWorld(locXY tilePos);

	LocAndOffsets ScreenToTile(int screenX, int screenY);

	XMFLOAT2 ScreenToTileLegacy(int x, int y);

private:
	friend class GameLoop;

	gfx::ResizeListenerRegistration mResizeListener;
	MainWindow &mMainWindow;
	gfx::RenderingDevice &mDevice;
	gfx::WorldCameraPtr mCamera;
	int mWidth;
	int mHeight;
	float mSceneScale;
	XMFLOAT4 mSceneRect;

	void Resize(int width, int height);
};

extern GameView *gameView;
