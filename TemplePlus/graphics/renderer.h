#pragma once

#include <stdint.h>

namespace gfx{
	class MdfMaterial;
}

class Graphics;

struct Line {
	D3DXVECTOR3 from;
	D3DXVECTOR3 to;
	D3DCOLOR diffuse;

	Line() {}

	Line(const D3DXVECTOR3 &_from, const D3DXVECTOR3 &_to, D3DCOLOR _color) :
		from(_from), to(_to), diffuse(_color) {}

	Line(const D3DXVECTOR2 &_from, const D3DXVECTOR2 &_to, D3DCOLOR _color)
		: from(_from.x, _from.y, 0.5f),
		to(_to.x, _to.y, 0.5f),
		diffuse(_color) {
	}
};

class Renderer {
public:
	explicit Renderer(Graphics& graphics);
	
	void DrawTris(int vertexCount,
	              D3DXVECTOR4* pos,
	              D3DXVECTOR4* normal,
	              D3DCOLOR* diffuse,
	              D3DXVECTOR2* uv1,
	              D3DXVECTOR2* uv2,
	              D3DXVECTOR2* uv3,
	              D3DXVECTOR2* uv4,
	              int primCount,
	              uint16_t* indices);

	void DrawTrisScreenSpace(int vertexCount,
		D3DXVECTOR4* pos,
		D3DCOLOR* diffuse,
		D3DXVECTOR2* uv,
		int primCount,
		uint16_t* indices);

	void DrawLines(const std::vector<Line> &lines, bool screenSpace = false);
	
private:
	Graphics& mGraphics;
};
