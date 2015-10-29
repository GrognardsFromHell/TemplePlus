#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "textures.h"

namespace gfx {

	using LegacyShaderId = int;

	/**
	Ownership of materials is generally shared between all those places that the
	materials are referenced by.
	*/
	using MaterialRef = std::shared_ptr<class Material>;
	
	class Material {
	public:
		virtual ~Material() {
		}
		
		virtual LegacyShaderId GetId() const = 0;

		virtual std::string GetName() const = 0;

		virtual bool IsValid() {
			return true;
		}

		static const MaterialRef &GetInvalidMaterial();

		virtual TextureRef GetPrimaryTexture() = 0;
	};

	class MdfMaterialFactory {
	public:
		virtual ~MdfMaterialFactory() {}

		virtual MaterialRef LoadMaterial(const std::string& name) = 0;
	};

	extern MdfMaterialFactory* gMdfMaterialFactory;
	
}
