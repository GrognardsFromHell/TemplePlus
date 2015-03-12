#include "stdafx.h"

#include "movies.h"
#include "graphics.h"
#include "tig_msg.h"

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
		LOG(error) << "Could not find function " << funcName << " in binkw32.dll";
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
			LOG(error) << "Unable to find binkw32.dll in memory!";
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
			LOG(error) << "Unable to lock texture for movie frame!";
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
	// Fit movie into rect
	float w = static_cast<float>(video->current_width);
	float h = static_cast<float>(video->current_height);
	float wFactor = w / movie->width;
	float hFactor = h / movie->height;
	float scale = min(wFactor, hFactor);
	float movieW = scale * movie->width;
	float movieH = scale * movie->height;

	// Center on screen
	MovieRect result;
	result.left = (video->current_width - movieW) / 2;
	result.top = (video->current_height - movieH) / 2;
	result.right = result.left + movieW;
	result.bottom = result.top + movieH;

	return result;

}

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
		LOG(error) << "Unable to load BINK movie " << filename << " with flags " << openFlags;
		return 13;
	}

	// The disasm apparently goes crazy for the conversion here
	int binkVolume = movieFuncs.MovieVolume * 258;
	binkFuncs.BinkSetVolume(movie, 0, binkVolume);

	auto d3dDevice = video->d3dDevice->delegate;

	// Create the movie texture we write to
	IDirect3DTexture9* texture;
	if (handleD3dError("CreateTexture", d3dDevice->CreateTexture(movie->width, movie->height, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr))) {
		LOG(error) << "Unable to create texture for bink video";
		return 0;
	}
	d3dDevice->SetTexture(0, texture);
	d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// Clear screen with black color and present immediately
	d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 0, 0);
	d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

	d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3dDevice->SetRenderState(D3DRS_CULLMODE, FALSE);

	processTigMessages();

	MovieRect movieRect = getMovieRect(movie);
	
	// TODO UV should be manipulated for certain vignettes since they have been letterboxed in the bink file!!!

	// Set vertex shader
	d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
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
	d3dDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(MovieVertex));
	
	bool keyPressed = false;
	while (!keyPressed && binkRenderFrame(movie, texture)) {
		handleD3dError("Clear", d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 0, 0));
		handleD3dError("BeginScene", d3dDevice->BeginScene());
		handleD3dError("DrawPrimitive", d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2));
		handleD3dError("EndScene", d3dDevice->EndScene());
		handleD3dError("Present", d3dDevice->Present(NULL, NULL, NULL, NULL));

		templeFuncs.ProcessSystemEvents();

		TigMsg msg;		
		while (!tigMsgFuncs.Process(&msg))
		{			
			// Flags 1 seems to disable skip via keyboard. Also seems unused.
			if (!(flags & 1) && msg.type == TMT_KEYSTATECHANGE && LOBYTE(msg.arg2) == 1) {
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

	return 0;
}

void HookedPlayLegalMovies() {
	if (!config.skipLegal) {
		movieFuncs.PlayMovie("movies\\AtariLogo.bik", 0, 0, 0);
		movieFuncs.PlayMovie("movies\\TroikaLogo.bik", 0, 0, 0);
		movieFuncs.PlayMovie("movies\\WotCLogo.bik", 0, 0, 0);
	}
}

void __cdecl HookedPlayMovie(char* filename, int a1, int a2, int a3) {
	// We skip the intro cinematic exactly once. So it can still be played
	// via the cinematics menu
	if (config.skipIntro && !strcmp(filename, "movies\\introcinematic.bik")) {
		static auto skippedIntro = false;
		if (!skippedIntro) {
			LOG(info) << "Skipping intro cinematic.";
			skippedIntro = true;
			return;
		}
	}

	movieFuncs.PlayMovie(filename, a1, a2, a3);
}

void hook_movies() {
	MH_CreateHook(movieFuncs.PlayLegalMovies, HookedPlayLegalMovies, reinterpret_cast<LPVOID*>(&movieFuncs.PlayLegalMovies));
	MH_CreateHook(movieFuncs.PlayMovie, HookedPlayMovie, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovie));
	MH_CreateHook(movieFuncs.PlayMovieBink, HookedPlayMovieBink, reinterpret_cast<LPVOID*>(&movieFuncs.PlayMovieBink));

}
