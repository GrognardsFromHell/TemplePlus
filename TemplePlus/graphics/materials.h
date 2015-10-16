#pragma once

#include <infrastructure/materials.h>
#include <infrastructure/mdfmaterial.h>

/*
	Wrapper around a MDF specification for rendering it.
*/
using LegacyShaderId = int;

class MdfRenderMaterial : public gfx::Material {
public:
	MdfRenderMaterial(LegacyShaderId id,
	                  const std::string& name,
	                  std::unique_ptr<class gfx::MdfMaterial>&& spec);

	LegacyShaderId GetId() const {
		return mId;
	}

	std::string GetName() const override {
		return mName;
	}

	gfx::TextureRef GetPrimaryTexture() override;

	const gfx::MdfMaterial* GetSpec() const {
		return mSpec.get();
	}

	gsl::array_view<const gfx::TextureRef, 4> GetTextures() const {
		return mTextures;
	}

	const gfx::TextureRef& GetGlossMap() const {
		return mGlossMap;
	}

private:
	static constexpr size_t MaxTextures = 4;
	const LegacyShaderId mId;
	const std::string mName;
	std::unique_ptr<class gfx::MdfMaterial> mSpec;
	gfx::TextureRef mTextures[MaxTextures]; // Hold one texture ref per sampler
	gfx::TextureRef mGlossMap;

	void LoadTexturedState(gfx::MdfTexturedMaterial* material);
	void LoadGeneralState(gfx::MdfGeneralMaterial* material);
};

class MdfMaterialFactory : public gfx::MdfMaterialFactory {
public:

	~MdfMaterialFactory();

	gfx::MaterialRef GetById(int id);

	gfx::MaterialRef LoadMaterial(const std::string& name) override;
private:
	std::unordered_map<LegacyShaderId, std::shared_ptr<MdfRenderMaterial>> mIdRegistry;
	std::unordered_map<std::string, std::shared_ptr<MdfRenderMaterial>> mNameRegistry;
	int mNextFreeId = 1;
};
