#include <sstream>

#include "infrastructure/materials.h"
#include "infrastructure/vfs.h"
#include "infrastructure/exception.h"
#include "infrastructure/stringutil.h"

namespace gfx {

	MdfMaterialFactory* gMdfMaterialFactory = nullptr;

	/**
		This class is used to represent an invalid material, which frees
		clients of the material manager from checking every result for null,
		but still allows an invalid material to be identified as such by
		calling the IsValid() method.
	*/
	class InvalidMaterial : public Material {
	public:
		std::string GetName() const override {
			static std::string name("<invalid>");
			return name;
		}

		TextureRef GetPrimaryTexture() override {
			return TextureRef(); // Null texture
		}

		bool IsValid() override {
			return false;
		}
	};

	const MaterialRef& Material::GetInvalidMaterial() {
		static MaterialRef result(std::make_shared<InvalidMaterial>());
		return result;
	}

}
