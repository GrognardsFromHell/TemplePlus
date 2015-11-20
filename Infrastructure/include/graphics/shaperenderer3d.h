
#pragma once

#include <memory>

#include "graphics/math.h"
#include "graphics/mdfmaterials.h"
#include "infrastructure/macros.h"

namespace gfx {

class RenderingDevice;

class ShapeRenderer3d {
public:
	explicit ShapeRenderer3d(RenderingDevice& device);
	~ShapeRenderer3d();

	void DrawDisc(const XMFLOAT3 &center, 
		float rotation, 
		float radius,
		gfx::MdfRenderMaterialPtr &material);

	void DrawLine(const XMFLOAT3 &from,
		const XMFLOAT3 &to,
		XMCOLOR color);

	/*
		occludedOnly means that the circle will only draw
		in already occluded areas (based on depth buffer)
	*/
	void DrawFilledCircle(const XMFLOAT3 &center,
		float radius,
		XMCOLOR borderColor,
		XMCOLOR fillColor,
		bool occludedOnly = false);

	NO_COPY_OR_MOVE(ShapeRenderer3d);

private:
	struct Impl;
	std::unique_ptr<Impl> mImpl;
};

}
