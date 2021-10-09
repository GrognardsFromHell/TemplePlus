
#include "platform/d3d.h"

#include "infrastructure/mdfparser.h"
#include "infrastructure/mesparser.h"
#include "infrastructure/vfs.h"
#include "infrastructure/logging.h"
#include "infrastructure/stringutil.h"

#include "graphics/mdfmaterials.h"
#include "graphics/shaders.h"
#include "graphics/device.h"

using namespace DirectX;

namespace gfx {

	/*
		Please note that the restrictive rules on constant buffer packing make it much easier to just
		keep using XMFLOAT4's although they are oversized. We might want to optimize the packing here by
		hand later.
	*/
	struct MdfGlobalConstants {
		XMFLOAT4X4 viewProj;
		XMFLOAT4 matDiffuse;
		XMFLOAT4 uvAnimTime;
		XMFLOAT4 uvRotation[4]; // One per texture stage

		static const uint32_t MaxLights = 8;

		// Lighting related
		XMFLOAT4 lightPos[MaxLights];
		XMFLOAT4 lightDir[MaxLights];
		XMFLOAT4 lightAmbient[MaxLights];
		XMFLOAT4 lightDiffuse[MaxLights];
		XMFLOAT4 lightSpecular[MaxLights];
		XMFLOAT4 lightRange[MaxLights];
		XMFLOAT4 lightAttenuation[MaxLights]; //1, D, D^2;
		XMFLOAT4 lightSpot[MaxLights]; //cos(theta/2), cos(phi/2), falloff
		
		XMINT4 bSpecular;
		XMFLOAT4 fMaterialPower;
		XMFLOAT4 matSpecular;
		uint32_t lightCount[4]; // Directional, point, spot
	};

	enum MdfShaderRegisters {
		// Projection Matrix (in ToEE's case this is viewProj)
		MDF_REG_VIEWPROJ = 0,
		MDF_REG_MATDIFFUSE = 4,
		MDF_REG_UVANIMTIME = 5,
		MDF_REG_UVROTATION = 6,

		// Lighting related registers
		MDF_REG_LIGHT_POS = 10,
		MDF_REG_LIGHT_DIR = 18,
		MDF_REG_LIGHT_AMBIENT = 26,
		MDF_REG_LIGHT_DIFFUSE = 34,
		MDF_REG_LIGHT_SPECULAR = 42,
		MDF_REG_LIGHT_RANGE = 50,
		MDF_REG_LIGHT_ATTENUATION = 58,
		MDF_REG_LIGHT_SPOT = 66,
		MDF_REG_LIGHT_SPECULARENABLE = 74,
		MDF_REG_LIGHT_SPECULARPOWER = 75,
		MDF_REG_LIGHT_MAT_SPECULAR = 76,
		MDF_REG_LIGHT_COUNT = 77
	};

	MdfRenderMaterial::MdfRenderMaterial(gfx::LegacyShaderId id,
		const std::string& name,
		std::unique_ptr<class gfx::MdfMaterial>&& spec,
		const Material& material)
		: mId(id), mName(name), mSpec(std::move(spec)), mDeviceMaterial(material) {
	}

	gfx::TextureRef MdfRenderMaterial::GetPrimaryTexture() {
		if (mDeviceMaterial.GetSamplers().empty()) {
			return nullptr;
		}
		return mDeviceMaterial.GetSamplers()[0].GetTexture();
	}

	void MdfRenderMaterial::Bind(RenderingDevice& device, 
		gsl::span<Light3d> lights,
		const MdfRenderOverrides *overrides) {

		device.SetMaterial(mDeviceMaterial);

		BindShader(device, lights, overrides);

	}

