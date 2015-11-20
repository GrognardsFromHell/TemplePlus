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
	mVertexBuffer = device.CreateVertexBufferRaw({ &data[vertexDataStart], vertexBufferSize });
	mIndexBuffer = device.CreateIndexBuffer({ reinterpret_cast<uint16_t*>(&data[indexDataStart]), mTriCount * 3 });
}

void ClippingMesh::FreeResources(RenderingDevice &device) {

	mIndexBuffer.reset();
	mVertexBuffer.reset();

}
