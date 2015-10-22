#include "stdafx.h"

#include <infrastructure/vfs.h>
#include <infrastructure/binaryreader.h>
#include <infrastructure/renderstates.h>

#include "clipping.h"
#include "graphics/shaders.h"
#include "clippingmesh.h"

class MapClipping::Impl : public ResourceListener {
public:
	explicit Impl(Graphics& g);

	void CreateResources(Graphics&) override;
	void FreeResources(Graphics&) override;

	std::vector<std::unique_ptr<ClippingMesh>> mClippingMeshes;

	Graphics& mGraphics;
	VertexShaderPtr mVertexShader;
	PixelShaderPtr mPixelShader;
	CComPtr<IDirect3DVertexDeclaration9> mVertexDecl;

	ResourceListenerRegistration mRegistration;
};

MapClipping::Impl::Impl(Graphics& g) : mGraphics(g), mRegistration(g, this) {

	mVertexShader = g.GetShaders().LoadVertexShader("clipping_vs");
	mPixelShader = g.GetShaders().LoadPixelShader("clipping_ps");

}

void MapClipping::Impl::CreateResources(Graphics& g) {
	D3DVERTEXELEMENT9 elements[] = {
		{0, 0,D3DDECLTYPE_FLOAT3 , D3DDECLMETHOD_DEFAULT , D3DDECLUSAGE_POSITION , 0},
		D3DDECL_END()
	};

	// Resources are created on-demand
	auto device = g.device();
	D3DLOG(device->CreateVertexDeclaration(&elements[0], &mVertexDecl));
}

void MapClipping::Impl::FreeResources(Graphics&) {
	mVertexDecl.Release();
}

MapClipping::MapClipping(Graphics& g) : mImpl(std::make_unique<Impl>(g)) {
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
		mImpl->mClippingMeshes.emplace_back(
			std::make_unique<ClippingMesh>(mImpl->mGraphics, meshFilename)
		);
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
	mesh->AddInstance(obj);
}

void MapClipping::Render() {

	if (mImpl->mClippingMeshes.empty()) {
		return;
	}

	auto device = mImpl->mGraphics.device();

	mImpl->mVertexShader->Bind(mImpl->mGraphics);
	mImpl->mPixelShader->Bind(mImpl->mGraphics);

	auto projMatrix = renderStates->Get3dProjectionMatrix();
	D3DLOG(device->SetVertexShaderConstantF(0, &projMatrix._11, 4));

	renderStates->SetColorWriteEnable(false, false, false, false);
	renderStates->SetZWriteEnable(true);
	renderStates->SetFVF(0);

	for (auto& mesh : mImpl->mClippingMeshes) {
	
		renderStates->SetStreamSource(0, mesh->GetVertexBuffer(), sizeof(D3DVECTOR));
		renderStates->SetIndexBuffer(mesh->GetIndexBuffer(), 0);
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

	mImpl->mVertexShader->Unbind(mImpl->mGraphics);
	mImpl->mPixelShader->Unbind(mImpl->mGraphics);

}