	void MdfRenderMaterial::BindShader(RenderingDevice &device,
		gsl::span<Light3d> lights,
		const MdfRenderOverrides *overrides) const {

		// Fill out the globals for the shader
		MdfGlobalConstants globals;

		XMMATRIX viewProj;
		if (overrides && overrides->uiProjection) {
			viewProj = XMLoadFloat4x4(&device.GetCamera().GetUiProjection());
		} else {
			viewProj = XMLoadFloat4x4(&device.GetCamera().GetViewProj());
		}

		// Should we use a separate world matrix?
		if (overrides && overrides->useWorldMatrix) {
			// Build a world * view * proj matrix
			auto worldViewProj{ XMLoadFloat4x4(&overrides->worldMatrix) * viewProj };
			XMStoreFloat4x4(&globals.viewProj, worldViewProj);
		} else {
			XMStoreFloat4x4(&globals.viewProj, viewProj);
		}
		
		// Set material diffuse color for shader
		XMVECTOR color;
		if (overrides && overrides->overrideColor) {
			 color = XMLoadColor(&overrides->overrideColor);
		} else {
			color = XMLoadColor(&XMCOLOR(mSpec->diffuse));
		}
		XMStoreFloat4(&globals.matDiffuse, color);
		if (overrides && overrides->alpha != 1.0f) {
			globals.matDiffuse.w *= overrides->alpha;
		}
		
		// Set time for UV animation in minutes as a floating point number
		auto animTime = device.GetLastFrameStart() - device.GetDeviceCreated();
		auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(animTime).count();
		globals.uvAnimTime.x = timeInMs / 60000.0f;
		// Clamp to [0, 1]
		if (globals.uvAnimTime.x > 1) {
			globals.uvAnimTime.x -= floor(globals.uvAnimTime.x);
		}

		// Swirl is more complicated due to cos/sin involvement
		// This means speedU is in "full rotations every 60 seconds" -> RPM
		for (size_t i = 0; i < mSpec->samplers.size(); ++i) {
			auto& sampler = mSpec->samplers[i];
			if (sampler.uvType != MdfUvType::Swirl) {
				continue;
			}
			auto &uvRot = globals.uvRotation[i];
			uvRot.x = cosf(sampler.speedU * globals.uvAnimTime.x * XM_2PI) * 0.1f;
			uvRot.y = sinf(sampler.speedV * globals.uvAnimTime.x * XM_2PI) * 0.1f;
		}

		if (!mSpec->notLit) {
			auto ignoreLighting = overrides && overrides->ignoreLighting;
			BindVertexLighting(globals, lights, ignoreLighting);
		}

		device.SetVertexShaderConstants(0, globals);
	}

	void MdfRenderMaterial::BindVertexLighting(MdfGlobalConstants &globals,
		gsl::span<Light3d> lights,
		bool ignoreLighting) const {

		constexpr auto MaxLights = MdfGlobalConstants::MaxLights;

		if (lights.size() > MaxLights) {
			lights = lights.subspan(0, MaxLights);
		}

		// To make indexing in the HLSL shader more efficient, we sort the
		// lights here in the following order: directional, point lights, spot lights

		auto directionalCount = 0;
		auto pointCount = 0;
		auto spotCount = 0;

		// In order to not have to use a different shader, we instead
		// opt for using a diffuse light with ambient 1,1,1,1
		if (ignoreLighting) {
			directionalCount = 1;
			globals.lightDir[0] = XMFLOAT4A(0, 0, 0, 0);
			globals.lightDiffuse[0] = XMFLOAT4A(0, 0, 0, 0);
			globals.lightSpecular[0] = XMFLOAT4A(0, 0, 0, 0);

			globals.lightAmbient[0] = XMFLOAT4A(1, 1, 1, 0);
		} else {
			// Count the number of lights of each type
			for (auto& light : lights) {
				if (light.type == Light3dType::Directional)
					++directionalCount;
				else if (light.type == Light3dType::Point)
					++pointCount;
				else if (light.type == Light3dType::Spot)
					++spotCount;
			}

			auto pointFirstIdx = directionalCount;
			auto spotFirstIdx = pointFirstIdx + pointCount;

			directionalCount = 0;
			pointCount = 0;
			spotCount = 0;

			for (auto& light : lights) {
				int lightIdx;
				if (light.type == Light3dType::Directional)
					lightIdx = directionalCount++;
				else if (light.type == Light3dType::Point)
					lightIdx = pointFirstIdx + pointCount++;
				else if (light.type == Light3dType::Spot)
					lightIdx = spotFirstIdx + spotCount++;
				else
					continue;

				globals.lightPos[lightIdx].x = light.pos.x;
				globals.lightPos[lightIdx].y = light.pos.y;
				globals.lightPos[lightIdx].z = light.pos.z;

				globals.lightDir[lightIdx].x = light.dir.x;
				globals.lightDir[lightIdx].y = light.dir.y;
				globals.lightDir[lightIdx].z = light.dir.z;

				globals.lightAmbient[lightIdx].x = light.ambient.x;
				globals.lightAmbient[lightIdx].y = light.ambient.y;
				globals.lightAmbient[lightIdx].z = light.ambient.z;
				globals.lightAmbient[lightIdx].w = 0;

				globals.lightDiffuse[lightIdx].x = light.color.x;
				globals.lightDiffuse[lightIdx].y = light.color.y;
				globals.lightDiffuse[lightIdx].z = light.color.z;
				globals.lightDiffuse[lightIdx].w = 0;

				globals.lightSpecular[lightIdx].x = light.color.x;
				globals.lightSpecular[lightIdx].y = light.color.y;
				globals.lightSpecular[lightIdx].z = light.color.z;
				globals.lightSpecular[lightIdx].w = 0;

				globals.lightRange[lightIdx].x = light.range;

				globals.lightAttenuation[lightIdx].x = 0;
				globals.lightAttenuation[lightIdx].y = 0;
				globals.lightAttenuation[lightIdx].z = 4.0f / (light.range * light.range);
				auto phiRad = XMConvertToRadians(light.phi);
				globals.lightSpot[lightIdx].x = cosf(phiRad * 0.6f * 0.5f);
				globals.lightSpot[lightIdx].y = cosf(phiRad * 0.5f);
				globals.lightSpot[lightIdx].z = 1;
			}
		}
				
		globals.bSpecular = { mSpec->specular ? 1 : 0, 0, 0, 0 };
		globals.fMaterialPower = { mSpec->specularPower, 0, 0, 0 };

		// Set the specular color
		XMStoreFloat4(&globals.matSpecular, XMLoadColor(&XMCOLOR(mSpec->specular)));

		globals.lightCount[0] = directionalCount;
		globals.lightCount[1] = globals.lightCount[0] + pointCount;
		globals.lightCount[2] = globals.lightCount[1] + spotCount;

	}

