
#include "stdafx.h"

#include <platform/d3d.h>
#include <graphics/device.h>
#include <graphics/materials.h>
#include <graphics/shaders.h>
#include <graphics/bufferbinding.h>
#include <graphics/dynamictexture.h>
#include <graphics/shaperenderer2d.h>
#include <infrastructure/images.h>

#include <temple/moviesystem.h>

#include "movies.h"
#include "messages/messagequeue.h"
#include "tig/tig_sound.h"
#include "ui/ui_render.h"

#include "tio/tio_utils.h"
#include "tig/tig_font.h"
#include "tig/tig_startup.h"
#include "temple_functions.h"

using namespace gfx;
using namespace temple;

MovieFuncs movieFuncs;

static Material CreateMaterial(RenderingDevice& device) {
	DepthStencilSpec depthStencilState;
	depthStencilState.depthEnable = false;
	SamplerSpec samplerState;
	samplerState.magFilter = TextureFilterType::Linear;
	samplerState.minFilter = TextureFilterType::Linear;
	samplerState.mipFilter = TextureFilterType::Linear;
	std::vector<MaterialSamplerSpec> samplers{
		{ nullptr, samplerState }
	};
	auto vs(device.GetShaders().LoadVertexShader("gui_vs"));
	auto ps(device.GetShaders().LoadPixelShader("textured_simple_ps"));
	
	return device.CreateMaterial({}, depthStencilState, {}, samplers, vs, ps);
}

static bool BinkRenderFrame(RenderingDevice &device, MovieFile &movie, gfx::DynamicTexture& texture) {
	if (movie.WaitForNextFrame()) {
		return true;
	}

	movie.DecodeFrame();

	{
		auto locked{ device.Map(texture) };

		movie.CopyFramePixels(
			locked.GetData(),
			locked.GetRowPitch(),
			movie.GetHeight());
	}

	if (movie.AtEnd()) {
		return false;
	}

	movie.NextFrame();	
	return true;
}

struct MovieVertex {
	XMFLOAT4 pos;
	XMCOLOR color = 0xFFFFFFFF;
	XMFLOAT2 uv;

	MovieVertex(float x, float y, float u, float v)
		: pos{ x, y, 0, 1 }, uv{ u,v } {}
};

struct MovieRect {
	float left;
	float top;
	float right;
	float bottom;
};

