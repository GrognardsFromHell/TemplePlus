
#pragma once

#include <memory>

#include "graphics/math.h"
#include "graphics/mdfmaterials.h"
#include "infrastructure/macros.h"

namespace gfx {

class RenderingDevice;

#pragma pack(push, 1)
struct ShapeVertex3d {
	XMFLOAT4 pos;
	XMFLOAT4 normal;
	XMFLOAT2 uv;
};
#pragma pack(pop)

class ShapeRenderer3d {
public:
	explicit ShapeRenderer3d(RenderingDevice& device);
	~ShapeRenderer3d();

	void DrawQuad(const std::array<ShapeVertex3d, 4> &corners,
		XMCOLOR color,
		const gfx::TextureRef &texture);

	void DrawQuad(const std::array<ShapeVertex3d, 4> &corners,
		gfx::MdfRenderMaterial &material,
		XMCOLOR color);

	void DrawDisc(const XMFLOAT3 &center, 
		float rotation, 
		float radius,
		gfx::MdfRenderMaterialPtr &material);

	void DrawLine(const XMFLOAT3 &from,
		const XMFLOAT3 &to,
		XMCOLOR color);

	void DrawLineWithoutDepth(const XMFLOAT3 &from,
		const XMFLOAT3 &to,
		XMCOLOR color);

	void DrawCylinder(const XMFLOAT3 &pos, float radius, float height);

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
