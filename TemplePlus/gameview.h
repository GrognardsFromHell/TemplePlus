
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

	XMFLOAT4 GetSceneRect() const {
		return mSceneRect;
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
	gfx::WorldCamera& GetCamera() {
		return *mCamera;
	}

	/**
	 * Gets the world coordinate that is at the center of the screen.
	 */
	XMFLOAT3 GetScreenCenterInWorld3d();

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
