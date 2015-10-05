
#pragma once

#include <memory>

#include <infrastructure/renderstates.h>

class Graphics;
std::unique_ptr<RenderStates> CreateLegacyRenderStates(Graphics &g);
