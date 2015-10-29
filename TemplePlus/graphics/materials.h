#pragma once

#include <infrastructure/materials.h>
#include <infrastructure/mdfmaterial.h>

/*
	Wrapper around a MDF specification for rendering it.
*/

/*
	Used as placeholders for special material such as HEAD, GLOVES, etc.
*/
enum class MaterialPlaceholderSlot : uint32_t {
	HEAD,
	GLOVES,
	CHEST,
	BOOTS
};

class MaterialPlaceholder : public gfx::Material {
public:
	explicit MaterialPlaceholder(gfx::LegacyShaderId id, 
								 MaterialPlaceholderSlot slot,
	                             const std::string& name)
		: mId(id), mSlot(slot), mName(name) {
	}

	gfx::LegacyShaderId GetId() const override {
		return mId;
	}

	std::string GetName() const override {
		return mName;
	}

	gfx::TextureRef GetPrimaryTexture() override {
		return gfx::Texture::GetInvalidTexture();
	}

private:
	const gfx::LegacyShaderId mId;
	const MaterialPlaceholderSlot mSlot;
	const std::string mName;
};

class MdfRenderMaterial : public gfx::Material {
public:
	MdfRenderMaterial(gfx::LegacyShaderId id,
	                  const std::string& name,
	                  std::unique_ptr<class gfx::MdfMaterial>&& spec);

	gfx::LegacyShaderId GetId() const override {
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
	const gfx::LegacyShaderId mId;
	const std::string mName;
	std::unique_ptr<class gfx::MdfMaterial> mSpec;
	gfx::TextureRef mTextures[MaxTextures]; // Hold one texture ref per sampler
	gfx::TextureRef mGlossMap;

	void LoadTexturedState(gfx::MdfTexturedMaterial* material);
	void LoadGeneralState(gfx::MdfGeneralMaterial* material);
};

class MdfMaterialFactory : public gfx::MdfMaterialFactory {
public:
	MdfMaterialFactory();
	~MdfMaterialFactory();

	gfx::MaterialRef GetById(int id);

	gfx::MaterialRef LoadMaterial(const std::string& name) override;
private:
	std::unordered_map<gfx::LegacyShaderId, gfx::MaterialRef> mIdRegistry;
	std::unordered_map<std::string, gfx::MaterialRef> mNameRegistry;
	int mNextFreeId = 1;
};
