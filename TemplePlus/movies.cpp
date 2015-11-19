
#include "stdafx.h"

#include <platform/d3d.h>
#include <graphics/device.h>
#include <graphics/materials.h>
#include <graphics/shaders.h>
#include <graphics/bufferbinding.h>
#include <infrastructure/images.h>

#include <temple/moviesystem.h>

#include "movies.h"
#include "tig/tig_msg.h"
#include "tig/tig_sound.h"
#include "ui/ui_render.h"

#include "tio/tio_utils.h"
#include "tig/tig_font.h"
#include "tig/tig_startup.h"
#include "temple_functions.h"

using namespace gfx;
using namespace temple;

MovieFuncs movieFuncs;

class MovieMaterial {
public:
	explicit MovieMaterial(RenderingDevice &device);
	void Bind();
private:
	RenderingDevice& mDevice;
	Material mMaterial;

	static Material CreateMaterial(RenderingDevice &device);
};

MovieMaterial::MovieMaterial(RenderingDevice& device)
	: mDevice(device), mMaterial(CreateMaterial(device)) {
}

void MovieMaterial::Bind() {
	mDevice.SetMaterial(mMaterial);

	auto device(mDevice.GetDevice());
	device->SetVertexShaderConstantF(0, &mDevice.GetCamera().GetUiProjection()._11, 4);
}

Material MovieMaterial::CreateMaterial(RenderingDevice& device) {
	BlendState blendState;
	DepthStencilState depthStencilState;
	depthStencilState.depthEnable = false;
	RasterizerState rasterizerState;
	SamplerState samplerState;
	samplerState.magFilter = D3DTEXF_LINEAR;
	samplerState.minFilter = D3DTEXF_LINEAR;
	samplerState.mipFilter = D3DTEXF_LINEAR;
	std::vector<MaterialSamplerBinding> samplers{
		{ nullptr, samplerState }
	};
	auto vs(device.GetShaders().LoadVertexShader("gui_vs"));
	auto ps(device.GetShaders().LoadPixelShader("textured_simple_ps"));
	
	return Material(blendState, depthStencilState, rasterizerState, samplers,
		vs, ps);
}

static bool binkRenderFrame(MovieFile &movie, IDirect3DTexture9* texture) {
	if (movie.WaitForNextFrame()) {
		return true;
	}

	movie.DecodeFrame();

	D3DLOCKED_RECT locked;
	HRESULT result;
	result = D3DLOG(texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD));
	if (result != D3D_OK) {
		logger->error("Unable to lock texture for movie frame!");
		return false;
	}

	movie.CopyFramePixels(
		locked.pBits,
		locked.Pitch,
		movie.GetHeight());

	D3DLOG(texture->UnlockRect(0));

	if (movie.AtEnd()) {
		return false;
	}

	movie.NextFrame();	
	return true;
}

struct MovieVertex {
	XMFLOAT3 pos;
	XMCOLOR color = 0xFFFFFFFF;
	XMFLOAT2 uv;

	MovieVertex(float x, float y, float u, float v)
		: pos{ x, y, 0 }, uv{ u,v } {}
};

struct MovieRect {
	float left;
	float top;
	float right;
	float bottom;
};

