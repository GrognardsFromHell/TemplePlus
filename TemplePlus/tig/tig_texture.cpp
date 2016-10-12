
#include "stdafx.h"
#include "tig_texture.h"
#include <tio/tio.h>
#include "tig_startup.h"
#include <graphics/textures.h>
#include <graphics/device.h>

TextureFuncs textureFuncs;
IdxTableWrapper<TigTextureRegistryEntry> textureRegistry(0x10EF2E90);

int TextureFuncs::RegisterTexture(const char * filename, int * pTexIdOut)
{
	auto& textures = tig->GetRenderingDevice().GetTextures();
	auto ref = textures.Resolve(filename, false);

	if (!ref->IsValid()) {
		*pTexIdOut = -1;
		logger->error("Unable to register texture {}", filename);
		return 17;
	}

	*pTexIdOut = ref->GetId();
	return 0;
}

int TextureFuncs::RegisterTextureOverride(const char * filename, int * textIdOut){

	auto& textures = tig->GetRenderingDevice().GetTextures();
	auto ref = textures.Override(filename, false);

	if (!ref->IsValid()) {
		*textIdOut = -1;
		logger->error("Unable to register texture {}", filename);
		return 17;
	}

	*textIdOut = ref->GetId();

	return 0;
}
