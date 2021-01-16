
#pragma once

#include <memory>

#include <gsl/span>

#include "graphics/math.h"
#include "infrastructure/macros.h"

namespace gfx {
	using MdfRenderMaterialPtr = std::shared_ptr<class MdfRenderMaterial>;
	
	class RenderingDevice;
	class Texture;
	using TextureRef = std::shared_ptr<Texture>;
	
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
		XMFLOAT4 pos;
		XMFLOAT4 normal;
		XMCOLOR diffuse;
		XMFLOAT2 uv;
	};
#pragma pack(pop)
	
	enum class SamplerType2d {
		CLAMP,
		WRAP,
		POINT
	};

	class SamplerState;

	/*
		Renders shapes in 2d screen space.
	*/
	class ShapeRenderer2d {
	public:
		explicit ShapeRenderer2d(RenderingDevice& g);
		~ShapeRenderer2d();

		void DrawRectangle(float x, float y, float width, float height, gfx::Texture& texture, uint32_t color = 0xFFFFFFFF, SamplerType2d samplerType = SamplerType2d::CLAMP) {
			DrawRectangle(x, y, width, height, &texture, color, samplerType);
		}
		
		void DrawRectangle(float x, float y, float width, float height, uint32_t color) {
			DrawRectangle(x, y, width, height, nullptr, color);
		}

		void DrawRectangle(gsl::span<Vertex2d, 4> corners,
			gfx::Texture* texture,
			gfx::Texture* mask = nullptr,
			SamplerType2d samplerType = SamplerType2d::CLAMP,
			bool blending = true);

		void DrawRectangle(gsl::span<Vertex2d, 4> corners,
			const gfx::MdfRenderMaterialPtr &material);

		void DrawRectangle(gsl::span<Vertex2d, 4> corners);

		void DrawLines(gsl::span<Line2d> lines);

		void DrawRectangleOutlineVanilla(XMFLOAT2 topLeft, XMFLOAT2 bottomRight, XMCOLOR color);
		void DrawRectangleOutline(XMFLOAT2 topLeft, XMFLOAT2 bottomRight, XMCOLOR color);

		void DrawFullScreenQuad();

		/**
		 * Renders a circle/pie segment for use with the radial menu.
  		 */
		void DrawPieSegment(int segments,
			int x, int y,
			float angleCenter, float angleWidth,
			int innerRadius, int innerOffset,
			int outerRadius, int outerOffset,
			XMCOLOR color1, XMCOLOR color2);
		
		NO_COPY_OR_MOVE(ShapeRenderer2d);

	private:

		void DrawRectangle(
			float x, float y, float width, float height,
			gfx::Texture* texture,
			uint32_t color = 0xFFFFFFFF, 
			SamplerType2d samplerType = SamplerType2d::CLAMP
		);

		SamplerState &getSamplerState(SamplerType2d type) const;

		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

}

