
#pragma once

#include <EASTL/hash_map.h>

#include <graphics/math.h>
#include <graphics/textures.h>
#include <graphics/textengine.h>
#include "ui/ui_assets.h"

class WidgetContent {
public:
	WidgetContent();
	virtual ~WidgetContent() = default;

	virtual void Render() = 0;

	void SetContentArea(const TigRect &contentArea) {
		mContentArea = contentArea;
		mDirty = true;
	}

	const TigRect &GetContentArea() const {
		return mContentArea;
	}

	const gfx::Size &GetPreferredSize() const {
		return mPreferredSize;
	}

protected:
	TigRect mContentArea;
	gfx::Size mPreferredSize;
	bool mDirty = true;
};

class WidgetImage : public WidgetContent {
public:
	WidgetImage(const std::string &path);

	void Render() override;

	void SetTexture(const std::string &path);
	
private:
	std::string mPath;
	gfx::TextureRef mTexture;
};

class WidgetText : public WidgetContent {
public:
	WidgetText();
	WidgetText(const std::string &text, const std::string &styleId);

	void SetText(const std::string &text);

	void SetStyleId(const std::string &id);
	const std::string &GetStyleId() const;

	void SetStyle(const gfx::TextStyle &style);
	const gfx::TextStyle &GetStyle() const;

	void Render() override;

private:
	gfx::FormattedText mText;
	std::string mStyleId;
	bool mWordWrap = false;

	void UpdateBounds();
};
