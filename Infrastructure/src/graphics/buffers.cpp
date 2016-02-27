
#include "graphics/buffers.h"
#include "infrastructure/exception.h"

namespace gfx {

IndexBufferLock::~IndexBufferLock() {
	if (!mMoved) {
		mLocked.Unlock();
	}
}

void IndexBufferLock::Unlock()
{
	Expects(!mMoved);
	mLocked.Unlock();
	mMoved = true;
}

IndexBuffer::IndexBuffer(CComPtr<IDirect3DIndexBuffer9> indexBuffer, size_t count)
	: mCount(count), mIndexBuffer(indexBuffer) {
}

IndexBuffer::~IndexBuffer() {
	if (mLocked) {
		Unlock();
	}
}

IndexBufferLock IndexBuffer::Lock() {
	Expects(!mLocked);
	void* lockedData;
	auto size = mCount * sizeof(uint16_t);
	if (D3DLOG(mIndexBuffer->Lock(0, size, &lockedData, D3DLOCK_DISCARD)) != D3D_OK) {
		throw TempleException("Unable to lock index buffer for update.");
	}
	mLocked = true;
	return IndexBufferLock(gsl::as_span(reinterpret_cast<uint16_t*>(lockedData), mCount), *this);
}

void IndexBuffer::Unlock() {
	Expects(mLocked);
	D3DLOG(mIndexBuffer->Unlock());
	mLocked = false;
}

void IndexBuffer::Update(gsl::span<uint16_t> data) {

	Expects(data.size() <= (int) mCount);

	auto lock(Lock());

	memcpy(&lock[0], &data[0], data.size() * sizeof(uint16_t));

}

VertexBuffer::VertexBuffer(CComPtr<IDirect3DVertexBuffer9> vertexBuffer, size_t size)
	: mSize(size), mVertexBuffer(vertexBuffer) {
}

VertexBuffer::~VertexBuffer() {
}

void VertexBuffer::Update(gsl::span<const uint8_t> data) {
	auto lock(Lock<uint8_t>());
	memcpy(&lock.GetData()[0], &data[0], data.size() * sizeof(uint8_t));
}

gsl::span<uint8_t> VertexBuffer::LockRaw(size_t size) {
	Expects(!mLocked);
	void* lockedData;

	if (!size) {
		size = mSize;
	}
	Expects(size <= mSize);

	if (D3DLOG(mVertexBuffer->Lock(0, size, &lockedData, D3DLOCK_DISCARD)) != D3D_OK) {
		throw TempleException("Unable to lock vertex buffer for update.");
	}
	mLocked = true;
	return gsl::as_span(reinterpret_cast<uint8_t*>(lockedData), size);
}

void VertexBuffer::Unlock() {
	Expects(mLocked);
	D3DLOG(mVertexBuffer->Unlock());
	mLocked = false;
}

}
