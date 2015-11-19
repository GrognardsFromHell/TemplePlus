
#include "platform/d3d.h"

#include "infrastructure/mdfparser.h"
#include "infrastructure/vfs.h"
#include "infrastructure/logging.h"
#include "infrastructure/stringutil.h"

#include "graphics/mdfmaterials.h"
#include "graphics/shaders.h"
#include "graphics/device.h"

using namespace DirectX;

namespace gfx {

	enum MdfShaderRegisters {
		// Projection Matrix (in ToEE's case this is viewProj)
		MDF_REG_VIEWPROJ = 0,
		MDF_REG_MATDIFFUSE = 4,
		MDF_REG_UVANIMTIME = 5,
		MDF_REG_UVROTATION = 6,

		// Lighting related registers
		MDF_REG_LIGHT_POS = 100,
		MDF_REG_LIGHT_DIR = 108,
		MDF_REG_LIGHT_AMBIENT = 116,
		MDF_REG_LIGHT_DIFFUSE = 124,
		MDF_REG_LIGHT_SPECULAR = 132,
		MDF_REG_LIGHT_RANGE = 140,
		MDF_REG_LIGHT_ATTENUATION = 148,
		MDF_REG_LIGHT_SPOT = 156,
		MDF_REG_LIGHT_SPECULARENABLE = 164,
		MDF_REG_LIGHT_SPECULARPOWER = 165,
		MDF_REG_LIGHT_MAT_SPECULAR = 166,
		MDF_REG_LIGHT_COUNT = 167
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
		gsl::array_view<Light3d> lights,
		const MdfRenderOverrides *overrides) {

		device.SetMaterial(mDeviceMaterial);

		BindShader(device, lights, overrides);

	}

	void MdfRenderMaterial::BindShader(RenderingDevice &device,
		gsl::array_view<Light3d> lights,
		const MdfRenderOverrides *overrides) const {

		auto d3d = device.GetDevice();
		
		d3d->SetVertexShaderConstantF(MDF_REG_VIEWPROJ,
			&device.GetCamera().GetViewProj()._11,
			4);

		// Set material diffuse color for shader
		DirectX::XMFLOAT4 floats;
		if (overrides && overrides->overrideColor) {
			floats = D3DColorToFloat4(overrides->overrideColor);
		} else {
			floats = D3DColorToFloat4(mSpec->diffuse);
		}
		d3d->SetVertexShaderConstantF(MDF_REG_MATDIFFUSE, &floats.x, 1);

		// Set time for UV animation in minutes as a floating point number
		auto animTime = device.GetLastFrameStart() - device.GetDeviceCreated();
		auto timeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(animTime).count();
		floats.x = timeInMs / 60000.0f;
		// Clamp to [0, 1]
		if (floats.x > 1) {
			floats.x -= floor(floats.x);
		}
		d3d->SetVertexShaderConstantF(MDF_REG_UVANIMTIME, &floats.x, 1);

		// Swirl is more complicated due to cos/sin involvement
		// This means speedU is in "full rotations every 60 seconds" -> RPM
		for (size_t i = 0; i < mSpec->samplers.size(); ++i) {
			auto& sampler = mSpec->samplers[i];
			if (sampler.uvType != MdfUvType::Swirl) {
				continue;
			}
			DirectX::XMFLOAT4 uvRot;
			uvRot.x = cosf(sampler.speedU * floats.x * XM_2PI) * 0.1f;
			uvRot.y = sinf(sampler.speedV * floats.x * XM_2PI) * 0.1f;
			d3d->SetVertexShaderConstantF(MDF_REG_UVROTATION + i, &uvRot.x, 1);
		}

		if (!mSpec->notLit) {
			auto ignoreLighting = overrides && overrides->ignoreLighting;
			BindVertexLighting(device, lights, ignoreLighting);
		}
	}

