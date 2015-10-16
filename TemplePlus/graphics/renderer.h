#pragma once

#include <stdint.h>

class Graphics;

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
	
private:
	Graphics& mGraphics;
};
