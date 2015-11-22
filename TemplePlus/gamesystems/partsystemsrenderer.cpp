
#include "stdafx.h"
#include "partsystemsrenderer.h"
#include "partsystems.h"

#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include <graphics/math.h>

#include <particles/render.h>
#include <particles/instances.h>

#include "ui/ui_render.h"

#include "util/config.h"

using namespace gfx;
using namespace particles;

ParticleSystemsRenderer::ParticleSystemsRenderer(
	RenderingDevice &renderingDevice,
	ShapeRenderer2d &shapeRenderer2d,
	AnimatedModelFactory &modelFactory,
	AnimatedModelRenderer &modelRenderer,
	ParticleSysSystem &particleSysSystem
	) : mRenderingDevice(renderingDevice), 
		mShapeRenderer2d(shapeRenderer2d),		
		mParticleSysSystem(particleSysSystem) {

	mRendererManager = std::make_unique<ParticleRendererManager>(
		renderingDevice,
		modelFactory,
		modelRenderer
	);

}

void ParticleSystemsRenderer::Render()
{

	auto& camera = mRenderingDevice.GetCamera();

	size_t total = 0;
	size_t rendered = 0;

	for (auto &entry : mParticleSysSystem) {

		auto& partSys = *entry.second;
		total++;

		auto screenBounds(partSys.GetScreenBounds());

		if (!camera.IsBoxOnScreen(partSys.GetScreenPosAbs(),
			screenBounds.left, screenBounds.top,
			screenBounds.right, screenBounds.bottom)) {
			continue;
		}

		rendered++;

		// each emitter is rendered individually
		for (auto &emitter : partSys) {
			auto type = emitter->GetSpec()->GetParticleType();
			auto& renderer = mRendererManager->GetRenderer(type);
			renderer.Render(*emitter);
		}

		if (config.debugPartSys) {
			RenderDebugInfo(partSys);
		}

	}

}

void ParticleSystemsRenderer::RenderDebugInfo(const particles::PartSys &sys)
{
	auto& camera = mRenderingDevice.GetCamera();

	auto dx = camera.Get2dTranslation().x;
	auto dy = camera.Get2dTranslation().y;

	auto screenX = dx - sys.GetScreenPosAbs().x;
	auto screenY = dy - sys.GetScreenPosAbs().y;
	auto screenBounds(sys.GetScreenBounds());

	screenX *= camera.GetScale();
	screenY *= camera.GetScale();

	screenX += camera.GetScreenWidth() * 0.5f;
	screenY += camera.GetScreenHeight() * 0.5f;

	float left = { screenX + screenBounds.left };
	float top = { screenY + screenBounds.top };
	float right = { screenX + screenBounds.right };
	float bottom = { screenY + screenBounds.bottom };

	XMCOLOR color(0, 0, 1, 1);

	std::array<Line2d, 4> lines{ {
		{ { left, top },{ right, top }, color },
		{ { right, top },{ right, bottom }, color },
		{ { right, bottom },{ left, bottom }, color },
		{ { left, bottom },{ left, top }, color },
		} };
	mShapeRenderer2d.DrawLines(lines);

	UiRenderer::PushFont(PredefinedFont::ARIAL_10);

	ColorRect textColor(0xFFFFFFFF);
	TigTextStyle style;
	style.field10 = 25;
	style.textColor = &textColor;

	auto text = fmt::format("{}", sys.GetSpec()->GetName());
	auto rect = UiRenderer::MeasureTextSize(text, style);

	rect.x = (int)left;
	rect.y = (int)top;
	UiRenderer::RenderText(text, rect, style);

	UiRenderer::PopFont();
}
