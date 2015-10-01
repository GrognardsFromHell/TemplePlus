#include <sstream>

#include "infrastructure/materials.h"
#include "infrastructure/vfs.h"
#include "infrastructure/exception.h"
#include "infrastructure/stringutil.h"

namespace gfx {

	std::unique_ptr<MdfMaterialFactory> gMdfMaterialFactory;

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

	MaterialRef MdfMaterialFactory::LoadMaterial(const std::string& name) {
		auto mdfContent(vfs->ReadAsString(name));

		std::istringstream in(mdfContent);
		std::string line;


		// Parse the material type

		while (std::getline(in, line)) {

		}


		// First line of MDF is always which type it is


		return MaterialRef();
	}

}
