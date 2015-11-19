#include "stdafx.h"

#include <infrastructure/vfs.h>
#include <infrastructure/binaryreader.h>
#include <infrastructure/renderstates.h>

#include "clipping.h"
#include "graphics/shaders.h"
#include "clippingmesh.h"

class MapClipping::Impl : public ResourceListener {
public:
	explicit Impl(RenderingDevice& g);

	void CreateResources(RenderingDevice&) override;
	void FreeResources(RenderingDevice&) override;

	std::vector<std::unique_ptr<ClippingMesh>> mClippingMeshes;

	RenderingDevice& mDevice;
	VertexShaderPtr mVertexShader;
	PixelShaderPtr mPixelShader;
	CComPtr<IDirect3DVertexDeclaration9> mVertexDecl;

	ResourceListenerRegistration mRegistration;
};

MapClipping::Impl::Impl(RenderingDevice& g) : mDevice(g), mRegistration(g, this) {

	mVertexShader = g.GetShaders().LoadVertexShader("clipping_vs");
	mPixelShader = g.GetShaders().LoadPixelShader("clipping_ps");

}

void MapClipping::Impl::CreateResources(RenderingDevice& g) {
	D3DVERTEXELEMENT9 elements[] = {
		{0, 0,D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT , D3DDECLUSAGE_POSITION , 0},
		D3DDECL_END()
	};

	// Resources are created on-demand
	auto d3dDevice = g.GetDevice();
	D3DLOG(d3dDevice->CreateVertexDeclaration(&elements[0], &mVertexDecl));
}

void MapClipping::Impl::FreeResources(RenderingDevice&) {
	mVertexDecl.Release();
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

	mImpl->mVertexShader->Bind();
	mImpl->mPixelShader->Bind();

	auto projMatrix = renderStates->Get3dProjectionMatrix();
	D3DLOG(device->SetVertexShaderConstantF(0, &projMatrix._11, 4));

	renderStates->SetColorWriteEnable(false, false, false, false);
	renderStates->SetZWriteEnable(true);
	renderStates->SetFVF(0);

	for (auto& mesh : mImpl->mClippingMeshes) {
				
		if (!mesh) {
			continue; // This is a mesh that failed to load
		}
	
		renderStates->SetStreamSource(0, mesh->GetVertexBuffer(), sizeof(D3DVECTOR));
		renderStates->SetIndexBuffer(mesh->GetIndexBuffer(), 0);
		renderStates->SetFVF(0);
		renderStates->Commit();
		
		D3DLOG(device->SetVertexDeclaration(mImpl->mVertexDecl));

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

			continue;
		}

	}

	renderStates->SetColorWriteEnable(true, true, true, true);

	D3DLOG(device->SetVertexDeclaration(nullptr));

	renderStates->SetFillMode(D3DFILL_SOLID);

	mImpl->mVertexShader->Unbind();
	mImpl->mPixelShader->Unbind();

}
