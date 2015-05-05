#include "stdafx.h"

#include "ui_text.h"
#include "graphics.h"
#include "renderstates.h"
#include "mainwindow.h"

#include "ui/ui_render_dx.h"
#include "tio/tio.h"
#include "tig/tig_mouse.h"

#include <Rocket/Core.h>
#include <Rocket/Debugger.h>

static void InitialiseKeymap();
static const int KEYMAP_SIZE = 256;
static Rocket::Core::Input::KeyIdentifier key_identifier_map[KEYMAP_SIZE];

class TempleFileInterface : public Rocket::Core::FileInterface {
public:
	Rocket::Core::FileHandle Open(const Rocket::Core::String& path) override {
		auto fh = tio_fopen(path.CString(), "rb");
		return (Rocket::Core::FileHandle) fh;
	}
	void Close(Rocket::Core::FileHandle file) override {
		auto tioFile = (TioFile*) file;
		tio_fclose(tioFile);
	}
	size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file) override {
		auto tioFile = (TioFile*)file;
		return tio_fread(buffer, 1, size, tioFile);
	}
	bool Seek(Rocket::Core::FileHandle file, long offset, int origin) override {
		auto tioFile = (TioFile*)file;
		auto result = tio_fseek(tioFile, offset, origin);
		return result == 0;
	}
	size_t Tell(Rocket::Core::FileHandle file) override {
		auto tioFile = (TioFile*)file;
		return tio_ftell(tioFile);
	}
	size_t Length(Rocket::Core::FileHandle file) override {
		auto tioFile = (TioFile*)file;
		return tio_filelength(tioFile);
	}
};

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

class GameMouseInputListener : public Rocket::Core::EventListener {
public:
	void ProcessEvent(Rocket::Core::Event& event) override;

	bool passThrough = false;
	bool capture = false;
	bool dragging = false;
};


class GameKeyInputListener : public Rocket::Core::EventListener {
public:
	void ProcessEvent(Rocket::Core::Event& event) override;

	bool passThrough = false;
	bool capture = false;
};

struct UiTextPrivate {
	Rocket::Core::Context *context = nullptr;
	RenderInterfaceDirectX renderer;
	TempleSystemInterface systemInterface;
	TempleFileInterface fileInterface;
	GameMouseInputListener mouseListener;
	GameKeyInputListener keyListener;
};

UiText uiText;

void UiText::Initialize() {
	assert(!d);

	InitialiseKeymap();

	d = new UiTextPrivate;
	d->renderer.SetDevice(graphics.device());
	
	SetRenderInterface(&d->renderer);
	SetSystemInterface(&d->systemInterface);
	SetFileInterface(&d->fileInterface);

	Rocket::Core::Initialise();

	Rocket::Core::StyleSheetSpecification::RegisterProperty("click-through", "false", false)
		.AddParser("keyword", "false,true");
		
	d->context = CreateContext("main", Rocket::Core::Vector2i(graphics.windowWidth(), graphics.windowHeight()));
	if (!d->context) {
		Rocket::Core::Shutdown();
		throw TempleException("Unable to initialize libRocket");
	}

	Rocket::Core::FontDatabase::LoadFontFace("Alegreya-Regular.otf");

	d->context->AddEventListener("mousemove", &d->mouseListener);
	d->context->AddEventListener("mouseup", &d->mouseListener);
	d->context->AddEventListener("mousedown", &d->mouseListener);
	d->context->AddEventListener("mousescroll", &d->mouseListener);

	d->context->AddEventListener("keydown", &d->keyListener);
	d->context->AddEventListener("keyup", &d->keyListener);
}

void UiText::Uninitialize() {
}

void UiText::Update() {
	d->context->Update();
}

void UiText::Render() {		

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

	// TODO This should be made more efficient
	renderStates.Reset();
}

static int GetKeyModifierState();