	MdfMaterialFactory::MdfMaterialFactory(RenderingDevice &device) 
		: mDevice(device), mTextures(device.GetTextures()) {
		/*
		// Register material placeholders that have special names
		auto head = std::make_shared<MaterialPlaceholder>(0x84000001, MaterialPlaceholderSlot::HEAD, "HEAD");
		mIdRegistry[head->GetId()] = head;
		mNameRegistry[tolower(head->GetName())] = head;

		auto gloves = std::make_shared<MaterialPlaceholder>(0x88000001, MaterialPlaceholderSlot::GLOVES, "GLOVES");
		mIdRegistry[gloves->GetId()] = gloves;
		mNameRegistry[tolower(gloves->GetName())] = gloves;

		auto chest = std::make_shared<MaterialPlaceholder>(0x90000001, MaterialPlaceholderSlot::CHEST, "CHEST");
		mIdRegistry[chest->GetId()] = chest;
		mNameRegistry[tolower(chest->GetName())] = chest;

		auto boots = std::make_shared<MaterialPlaceholder>(0xA0000001, MaterialPlaceholderSlot::BOOTS, "BOOTS");
		mIdRegistry[boots->GetId()] = boots;
		mNameRegistry[tolower(boots->GetName())] = boots;
		*/
	}

	MdfMaterialFactory::~MdfMaterialFactory() = default;

	MdfRenderMaterialPtr MdfMaterialFactory::GetById(int id) {
		auto it = mIdRegistry.find(id);
		if (it != mIdRegistry.end()) {
			return it->second;
		}
		return nullptr;
	}

	MdfRenderMaterialPtr MdfMaterialFactory::GetByName(const std::string &name){
		auto nameLower = tolower(name);
		auto it = mNameRegistry.find(nameLower);
		if (it != mNameRegistry.end()) {
			return it->second;
		}
		return nullptr;
	}

	

	MdfRenderMaterialPtr MdfMaterialFactory::LoadMaterial(const std::string& name, std::function<void(gfx::MdfMaterial&)> customizer) {

		auto nameLower = tolower(name);
		if (!customizer) {  // Do not cache if a customizer is present
			auto it = mNameRegistry.find(nameLower);
			if (it != mNameRegistry.end()) {
				return it->second;
			}
		}
		

		try {
			auto mdfContent = vfs->ReadAsString(name);
			gfx::MdfParser parser(name, mdfContent);
			auto mdfMaterial(parser.Parse());

			if (customizer) {
				customizer(*mdfMaterial.get());
			}

			Expects(mNextFreeId < 0x80000000);
			// Assign ID
			auto id = mNextFreeId++;

			auto material(CreateDeviceMaterial(name, *mdfMaterial));

			auto result(std::make_shared<MdfRenderMaterial>(id, name, std::move(mdfMaterial), material));

			mIdRegistry[id] = result;
			if (!customizer) {
				mNameRegistry[nameLower] = result;
			}
			

			return result;
		} catch (std::exception& e) {
			// logger->error("Unable to load MDF file '{}': {}", name, e.what()); // produces massive logspam when casting spells
			return nullptr;
		}

	}

