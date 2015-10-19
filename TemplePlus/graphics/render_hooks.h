
#pragma once

#include "util/fixes.h"

struct TigBuffer;
struct TigRect;
struct ImgFile;

struct D3DXVECTOR2;

#pragma pack(push, 1)
struct Render2dArgs {

	static constexpr uint32_t FLAG_VERTEXCOLORS = 1;
	static constexpr uint32_t FLAG_VERTEXZ = 2;
	static constexpr uint32_t FLAG_VERTEXALPHA = 4;
	static constexpr uint32_t FLAG_FLIPH = 0x10;
	static constexpr uint32_t FLAG_FLIPV = 0x20;
	static constexpr uint32_t FLAG_BUFFER = 0x80;
	static constexpr uint32_t FLAG_FLOATSRCRECT = 0x100;
	static constexpr uint32_t FLAG_ROTATE = 0x200;

	uint32_t flags;
	int textureId; // Unused for shaders
	int textureId2; // Unused for shaders
	TigBuffer* texBuffer; // Unused for shaders
	int shaderId;
	TigRect* srcRect;
	TigRect* destRect;
	D3DCOLOR* vertexColors;
	float vertexZ;
	float rotation;
	float rotationX;
	float rotationY;
};
#pragma pack(pop)

class RenderHooks : TempleFix {
public:
	const char* name() override;

	void apply() override;

	static int ShaderRender3d(int vertexCount,
		D3DXVECTOR4* vertices,
		D3DXVECTOR4* normals,
		D3DCOLOR* diffuse,
		D3DXVECTOR2* uv,
		int primCount,
		uint16_t* indices,
		int shaderId);

	// This is mostly used to render the icons in the radial menu
	static int ShaderRender2d(const Render2dArgs* args);

	// Mostly used by UI, but also by the map tiles
	static int TextureRender2d(const Render2dArgs* args);

	// Renders a tiled img on screen
	static void RenderImgFile(ImgFile *img, int x, int y);

	static int RenderRect(D3DXVECTOR2 topLeft, D3DXVECTOR2 bottomRight, D3DCOLOR color);

};