bool UiText::HandleMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (!d || !d->context) {
		return false;
	}
	
	d->mouseListener.passThrough = false;
	d->keyListener.passThrough = false;

	bool mouseClickEvent = false;

	switch (msg) {
	case WM_LBUTTONDOWN:
		d->context->ProcessMouseButtonDown(0, GetKeyModifierState());
		mouseClickEvent = true;
		break;

	case WM_LBUTTONUP:
		d->context->ProcessMouseButtonUp(0, GetKeyModifierState());
		break;

	case WM_RBUTTONDOWN:
		d->context->ProcessMouseButtonDown(1, GetKeyModifierState());
		mouseClickEvent = true;
		break;

	case WM_RBUTTONUP:
		d->context->ProcessMouseButtonUp(1, GetKeyModifierState());
		break;

	case WM_MBUTTONDOWN:
		d->context->ProcessMouseButtonDown(2, GetKeyModifierState()); 
		mouseClickEvent = true;
		break;

	case WM_MBUTTONUP:
		d->context->ProcessMouseButtonUp(2, GetKeyModifierState());
		break;

	case WM_MOUSEMOVE:
		d->context->ProcessMouseMove(LOWORD(lparam), HIWORD(lparam), GetKeyModifierState());
		break;

	case WM_MOUSEWHEEL:
		d->context->ProcessMouseWheel(((short)HIWORD(wparam)) / -WHEEL_DELTA, GetKeyModifierState());
		break;

	case WM_KEYDOWN:
	{
		Rocket::Core::Input::KeyIdentifier key_identifier = key_identifier_map[wparam];
		int key_modifier_state = GetKeyModifierState();

		// Check for a shift-~ to toggle the debugger.
		if (key_identifier == Rocket::Core::Input::KI_OEM_3 &&
			key_modifier_state & Rocket::Core::Input::KM_SHIFT)
		{
			Rocket::Debugger::SetVisible(!Rocket::Debugger::IsVisible());
			break;
		}

		d->context->ProcessKeyDown(key_identifier, key_modifier_state);
	}
	break;

	case WM_CHAR:
	{
		// Only send through printable characters.
		if (wparam >= 32)
			d->context->ProcessTextInput((Rocket::Core::word) wparam);
		// Or endlines - Windows sends them through as carriage returns.
		else if (wparam == '\r')
			d->context->ProcessTextInput((Rocket::Core::word) '\n');
	}
	break;

	case WM_KEYUP:
		d->context->ProcessKeyUp(key_identifier_map[wparam], GetKeyModifierState());
		break;

	default:
		return false;
	}

	/*
		If any mouse event passes through to ToEE, we assume that it's stealing focus from 
		librocket.
	*/
	if (mouseClickEvent && d->mouseListener.passThrough) {
		auto focusElem = d->context->GetFocusElement();
		d->context->GetRootElement()->Focus();
	}
	
	return !d->mouseListener.passThrough && !d->keyListener.passThrough;
}

static int GetKeyModifierState()
{
	int key_modifier_state = 0;

	// Query the state of all modifier keys
	if (GetKeyState(VK_CAPITAL) & 1)
	{
		key_modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;
	}

	if (HIWORD(GetKeyState(VK_SHIFT)) & 1)
	{
		key_modifier_state |= Rocket::Core::Input::KM_SHIFT;
	}

	if (GetKeyState(VK_NUMLOCK) & 1)
	{
		key_modifier_state |= Rocket::Core::Input::KM_NUMLOCK;
	}

	if (HIWORD(GetKeyState(VK_CONTROL)) & 1)
	{
		key_modifier_state |= Rocket::Core::Input::KM_CTRL;
	}

	if (HIWORD(GetKeyState(VK_MENU)) & 1)
	{
		key_modifier_state |= Rocket::Core::Input::KM_ALT;
	}

	return key_modifier_state;
}

