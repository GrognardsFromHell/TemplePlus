
#pragma once

#include "util/fixes.h"

#include <graphics/math.h>

struct TigBuffer;
struct TigRect;
struct ImgFile;
struct LocAndOffsets;

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
	static constexpr uint32_t FLAG_MASK = 0x400;
	static constexpr uint32_t FLAG_WRAP = 0x1000;

	uint32_t flags;
	int textureId; // Unused for shaders
	int textureId2; // Unused for shaders
	TigBuffer* texBuffer; // Unused for shaders
	int shaderId;
	TigRect* srcRect;
	const TigRect* destRect;
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

	// Only used by radial menu checkboxes and the gfade overlay
	static int RenderTexturedQuad(D3DXVECTOR3 *vertices, float *u, float *v, int textureId, D3DCOLOR color);

	// Renders a tiled img on screen
	static void RenderImgFile(ImgFile *img, int x, int y);

	static void RenderRect(float left, float top, float right, float bottom, D3DCOLOR color);

	static void RenderRectInt(int left, int top, int width, int height, D3DCOLOR color);

	static void RenderDisc3d(LocAndOffsets &loc, int shaderId, float rotation, float radius);

	static int RenderUnk3d(int posCount, XMFLOAT3 *positions, XMCOLOR color, int unk1, int unk2);

	static void SetDrawCircleZMode(int type);

	static void DrawCircle3d(LocAndOffsets center, 
		float unk, 
		XMCOLOR fillColor, 
		XMCOLOR borderColor, 
		float radius);
		
private:

	static int RenderRectIndirect(XMFLOAT2 *topLeft, XMFLOAT2 *bottomRight, XMCOLOR color);

	static int RenderLine3d(XMFLOAT3* from, XMFLOAT3* to, XMCOLOR color);

	static bool specialZ;

};
