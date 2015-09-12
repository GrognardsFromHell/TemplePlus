#include "stdafx.h"

#include "movies.h"
#include "graphics.h"
#include "tig/tig_msg.h"
#include "tig/tig_sound.h"
#include "ui/ui_render.h"
#include "renderstates.h"
#include "tio/tio_utils.h"
#include "atlbase.h"

#include <d3dx9.h>
#include <d3dx9tex.h>

MovieFuncs movieFuncs;

/*
	This is an incomplete description of the bink structure that is returned by
	BinkOpen. It represents what is currently loaded for a given movie.
*/
struct BinkMovie {
	uint32_t width;
	uint32_t height;
	uint32_t frameCount;
	uint32_t currentFrame;
};

inline void* getBinkProc(HMODULE module, const char* funcName) {
	void* result = GetProcAddress(module, funcName);
	if (!result) {
		logger->error("Could not find function {} in binkw32.dll", funcName);
	}
	return result;
}

struct BinkFuncs {
	void* BinkOpenMiles; // Only passed to BinkSetSoundSystem
	typedef int (__stdcall *FNBinkSetSoundSystem)(void*, void*);
	typedef void (__stdcall *FNBinkSetSoundTrack)(uint32_t trackCount, uint32_t* trackIds);
	typedef BinkMovie*(__stdcall *FNBinkOpen)(const char* name, uint32_t flags);
	typedef void (__stdcall *FNBinkSetVolume)(BinkMovie* movie, uint32_t trackId, int volume);
	typedef void (__stdcall *FNBinkNextFrame)(BinkMovie* movie);
	typedef int (__stdcall *FNBinkWait)(BinkMovie* movie);
	typedef int (__stdcall *FNBinkDoFrame)(BinkMovie* movie);
	typedef int (__stdcall *FNBinkCopyToBuffer)(BinkMovie* movie, void* dest, int destpitch, int destheight, int destx, int desty, int flags);
	typedef void (__stdcall *FNBinkClose)(BinkMovie* movie);

	FNBinkSetSoundSystem BinkSetSoundSystem;
	FNBinkSetSoundTrack BinkSetSoundTrack;
	FNBinkOpen BinkOpen;
	FNBinkSetVolume BinkSetVolume;
	FNBinkNextFrame BinkNextFrame;
	FNBinkWait BinkWait;
	FNBinkDoFrame BinkDoFrame;
	FNBinkCopyToBuffer BinkCopyToBuffer;
	FNBinkClose BinkClose;

	void initialize() {
		static bool initialized = false;
		if (initialized) {
			return;
		}

		initialized = true;

		auto module = GetModuleHandleA("binkw32");

		if (!module) {
			logger->error("Unable to find binkw32.dll in memory!");
			abort();
		}

		// Not used or set by us, but rather by tig_movie_init
		BinkOpenMiles = getBinkProc(module, "_BinkOpenMiles@4");
		BinkSetSoundSystem = reinterpret_cast<FNBinkSetSoundSystem>(getBinkProc(module, "_BinkSetSoundSystem@8"));

		BinkSetSoundTrack = reinterpret_cast<FNBinkSetSoundTrack>(getBinkProc(module, "_BinkSetSoundTrack@8"));
		BinkOpen = reinterpret_cast<FNBinkOpen>(getBinkProc(module, "_BinkOpen@8"));
		BinkSetVolume = reinterpret_cast<FNBinkSetVolume>(getBinkProc(module, "_BinkSetVolume@12"));
		BinkNextFrame = reinterpret_cast<FNBinkNextFrame>(getBinkProc(module, "_BinkNextFrame@4"));
		BinkWait = reinterpret_cast<FNBinkWait>(getBinkProc(module, "_BinkWait@4"));
		BinkDoFrame = reinterpret_cast<FNBinkDoFrame>(getBinkProc(module, "_BinkDoFrame@4"));
		BinkCopyToBuffer = reinterpret_cast<FNBinkCopyToBuffer>(getBinkProc(module, "_BinkCopyToBuffer@28"));
		BinkClose = reinterpret_cast<FNBinkClose>(getBinkProc(module, "_BinkClose@4"));

	}

} binkFuncs;

// indicates that the bink sound system has been set
GlobalBool<0x103010FC> binkInitialized;

