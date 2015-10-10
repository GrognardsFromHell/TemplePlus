
#pragma once

#include <infrastructure/textures.h>
#include <unordered_map>

class TextureManager : public gfx::TextureManager {
friend class Texture;
public:

	TextureManager(IDirect3DDevice9 *device, size_t memoryBudget);
	
	/*
		Call this after every frame to free texture memory by
		freeing the least recently used textures.
	*/
	void FreeUnusedTextures();

	// Frees all GPU texture memory i.e. after a device reset
	void FreeAllTextures();

	gfx::TextureRef Resolve(const std::string& filename, bool withMipMaps) override;
	
	gfx::TextureRef GetById(int textureId) override;

	int GetLoaded();
	int GetRegistered();
	size_t GetUsageEstimate();
	size_t GetMemoryBudget();

private:
	std::shared_ptr<class TextureLoader> mLoader;
	int mNextFreeId = 1;
	std::unordered_map<int, gfx::TextureRef> mTexturesById;
	std::unordered_map<std::string, gfx::TextureRef> mTexturesByName;
};