	void MdfRenderMaterial::BindVertexLighting(RenderingDevice &device,
		gsl::array_view<Light3d> lights,
		bool ignoreLighting) const {

		auto d3d = device.GetDevice();

		// To make indexing in the HLSL shader more efficient, we sort the
		// lights here in the following order: directional, point lights, spot lights
		std::array<DirectX::XMFLOAT4A, 8> lightPos;
		std::array<DirectX::XMFLOAT4A, 8> lightDir;
		std::array<DirectX::XMFLOAT4A, 8> lightAmbient;
		std::array<DirectX::XMFLOAT4A, 8> lightDiffuse;
		std::array<DirectX::XMFLOAT4A, 8> lightSpecular;
		std::array<DirectX::XMFLOAT4A, 8> lightRange;
		std::array<DirectX::XMFLOAT4A, 8> lightAttenuation;
		std::array<DirectX::XMFLOAT4A, 8> lightSpot;

		auto directionalCount = 0;
		auto pointCount = 0;
		auto spotCount = 0;

		// In order to not have to use a different shader, we instead
		// opt for using a diffuse light with ambient 1,1,1,1
		if (ignoreLighting) {
			directionalCount = 1;
			lightDir[0] = XMFLOAT4A(0, 0, 0, 0);
			lightDiffuse[0] = XMFLOAT4A(0, 0, 0, 0);
			lightSpecular[0] = XMFLOAT4A(0, 0, 0, 0);

			lightAmbient[0].x = 1;
			lightAmbient[0].y = 1;
			lightAmbient[0].z = 1;
			lightAmbient[0].w = 0;
		} else {
			// Count the number of lights of each type
			for (auto& light : lights) {
				if (light.type == Light3dType::Directional)
					++directionalCount;
				else if (light.type == Light3dType::Point)
					++pointCount;
				else if (light.type == Light3dType::Directional)
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
				else if (light.type == Light3dType::Directional)
					lightIdx = spotFirstIdx + spotCount++;
				else
					continue;

				lightPos[lightIdx].x = light.pos.x;
				lightPos[lightIdx].y = light.pos.y;
				lightPos[lightIdx].z = light.pos.z;

				lightDir[lightIdx].x = light.dir.x;
				lightDir[lightIdx].y = light.dir.y;
				lightDir[lightIdx].z = light.dir.z;

				lightAmbient[lightIdx].x = 0;
				lightAmbient[lightIdx].y = 0;
				lightAmbient[lightIdx].z = 0;
				lightAmbient[lightIdx].w = 0;

				lightDiffuse[lightIdx].x = light.color.x;
				lightDiffuse[lightIdx].y = light.color.y;
				lightDiffuse[lightIdx].z = light.color.z;
				lightDiffuse[lightIdx].w = 0;

				lightSpecular[lightIdx].x = light.color.x;
				lightSpecular[lightIdx].y = light.color.y;
				lightSpecular[lightIdx].z = light.color.z;
				lightSpecular[lightIdx].w = 0;

				lightRange[lightIdx].x = light.range;

				lightAttenuation[lightIdx].x = 0;
				lightAttenuation[lightIdx].y = 0;
				lightAttenuation[lightIdx].z = 4.0f / (light.range * light.range);
				lightSpot[lightIdx].x = cosf(XMConvertToRadians(light.phi) * 0.60000002f * 0.5f);
				lightSpot[lightIdx].y = cosf(XMConvertToRadians(light.phi) * 0.5f);
				lightSpot[lightIdx].z = 0;
			}
		}

		// Copy the lighting state to the VS
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_POS, &lightPos[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_DIR, &lightDir[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_AMBIENT, &lightAmbient[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_DIFFUSE, &lightDiffuse[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_SPECULAR, &lightSpecular[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_RANGE, &lightRange[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_ATTENUATION, &lightAttenuation[0].x, 8);
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_SPOT, &lightSpot[0].x, 8);

		DirectX::XMFLOAT4A specularEnable = { mSpec->specular ? 1.0f : 0, 0, 0, 0 };
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_SPECULARENABLE, &specularEnable.x, 1);
		DirectX::XMFLOAT4A matPower = { mSpec->specularPower, 0, 0, 0 };
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_SPECULARPOWER, &matPower.x, 1);

		// Set the specular color
		DirectX::XMFLOAT4A matSpecular;
		matSpecular.x = GetD3DColorRed(mSpec->specular) / 255.f;
		matSpecular.y = GetD3DColorGreen(mSpec->specular) / 255.f;
		matSpecular.z = GetD3DColorBlue(mSpec->specular) / 255.f;
		matSpecular.w = GetD3DColorAlpha(mSpec->specular) / 255.f;
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_MAT_SPECULAR, &matSpecular.x, 1);

		DirectX::XMFLOAT4A lightCountsVec;
		lightCountsVec.x = (float)directionalCount;
		lightCountsVec.y = lightCountsVec.x + pointCount;
		lightCountsVec.z = lightCountsVec.y + spotCount;
		d3d->SetVertexShaderConstantF(MDF_REG_LIGHT_COUNT, &lightCountsVec.x, 1);

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

	MdfRenderMaterialPtr MdfMaterialFactory::LoadMaterial(const std::string& name) {

		auto nameLower = tolower(name);

		auto it = mNameRegistry.find(nameLower);
		if (it != mNameRegistry.end()) {
			return it->second;
		}

		try {
			auto mdfContent = vfs->ReadAsString(name);
			gfx::MdfParser parser(name, mdfContent);
			auto mdfMaterial(parser.Parse());

			Expects(mNextFreeId < 0x80000000);
			// Assign ID
			auto id = mNextFreeId++;

			auto material(CreateDeviceMaterial(name, *mdfMaterial));

			auto result(std::make_shared<MdfRenderMaterial>(id, name, std::move(mdfMaterial), material));

			mIdRegistry[id] = result;
			mNameRegistry[nameLower] = result;

			return result;
		} catch (std::exception& e) {
			logger->error("Unable to load MDF file '{}': {}", name, e.what());
			return nullptr;
		}

	}

	Material MdfMaterialFactory::CreateDeviceMaterial(const std::string &name, MdfMaterial & spec)
	{
		RasterizerState rasterizerState;

		// Wireframe mode
		if (spec.wireframe) {
			rasterizerState.fillMode = D3DFILL_WIREFRAME;
		}

		// Cull mode
		if (!spec.faceCulling) {
			rasterizerState.cullMode = D3DCULL_NONE;
		}

		BlendState blendState;

		switch (spec.blendType) {
		case MdfBlendType::Alpha:
			blendState.blendEnable = true;
			blendState.srcBlend = D3DBLEND_SRCALPHA;
			blendState.destBlend = D3DBLEND_INVSRCALPHA;
			break;
		case MdfBlendType::Add:
			blendState.blendEnable = true;
			blendState.srcBlend = D3DBLEND_ONE;
			blendState.destBlend = D3DBLEND_ONE;
			break;
		case MdfBlendType::AlphaAdd:
			blendState.blendEnable = true;
			blendState.srcBlend = D3DBLEND_SRCALPHA;
			blendState.destBlend = D3DBLEND_ONE;
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

		DepthStencilState depthStencilState;
		depthStencilState.depthEnable = !spec.disableZ;
		depthStencilState.depthWrite = spec.enableZWrite;
		depthStencilState.depthFunc = D3DCMP_LESSEQUAL;

		// Resolve texture references based on type
		Expects(spec.samplers.size() <= 4);

		Shaders::ShaderDefines psDefines;
		Shaders::ShaderDefines vsDefines;

		std::vector<MaterialSamplerBinding> samplers;
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

			SamplerState samplerState;
			// Set up the addressing
			if (spec.clamp) {
				samplerState.addressU = D3DTADDRESS_CLAMP;
				samplerState.addressV = D3DTADDRESS_CLAMP;
			}

			// Set up filtering
			if (spec.linearFiltering) {
				samplerState.magFilter = D3DTEXF_LINEAR;
				samplerState.minFilter = D3DTEXF_LINEAR;
				samplerState.mipFilter = D3DTEXF_LINEAR;
			} else {
				samplerState.magFilter = D3DTEXF_POINT;
				samplerState.minFilter = D3DTEXF_POINT;
				samplerState.mipFilter = D3DTEXF_POINT;
			}
			
			samplers.emplace_back(MaterialSamplerBinding(texture, samplerState));

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

		PixelShaderPtr pixelShader(mDevice.GetShaders().LoadPixelShader("mdf_ps", psDefines));

		// Select the right vertex shader to use
		auto vertexShader(mDevice.GetShaders().LoadVertexShader("mdf_vs", vsDefines));

		if (!pixelShader || !vertexShader) {
			throw TempleException("Unable to create MDF material {} due to missing shaders.",
				name);
		}

		return Material(blendState, depthStencilState, rasterizerState, samplers, vertexShader, pixelShader);

	}

}
