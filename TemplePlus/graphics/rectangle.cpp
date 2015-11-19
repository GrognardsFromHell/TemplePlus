
#include "stdafx.h"
#include "rectangle.h"

#include "render_hooks.h"

UiRectangle::UiRectangle() : mArgs(std::make_unique<Render2dArgs>()) {
	mRect = { 0, 0, 0, 0 };

	memset(mArgs.get(), 0, sizeof(Render2dArgs));
	mColors.fill(0);

	mArgs->srcRect = &mRect;
	mArgs->destRect = &mRect;
	mArgs->vertexColors = (D3DCOLOR*)&mColors[0];
	mArgs->flags = Render2dArgs::FLAG_VERTEXCOLORS;
}

UiRectangle::~UiRectangle() = default;

void UiRectangle::SetX(int x) {
	mRect.x = x;
}

void UiRectangle::SetY(int y) {
	mRect.y = y;
}

void UiRectangle::SetWidth(int width) {
	mRect.width = width;
}

void UiRectangle::SetHeight(int height) {
	mRect.height = height;
}

void UiRectangle::SetColor(uint32_t color) {
	mColors.fill(color);
	if ((color & 0xFF000000) != 0xFF000000) {
		mArgs->flags |= Render2dArgs::FLAG_VERTEXALPHA;
	} else {
		mArgs->flags &= ~Render2dArgs::FLAG_VERTEXALPHA;
	}
}

void UiRectangle::Render() {
	RenderHooks::TextureRender2d(mArgs.get());
}
