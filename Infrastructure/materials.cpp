
#include "infrastructure/materials.h"
#include "infrastructure/logging.h"

namespace gfx {

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

	std::unique_ptr<MaterialFactory> materialManager;
	
	MaterialFactory::~MaterialFactory() {
	}

	MaterialRef MaterialFactory::GetInvalidMaterial() {
		static MaterialRef result(new InvalidMaterial);
		return result;
	}

}