	void MdfMaterialFactory::LoadReplacementSets(const std::string & filename)
	{

		auto mapping = MesFile::ParseFile(filename);

		for (auto &entry : mapping) {
			AddReplacementSetEntry((uint32_t) entry.first, entry.second);
		}

	}

	void MdfMaterialFactory::AddReplacementSetEntry(uint32_t id, const std::string & entry)
	{
		static std::map<std::string, MaterialPlaceholderSlot> sSlotMapping {
			{ "CHEST", MaterialPlaceholderSlot::CHEST },
			{ "BOOTS", MaterialPlaceholderSlot::BOOTS },
			{ "GLOVES", MaterialPlaceholderSlot::GLOVES },
			{ "HEAD", MaterialPlaceholderSlot::HEAD }
		};

		if (entry.empty()) {
			return;
		}

		ReplacementSet set;

		using gsl::cstring_span;
		std::vector<cstring_span<>> elems;
		std::vector<cstring_span<>> subElems;
		std::string slotName, mdfName;
		split(entry, ' ', elems, true);

		for (auto &elem : elems) {
			split(elem, ':', subElems, false);

			if (subElems.size() < 2) {
				logger->warn("Invalid material replacement set {}: {}", id, entry);
				continue;
			}

			slotName.assign(subElems[0].data(), subElems[0].size());
			mdfName.assign(subElems[1].data(), subElems[1].size());

			// Map the slot name to the enum literal for it
			auto slotIt = sSlotMapping.find(slotName);
			if (slotIt == sSlotMapping.end()) {
				logger->warn("Invalid material replacement set {}: {}", id, entry);
				continue;
			}

			// Resolve the referenced material
			auto material = LoadMaterial(mdfName);
			if (!material) {
				logger->warn("Material replacement set {} references unknown MDF: {}", id, mdfName);
				continue;
			}

			set[slotIt->second] = material;
		}

		mReplacementSets[id] = set;

	}

	const MdfMaterialFactory::ReplacementSet &MdfMaterialFactory::GetReplacementSet(uint32_t id, int fallbackId)
	{
		static const ReplacementSet sEmptyResult;

		auto it = mReplacementSets.find(id);

		if (it != mReplacementSets.end()) {
			return it->second;
		}

		if (fallbackId != -1){
			it = mReplacementSets.find((uint32_t) fallbackId);
			if (it != mReplacementSets.end()) {
				return it->second;
			}
		}

		return sEmptyResult;
	}

