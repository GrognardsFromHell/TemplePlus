
#include "stdafx.h"
#include "ui_render.h"
#include <temple/dll.h>
#include "tig/tig_font.h"
#include "tig/tig_texture.h"
#include "ui\ui.h"

#pragma pack(push, 1)
struct DrawTexturedQuadArgs {
	int flags = 0;
	int textureId;
	int field8;
	void *texBuffer;
	int shaderId;
	const TigRect *srcRect;
	const TigRect *destRect;	
	ColorRect *vertexColors;
	float vertexZ;
	int field24; // May be padding from here on out
	int field28;
	int field2c;
};
#pragma pack(pop)

static struct UiRenderFuncs : temple::AddressTable {
	
	/*
		This is one of the primary functions used to draw textured quads for UI purposes.
		It's a pretty flexible function. Not all arguments have been discovered yet.
	*/
	int (__cdecl *DrawTexturedQuad)(const DrawTexturedQuadArgs &);

	/*
		Simply calls TIG Font Draw but positions the given rectangle relative to the given widget.
	*/
	bool (__cdecl *DrawTextInWidget)(int widgetId, const char *text, const TigRect &rect, const TigTextStyle &style);



	UiRenderFuncs() {
		rebase(DrawTexturedQuad, 0x101D9300);
		rebase(DrawTextInWidget, 0x101F87C0);
	}
} uiRenderFuncs;

void UiRenderer::DrawTexture(int texId, const TigRect &destRect, int flags) {

	DrawTexturedQuadArgs args;
	args.flags = flags;
	args.destRect = &destRect;

	// This function assumes dest rect encompasses the entire src rect at 0,0
	TigRect srcRect(0, 0, destRect.width, destRect.height);
	args.srcRect = &srcRect;

	args.textureId = texId;

	if (uiRenderFuncs.DrawTexturedQuad(args)) {
		logger->warn("DrawTexturedQuad failed!");
	}

}

void UiRenderer::DrawTexture(int texId, const TigRect& destRect, const TigRect& srcRect){
	DrawTexturedQuadArgs args;
	args.destRect = &destRect;
	args.srcRect = &srcRect;

	args.textureId = texId;

	if (uiRenderFuncs.DrawTexturedQuad(args)) {
		logger->warn("DrawTexturedQuad failed!");
	}
}

void UiRenderer::DrawTextureInWidget(int widId, int texId, const TigRect & destRect, const TigRect & srcRect, int flags){
	auto wid = uiManager->GetWidget(widId);
	if (!wid)
		return;
	
	TigRect destRectAdj (destRect.x + wid->x, destRect.y + wid->y, destRect.width, destRect.height);

	DrawTexturedQuadArgs args;
	args.destRect = &destRectAdj;
	args.srcRect = &srcRect;
	args.flags = flags;
	args.textureId = texId;

	if (uiRenderFuncs.DrawTexturedQuad(args)) {
		logger->warn("DrawTexturedQuad failed!");
	}
}

void UiRenderer::PushFont(PredefinedFont font) {
	switch (font) {
	case PredefinedFont::ARIAL_10:
		tigFont.PushFont("arial-10", 10, true);
		break;
	case PredefinedFont::ARIAL_12: 
		tigFont.PushFont("arial-12", 12, true);
		break;
	case PredefinedFont::ARIAL_BOLD_10: 
		tigFont.PushFont("arial-bold-10", 10, true);
		break;
	case PredefinedFont::ARIAL_BOLD_24: 
		tigFont.PushFont("arial-bold-24", 24, true);
		break;
	case PredefinedFont::PRIORY_12: 
		tigFont.PushFont("priory-12", 12, true);
		break;
	case PredefinedFont::SCURLOCK_48: 
		tigFont.PushFont("scurlock-48", 48, true);
		break;
	default: 
		throw TempleException("Unknown font literal was used!");
	}
}

void UiRenderer::PushFont(const std::string& faceName, int pixelSize, bool antialiased) {
	tigFont.PushFont(faceName.c_str(), pixelSize, antialiased);
}

void UiRenderer::PopFont() {
	tigFont.PopFont();
}

bool UiRenderer::DrawText(const string& text, const TigRect& rect, const TigTextStyle& style)
{
	if (text.empty())
		return 1;

	return tigFont.Draw(text.c_str(), rect, style) == TRUE;
}

bool UiRenderer::DrawTextInWidget(int widgetId, const string &text, const TigRect &rect, const TigTextStyle &style) {
	auto wid = uiManager->GetWidget(widgetId);
	if (!wid)
		return 1;
	if (text.empty())
		return 1;

	TigRect extents(rect.x + wid->x , rect.y + wid->y , rect.width,rect.height);

	return tigFont.Draw(text.c_str(), extents, style) == TRUE;

	// return uiRenderFuncs.DrawTextInWidget(widgetId, text.c_str(), rect, style);
}

bool UiRenderer::DrawTextInWidgetCentered(int widgetId, const string & text, const TigRect & rect, const TigTextStyle & style)
{
	auto meas = MeasureTextSize(text, style);
	TigRect rect2( (rect.width - meas.width)/2 + rect.x, 
				   (rect.height - meas.height) / 2 + rect.y,
					meas.width, meas.height);
	return DrawTextInWidget(widgetId, text, rect2, style);
}

bool UiRenderer::RenderText(const string &text, TigRect &rect, const TigTextStyle &style) {
	return tigFont.Draw(text.c_str(), rect, style) == 0;
}

TigRect UiRenderer::MeasureTextSize(const string &text, const TigTextStyle &style, int maxWidth, int maxHeight) {
	TigFontMetrics metrics;
	metrics.text = text.c_str();
	metrics.width = maxWidth;
	metrics.height = maxHeight;
	tigFont.Measure(style, metrics);
	TigRect result;
	result.x = 0;
	result.y = 0;
	result.width = metrics.width;
	result.height = metrics.height;
	return result;
}
