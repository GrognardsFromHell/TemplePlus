#include "stdafx.h"

#include <util/fixes.h>
#include <infrastructure/textures.h>

#include <d3d8adapter.h>

#include "../tig/tig_texture.h"

static class TextureHooks : TempleFix {
public:
	const char* name() override {
		return "Texture Registry Enhancements";
	}

	void apply() override;

private:
	using MapArtResolverFn = const char*(int);

	static void UnloadAll();
	static void UnloadId(int textureId);
	static void SetMapTileFilenameResolver(MapArtResolverFn*);
	static int GetMapTileArtId(uint16_t mapId, uint8_t x, uint8_t y, int* textureIdOut);

	static int RegisterUiTexture(const char *filename, int *textureIdOut);
	static int RegisterMdfTexture(const char *filename, int *textureIdOut);
	static int RegisterFontTexture(const char *filename, int *textureIdOut);

	static int LoadTexture(int textureId, TigTextureRegistryEntry *textureOut);

	static MapArtResolverFn* mapArtResolver;
	static std::unordered_map<int, int> mapTileIdCache;
} hooks;

TextureHooks::MapArtResolverFn* TextureHooks::mapArtResolver = nullptr;

void TextureHooks::apply() {

	replaceFunction(0x101EE170, UnloadAll);
	replaceFunction(0x101EE240, UnloadId);
	replaceFunction(0x101EE270, SetMapTileFilenameResolver);
	replaceFunction(0x101EE280, GetMapTileArtId);
	replaceFunction(0x101EE7B0, RegisterUiTexture);
	replaceFunction(0x101EE8A0, RegisterMdfTexture);
	replaceFunction(0x101EE990, RegisterFontTexture);
	replaceFunction(0x101EECA0, LoadTexture);

}

// Called by cleanup buffers, it's a noop because we handle this ourselves
void TextureHooks::UnloadAll() {
}

// Unloads a single texture by id
void TextureHooks::UnloadId(int textureId) {
	auto ref = gfx::textureManager->GetById(textureId);
	if (ref) {
		ref->FreeDeviceTexture();
	}
}

// Used during map loading to register a function that resolves map tile ids -> filenames
void TextureHooks::SetMapTileFilenameResolver(MapArtResolverFn* resolver) {
	mapArtResolver = resolver;
}

int TextureHooks::GetMapTileArtId(uint16_t mapId, uint8_t x, uint8_t y, int* textureIdOut) {
	auto tileId = (y << 16) | x;
	std::string tileFilename;
	if (!mapArtResolver) {
		tileFilename = fmt::format("art\\ground\\{}\\{:08x}.jpg", mapId, tileId);
	} else {
		auto mapDirname = mapArtResolver(mapId);
		tileFilename = fmt::format("art\\ground\\{}\\{:08x}.jpg", mapDirname, tileId);
	}

	auto texture = gfx::textureManager->Resolve(tileFilename);

	*textureIdOut = texture->GetId();
	return texture->IsValid() ? 0 : 17;
}

int TextureHooks::RegisterUiTexture(const char* filename, int* textureIdOut) {
	auto ref = gfx::textureManager->Resolve(filename, false);

	if (!ref->IsValid()) {
		*textureIdOut = -1;
		logger->error("Unable to register texture {}", filename);
		return 17;
	}

	*textureIdOut = ref->GetId();
	return 0;
}

int TextureHooks::RegisterMdfTexture(const char* filename, int* textureIdOut) {
	auto ref = gfx::textureManager->Resolve(filename, true);

	if (!ref->IsValid()) {
		*textureIdOut = -1;
		logger->error("Unable to register texture {}", filename);
		return 17;
	}

	*textureIdOut = ref->GetId();
	return 0;
}

int TextureHooks::RegisterFontTexture(const char* filename, int* textureIdOut) {
	auto ref = gfx::textureManager->Resolve(filename, false);

	if (!ref->IsValid()) {
		*textureIdOut = -1;
		logger->error("Unable to register texture {}", filename);
		return 17;
	}

	*textureIdOut = ref->GetId();
	return 0;
}

int TextureHooks::LoadTexture(int textureId, TigTextureRegistryEntry* textureOut) {
	// Only one tig buffer can be returned at a time by this function,
	// Since we don't want to memory manage
	static TigBuffer buffer { 0, };

	auto texture = gfx::textureManager->GetById(textureId);
	if (!texture->IsValid()) {
		return 17;
	}

	auto deviceTexture = texture->GetDeviceTexture();
	if (!deviceTexture) {
		// Loading failed...
		return 17;
	}

	textureOut->comes_from_mdf = false; // Irrelevant
	textureOut->textureId = textureId;
	textureOut->name[0] = 0; // Never read

	auto size = texture->GetSize();
	textureOut->width = size.width;
	textureOut->height = size.height;

	const auto &rect = texture->GetContentRect();
	textureOut->rect.x = rect.x;
	textureOut->rect.y = rect.y;
	textureOut->rect.width = rect.width;
	textureOut->rect.height = rect.height;

	textureOut->buffer = &buffer;
	if (buffer.d3dtexture) {
		// DeleteTextureAdapter(buffer.d3dtexture); // TODO: Ownership semantics...
	}
	buffer.d3dtexture = CreateTextureAdapter(deviceTexture);
	buffer.texturewidth = size.width;
	buffer.textureheight = size.height;

	return 0;
}