static bool binkRenderFrame(BinkMovie* movie, IDirect3DTexture9* texture) {
	if (!binkFuncs.BinkWait(movie)) {
		binkFuncs.BinkDoFrame(movie);

		D3DLOCKED_RECT locked;
		HRESULT result;
		result = texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD);
		if (result != D3D_OK) {
			logger->error("Unable to lock texture for movie frame!");
			handleD3dError("LockRect", result);
			return false;
		}

		binkFuncs.BinkCopyToBuffer(
			movie,
			locked.pBits,
			locked.Pitch,
			movie->height,
			0,
			0,
			0xF0000000 | 3);

		handleD3dError("UnlockRect", texture->UnlockRect(0));

		if (movie->currentFrame >= movie->frameCount)
			return false;

		binkFuncs.BinkNextFrame(movie);
	}
	return true;
}

struct MovieVertex {
	float x;
	float y;
	float z = 0;
	float rhw = 1;
	D3DCOLOR color = 0xFFFFFFFF;
	float u;
	float v;

	MovieVertex(float x, float y, float u, float v)
		: x(x - 0.5f),
		  y(y - 0.5f),
		  u(u),
		  v(v) {
	}
};

struct MovieRect {
	float left;
	float top;
	float right;
	float bottom;
};

static MovieRect getMovieRect(BinkMovie *movie) {
	auto screenWidth = graphics.backBufferDesc().Width;
	auto screenHeight = graphics.backBufferDesc().Height;

	// Fit movie into rect
	float w = static_cast<float>(screenWidth);
	float h = static_cast<float>(screenHeight);
	float wFactor = w / movie->width;
	float hFactor = h / movie->height;
	float scale = min(wFactor, hFactor);
	float movieW = scale * movie->width;
	float movieH = scale * movie->height;

	// Center on screen
	MovieRect result;
	result.left = (screenWidth - movieW) / 2;
	result.top = (screenHeight - movieH) / 2;
	result.right = result.left + movieW;
	result.bottom = result.top + movieH;

	return result;

}

class SubtitleRenderer {
public:
	SubtitleRenderer(const SubtitleLine *firstLine) : mLine(firstLine) {
		mMovieStarted = timeGetTime();
		InitializeSubtitleStyle();
	}
	
	void Render() {
		if (!mLine) {
			return;
		}

		// Update elapsed time
		mElapsedTime = timeGetTime() - mMovieStarted;

		// Update current line
		AdvanceLine();

		// Should we display the current line?
		if (IsCurrentLineVisible()) {
			RenderCurrentLine();
		}
	}

private:
	void AdvanceLine() {
		// Go to the next appropriate line
		auto nextLine = mLine->nextLine;
		while (nextLine && nextLine->startMs < mElapsedTime) {
			mLine = nextLine;
			nextLine = mLine->nextLine;
		}
	}

	bool IsCurrentLineVisible() {
		if (!mLine) {
			return false;
		}
		return mLine->startMs <= mElapsedTime
			&& mLine->startMs + mLine->durationMs > mElapsedTime;
	}

	void RenderCurrentLine() {
		
		UiRenderer::PushFont(mLine->fontname, 0, 0);

		auto extents = UiRenderer::MeasureTextSize(mLine->text, mSubtitleStyle, 700, 150);

		extents.x = (graphics.windowWidth() - extents.width) / 2;
		extents.y = graphics.windowHeight() - graphics.windowHeight() / 10;

		UiRenderer::RenderText(mLine->text, extents, mSubtitleStyle);

		UiRenderer::PopFont();

	}

	void InitializeSubtitleStyle() {
		mSubtitleStyle.flags = TTSF_DROP_SHADOW | TTSF_CENTER;
		mSubtitleStyle.bgColor = &mSubtitleBgColor;
		mSubtitleStyle.field2c = -1;
		mSubtitleStyle.shadowColor = &mSubtitleShadowColor;
		mSubtitleStyle.textColor = &mSubtitleTextColor;
		mSubtitleStyle.kerning = 1;
		mSubtitleStyle.tracking = 4;
	}

	ColorRect mSubtitleBgColor = ColorRect(D3DCOLOR_ARGB(153, 17, 17, 17));
	ColorRect mSubtitleTextColor = ColorRect(D3DCOLOR_ARGB(255, 255, 255, 255));
	ColorRect mSubtitleShadowColor = ColorRect(D3DCOLOR_ARGB(255, 0, 0, 0));
	TigTextStyle mSubtitleStyle;
	const SubtitleLine *mLine;
	uint32_t mMovieStarted;
	uint32_t mElapsedTime;
};

