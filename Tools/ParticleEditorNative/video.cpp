
#include <particles/instances.h>
#include <particles/render.h>
#include <graphics/device.h>
#include <aas/aas_renderer.h>
#include <atlcomcli.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi")

using namespace particles;

#include "api.h"
#include "video.h"

static float GetTotalLifetime(const PartSysPtr& sys, bool& permanent) {
	auto result = 0.0f;

	for (const auto& emitter : *sys) {
		auto spec = emitter->GetSpec();

		auto lifetime = spec->GetDelay();

		if (emitter->GetSpec()->IsPermanent()) {
			auto maxParticlesReachedIn = spec->GetMaxParticles() / (float)spec->GetParticleRate();
			lifetime += maxParticlesReachedIn + spec->GetParticleLifespan();
			permanent = true;
		} else {
			lifetime += spec->GetLifespan() + spec->GetParticleLifespan();
		}

		if (lifetime > result) {
			result = lifetime;
		}
	}

	return result;
}

bool ParticleSystem_RenderVideo(TempleDll *dll, XMCOLOR background, const wchar_t* outputFile, int fps) {

	auto orgSys = dll->partSys.get();

	// The assumption is that the screen BB of the part sys encompasses the entire system
	// so we use that to render it to a video file
	auto screenBounds = orgSys->GetScreenBounds();

	auto w = (int)abs(screenBounds.right - screenBounds.left) + 10;
	auto h = (int)abs(screenBounds.bottom - screenBounds.top) + 10;
	// Needs to be divisible by 2 for h264
	if (w % 2)
		w++;
	if (h % 2)
		h++;

	const auto scale = 1.0f;
	w *= 1;
	h *= 1;

	auto encoder(VideoEncoder::Create(outputFile));
	encoder->Init(w, h, fps);

	// Create a clone here to not influence the original system
	auto sys = std::make_shared<PartSys>(orgSys->GetSpec());
	
	// Create a render target
	auto renderTarget = dll->renderingDevice.CreateRenderTargetTexture(gfx::BufferFormat::A8R8G8B8, w, h);
	auto depthTarget = dll->renderingDevice.CreateRenderTargetDepthStencil(w, h);

	auto stagingTexture = dll->renderingDevice.CreateDynamicStagingTexture(gfx::BufferFormat::A8R8G8B8, w, h);

	auto timeStepSec = 1.0f / fps;
	auto elapsed = 0.0f;
	bool permanent;
	auto totalTime = GetTotalLifetime(sys, permanent);

	if (permanent) {
		sys->Simulate(5.0f);
	}
	
	dll->renderingDevice.PushRenderTarget(renderTarget, depthTarget);

	dll->renderingDevice.GetCamera().CenterOn(0, 0, 0);

	int frameId = 0;

	while (elapsed < totalTime) {
		sys->Simulate(timeStepSec);
		elapsed += timeStepSec;

		dll->renderingDevice.ClearCurrentColorTarget(XMCOLOR(0, 0, 0, 1));
		dll->renderingDevice.ClearCurrentDepthTarget();

		for (auto& emitter : *sys) {
			auto& renderer = dll->renderManager.GetRenderer(emitter->GetSpec()->GetParticleType());
			renderer.Render(*emitter);
		}
		
		dll->renderingDevice.Flush();

		dll->renderingDevice.CopyRenderTarget(*renderTarget, *stagingTexture);
		
#ifdef WRITE_FRAME_BMPS
		wchar_t frameFile[MAX_PATH];
		wcsncpy(frameFile, outputFile, wcslen(outputFile));
		PathRemoveFileSpec(frameFile);
		PathAppend(frameFile, fmt::format(L"output{:04}.bmp", frameId++).c_str());

		result = D3DXSaveSurfaceToFile(frameFile, D3DXIFF_BMP, sysMemSurface, nullptr, nullptr);
#endif
		
		auto locked = dll->renderingDevice.Map(*stagingTexture, gfx::MapMode::Read);
		encoder->WriteFrame((uint8_t*) locked.GetData(), locked.GetRowPitch());
	}

	encoder->Finish();

	dll->renderingDevice.PopRenderTarget();

	return true;

}
