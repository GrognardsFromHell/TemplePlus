#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace gfx {

	enum class MdfType {
		Textured,
		General,
		Clipper
	};
	
	enum class MdfTextureBlendType : uint32_t {
		Modulate,
		Add,
		TextureAlpha,
		CurrentAlpha,
		CurrentAlphaAdd
	};

	enum class MdfBlendType {
		None,
		Alpha,
		Add,
		AlphaAdd
	};

	enum class MdfUvType {
		Mesh,
		Environment,
		Drift,
		Swirl,
		Wavey
	};

	struct MdfGeneralMaterialSampler {
		float speedU = 60.0f;
		float speedV = 0;
		std::string filename;
		MdfTextureBlendType blendType = MdfTextureBlendType::Modulate;
		MdfUvType uvType = MdfUvType::Mesh;
	};

	class MdfMaterial {
	public:
		explicit MdfMaterial(MdfType type) : type(type), samplers(4) {
		}

		const MdfType type;
		MdfBlendType blendType = MdfBlendType::Alpha;
		float specularPower = 50.0f;
		uint32_t specular = 0;
		uint32_t diffuse = 0xFFFFFFFF;
		std::string glossmap; // Filename of glossmap texture
		bool faceCulling = true;
		bool linearFiltering = false;
		bool recalculateNormals = false;
		bool enableZWrite = true;
		bool disableZ = false;
		bool enableColorWrite = true;
		bool notLit = false;
		bool clamp = false;
		bool outline = false; // Ignored during rendering
		bool wireframe = false; // Ignored during rendering
		std::vector<MdfGeneralMaterialSampler> samplers;
	};
	
}
