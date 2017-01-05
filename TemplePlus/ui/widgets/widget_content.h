
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


	void SetX(int x) {
		mX = x;
	}
	int GetX() const {
		return mX;
	}
	void SetY(int y) {
		mY = y;
	}
	int GetY() const {
		return mY;
	}
	void SetFixedWidth(int width) {
		mFixedWidth = width;
	}
	int GetFixedWidth() const {
		return mFixedWidth;
	}
	void SetFixedHeight(int height) {
		mFixedHeight = height;
	}
	int GetFixedHeight() const {
		return mFixedHeight;
	}

protected:
	TigRect mContentArea;
	gfx::Size mPreferredSize;
	bool mDirty = true;

	int mFixedWidth = 0;
	int mFixedHeight = 0;
	int mX = 0;
	int mY = 0;
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
