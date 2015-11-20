#include "stdafx.h"

#include <graphics/bufferbinding.h>
#include <graphics/materials.h>

#include <infrastructure/vfs.h>
#include <infrastructure/binaryreader.h>

#include "clipping.h"
#include "graphics/shaders.h"
#include "clippingmesh.h"

using namespace gfx;

class MapClipping::Impl {
public:
	explicit Impl(RenderingDevice& g);

	std::vector<std::unique_ptr<ClippingMesh>> mClippingMeshes;

	RenderingDevice& mDevice;
	Material mMaterial;
	BufferBinding mBufferBinding;

	static Material CreateMaterial(RenderingDevice &device);
};

MapClipping::Impl::Impl(RenderingDevice& g) : mDevice(g), mMaterial(CreateMaterial(g)) {

	mBufferBinding.AddBuffer(nullptr, 0, sizeof(XMFLOAT3))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position);

}

Material MapClipping::Impl::CreateMaterial(RenderingDevice& device) {

	BlendState blendState;
	// Clipping geometry does not write color
	blendState.writeAlpha = false;
	blendState.writeRed = false;
	blendState.writeGreen = false;
	blendState.writeBlue = false;
	blendState.writeAlpha = false;
	DepthStencilState depthStencilState;
	RasterizerState rasterizerState;
	
	auto vs(device.GetShaders().LoadVertexShader("clipping_vs"));
	auto ps(device.GetShaders().LoadPixelShader("clipping_ps"));

	return Material(blendState, depthStencilState, rasterizerState, {}, vs, ps);

}

MapClipping::MapClipping(RenderingDevice& g) : mImpl(std::make_unique<Impl>(g)) {
}

MapClipping::~MapClipping() {
}

void MapClipping::Load(const std::string& directory) {

	Unload();

	LoadMeshes(directory);
	LoadObjects(directory);

}

void MapClipping::Unload() {

	mImpl->mClippingMeshes.clear();

}

void MapClipping::LoadMeshes(const std::string& directory) {

	auto filename(fmt::format("{}\\clipping.cgf", directory));
	auto data(vfs->ReadAsBinary(filename));

	BinaryReader reader(data);

	auto count = reader.Read<int32_t>();
	mImpl->mClippingMeshes.reserve(count);

	for (auto i = 0; i < count; ++i) {
		auto meshFilename(reader.ReadFixedString(260));
		if (meshFilename.empty()) {
			logger->error("Failed to read filename of clipping mesh #{} from {}", i, filename);
			mImpl->mClippingMeshes.push_back({});
			continue;
		}

		try {
			mImpl->mClippingMeshes.emplace_back(
				std::make_unique<ClippingMesh>(mImpl->mDevice, meshFilename)
			);
		} catch (TempleException &e) {
			logger->error("Failed to load clipping mesh {}: {}", filename, e.what());
			mImpl->mClippingMeshes.push_back({});
		}
	}

}

void MapClipping::LoadObjects(const std::string& directory) {

	auto indexFileName(fmt::format("{}\\clipping.cif", directory));
	auto data(vfs->ReadAsBinary(indexFileName));

	BinaryReader reader(data);

	auto count = reader.Read<int32_t>();

	for (auto i = 0; i < count; ++i) {
		LoadObject(reader);
	}
}

void MapClipping::LoadObject(BinaryReader& reader) {

	auto meshIdx = reader.Read<size_t>();

	auto mesh = mImpl->mClippingMeshes[meshIdx].get();
	
	ClippingMeshObj obj;
	obj.posX = reader.Read<float>();
	obj.posY = reader.Read<float>();
	obj.posZ = reader.Read<float>();
	obj.scaleX = reader.Read<float>();
	obj.scaleY = reader.Read<float>();
	obj.scaleZ = reader.Read<float>();
	obj.rotation = reader.Read<float>();

	if (!mesh) {
		logger->warn("Discarding clipping instance for invalid mesh #{}", meshIdx);
		return;
	}

	mesh->AddInstance(obj);
}

void MapClipping::Render() {

	if (mImpl->mClippingMeshes.empty()) {
		return;
	}

	auto device = mImpl->mDevice.GetDevice();

	mImpl->mDevice.SetMaterial(mImpl->mMaterial);

	auto viewProjMatrix = mImpl->mDevice.GetCamera().GetViewProj();
	D3DLOG(device->SetVertexShaderConstantF(0, &viewProjMatrix._11, 4));


	for (auto& mesh : mImpl->mClippingMeshes) {
				
		if (!mesh) {
			continue; // This is a mesh that failed to load
		}
	
		mImpl->mBufferBinding.SetBuffer(0, mesh->GetVertexBuffer());
		mImpl->mBufferBinding.Bind();
		mImpl->mDevice.GetDevice()->SetIndices(mesh->GetIndexBuffer()->GetBuffer());
		
		for (auto& obj : mesh->GetInstances()) {
			D3DXVECTOR4 constants = { 0, 0, 0, 0 };
			constants.x = cosf(obj.rotation);
			D3DLOG(device->SetVertexShaderConstantF(4, &constants.x, 1));

			constants.x = sinf(obj.rotation);
			D3DLOG(device->SetVertexShaderConstantF(5, &constants.x, 1));

			constants.x = obj.posX;
			constants.y = obj.posY;
			constants.z = obj.posZ;
			D3DLOG(device->SetVertexShaderConstantF(6, &constants.x, 1));

			constants.x = obj.scaleX;
			constants.y = obj.scaleY;
			constants.z = obj.scaleZ;
			D3DLOG(device->SetVertexShaderConstantF(7, &constants.x, 1));
			
			D3DLOG(device->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, 
				0, 
				0, 
				mesh->GetVertexCount(),
				0, 
				mesh->GetTriCount()));
		}

	}

}
