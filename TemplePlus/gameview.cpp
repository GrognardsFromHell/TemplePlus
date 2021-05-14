
#include "stdafx.h"
#include "gameview.h"

#include "config/config.h"
#include "tig/tig_mouse.h"

GameView::GameView(MainWindow &mainWindow, gfx::RenderingDevice &device, int width, int height)
    : mMainWindow(mainWindow), mDevice(device), mWidth(width), mHeight(height),
      mResizeListener(device.AddResizeListener(
          [this](int w, int h) { this->Resize(w, h); })),

	mCamera(std::make_shared<gfx::WorldCamera>()) {
	mCamera->SetScreenSize((float) width, (float) height);
	
	auto &backBufferSize = device.GetBackBufferSize();
	Resize(backBufferSize.width, backBufferSize.height);

	mainWindow.SetMouseMoveHandler([this](int x, int y, int wheelDelta) {
		// Map to game view
		auto pos = MapToScene(x, y);
		x = pos.x;
		y = pos.y;
		
		// Account for a resized screen
		if (x < 0 || y < 0 || x >= mWidth || y >= mHeight)
		{
			if (config.windowed)
			{
				if ((x > -7 && x < mWidth + 7 && x > -7 && y < mHeight + 7))
				{
					if (x < 0)
						x = 0;
					else if (x > mWidth)
						x = mWidth;
					if (y < 0)
						y = 0;
					else if (y > mHeight)
						y = mHeight;
					mouseFuncs.MouseOutsideWndSet(false);
					mouseFuncs.SetPos(x, y, wheelDelta);
					return;
				}
				else
				{
					mouseFuncs.MouseOutsideWndSet(true);
				}
			}
			else
			{
				logger->info("Mouse outside resized window: x: {}, y: {}, wheel: {}", x, y, wheelDelta);
			}

			return;
		}
		mouseFuncs.MouseOutsideWndSet(false);
		mouseFuncs.SetPos(x, y, wheelDelta);
	});

  if (!gameView) {
	  gameView = this;
  }
}

GameView::~GameView() {
	mMainWindow.SetMouseMoveHandler(nullptr);

	if (gameView == this) {
		gameView = nullptr;
	}
}

XMINT2 GameView::MapToScene(int x, int y) const
{
	// Move it into the scene rectangle coordinate space
	auto localX = x - mSceneRect.x;
	auto localY = y - mSceneRect.y;
	
	// Scale it to the coordinate system that was used to render the scene
	localX = floor(localX / mSceneScale);
	localY = floor(localY / mSceneScale);

	return{ (int) localX, (int) localY };
}

XMINT2 GameView::MapFromScene(int x, int y) const{

	auto localX = x * mSceneScale;
	auto localY = y * mSceneScale;
	
	// move it into the scene rectangle coordinate space
	localX += mSceneRect.x+1;
	localY += mSceneRect.y+1;

	return{ (int)localX, (int)localY };
}

XMFLOAT3 GameView::GetScreenCenterInWorld3d()
{
	auto& camera = GetCamera();
	return camera.ScreenToWorld(
		camera.GetScreenWidth() * 0.5f,
		camera.GetScreenHeight() * 0.5f
	);
}

void GameView::Resize(int width, int height)
{
	auto widthFactor = (float) width / (float)mWidth;
	auto heightFactor = (float) height / (float)mHeight;
	mSceneScale = std::min<float>(widthFactor, heightFactor);

	// Calculate the rectangle on the back buffer where the scene will
	// be stretched to
	auto drawWidth = mSceneScale * mWidth;
	auto drawHeight = mSceneScale * mHeight;
	auto drawX = ((float) width - drawWidth) / 2;
	auto drawY = ((float) height - drawHeight) / 2;
	mSceneRect = XMFLOAT4(drawX, drawY, drawWidth, drawHeight);
	
}

GameView *gameView;