static MovieRect GetMovieRect(int movieWidth, int movieHeight) {
	auto &device = tig->GetRenderingDevice();

	auto screenWidth = device.GetScreenWidthF();
	auto screenHeight = device.GetScreenHeightF();

	// Fit movie into rect
	float wFactor = screenWidth / movieWidth;
	float hFactor = screenHeight / movieHeight;
	float scale = min(wFactor, hFactor);
	float movieW = scale * movieWidth;
	float movieH = scale * movieHeight;

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
	SubtitleRenderer(gfx::RenderingDevice &device, const SubtitleLine* firstLine) 
		: mDevice(device), mLine(firstLine) {
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

	bool IsCurrentLineVisible() const {
		if (!mLine) {
			return false;
		}
		return mLine->startMs <= mElapsedTime
			&& mLine->startMs + mLine->durationMs > mElapsedTime;
	}

	void RenderCurrentLine() const {

		UiRenderer::PushFont(mLine->fontname, 0, 0);

		auto extents = UiRenderer::MeasureTextSize(mLine->text, mSubtitleStyle, 700, 150);

		extents.x = (mDevice.GetScreenWidth() - extents.width) / 2;
		extents.y = mDevice.GetScreenHeight() - mDevice.GetScreenHeight() / 10;

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

	RenderingDevice& mDevice;
	ColorRect mSubtitleBgColor = ColorRect(D3DCOLOR_ARGB(153, 17, 17, 17));
	ColorRect mSubtitleTextColor = ColorRect(D3DCOLOR_ARGB(255, 255, 255, 255));
	ColorRect mSubtitleShadowColor = ColorRect(D3DCOLOR_ARGB(255, 0, 0, 0));
	TigTextStyle mSubtitleStyle;
	const SubtitleLine* mLine;
	uint32_t mMovieStarted;
	uint32_t mElapsedTime;
};

int HookedPlayMovieBink(const char* filename, const SubtitleLine* subtitles, int flags, uint32_t soundtrackId) {

	movieFuncs.MovieIsPlaying = true;

	auto movie(tig->GetMovieSystem().OpenMovie(filename, soundtrackId));
	if (!movie) {
		return 13;
	}

	// The disasm apparently goes crazy for the conversion here
	auto binkVolume = (*tigSoundAddresses.movieVolume) / 127.0f;
	movie->SetVolume(binkVolume);

	auto& device(tig->GetRenderingDevice());
	auto d3dDevice = device.GetDevice();

	d3dDevice->ShowCursor(FALSE);
	
	// Create the movie texture we write to
	CComPtr<IDirect3DTexture9> texture;
	if (D3DLOG(d3dDevice->CreateTexture(movie->GetWidth(), movie->GetHeight(), 
		1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr)) != D3D_OK) {
		logger->error("Unable to create texture for bink video");
		return 0;
	}

	// Clear screen with black color and present immediately
	d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0);
	d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

	processTigMessages();

	auto movieRect(GetMovieRect(movie->GetWidth(), movie->GetHeight()));

	// TODO UV should be manipulated for certain vignettes since they have been letterboxed in the bink file!!!

	// Set vertex shader
	std::vector<MovieVertex> vertices {
		{movieRect.left, movieRect.top, 0, 0},
		{movieRect.right, movieRect.top, 1, 0},
		{movieRect.right, movieRect.bottom, 1, 1},
		{movieRect.left, movieRect.bottom, 0, 1}
	};
	auto vertexBuffer(device.CreateVertexBuffer<MovieVertex>(vertices));

	BufferBinding binding;
	binding.AddBuffer(vertexBuffer, 0, sizeof(MovieVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
	binding.Bind();

	MovieMaterial material(device);
	material.Bind();

	SubtitleRenderer subtitleRenderer(device, subtitles);
	
	auto keyPressed = false;
	while (!keyPressed && binkRenderFrame(*movie, texture)) {
		D3DLOG(d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0));
		D3DLOG(d3dDevice->BeginScene());

		D3DLOG(d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));
		subtitleRenderer.Render();
		D3DLOG(d3dDevice->EndScene());
		D3DLOG(d3dDevice->Present(NULL, NULL, NULL, NULL));

		templeFuncs.ProcessSystemEvents();

		TigMsg msg;
		while (!msgFuncs.Process(&msg)) {
			// Flags 1 seems to disable skip via keyboard. Also seems unused.
			if (!(flags & 1) && msg.type == TigMsgType::KEYSTATECHANGE && LOBYTE(msg.arg2) == 1) {
				// TODO Wait for the key to be unpressed again
				keyPressed = true;
				break;
			}
		}
	}
	
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

int __cdecl HookedPlayMovieSlide(const char* imageFile, const char* soundFile, const SubtitleLine* subtitles, int flags, int soundtrackId) {
	logger->info("Play Movie Slide {} {} {} {}", imageFile, soundFile, flags, soundtrackId);

	// Load img into memory using TIO
	unique_ptr<vector<uint8_t>> imgData(TioReadBinaryFile(imageFile));

	if (!imgData) {
		logger->error("Unable to load the image file {}", imageFile);
		return 1; // Can't play because we cant load the file
	}

	auto device = tig->GetRenderingDevice().GetDevice();
	gfx::ImageFileInfo info;
	auto surface(gfx::LoadImageToSurface(device, *imgData.get(), info));
	
	movieFuncs.MovieIsPlaying = true;

	device->ShowCursor(FALSE);

	// Clear screen with black color and present immediately
	device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0);
	device->Present(nullptr, nullptr, nullptr, nullptr);

	SubtitleRenderer subtitleRenderer(tig->GetRenderingDevice(), subtitles);

	TigRect bbRect(0, 0, tig->GetRenderingDevice().GetScreenWidth(), tig->GetRenderingDevice().GetScreenHeight());
	TigRect destRect(0, 0, info.width, info.height);
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
	
	CComPtr<IDirect3DSurface9> backBuffer;
	device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);

	bool keyPressed = false;
	while (!keyPressed && (!stream.IsValid() || stream.IsPlaying() || sw.GetElapsedMs() < 3000)) {
		D3DLOG(device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0));
		
		D3DLOG(device->BeginScene());
		D3DLOG(device->StretchRect(surface, NULL, backBuffer, &fitDestRect, D3DTEXF_LINEAR));
		subtitleRenderer.Render();
		D3DLOG(device->EndScene());
		D3DLOG(device->Present(NULL, NULL, NULL, NULL));

		templeFuncs.ProcessSystemEvents();

		TigMsg msg;
		while (!msgFuncs.Process(&msg)) {
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
