
#pragma once

#include <materials.h>

class LegacyTextureManager;

/*
Loads materials by really loading tig_shader's
*/
class LegacyMaterialManager : public gfx::MaterialManager {
public:

	explicit LegacyMaterialManager(LegacyTextureManager& textures);

	gfx::MaterialRef Resolve(const std::string& materialName) override;

private:
	LegacyTextureManager &mTextures;
};
