
#pragma once

#include <memory>

#include <gsl/gsl>
#include "../platform/d3d.h"
#include "../infrastructure/macros.h"

namespace gfx {

class IndexBuffer {
	friend class RenderingDevice;
public:
	IndexBuffer(CComPtr<ID3D11Buffer> buffer, size_t count);
	~IndexBuffer();

	void Update(gsl::span<uint16_t> data);

	NO_COPY_OR_MOVE(IndexBuffer)
private:
	void Unlock();

	size_t mCount;
	CComPtr<ID3D11Buffer> mBuffer;
};

using IndexBufferPtr = std::shared_ptr<IndexBuffer>;

class VertexBuffer {
	friend class RenderingDevice;
	friend class BufferBinding;
public:
	VertexBuffer(CComPtr<ID3D11Buffer> vertexBufferNew, size_t size);
	~VertexBuffer();

	void Update(gsl::span<const uint8_t> data);

	template <typename T>
	void Update(gsl::span<T> data) {
		Update(gsl::span(reinterpret_cast<const uint8_t*>(&data[0]), data.size_bytes()));
	}

	NO_COPY_OR_MOVE(VertexBuffer);
private:
	size_t mSize;
	CComPtr<ID3D11Buffer> mBuffer;
};

using VertexBufferPtr = std::shared_ptr<VertexBuffer>;

}