int HookedPlayMovieBink(const char* filename, const SubtitleLine* subtitles, int flags, uint32_t soundtrackId) {

	if (!binkInitialized) {
		return 0;
	}

	movieFuncs.MovieIsPlaying = true;

	binkFuncs.initialize();

	uint32_t openFlags = 0;
	if (soundtrackId) {
		binkFuncs.BinkSetSoundTrack(1, &soundtrackId);
		openFlags |= 0x4000; // Apparently tells BinkOpen a soundtrack was used
	}

	auto movie = binkFuncs.BinkOpen(filename, openFlags);
	if (!movie) {
		logger->error("Unable to load BINK movie {} with flags {}", filename, openFlags);
		return 13;
	}

	// The disasm apparently goes crazy for the conversion here
	int binkVolume = (*tigSoundAddresses.movieVolume) * 258;
	binkFuncs.BinkSetVolume(movie, 0, binkVolume);

	auto d3dDevice = graphics.device();

	d3dDevice->ShowCursor(FALSE);

	// Create the movie texture we write to
	IDirect3DTexture9* texture;
	if (D3DLOG(d3dDevice->CreateTexture(movie->width, movie->height, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr)) != D3D_OK) {
		logger->error("Unable to create texture for bink video");
		return 0;
	}

	// Clear screen with black color and present immediately
	d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 0, 0);
	d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

	processTigMessages();

	MovieRect movieRect = getMovieRect(movie);
	
	// TODO UV should be manipulated for certain vignettes since they have been letterboxed in the bink file!!!

	// Set vertex shader
	MovieVertex vertices[4] = {
		{ movieRect.left, movieRect.top, 0, 0 },
		{ movieRect.right, movieRect.top, 1, 0 },
		{ movieRect.right, movieRect.bottom, 1, 1 },
		{ movieRect.left, movieRect.bottom, 0, 1 }
	};

	IDirect3DVertexBuffer9* vertexBuffer;
	handleD3dError("CreateVertexBuffer", d3dDevice->CreateVertexBuffer(sizeof(vertices), 0,
		D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &vertexBuffer, nullptr));
	void* data;
	handleD3dError("Lock", vertexBuffer->Lock(0, 0, &data, 0));
	memcpy(data, vertices, sizeof(vertices));
	vertexBuffer->Unlock();
	
	SubtitleRenderer subtitleRenderer(subtitles);
	
	bool keyPressed = false;
	while (!keyPressed && binkRenderFrame(movie, texture)) {
		handleD3dError("Clear", d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0));
		handleD3dError("BeginScene", d3dDevice->BeginScene());

		renderStates->SetTexture(0, texture);
		renderStates->SetTextureMinFilter(0, D3DTEXF_LINEAR);
		renderStates->SetTextureMagFilter(0, D3DTEXF_LINEAR);
		renderStates->SetTextureMipFilter(0, D3DTEXF_LINEAR);
		renderStates->SetLighting(false);
		renderStates->SetZEnable(false);
		renderStates->SetCullMode(D3DCULL_NONE);
		renderStates->SetTextureTransformFlags(0, D3DTTFF_DISABLE);
		renderStates->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		renderStates->SetStreamSource(0, vertexBuffer, sizeof(MovieVertex));
		renderStates->SetTextureColorOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureColorArg1(0, D3DTA_TEXTURE);
		renderStates->SetTextureAlphaOp(0, D3DTOP_SELECTARG1);
		renderStates->SetTextureAlphaArg1(0, D3DTA_TEXTURE);
		renderStates->Commit();

		handleD3dError("DrawPrimitive", d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));
		subtitleRenderer.Render();
		handleD3dError("EndScene", d3dDevice->EndScene());
		handleD3dError("Present", d3dDevice->Present(NULL, NULL, NULL, NULL));

		templeFuncs.ProcessSystemEvents();

		TigMsg msg;		
		while (!msgFuncs.Process(&msg))
		{			
			// Flags 1 seems to disable skip via keyboard. Also seems unused.
			if (!(flags & 1) && msg.type == TigMsgType::KEYSTATECHANGE && LOBYTE(msg.arg2) == 1) {
				// TODO Wait for the key to be unpressed again
				keyPressed = true;
				break;
			}
		}
	}

	binkFuncs.BinkClose(movie);
	texture->Release();
	vertexBuffer->Release();

	// Unclear what this did (black out screen after last frame?)
	/*
		Haven't found a single occasion where it's used.
		if (!(flags & 8))
		{
			sub_101D8790(0);
			play_movie_present();
		}
	*/
	movieFuncs.MovieIsPlaying = false;
	d3dDevice->ShowCursor(TRUE);

	return 0;
}

