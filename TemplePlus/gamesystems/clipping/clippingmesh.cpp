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

	mBoundingSphereOrigin = reader.Read<XMFLOAT3>();
	mBoundingSphereRadius = reader.Read<float>();
	auto objCount = reader.Read<int>();
	auto dataStart = reader.Read<int>();

	if (objCount != 1) {
		throw TempleException("TemplePlus only supports DAG with a "
			"single object. {} has {} objects.", mFilename, objCount);
	}

	// Start reading again @ the data start offset
	reader = BinaryReader(std::span<uint8_t>(data).subspan(dataStart));

	mVertexCount = reader.Read<int>();
	mTriCount = reader.Read<int>();
	auto vertexDataStart = reader.Read<int>();
	auto indexDataStart = reader.Read<int>();

	// Swap the indices to achieve clock wise tri ordering
	auto indices = reinterpret_cast<uint16_t*>(&data[indexDataStart]);
	for (size_t i = 0; i < mTriCount * 3; i += 3) {
		std::swap(indices[i], indices[i + 2]);
	}

	auto vertexBufferSize = mVertexCount * sizeof(XMFLOAT3);
	mVertexBuffer = device.CreateVertexBufferRaw(std::span(&data[vertexDataStart], vertexBufferSize));
	mIndexBuffer = device.CreateIndexBuffer(std::span(indices, mTriCount * 3));
}

void ClippingMesh::FreeResources(RenderingDevice &device) {

	mIndexBuffer.reset();
	mVertexBuffer.reset();

}
