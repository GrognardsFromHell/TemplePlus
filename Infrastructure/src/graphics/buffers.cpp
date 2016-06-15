
#include "graphics/buffers.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"

namespace gfx {

IndexBuffer::IndexBuffer(CComPtr<ID3D11Buffer> buffer, size_t count)
	: mCount(count), mBuffer(buffer) {
}

IndexBuffer::~IndexBuffer() {
}

void IndexBuffer::Update(gsl::span<uint16_t> data) {
	renderingDevice->UpdateBuffer(*this, data);
}

VertexBuffer::VertexBuffer(CComPtr<ID3D11Buffer> buffer, size_t size) : mSize(size), mBuffer(buffer) {
}

VertexBuffer::~VertexBuffer() {
}

void VertexBuffer::Update(gsl::span<const uint8_t> data) {
	renderingDevice->UpdateBuffer(*this, data);
}

}
