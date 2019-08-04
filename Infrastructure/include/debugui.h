
#pragma once

#include <cstdint>

#include "imgui.h"

namespace gfx {
	class RenderingDevice;
}
class DebugUI {
public:
	DebugUI(gfx::RenderingDevice &device);
	~DebugUI();

	void NewFrame();
	void Render();

	bool HandleMessage(uint32_t message, uint32_t wparam, uint32_t lparam);
};
