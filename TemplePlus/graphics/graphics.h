#pragma once

#include <string>
#include <chrono>
#include <list>
#include <atlcomcli.h>

class Graphics;
struct locXY;
class MainWindow;

class ResourceListener {
public:
	virtual ~ResourceListener();

	virtual void CreateResources(Graphics&) = 0;
	virtual void FreeResources(Graphics&) = 0;
};

class Graphics {
public:
	explicit Graphics(MainWindow& mainWindow);
	~Graphics();

	bool BeginFrame();
	bool Present();

	void ResetDevice();

	void UpdateScreenSize(int w, int h);

	/*
		The back buffer size of the D3D context. This is not
		necessarily the size at which ToEE is rendered:
	*/
	void UpdateWindowSize(int w, int h);

	/*
		Converts a x, y screen position (Careful this is in render space, relative to the
		scene surface, not the actual window) and returns the tile below that point.
		Returns true if there is an actual tile at the position.
	*/
	bool ScreenToTile(int x, int y, locXY& tileOut);

	void ShakeScreen(float amount, float duration);

	/*
		Take a screenshot with the given size. The image will be stretched to the given
		size.
	*/
	void TakeScaledScreenshot(const std::string& filename, int width, int height);

	int GetMaxActiveLights() const {
		return mCaps.MaxActiveLights;
	}
	void EnableLighting();
	void DisableLighting();

	// Returns the current back buffer surface description
	const D3DSURFACE_DESC& backBufferDesc() {
		return mBackBufferDesc;
	}

	IDirect3DSurface9* backBuffer() {
		return mBackBuffer;
	}

	IDirect3DSurface9* backBufferDepth() {
		return mBackBufferDepth;
	}


	IDirect3DSurface9* sceneSurface() {
		return mSceneSurface;
	}

	IDirect3DSurface9* sceneDepthSurface() {
		return mSceneDepthSurface;
	}

	int windowWidth() const {
		return mWindowWidth;
	}

	int windowHeight() const {
		return mWindowHeight;
	}

	int GetSceneWidth() const {
		return mSceneRect.right - mSceneRect.left;
	}

	int GetSceneHeight() const {
		return mSceneRect.bottom - mSceneRect.top;
	}

	IDirect3DDevice9Ex* device() {
		return mDevice;
	}

	const RECT& sceneRect() const {
		return mSceneRect;
	}

	float sceneScale() const {
		return mSceneScale;
	}

	const D3DCAPS9& GetCaps() const {
		return mCaps;
	}

	size_t GetVideoMemory() const {
		return mVideoMemory;
	}
	
private:
	friend class ResourceListenerRegistration;
	void AddResourceListener(ResourceListener* resourceListener);
	void RemoveResourceListener(ResourceListener* resourceListener);

	void RenderGFade();

	void RefreshSceneRect();

	void InitializeDirect3d();
	void ReadCaps();
	void SetDefaultRenderStates();
	
	// Free resources allocated in video memory
	void FreeResources();

	// Create resources stored in video memory
	void CreateResources();

	D3DPRESENT_PARAMETERS CreatePresentParams();

	std::list<ResourceListener*> mResourcesListeners;

	CComPtr<IDirect3D9Ex> mDirect3d9;
	CComPtr<IDirect3DDevice9Ex> mDevice;
	CComPtr<IDirect3DSurface9> mBackBuffer;
	CComPtr<IDirect3DSurface9> mBackBufferDepth;

	// Render targets used to draw the ToEE scene at a different resolution
	// than the screen resolution
	CComPtr<IDirect3DSurface9> mSceneSurface;
	CComPtr<IDirect3DSurface9> mSceneDepthSurface;

	D3DSURFACE_DESC mBackBufferDesc;

	D3DCAPS9 mCaps;
	size_t mVideoMemory = 0;

	MainWindow& mMainWindow;

	std::unique_ptr<class TextureManager> mTextureManager;
	std::unique_ptr<class MdfMaterialFactory> mMdfMaterialFactory;

	// Vertex Buffers used in ToEE for UI rendering

	int mWindowWidth = 0;
	int mWindowHeight = 0;

	RECT mSceneRect;
	float mSceneScale;

	D3DVECTOR mScreenCorners[4];
	int mFrameDepth = 0;
	
	bool mResourcesCreated = false;

	using Clock = std::chrono::high_resolution_clock;
	std::chrono::time_point<Clock> mLastFrameStart = Clock::now();
};

// RAII class for managing resource listener registrations
class ResourceListenerRegistration {
public:
	explicit ResourceListenerRegistration(Graphics& graphics, ResourceListener* listener)
		: mGraphics(graphics), mListener(listener) {
		mGraphics.AddResourceListener(listener);
	}

	~ResourceListenerRegistration() {
		mGraphics.RemoveResourceListener(mListener);
	}

	ResourceListenerRegistration(const ResourceListenerRegistration&) = delete;
	ResourceListenerRegistration(const ResourceListenerRegistration&&) = delete;
	ResourceListenerRegistration& operator =(const ResourceListenerRegistration&) = delete;
	ResourceListenerRegistration& operator =(const ResourceListenerRegistration&&) = delete;

private:
	Graphics& mGraphics;
	ResourceListener* mListener;
};

extern Graphics* graphics;
