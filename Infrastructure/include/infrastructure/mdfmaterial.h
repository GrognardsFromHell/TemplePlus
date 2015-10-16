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

	class MdfMaterial {
	public:
		virtual ~MdfMaterial() {
		}

		virtual MdfType GetType() const = 0;
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

	class MdfGeneralMaterial : public MdfMaterial {
	public:

		MdfGeneralMaterial() : samplers(4) {
		}

		MdfType GetType() const override {
			return MdfType::General;
		}

		MdfBlendType blendType = MdfBlendType::Alpha;
		float specularPower = 50.0f;
		uint32_t specular = 0;
		uint32_t diffuse = 0xFFFFFFFF;
		std::string glossmap; // Filename of glossmap texture
		bool faceCulling = true;
		bool linearFiltering = false;
		bool recalculateNormals = false;
		bool zFillOnly = false;
		bool disableZ = false;
		bool colorFillOnly = false;
		bool notLit = false;		
		std::vector<MdfGeneralMaterialSampler> samplers;		
	};

	/*
	This material type is used to draw the depth art geometry,
	which fills the zbuffer with the appropriate depth values
	for the geometry that is visible on the pre-rendered
	backgrounds.
	*/
	class MdfClipperMaterial : public MdfMaterial {
	public:

		MdfClipperMaterial(bool zFill, bool outline, bool wireframe)
			: mZFill(zFill),
			  mOutline(outline),
			  mWireframe(wireframe) {
		}

		const bool& GetZFill() const {
			return mZFill;
		}

		const bool& GetOutline() const {
			return mOutline;
		}

		const bool& GetWireframe() const {
			return mWireframe;
		}

		MdfType GetType() const override {
			return MdfType::Clipper;
		}

	private:
		const bool mZFill;
		const bool mOutline;
		const bool mWireframe;
	};

	class MdfTexturedMaterial : public MdfMaterial {
	public:
		const std::string& GetTexture() const {
			return mTexture;
		}

		void SetTexture(const std::string& texture) {
			mTexture = texture;
		}

		const uint32_t& GetDiffuse() const {
			return mDiffuse;
		}

		void SetDiffuse(uint32_t diffuse) {
			mDiffuse = diffuse;
		}

		const bool& GetColorFillOnly() const {
			return mColorFillOnly;
		}

		void SetColorFillOnly(bool colorFillOnly) {
			mColorFillOnly = colorFillOnly;
		}

		const bool& GetNotLit() const {
			return mNotLit;
		}

		void SetNotLit(bool notLit) {
			mNotLit = notLit;
		}

		const bool& GetDisableZ() const {
			return mDisableZ;
		}

		void SetDisableZ(bool disableZ) {
			mDisableZ = disableZ;
		}

		const bool& GetDouble() const {
			return mDouble;
		}

		void SetDouble(bool double_) {
			mDouble = double_;
		}

		const bool& GetClamp() const {
			return mClamp;
		}

		void SetClamp(bool clamp) {
			mClamp = clamp;
		}

		MdfType GetType() const override {
			return MdfType::Textured;
		}

	private:

		std::string mTexture;
		uint32_t mDiffuse = 0xFFFFFFFF;
		bool mColorFillOnly = false;
		bool mNotLit = false;
		bool mDisableZ = false;
		bool mDouble = false;
		bool mClamp = false;
	};

}