	Material MdfMaterialFactory::CreateDeviceMaterial(const std::string &name, MdfMaterial & spec)
	{
		RasterizerSpec rasterizerState;

		// Wireframe mode
		rasterizerState.wireframe = spec.wireframe;

		// Cull mode
		if (!spec.faceCulling) {
			rasterizerState.cullMode = CullMode::None;
		}

		BlendSpec blendState;

		switch (spec.blendType) {
		case MdfBlendType::Alpha:
			blendState.blendEnable = true;
			blendState.srcBlend = BlendOperand::SrcAlpha;
			blendState.destBlend = BlendOperand::InvSrcAlpha;
			break;
		case MdfBlendType::Add:
			blendState.blendEnable = true;
			blendState.srcBlend = BlendOperand::One;
			blendState.destBlend = BlendOperand::One;
			break;
		case MdfBlendType::AlphaAdd:
			blendState.blendEnable = true;
			blendState.srcBlend = BlendOperand::SrcAlpha;
			blendState.destBlend = BlendOperand::One;
			break;
		default:
		case MdfBlendType::None:
			break;
		}

		if (!spec.enableColorWrite) {
			blendState.writeAlpha = false;
			blendState.writeRed = false;
			blendState.writeGreen = false;
			blendState.writeBlue = false;
		}

		DepthStencilSpec depthStencilState;
		depthStencilState.depthEnable = !spec.disableZ;
		depthStencilState.depthWrite = spec.enableZWrite;
		depthStencilState.depthFunc = ComparisonFunc::LessEqual;

		// Resolve texture references based on type
		Expects(spec.samplers.size() <= 4);

		Shaders::ShaderDefines psDefines;
		Shaders::ShaderDefines vsDefines;

		std::vector<MaterialSamplerSpec> samplers;
		samplers.reserve(spec.samplers.size());
		// A general MDF can reference up to 4 textures
		for (size_t i = 0; i < spec.samplers.size(); ++i) {
			const auto &sampler = spec.samplers[i];
			if (sampler.filename.empty())
				continue;

			auto texture(mTextures.Resolve(sampler.filename, true));
			if (!texture) {
				logger->warn("General shader {} references invalid texture '{}' in sampler {}",
					name, sampler.filename, i);
			}

			SamplerSpec samplerState;
			// Set up the addressing
			if (spec.clamp) {
				samplerState.addressU = TextureAddress::Clamp;
				samplerState.addressV = TextureAddress::Clamp;
			}

			// Set up filtering
			if (spec.linearFiltering) {
				samplerState.magFilter = TextureFilterType::Linear;
				samplerState.minFilter = TextureFilterType::Linear;
				samplerState.mipFilter = TextureFilterType::Linear;
			} else {
				samplerState.magFilter = TextureFilterType::NearestNeighbor;
				samplerState.minFilter = TextureFilterType::NearestNeighbor;
				samplerState.mipFilter = TextureFilterType::NearestNeighbor;
			}
			
			samplers.push_back({ texture, samplerState });

			/*
				Set the stage's blending type in the pixel shader defines.
				The numbers correlate with the defines in mdf_ps.hlsl
			*/
			auto stageId = samplers.size();
			auto defName = fmt::format("TEXTURE_STAGE{}_MODE", stageId);
			switch (sampler.blendType) {
			default:
			case MdfTextureBlendType::Modulate:
				psDefines[defName] = "1";
				break;
			case MdfTextureBlendType::Add:
				psDefines[defName] = "2";
				break;
			case MdfTextureBlendType::TextureAlpha:
				psDefines[defName] = "3";
				break;
			case MdfTextureBlendType::CurrentAlpha:
				psDefines[defName] = "4";
				break;
			case MdfTextureBlendType::CurrentAlphaAdd:
				psDefines[defName] = "5";
				break;
			}

			/*
				If the stage requires animated texture coordinates,
				we have to set this on the vertex shader. The numbers used
				correlate with defines in mdf_vs.hlsl
			*/
			defName = fmt::format("TEXTURE_STAGE{}_UVANIM", stageId);
			vsDefines[fmt::format("TEXTURE_STAGE{}_SPEEDU", stageId)] = std::to_string(sampler.speedU);
			vsDefines[fmt::format("TEXTURE_STAGE{}_SPEEDV", stageId)] = std::to_string(sampler.speedV);
			switch (sampler.uvType) {				
			case MdfUvType::Environment:
				vsDefines[defName] = "1";
				break;
			case MdfUvType::Drift:
				vsDefines[defName] = "2";
				break;
			case MdfUvType::Swirl:
				vsDefines[defName] = "3";
				break;
			case MdfUvType::Wavey:
				vsDefines[defName] = "4";
				break;
			}
		}

		if (!spec.glossmap.empty()) {
			auto glossMap = mTextures.Resolve(spec.glossmap, true);
			if (!glossMap) {
				logger->warn("General shader {} references invalid gloss map texture '{}'",
					name, spec.glossmap);
			}
		}

		psDefines["TEXTURE_STAGES"] = std::to_string(samplers.size());
		vsDefines["TEXTURE_STAGES"] = std::to_string(samplers.size());
		if (spec.notLit) {
			vsDefines["LIGHTING"] = "0";
		} else {
			vsDefines["LIGHTING"] = "1";
		}
		vsDefines["PER_VERTEX_COLOR"] = spec.perVertexColor ? "1" : "0";

		// Special case for highlight shaders until we're able to encode this
		// in the material file itself
		if (name == "art/meshes/mouseover.mdf"
			 || name == "art/meshes/hilight.mdf"
			 || name.find("art/meshes/wg_") == 0) {
			rasterizerState.cullMode = CullMode::Front;
			vsDefines["HIGHLIGHT"] = "1";
		}

		auto pixelShader(mDevice.GetShaders().LoadPixelShader("mdf_ps", psDefines));

		// Select the right vertex shader to use
		auto vertexShader(mDevice.GetShaders().LoadVertexShader("mdf_vs", vsDefines));

		if (!pixelShader || !vertexShader) {
			throw TempleException("Unable to create MDF material {} due to missing shaders.",
				name);
		}

		return mDevice.CreateMaterial(blendState, depthStencilState, rasterizerState, samplers, vertexShader, pixelShader);

	}

}
