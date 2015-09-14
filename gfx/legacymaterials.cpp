
#include "stdafx.h"

#include "tig/tig_shader.h"

#include <materials.h>
#include <textures.h>

#include "legacytextures.h"
#include "legacymaterials.h"

LegacyMaterialManager::LegacyMaterialManager(LegacyTextureManager& textures) : mTextures(textures) {
}

class LegacyMaterial : public gfx::Material {
public:
	
	LegacyMaterial(int id) : mId(id) {
	}

	std::string GetName() const override {
		auto entry = shaderRegistry.get(mId);
		if (entry) {
			return entry->name;
		}
		return string();
	}

	gfx::TextureRef GetPrimaryTexture() override {
		TigShader shader;
		if (shaderFuncs.GetLoaded(mId, &shader) != 0) {
			logger->warn("Unable to load the TIG shader {}: {}", mName, mId);
		}

		int textureId;
		shader.GetTextureId(shader.data, &textureId);
		return make_shared<LegacyTexture>(textureId);
	}

private:
	int mId;
	std::string mName;
};

gfx::MaterialRef LegacyMaterialManager::Resolve(const std::string& name) {

	int shaderId;
	int result;

	if ((result = shaderFuncs.GetId(name.c_str(), &shaderId)) != 0) {
		auto message = fmt::format("Unable to load tig_shader {}: {}", name, result);
		return GetInvalidMaterial();
	}

	return make_shared<LegacyMaterial>(shaderId);

}
