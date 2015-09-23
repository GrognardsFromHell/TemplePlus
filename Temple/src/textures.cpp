
#include <d3d8to9_texture.h>
#include <infrastructure/textures.h>

#include "temple/textures.h"

const std::string& LegacyTexture::GetName() const {
	const_cast<LegacyTexture*>(this)->LoadMetadata();	
	return mName;
}

const gfx::ContentRect& LegacyTexture::GetContentRect() const {
	const_cast<LegacyTexture*>(this)->LoadMetadata();
	return mContentRect;
}

const gfx::Size& LegacyTexture::GetSize() const {
	const_cast<LegacyTexture*>(this)->LoadMetadata();
	return mSize;
}

IDirect3DTexture9* LegacyTexture::GetDeviceTexture() {
	/*TigTextureRegistryEntry entry;
	if (textureFuncs.LoadTexture(mId, &entry)) {
		return nullptr; // error
	}
	if (!entry.buffer) {
		return nullptr; // Also an error
	}
	// The TIG portion of the buffer still has a D3D8to9 adapter
	if (entry.buffer->d3dtexture) {
		return entry.buffer->d3dtexture->delegate;
	}*/
	return nullptr;
}

void LegacyTexture::LoadMetadata() {
	if (mMetadataKnown) {
		return;
	}

	/*TigTextureRegistryEntry entry;
	textureFuncs.LoadTexture(mId, &entry);
	mName = entry.name;
	mContentRect.x = entry.rect.x;
	mContentRect.y = entry.rect.y;
	mContentRect.width = entry.rect.width;
	mContentRect.height = entry.rect.height;
	mSize.width = entry.width;
	mSize.height = entry.height;
	mMetadataKnown = true;*/
}
