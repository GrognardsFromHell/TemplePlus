
#pragma once

#include <textures.h>

class LegacyTexture : public gfx::Texture {
public:
	explicit LegacyTexture(int id) : mId(id) {
	}

	const std::string& GetName() const override;
	
	const gfx::ContentRect& GetContentRect() const override;

	const gfx::Size& GetSize() const override;
	
	IDirect3DTexture9* GetDeviceTexture() override;

private:
	int mId;
	bool mMetadataKnown = false;
	void LoadMetadata();
	std::string mName;
	gfx::ContentRect mContentRect;
	gfx::Size mSize;
};
