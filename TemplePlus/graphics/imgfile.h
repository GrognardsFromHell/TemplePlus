
#pragma once

#include <graphics/textures.h>
#include <string>

struct ImgFile;

class Graphics;

class CombinedImgFile {
public:
	CombinedImgFile(const std::string &filename);
	~CombinedImgFile();

	void SetX(int x) {
		mX = x;
	}
	void SetY(int y) {
		mY = y;
	}
	int GetWidth() const;
	int GetHeight() const;

	void Render();

private:
	std::string mFilename;
	std::unique_ptr<ImgFile> mImgFile;
	int mX = 0; 
	int mY = 0;
};
