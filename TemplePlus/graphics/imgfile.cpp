#include "stdafx.h"
#include "imgfile.h"

#include "render_hooks.h"

#include <ui/ui_assets.h>

CombinedImgFile::CombinedImgFile(const std::string& filename)
	: mFilename(filename) {
	mImgFile.reset(uiAssets->LoadImg(filename.c_str()));
}

CombinedImgFile::~CombinedImgFile() = default;

int CombinedImgFile::GetWidth() const {
	return mImgFile ? mImgFile->width : 0;
}

int CombinedImgFile::GetHeight() const {
	return mImgFile ? mImgFile->height: 0;
}

void CombinedImgFile::Render() {
	RenderHooks::RenderImgFile(mImgFile.get(), mX, mY);
}
