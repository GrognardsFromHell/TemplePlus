
#pragma once

#include <string>

#include "graphics/math.h"

#include "materials.h"

namespace gfx {

	struct MdfRenderOverrides {
		bool ignoreLighting = false;
		bool overrideDiffuse = false;
		XMCOLOR overrideColor = 0;
		float alpha = 1;
		bool useWorldMatrix = false;
		XMFLOAT4X4 worldMatrix;
	};

	/*
		Wrapper around a MDF specification for rendering it.
	*/
	class RenderingDevice;

	/*
	Used as placeholders for special material such as HEAD, GLOVES, etc.
	*/
	enum class MaterialPlaceholderSlot {
		HEAD,
		GLOVES,
		CHEST,
		BOOTS
	};

	/**
	* Stores rendering engine resources for a single MdfMaterial.
	* Most importantly those are the texture handles used for the
	* samplers specified in the MDF.
	*/
	using LegacyShaderId = int;

	class MdfRenderMaterial {
	public:
		MdfRenderMaterial(LegacyShaderId id,
			const std::string& name,
			std::unique_ptr<class MdfMaterial>&& spec,
			const Material &material);

		LegacyShaderId GetId() const {
			return mId;
		}

		std::string GetName() const {
			return mName;
		}

		TextureRef GetPrimaryTexture();

		const TextureRef& GetGlossMap() const {
			return mGlossMap;
		}

		const MdfMaterial* GetSpec() const {
			return mSpec.get();
		}

		void Bind(RenderingDevice& g, 
			gsl::array_view<Light3d> lights, 
			const MdfRenderOverrides *overrides = nullptr);

	private:
		static constexpr size_t MaxTextures = 4;
		const LegacyShaderId mId;
		const std::string mName;
		std::unique_ptr<class MdfMaterial> mSpec;
		TextureRef mGlossMap;
		Material mDeviceMaterial;

		void BindShader(RenderingDevice &device,
			gsl::array_view<Light3d> lights,
			const MdfRenderOverrides *overrides) const;
		void BindVertexLighting(RenderingDevice &device,
			gsl::array_view<Light3d> lights,
			bool ignoreLighting) const;
	};

	using MdfRenderMaterialPtr = std::shared_ptr<MdfRenderMaterial>;

	class MdfMaterialFactory {
	public:
		MdfMaterialFactory(RenderingDevice &device);
		~MdfMaterialFactory();

		MdfRenderMaterialPtr GetById(int id);

		MdfRenderMaterialPtr LoadMaterial(const std::string& name);

		// Retrieves a material replacement set from rules/materials.mes
		using ReplacementSet = std::map<MaterialPlaceholderSlot, MdfRenderMaterialPtr>;
		const ReplacementSet& GetReplacementSet(uint32_t id);
		void LoadReplacementSets(const std::string &filename);

	private:
		std::unordered_map<LegacyShaderId, MdfRenderMaterialPtr> mIdRegistry;
		std::unordered_map<std::string, MdfRenderMaterialPtr> mNameRegistry;
		RenderingDevice &mDevice;
		Textures& mTextures;
		int mNextFreeId = 1;

		// Loaded from rules/materials.mes
		std::unordered_map<uint32_t, ReplacementSet> mReplacementSets;
		void AddReplacementSetEntry(uint32_t id, const std::string &entry);

		Material CreateDeviceMaterial(const std::string &name, MdfMaterial &spec);
	};

}