int __cdecl HookedPlayMovieSlide(const char *imageFile, const char *soundFile, const SubtitleLine *subtitles, int flags, int soundtrackId) {
	logger->info("Play Movie Slide {} {} {} {}", imageFile, soundFile, flags, soundtrackId);

	// Load img into memory using TIO
	unique_ptr<vector<char>> imgData(TioReadBinaryFile(imageFile));

	if (!imgData) {
		logger->error("Unable to load the image file {}", imageFile);
		return 1; // Can't play because we cant load the file
	}

	D3DXIMAGE_INFO imgInfo;
	if (!SUCCEEDED(D3DXGetImageInfoFromFileInMemory(imgData->data(), imgData->size(), &imgInfo))) {		
		logger->error("Unable to determine image format of {}", imageFile);
		return 1;
	}

	auto device = graphics.device();
	CComPtr<IDirect3DSurface9> imgSurface;
	if (!SUCCEEDED(D3DLOG(device->CreateOffscreenPlainSurface(imgInfo.Width, imgInfo.Height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &imgSurface, nullptr)))) {
		logger->error("Unable to create offscreen surface for slide image.");
		return 1;
	}

	if (!SUCCEEDED(D3DXLoadSurfaceFromFileInMemory(imgSurface, NULL, NULL, imgData->data(), imgData->size(), NULL, D3DX_DEFAULT, 0, &imgInfo))) {
		logger->error("Unable to load slide image into a surface.");
		return 1;
	}
		
	movieFuncs.MovieIsPlaying = true;

	device->ShowCursor(FALSE);
	
	// Clear screen with black color and present immediately
	device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0);
	device->Present(nullptr, nullptr, nullptr, nullptr);
	
	SubtitleRenderer subtitleRenderer(subtitles);

	TigRect bbRect(0, 0, graphics.backBufferDesc().Width, graphics.backBufferDesc().Height);
	TigRect destRect(0, 0, imgInfo.Width, imgInfo.Height);
	destRect.FitInto(bbRect);
	RECT fitDestRect = destRect.ToRect();

	Stopwatch sw;

	TigSoundStreamWrapper stream;

	if (soundFile) {
		if (!stream.Play(soundFile, TigSoundType::Voice)) {
			logger->error("Unable to play sound {} during slideshow.", soundFile);
		} else {
			stream.SetVolume(*tigSoundAddresses.movieVolume);
		}
	}

	bool keyPressed = false;
	while (!keyPressed && (!stream.IsValid() || stream.IsPlaying() || sw.GetElapsedMs() < 3000)) {
		D3DLOG(device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0));
				
		D3DLOG(device->BeginScene());
		D3DLOG(device->StretchRect(imgSurface, NULL, graphics.backBuffer(), &fitDestRect, D3DTEXF_LINEAR));
		subtitleRenderer.Render();
		D3DLOG(device->EndScene());
		D3DLOG(device->Present(NULL, NULL, NULL, NULL));

		templeFuncs.ProcessSystemEvents();

		TigMsg msg;
		while (!msgFuncs.Process(&msg))
		{
			// Flags 1 seems to disable skip via keyboard. Also seems unused.
			if (!(flags & 1) && msg.type == TigMsgType::KEYSTATECHANGE && LOBYTE(msg.arg2) == 1) {
				// TODO Wait for the key to be unpressed again
				keyPressed = true;
				break;
			}
		}
	}

	movieFuncs.MovieIsPlaying = false;
	device->ShowCursor(TRUE);

	return 0;
}

void hook_movies() {
	// MH_CreateHook(movieFuncs.PlayLegalMovies, HookedPlayLegalMovies, reinterpret_cast<LPVOID*>(&movieFuncs.PlayLegalMovies));
	// MH_CreateHook(movieFuncs.PlayMovie, HookedPlayMovie, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovie));
	MH_CreateHook(movieFuncs.PlayMovieBink, HookedPlayMovieBink, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovieBink));
	MH_CreateHook(movieFuncs.PlayMovieSlide, HookedPlayMovieSlide, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovieSlide));
}
