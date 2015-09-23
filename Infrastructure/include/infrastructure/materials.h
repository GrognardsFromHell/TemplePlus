#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "textures.h"

namespace gfx {

	/*
		This is basically a wrapper around tig_shader
	*/
	class Material {
	public:
		virtual ~Material() {
		}

		virtual std::string GetName() const = 0;

		virtual bool IsValid() {
			return true;
		}

		virtual TextureRef GetPrimaryTexture() = 0;
	};

	using MaterialRef = std::shared_ptr<Material>;

	class MaterialManager {
	public:

		virtual ~MaterialManager();

		virtual MaterialRef Resolve(const std::string& materialName) = 0;

		static MaterialRef GetInvalidMaterial();

	};

	using MaterialManagerPtr = std::shared_ptr<MaterialManager>;
	
}
