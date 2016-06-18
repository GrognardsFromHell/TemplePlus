
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
	static constexpr uint32_t FLAG_UNK = 0x40; // Does not seem to have an effect
	static constexpr uint32_t FLAG_BUFFER = 0x80;
	static constexpr uint32_t FLAG_FLOATSRCRECT = 0x100;
	static constexpr uint32_t FLAG_ROTATE = 0x200;
	static constexpr uint32_t FLAG_MASK = 0x400;
	// This is not 100% correct, but it should do the trick
	static constexpr uint32_t FLAG_DISABLEBLENDING = 0x800;
	static constexpr uint32_t FLAG_WRAP = 0x1000;
	static constexpr uint32_t FLAG_BUFFERTEXTURE = 0x2000;

	uint32_t flags;
	int textureId; // Unused for shaders
	int textureId2; // Unused for shaders
	TigBuffer* texBuffer; // Unused for shaders
	int shaderId;
	TigRect* srcRect;
	const TigRect* destRect;
	XMCOLOR* vertexColors;
	float vertexZ;
	float rotation;
	float rotationX;
	float rotationY;
};
#pragma pack(pop)

class RenderHooks : TempleFix {
public:
	void apply() override;

	static int ShaderRender3d(int vertexCount,
		XMFLOAT4* vertices,
		XMFLOAT4* normals,
		XMCOLOR* diffuse,
		XMFLOAT2* uv,
		int primCount,
		uint16_t* indices,
		int shaderId);

	// This is mostly used to render the icons in the radial menu
	static int ShaderRender2d(const Render2dArgs* args);

	// Mostly used by UI, but also by the map tiles
	static int TextureRender2d(const Render2dArgs* args);

	// Only used by radial menu checkboxes and the gfade overlay
	static int RenderTexturedQuad(XMFLOAT3 *vertices, float *u, float *v, int textureId, XMCOLOR color);

	// Renders a tiled img on screen
	static void RenderImgFile(ImgFile *img, int x, int y);

	static void RenderRect(float left, float top, float right, float bottom, XMCOLOR color);

	static void RenderRectInt(int left, int top, int width, int height, XMCOLOR color);

	static void RenderDisc3d(LocAndOffsets &loc, int shaderId, float rotation, float radius);

	static void SetDrawCircleZMode(int type);

	static void DrawCircle3d(LocAndOffsets center, 
		float unk, 
		XMCOLOR fillColor, 
		XMCOLOR borderColor, 
		float radius);
		
private:

	static int RenderRectIndirect(XMFLOAT2 *topLeft, XMFLOAT2 *bottomRight, XMCOLOR color);
	
	static int RenderLine3d(XMFLOAT3* from, XMFLOAT3* to, XMCOLOR color);

	/*
		These replace a call to the same function with two different values for the first parameter.
		ToEE passes the parameter in the eax register, which is painful to hook, so instead we replace
		both call sites and redirect to different functions.
	*/
	static int DrawRadialMenuSegment1(signed int x, signed int y, float a5, float a6, signed int a7, signed int a8, signed int a9, signed int a10, XMCOLOR color1, XMCOLOR color2);
	static int DrawRadialMenuSegment2(signed int x, signed int y, float a5, float a6, signed int a7, signed int a8, signed int a9, signed int a10, XMCOLOR color1, XMCOLOR color2);

	static bool specialZ;

};
