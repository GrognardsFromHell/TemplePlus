
#include "debugui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"

using namespace gfx;

DebugUI::DebugUI(RenderingDevice & device)
{
	auto hwnd = device.mWindowHandle;
	ID3D11Device *d3dDevice = device.mD3d11Device;
	ID3D11DeviceContext *context = device.mContext;

	if (!ImGui_ImplDX11_Init(hwnd, d3dDevice, context)) {
		throw TempleException("Unable to initialize IMGui!");
	}

}

DebugUI::~DebugUI()
{
	ImGui_ImplDX11_Shutdown();
}

void DebugUI::NewFrame()
{
	ImGui_ImplDX11_NewFrame();
}

void DebugUI::Render()
{
	ImGui::Render();
}

bool DebugUI::HandleMessage(uint32_t message, uint32_t wParam, uint32_t lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (message)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return io.WantCaptureMouse;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return io.WantCaptureMouse;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return io.WantCaptureMouse;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return io.WantCaptureMouse;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		return io.WantCaptureMouse;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return io.WantCaptureMouse;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return io.WantCaptureMouse;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return false; // Always update, never take it
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return io.WantCaptureKeyboard;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return io.WantCaptureKeyboard;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return io.WantCaptureKeyboard;
	default:
		return false;
	}
}
