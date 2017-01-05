#include "stdafx.h"
#include "widget_content.h"

#include <infrastructure/binaryreader.h>
#include <infrastructure/stringutil.h>
#include <infrastructure/vfs.h>

#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>

#include "widget_styles.h"
#include "fonts/fonts.h"

#include "tig/tig_startup.h"

#include "ui/ui_render.h"

// Render this font using the old engine
static eastl::string sScurlockFont = "Scurlock";

static TigTextStyle GetScurlockStyle(const gfx::Brush &brush) {
	static ColorRect sColorRect;
	sColorRect.topLeft = brush.primaryColor;
	sColorRect.topRight = brush.primaryColor;
	if (brush.gradient) {
		sColorRect.bottomLeft = brush.secondaryColor;
		sColorRect.bottomRight = brush.secondaryColor;
	} else {
		sColorRect.bottomLeft = brush.primaryColor;
		sColorRect.bottomRight = brush.primaryColor;
	}

	static ColorRect sShadowColor(XMCOLOR{ 0, 0, 0, 0.5 });

	TigTextStyle textStyle(&sColorRect);
	textStyle.leading = 1;
	textStyle.kerning = 0;
	textStyle.tracking = 10;
	textStyle.flags = TTSF_DROP_SHADOW;
	textStyle.shadowColor = &sShadowColor;

	return textStyle;
}

WidgetContent::WidgetContent()
{
	mContentArea = { 0, 0, 0, 0 };
	mPreferredSize = { 0, 0 };
}

WidgetImage::WidgetImage(const std::string &path)
{
	SetTexture(path);
}

void WidgetImage::Render()
{
	auto &renderer = tig->GetShapeRenderer2d();
	renderer.DrawRectangle(
		(float) mContentArea.x, 
		(float) mContentArea.y,
		(float) mContentArea.width,
		(float) mContentArea.height,
		*mTexture
	);
}

void WidgetImage::SetTexture(const std::string & path)
{
	mPath = path;
	mTexture = tig->GetRenderingDevice().GetTextures().Resolve(path, false);
	if (mTexture->IsValid()) {
		mPreferredSize = mTexture->GetSize();
	}
}

WidgetText::WidgetText()
{
	mText.defaultStyle = widgetTextStyles->GetDefaultStyle();
}

WidgetText::WidgetText(const std::string & text, const std::string &styleId)
{
	mText.defaultStyle = widgetTextStyles->GetTextStyle(styleId.c_str());
	SetText(text);
}

void WidgetText::SetText(const std::string & text)
{
	// TODO: Process mes file placeholders
	mText.text = local_to_ucs2(uiAssets->ApplyTranslation(text));
	UpdateBounds();
}

void WidgetText::SetStyleId(const std::string & id)
{
	mStyleId = id;
	mText.defaultStyle = widgetTextStyles->GetTextStyle(id.c_str());
	UpdateBounds();
}

const std::string & WidgetText::GetStyleId() const
{
	return mStyleId;
}

void WidgetText::SetStyle(const gfx::TextStyle & style)
{
	mText.defaultStyle = style;
	UpdateBounds();
}

const gfx::TextStyle & WidgetText::GetStyle() const
{
	return mText.defaultStyle;
}

void WidgetText::Render()
{
	if (mText.defaultStyle.fontFace == sScurlockFont) {
		auto textStyle = GetScurlockStyle(mText.defaultStyle.foreground);
		
		auto area = mContentArea; // Will be modified below
		if (mText.defaultStyle.align == gfx::TextAlign::Center) {
			textStyle.flags |= TTSF_CENTER;
		}

		UiRenderer::PushFont(PredefinedFont::SCURLOCK_48);

		auto text = ucs2_to_local(mText.text);
		auto &textLayouter = tig->GetTextLayouter();
		tigFont.Draw(text.c_str(), area, textStyle);

		UiRenderer::PopFont();
	} else {
		tig->GetRenderingDevice().GetTextEngine().RenderText(mContentArea, mText);
	}
}

void WidgetText::UpdateBounds()
{

	if (mText.defaultStyle.fontFace == sScurlockFont) {
		auto textStyle = GetScurlockStyle(mText.defaultStyle.foreground);
		if (mText.defaultStyle.align == gfx::TextAlign::Center) {
			textStyle.flags |= TTSF_CENTER;
		}
		UiRenderer::PushFont(PredefinedFont::SCURLOCK_48);
		auto rect = UiRenderer::MeasureTextSize(ucs2_to_local(mText.text), textStyle, 0, 0);
		UiRenderer::PopFont();		
		if (mText.defaultStyle.align == gfx::TextAlign::Center) {
			// Return 0 here to be in sync with the new renderer
			mPreferredSize.width = 0;
		} else {
			mPreferredSize.width = rect.width;
		}
		mPreferredSize.height = rect.height;
	} else {
		gfx::TextMetrics textMetrics;
		tig->GetRenderingDevice().GetTextEngine().MeasureText(mText, textMetrics);
		mPreferredSize.width = textMetrics.width;
		mPreferredSize.height = textMetrics.height;
	}

}
