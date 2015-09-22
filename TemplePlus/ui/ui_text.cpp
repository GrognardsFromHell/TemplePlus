#include "stdafx.h"

#include "ui_text.h"
#include "graphics.h"
/*
#include "ui/ui_render_dx.h"

#include <Rocket/Core.h>
#include <Rocket/Debugger.h>

class TempleSystemInterface : public Rocket::Core::SystemInterface {
public:
	float GetElapsedTime() override {
		return timeGetTime() / 1000.0f;
	}

	bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message) override {
		switch (type) {
		case Rocket::Core::Log::LT_ALWAYS:
			logger->info("libRocket: {}", message.CString());
			break;
		case Rocket::Core::Log::LT_ERROR:
			logger->error("libRocket: {}", message.CString());
			break;
		case Rocket::Core::Log::LT_ASSERT:
			logger->alert("libRocket: {}", message.CString());
			break;
		case Rocket::Core::Log::LT_WARNING:
			logger->warn("libRocket: {}", message.CString());
			break;
		case Rocket::Core::Log::LT_INFO:
			logger->info("libRocket: {}", message.CString());
			break;
		case Rocket::Core::Log::LT_DEBUG:
			logger->debug("libRocket: {}", message.CString());
			break;
		default: break;
		}
		return true;
	}
};
*/
struct UiTextPrivate {
	/*
	Rocket::Core::Context *context = nullptr;
	RenderInterfaceDirectX renderer;
	TempleSystemInterface systemInterface;
	*/
};

UiText uiText;

void UiText::Initialize() {
	/*
	assert(!d);

	d = new UiTextPrivate;
	d->renderer.SetDevice(graphics.device());
	
	SetRenderInterface(&d->renderer);
	SetSystemInterface(&d->systemInterface);

	Rocket::Core::Initialise();
		
	d->context = CreateContext("main", Rocket::Core::Vector2i(graphics.windowWidth(), graphics.windowHeight()));
	if (!d->context) {
		Rocket::Core::Shutdown();
		throw TempleException("Unable to initialize libRocket");
	}

	Rocket::Core::FontDatabase::LoadFontFace("Alegreya-Regular.otf");

	auto document = d->context->LoadDocument("test.rml");
	if (document)
		document->Show();
		*/
}

void UiText::Uninitialize() {
}

void UiText::Update() {
	//d->context->Update();
}

void UiText::Render() {
/*
	auto device = graphics.device();

	D3DXMATRIX oldProj, oldView;
	device->GetTransform(D3DTS_PROJECTION, &oldProj);
	device->GetTransform(D3DTS_VIEW, &oldView);

	D3DXMATRIX projection;
	D3DXMatrixOrthoOffCenterLH(&projection, 0, (float)graphics.windowWidth(), (float)graphics.windowHeight(), 0, -1, 1);

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	device->SetTransform(D3DTS_PROJECTION, &projection);
	device->SetTransform(D3DTS_VIEW, &identity);

	d->context->Render();

	device->SetTransform(D3DTS_PROJECTION, &oldProj);
	device->SetTransform(D3DTS_VIEW, &oldView);
	device->SetTransform(D3DTS_WORLD, &identity);

	videoFuncs.ReadInitialState();
	memcpy(videoFuncs.renderStates.ptr(), videoFuncs.activeRenderStates.ptr(), sizeof(TigRenderStates));
*/
}
