#include "stdafx.h"
#include "widget_content.h"

#include <infrastructure/binaryreader.h>
#include <infrastructure/stringutil.h>
#include <infrastructure/vfs.h>

#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>

#include <tig/tig_startup.h>

#include "widget_styles.h"

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
	mText.text = local_to_ucs2(text);
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
	tig->GetRenderingDevice().GetTextEngine().RenderText(mContentArea, mText);
}

void WidgetText::UpdateBounds()
{
	gfx::TextMetrics textMetrics;
	tig->GetRenderingDevice().GetTextEngine().MeasureText(mText, textMetrics);
	mPreferredSize.width = textMetrics.width;
	mPreferredSize.height = textMetrics.height;
}