static MovieRect GetMovieRect(int movieWidth, int movieHeight) {
	auto &device = tig->GetRenderingDevice();

	auto screenWidth = device.GetCurrentCamera().GetScreenWidth();
	auto screenHeight = device.GetCurrentCamera().GetScreenHeight();

	// Fit movie into rect
	float wFactor = screenWidth / movieWidth;
	float hFactor = screenHeight / movieHeight;
	float scale = std::min(wFactor, hFactor);
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
		
		auto& camera = mDevice.GetCurrentCamera();

		extents.x = (int)((camera.GetScreenWidth() - extents.width) / 2);
		extents.y = (int)(camera.GetScreenHeight() - camera.GetScreenHeight() / 10);

		// The text renderer uses the standard UI projection matrix, which we need to extend to full screen size here
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
	ColorRect mSubtitleBgColor = ColorRect(XMCOLOR_ARGB(153, 17, 17, 17));
	ColorRect mSubtitleTextColor = ColorRect(XMCOLOR(1, 1, 1, 1));
	ColorRect mSubtitleShadowColor = ColorRect(XMCOLOR(0, 0, 0, 1));
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

	XMCOLOR clearColor(0, 0, 0, 0);

	// The disasm apparently goes crazy for the conversion here
	auto binkVolume = (*tigSoundAddresses.movieVolume) / 127.0f;
	movie->SetVolume(binkVolume);

	auto& device(tig->GetRenderingDevice());

	// Do a one-time empty present to clear the screen quickly
	device.ClearCurrentColorTarget(clearColor);
	device.PresentForce();

	device.HideCursor();

	// Create the movie texture we write to
	auto texture = device.CreateDynamicTexture(BufferFormat::X8R8G8B8, movie->GetWidth(), movie->GetHeight());
	
	messageQueue->ProcessMessages();

	auto movieRect(GetMovieRect(movie->GetWidth(), movie->GetHeight()));

	// TODO UV should be manipulated for certain vignettes since they have been letterboxed in the bink file!!!

	// Set vertex shader
	eastl::fixed_vector<MovieVertex, 4> vertices {
		{movieRect.left, movieRect.top, 0, 0},
		{movieRect.right, movieRect.top, 1, 0},
		{movieRect.right, movieRect.bottom, 1, 1},
		{movieRect.left, movieRect.bottom, 0, 1}
	};
	auto vertexBuffer(device.CreateVertexBuffer<MovieVertex>(vertices));

	std::vector<uint16_t> indices{
		0, 1, 2,
		0, 2, 3
	};
	auto indexBuffer(device.CreateIndexBuffer(indices));

	SubtitleRenderer subtitleRenderer(device, subtitles);

	Material material = CreateMaterial(device);
	BufferBinding binding(material.GetVertexShader());
	binding.AddBuffer(vertexBuffer, 0, sizeof(MovieVertex))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
	
	auto keyPressed = false;
	while (!keyPressed && BinkRenderFrame(device, *movie, *texture)) {
		device.ClearCurrentColorTarget(clearColor);

		device.SetMaterial(material);
		device.SetVertexShaderConstant(0, gfx::StandardSlotSemantic::UiProjMatrix);
		binding.Bind();

		device.SetIndexBuffer(*indexBuffer);

		device.SetTexture(0, *texture);
		device.DrawIndexed(gfx::PrimitiveType::TriangleList, 4, 6);

		subtitleRenderer.Render();
		
		device.PresentForce();

		messageQueue->PollExternalEvents();

		TigMsg msg;
		while (messageQueue->Process(msg)) {
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
	device.ShowCursor();

	return 0;
}

int __cdecl HookedPlayMovieSlide(const char* imageFile, const char* soundFile, const SubtitleLine* subtitles, int flags, int soundtrackId) {
	logger->info("Play Movie Slide {} {} {} {}", imageFile, soundFile, flags, soundtrackId);

	auto &device = tig->GetRenderingDevice();

	auto texture(device.GetTextures().ResolveUncached(imageFile, false));
	
	movieFuncs.MovieIsPlaying = true;

	device.HideCursor();

	// We have to render directly to the screen
	device.PushBackBufferRenderTarget();

	XMCOLOR clearColor(0, 0, 0, 0);

	// Clear screen with black color and present immediately
	device.ClearCurrentColorTarget(clearColor);
	device.PresentForce();

	SubtitleRenderer subtitleRenderer(device, subtitles);

	auto &textureSize = texture->GetSize();

	auto &camera = device.GetCurrentCamera();
	TigRect bbRect(0, 0, (int)camera.GetScreenWidth(), (int)camera.GetScreenHeight());
	TigRect destRect(0, 0, textureSize.width, textureSize.height);
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

	auto &shapeRenderer2d = tig->GetShapeRenderer2d();
	
	
	bool keyPressed = false;
	while (!keyPressed && (!stream.IsValid() || stream.IsPlaying() || sw.GetElapsedMs() < 3000)) {
		
		device.ClearCurrentColorTarget(clearColor);
		
		shapeRenderer2d.DrawRectangle(
			(float) destRect.x,
			(float) destRect.y,
			(float) destRect.width,
			(float) destRect.height,
			*texture
		);
		
		subtitleRenderer.Render();

		device.PresentForce();

		messageQueue->PollExternalEvents();

		TigMsg msg;
		while (messageQueue->Process(msg)) {
			// Flags 1 seems to disable skip via keyboard. Also seems unused.
			if (!(flags & 1) && msg.type == TigMsgType::KEYSTATECHANGE && LOBYTE(msg.arg2) == 1) {
				// TODO Wait for the key to be unpressed again
				keyPressed = true;
				break;
			}
		}
	}

	movieFuncs.MovieIsPlaying = false;
	device.ShowCursor();

	device.PopRenderTarget();

	return 0;
}

void hook_movies() {
	// MH_CreateHook(movieFuncs.PlayLegalMovies, HookedPlayLegalMovies, reinterpret_cast<LPVOID*>(&movieFuncs.PlayLegalMovies));
	// MH_CreateHook(movieFuncs.PlayMovie, HookedPlayMovie, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovie));
	MH_CreateHook(movieFuncs.PlayMovieBink, HookedPlayMovieBink, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovieBink));
	MH_CreateHook(movieFuncs.PlayMovieSlide, HookedPlayMovieSlide, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovieSlide));
}
