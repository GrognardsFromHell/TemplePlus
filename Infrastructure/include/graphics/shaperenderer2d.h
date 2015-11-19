
#pragma once

#include <memory>

#include "gsl/array_view.h"
#include "graphics/math.h"
#include "infrastructure/macros.h"

struct IDirect3DTexture9;

namespace gfx {

	class RenderingDevice;
	using TextureRef = std::shared_ptr<class Texture>;
	
	struct Line2d {
		XMFLOAT2 from;
		XMFLOAT2 to;
		XMCOLOR diffuse;

		Line2d() {}

		Line2d(XMFLOAT2 from, XMFLOAT2 to, XMCOLOR color)
			: from(from), to(to), diffuse(color) {}
	};

#pragma pack(push, 1)
	struct Vertex2d {
		XMFLOAT3 pos;
		XMCOLOR diffuse;
		XMFLOAT2 uv;
	};
#pragma pack(pop)
	
	/*
		Renders shapes in 2d screen space.
	*/
	class ShapeRenderer2d {
	public:
		explicit ShapeRenderer2d(RenderingDevice& g);
		~ShapeRenderer2d();

		void DrawRectangle(
			float x, float y, float width, float height,
			const TextureRef& texture,
			uint32_t color = 0xFFFFFFFF
			);

		void DrawRectangle(
			float x, float y, float width, float height,
			uint32_t color
			);

		void DrawRectangle(gsl::array_view<Vertex2d> corners,
			IDirect3DTexture9* texture,
			IDirect3DTexture9* mask = nullptr,
			bool wrap = false);

		void DrawRectangle(gsl::array_view<Vertex2d, 4> corners);

		void DrawLines(gsl::array_view<Line2d> lines);

		void DrawRectangleOutline(XMFLOAT2 topLeft, XMFLOAT2 bottomRight, XMCOLOR color);

		NO_COPY_OR_MOVE(ShapeRenderer2d);

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

}

