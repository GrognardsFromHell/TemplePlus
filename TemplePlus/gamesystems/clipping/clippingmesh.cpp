#include "stdafx.h"

#include <infrastructure/vfs.h>

#include "clippingmesh.h"
#include <infrastructure/binaryreader.h>

ClippingMesh::ClippingMesh(RenderingDevice &device, const std::string& filename)
	: mFilename(filename), mRegistration(device, this) {	
}

ClippingMesh::~ClippingMesh() {
}

void ClippingMesh::AddInstance(const ClippingMeshObj& obj) {
	mInstances.push_back(obj);
}

void ClippingMesh::CreateResources(RenderingDevice &device) {

	auto data(vfs->ReadAsBinary(mFilename));
	BinaryReader reader(data);

	auto boundingSphereOffset = reader.Read<D3DVECTOR>();
	auto boundingSphereRadius = reader.Read<float>();
	auto objCount = reader.Read<int>();
	auto dataStart = reader.Read<int>();

	if (objCount != 1) {
		throw TempleException("TemplePlus only supports DAG with a "
			"single object. {} has {} objects.", mFilename, objCount);
	}

	// Start reading again @ the data start offset
	reader = BinaryReader(gsl::array_view<uint8_t>(data).sub(dataStart));

	mVertexCount = reader.Read<int>();
	mTriCount = reader.Read<int>();
	auto vertexDataStart = reader.Read<int>();
	auto indexDataStart = reader.Read<int>();

	auto vertexBufferSize = mVertexCount * sizeof(D3DVECTOR);
	auto d3dDevice = device.GetDevice();
	if (D3DLOG(d3dDevice->CreateVertexBuffer(vertexBufferSize, 0, 0, D3DPOOL_DEFAULT, &mVertexBuffer, nullptr)) != D3D_OK) {
		return;
	}

	void *vertexBufferData;
	if (D3DLOG(mVertexBuffer->Lock(0, vertexBufferSize, &vertexBufferData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	memcpy(vertexBufferData, &data[vertexDataStart], vertexBufferSize);

	D3DLOG(mVertexBuffer->Unlock());

	auto indexBufferSize = mTriCount * 3 * sizeof(uint16_t);
	if (D3DLOG(d3dDevice->CreateIndexBuffer(indexBufferSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &mIndexBuffer, nullptr)) != D3D_OK) {
		return;
	}

	void *indexBufferData;
	if (D3DLOG(mIndexBuffer->Lock(0, indexBufferSize, &indexBufferData, D3DLOCK_DISCARD)) != D3D_OK) {
		return;
	}

	memcpy(indexBufferData, &data[indexDataStart], indexBufferSize);

	D3DLOG(mIndexBuffer->Unlock());
}

void ClippingMesh::FreeResources(RenderingDevice &device) {

	mIndexBuffer.Release();
	mVertexBuffer.Release();

}
