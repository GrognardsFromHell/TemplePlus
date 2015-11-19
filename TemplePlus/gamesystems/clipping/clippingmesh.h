#pragma once

#include <string>
#include <vector>
#include <atlcomcli.h>

#include <graphics/device.h>
#include <graphics/buffers.h>

struct ClippingMeshObj {
	float posX;
	float posY;
	float posZ;
	float scaleX;
	float scaleY;
	float scaleZ;
	float rotation;
};

using namespace gfx;

class ClippingMesh : public ResourceListener {
public:
	explicit ClippingMesh(RenderingDevice &device, const std::string& filename);
	~ClippingMesh();

	void AddInstance(const ClippingMeshObj& obj);

	const std::vector<ClippingMeshObj>& GetInstances() const {
		return mInstances;
	}

	IDirect3DVertexBuffer9* GetVertexBuffer() const {
		return mVertexBuffer;
	}

	IDirect3DIndexBuffer9* GetIndexBuffer() const {
		return mIndexBuffer;
	}

	size_t GetVertexCount() const {
		return mVertexCount;
	}

	size_t GetTriCount() const {
		return mTriCount;
	}

	void CreateResources(RenderingDevice &device) override;
	void FreeResources(RenderingDevice &device) override;
private:
	CComPtr<IDirect3DVertexBuffer9> mVertexBuffer;
	CComPtr<IDirect3DIndexBuffer9> mIndexBuffer;

	size_t mVertexCount = 0;
	size_t mTriCount = 0;

	std::string mFilename;
	std::vector<ClippingMeshObj> mInstances;

	ResourceListenerRegistration mRegistration;
};