void GameMouseInputListener::ProcessEvent(Rocket::Core::Event& evt) {

	auto target = evt.GetTargetElement();
	passThrough = false;
	if (target->GetTagName() == "#root") {
		passThrough = true;
	} else {
		passThrough = target->GetProperty("click-through")->Get<int>() == 1;
	}

	if (!passThrough) {
		if (evt.GetType() == "mousedown") {
			capture = true;
		}
	}
	if (capture && evt.GetType() == "mouseup" && !dragging) {
		capture = false;
		passThrough = false;
	}

	if (evt.GetType() == "dragstart") {
		capture = true;
		dragging = true;
	} else if (evt.GetType() == "dragend") {
		capture = false;
		passThrough = true;
		dragging = false;
	}

	if (dragging) {
		passThrough = true;
	}
	
}

void GameKeyInputListener::ProcessEvent(Rocket::Core::Event& evt) {

	auto target = evt.GetTargetElement();
	passThrough = false;
	if (target->GetTagName() == "#root") {
		passThrough = true;
	}

	if (!passThrough && evt.GetType() == "keydown") {
		capture = true;
	}
	if (evt.GetType() == "keyup") {
		capture = false;
	}

	if (capture) {
		passThrough = false;
	}

}


static void InitialiseKeymap()
{
	// Initialise the key map with default values.
	memset(key_identifier_map, 0, sizeof(key_identifier_map));

	// Assign individual values.
	key_identifier_map['A'] = Rocket::Core::Input::KI_A;
	key_identifier_map['B'] = Rocket::Core::Input::KI_B;
	key_identifier_map['C'] = Rocket::Core::Input::KI_C;
	key_identifier_map['D'] = Rocket::Core::Input::KI_D;
	key_identifier_map['E'] = Rocket::Core::Input::KI_E;
	key_identifier_map['F'] = Rocket::Core::Input::KI_F;
	key_identifier_map['G'] = Rocket::Core::Input::KI_G;
	key_identifier_map['H'] = Rocket::Core::Input::KI_H;
	key_identifier_map['I'] = Rocket::Core::Input::KI_I;
	key_identifier_map['J'] = Rocket::Core::Input::KI_J;
	key_identifier_map['K'] = Rocket::Core::Input::KI_K;
	key_identifier_map['L'] = Rocket::Core::Input::KI_L;
	key_identifier_map['M'] = Rocket::Core::Input::KI_M;
	key_identifier_map['N'] = Rocket::Core::Input::KI_N;
	key_identifier_map['O'] = Rocket::Core::Input::KI_O;
	key_identifier_map['P'] = Rocket::Core::Input::KI_P;
	key_identifier_map['Q'] = Rocket::Core::Input::KI_Q;
	key_identifier_map['R'] = Rocket::Core::Input::KI_R;
	key_identifier_map['S'] = Rocket::Core::Input::KI_S;
	key_identifier_map['T'] = Rocket::Core::Input::KI_T;
	key_identifier_map['U'] = Rocket::Core::Input::KI_U;
	key_identifier_map['V'] = Rocket::Core::Input::KI_V;
	key_identifier_map['W'] = Rocket::Core::Input::KI_W;
	key_identifier_map['X'] = Rocket::Core::Input::KI_X;
	key_identifier_map['Y'] = Rocket::Core::Input::KI_Y;
	key_identifier_map['Z'] = Rocket::Core::Input::KI_Z;

	key_identifier_map['0'] = Rocket::Core::Input::KI_0;
	key_identifier_map['1'] = Rocket::Core::Input::KI_1;
	key_identifier_map['2'] = Rocket::Core::Input::KI_2;
	key_identifier_map['3'] = Rocket::Core::Input::KI_3;
	key_identifier_map['4'] = Rocket::Core::Input::KI_4;
	key_identifier_map['5'] = Rocket::Core::Input::KI_5;
	key_identifier_map['6'] = Rocket::Core::Input::KI_6;
	key_identifier_map['7'] = Rocket::Core::Input::KI_7;
	key_identifier_map['8'] = Rocket::Core::Input::KI_8;
	key_identifier_map['9'] = Rocket::Core::Input::KI_9;

	key_identifier_map[VK_BACK] = Rocket::Core::Input::KI_BACK;
	key_identifier_map[VK_TAB] = Rocket::Core::Input::KI_TAB;

	key_identifier_map[VK_CLEAR] = Rocket::Core::Input::KI_CLEAR;
	key_identifier_map[VK_RETURN] = Rocket::Core::Input::KI_RETURN;

	key_identifier_map[VK_PAUSE] = Rocket::Core::Input::KI_PAUSE;
	key_identifier_map[VK_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;

	key_identifier_map[VK_KANA] = Rocket::Core::Input::KI_KANA;
	key_identifier_map[VK_HANGUL] = Rocket::Core::Input::KI_HANGUL;
	key_identifier_map[VK_JUNJA] = Rocket::Core::Input::KI_JUNJA;
	key_identifier_map[VK_FINAL] = Rocket::Core::Input::KI_FINAL;
	key_identifier_map[VK_HANJA] = Rocket::Core::Input::KI_HANJA;
	key_identifier_map[VK_KANJI] = Rocket::Core::Input::KI_KANJI;

	key_identifier_map[VK_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;

	key_identifier_map[VK_CONVERT] = Rocket::Core::Input::KI_CONVERT;
	key_identifier_map[VK_NONCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
	key_identifier_map[VK_ACCEPT] = Rocket::Core::Input::KI_ACCEPT;
	key_identifier_map[VK_MODECHANGE] = Rocket::Core::Input::KI_MODECHANGE;

	key_identifier_map[VK_SPACE] = Rocket::Core::Input::KI_SPACE;
	key_identifier_map[VK_PRIOR] = Rocket::Core::Input::KI_PRIOR;
	key_identifier_map[VK_NEXT] = Rocket::Core::Input::KI_NEXT;
	key_identifier_map[VK_END] = Rocket::Core::Input::KI_END;
	key_identifier_map[VK_HOME] = Rocket::Core::Input::KI_HOME;
	key_identifier_map[VK_LEFT] = Rocket::Core::Input::KI_LEFT;
	key_identifier_map[VK_UP] = Rocket::Core::Input::KI_UP;
	key_identifier_map[VK_RIGHT] = Rocket::Core::Input::KI_RIGHT;
	key_identifier_map[VK_DOWN] = Rocket::Core::Input::KI_DOWN;
	key_identifier_map[VK_SELECT] = Rocket::Core::Input::KI_SELECT;
	key_identifier_map[VK_PRINT] = Rocket::Core::Input::KI_PRINT;
	key_identifier_map[VK_EXECUTE] = Rocket::Core::Input::KI_EXECUTE;
	key_identifier_map[VK_SNAPSHOT] = Rocket::Core::Input::KI_SNAPSHOT;
	key_identifier_map[VK_INSERT] = Rocket::Core::Input::KI_INSERT;
	key_identifier_map[VK_DELETE] = Rocket::Core::Input::KI_DELETE;
	key_identifier_map[VK_HELP] = Rocket::Core::Input::KI_HELP;

	key_identifier_map[VK_LWIN] = Rocket::Core::Input::KI_LWIN;
	key_identifier_map[VK_RWIN] = Rocket::Core::Input::KI_RWIN;
	key_identifier_map[VK_APPS] = Rocket::Core::Input::KI_APPS;

	key_identifier_map[VK_SLEEP] = Rocket::Core::Input::KI_SLEEP;

	key_identifier_map[VK_NUMPAD0] = Rocket::Core::Input::KI_NUMPAD0;
	key_identifier_map[VK_NUMPAD1] = Rocket::Core::Input::KI_NUMPAD1;
	key_identifier_map[VK_NUMPAD2] = Rocket::Core::Input::KI_NUMPAD2;
	key_identifier_map[VK_NUMPAD3] = Rocket::Core::Input::KI_NUMPAD3;
	key_identifier_map[VK_NUMPAD4] = Rocket::Core::Input::KI_NUMPAD4;
	key_identifier_map[VK_NUMPAD5] = Rocket::Core::Input::KI_NUMPAD5;
	key_identifier_map[VK_NUMPAD6] = Rocket::Core::Input::KI_NUMPAD6;
	key_identifier_map[VK_NUMPAD7] = Rocket::Core::Input::KI_NUMPAD7;
	key_identifier_map[VK_NUMPAD8] = Rocket::Core::Input::KI_NUMPAD8;
	key_identifier_map[VK_NUMPAD9] = Rocket::Core::Input::KI_NUMPAD9;
	key_identifier_map[VK_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
	key_identifier_map[VK_ADD] = Rocket::Core::Input::KI_ADD;
	key_identifier_map[VK_SEPARATOR] = Rocket::Core::Input::KI_SEPARATOR;
	key_identifier_map[VK_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
	key_identifier_map[VK_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
	key_identifier_map[VK_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
	key_identifier_map[VK_F1] = Rocket::Core::Input::KI_F1;
	key_identifier_map[VK_F2] = Rocket::Core::Input::KI_F2;
	key_identifier_map[VK_F3] = Rocket::Core::Input::KI_F3;
	key_identifier_map[VK_F4] = Rocket::Core::Input::KI_F4;
	key_identifier_map[VK_F5] = Rocket::Core::Input::KI_F5;
	key_identifier_map[VK_F6] = Rocket::Core::Input::KI_F6;
	key_identifier_map[VK_F7] = Rocket::Core::Input::KI_F7;
	key_identifier_map[VK_F8] = Rocket::Core::Input::KI_F8;
	key_identifier_map[VK_F9] = Rocket::Core::Input::KI_F9;
	key_identifier_map[VK_F10] = Rocket::Core::Input::KI_F10;
	key_identifier_map[VK_F11] = Rocket::Core::Input::KI_F11;
	key_identifier_map[VK_F12] = Rocket::Core::Input::KI_F12;
	key_identifier_map[VK_F13] = Rocket::Core::Input::KI_F13;
	key_identifier_map[VK_F14] = Rocket::Core::Input::KI_F14;
	key_identifier_map[VK_F15] = Rocket::Core::Input::KI_F15;
	key_identifier_map[VK_F16] = Rocket::Core::Input::KI_F16;
	key_identifier_map[VK_F17] = Rocket::Core::Input::KI_F17;
	key_identifier_map[VK_F18] = Rocket::Core::Input::KI_F18;
	key_identifier_map[VK_F19] = Rocket::Core::Input::KI_F19;
	key_identifier_map[VK_F20] = Rocket::Core::Input::KI_F20;
	key_identifier_map[VK_F21] = Rocket::Core::Input::KI_F21;
	key_identifier_map[VK_F22] = Rocket::Core::Input::KI_F22;
	key_identifier_map[VK_F23] = Rocket::Core::Input::KI_F23;
	key_identifier_map[VK_F24] = Rocket::Core::Input::KI_F24;

	key_identifier_map[VK_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
	key_identifier_map[VK_SCROLL] = Rocket::Core::Input::KI_SCROLL;

	key_identifier_map[VK_OEM_NEC_EQUAL] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;

	key_identifier_map[VK_OEM_FJ_JISHO] = Rocket::Core::Input::KI_OEM_FJ_JISHO;
	key_identifier_map[VK_OEM_FJ_MASSHOU] = Rocket::Core::Input::KI_OEM_FJ_MASSHOU;
	key_identifier_map[VK_OEM_FJ_TOUROKU] = Rocket::Core::Input::KI_OEM_FJ_TOUROKU;
	key_identifier_map[VK_OEM_FJ_LOYA] = Rocket::Core::Input::KI_OEM_FJ_LOYA;
	key_identifier_map[VK_OEM_FJ_ROYA] = Rocket::Core::Input::KI_OEM_FJ_ROYA;

	key_identifier_map[VK_SHIFT] = Rocket::Core::Input::KI_LSHIFT;
	key_identifier_map[VK_CONTROL] = Rocket::Core::Input::KI_LCONTROL;
	key_identifier_map[VK_MENU] = Rocket::Core::Input::KI_LMENU;

	key_identifier_map[VK_BROWSER_BACK] = Rocket::Core::Input::KI_BROWSER_BACK;
	key_identifier_map[VK_BROWSER_FORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
	key_identifier_map[VK_BROWSER_REFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
	key_identifier_map[VK_BROWSER_STOP] = Rocket::Core::Input::KI_BROWSER_STOP;
	key_identifier_map[VK_BROWSER_SEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
	key_identifier_map[VK_BROWSER_FAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
	key_identifier_map[VK_BROWSER_HOME] = Rocket::Core::Input::KI_BROWSER_HOME;

	key_identifier_map[VK_VOLUME_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
	key_identifier_map[VK_VOLUME_DOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
	key_identifier_map[VK_VOLUME_UP] = Rocket::Core::Input::KI_VOLUME_UP;
	key_identifier_map[VK_MEDIA_NEXT_TRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
	key_identifier_map[VK_MEDIA_PREV_TRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
	key_identifier_map[VK_MEDIA_STOP] = Rocket::Core::Input::KI_MEDIA_STOP;
	key_identifier_map[VK_MEDIA_PLAY_PAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
	key_identifier_map[VK_LAUNCH_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
	key_identifier_map[VK_LAUNCH_MEDIA_SELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;
	key_identifier_map[VK_LAUNCH_APP1] = Rocket::Core::Input::KI_LAUNCH_APP1;
	key_identifier_map[VK_LAUNCH_APP2] = Rocket::Core::Input::KI_LAUNCH_APP2;

	key_identifier_map[VK_OEM_1] = Rocket::Core::Input::KI_OEM_1;
	key_identifier_map[VK_OEM_PLUS] = Rocket::Core::Input::KI_OEM_PLUS;
	key_identifier_map[VK_OEM_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
	key_identifier_map[VK_OEM_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
	key_identifier_map[VK_OEM_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
	key_identifier_map[VK_OEM_2] = Rocket::Core::Input::KI_OEM_2;
	key_identifier_map[VK_OEM_3] = Rocket::Core::Input::KI_OEM_3;

	key_identifier_map[VK_OEM_4] = Rocket::Core::Input::KI_OEM_4;
	key_identifier_map[VK_OEM_5] = Rocket::Core::Input::KI_OEM_5;
	key_identifier_map[VK_OEM_6] = Rocket::Core::Input::KI_OEM_6;
	key_identifier_map[VK_OEM_7] = Rocket::Core::Input::KI_OEM_7;
	key_identifier_map[VK_OEM_8] = Rocket::Core::Input::KI_OEM_8;

	key_identifier_map[VK_OEM_AX] = Rocket::Core::Input::KI_OEM_AX;
	key_identifier_map[VK_OEM_102] = Rocket::Core::Input::KI_OEM_102;
	key_identifier_map[VK_ICO_HELP] = Rocket::Core::Input::KI_ICO_HELP;
	key_identifier_map[VK_ICO_00] = Rocket::Core::Input::KI_ICO_00;

	key_identifier_map[VK_PROCESSKEY] = Rocket::Core::Input::KI_PROCESSKEY;

	key_identifier_map[VK_ICO_CLEAR] = Rocket::Core::Input::KI_ICO_CLEAR;

	key_identifier_map[VK_ATTN] = Rocket::Core::Input::KI_ATTN;
	key_identifier_map[VK_CRSEL] = Rocket::Core::Input::KI_CRSEL;
	key_identifier_map[VK_EXSEL] = Rocket::Core::Input::KI_EXSEL;
	key_identifier_map[VK_EREOF] = Rocket::Core::Input::KI_EREOF;
	key_identifier_map[VK_PLAY] = Rocket::Core::Input::KI_PLAY;
	key_identifier_map[VK_ZOOM] = Rocket::Core::Input::KI_ZOOM;
	key_identifier_map[VK_PA1] = Rocket::Core::Input::KI_PA1;
	key_identifier_map[VK_OEM_CLEAR] = Rocket::Core::Input::KI_OEM_CLEAR;
}
