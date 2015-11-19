
#pragma once

#include <memory>

#include "../gsl/gsl.h"
#include "../platform/d3d.h"
#include "../infrastructure/macros.h"

namespace gfx {

class VertexBuffer;
class IndexBuffer;

class IndexBufferLock {
public:
	IndexBufferLock(gsl::array_view<uint16_t> data, IndexBuffer& locked)
		: mData(data), mLocked(locked) {
	}

	IndexBufferLock(IndexBufferLock&& o) : mData(o.mData), mLocked(o.mLocked) {
		o.mMoved = true;
	}

	~IndexBufferLock();

	uint16_t &operator[](size_t idx) {
		Expects(!mMoved);
		return mData[idx];
	}

	void Unlock();

private:
	gsl::array_view<uint16_t> mData;
	IndexBuffer& mLocked;
	bool mMoved = false;
};

class IndexBuffer {
	friend class IndexBufferLock;
public:
	IndexBuffer(CComPtr<IDirect3DIndexBuffer9> indexBuffer, size_t count);
	~IndexBuffer();

	void Update(gsl::array_view<uint16_t> data);

	IndexBufferLock Lock();

	IDirect3DIndexBuffer9* GetBuffer() const {
		Expects(!mLocked);
		return mIndexBuffer;
	}

	NO_COPY_OR_MOVE(IndexBuffer)
private:
	void Unlock();

	std::shared_ptr<uint16_t[]> mData;
	size_t mCount;
	bool mLocked = false;
	CComPtr<IDirect3DIndexBuffer9> mIndexBuffer;
};

using IndexBufferPtr = std::shared_ptr<IndexBuffer>;

template <typename T>
class VertexBufferLock {
public:
	using DataView = gsl::array_view<T>;

	VertexBufferLock(DataView data, VertexBuffer& locked)
		: mData(data), mLocked(locked) {
	}

	VertexBufferLock(VertexBufferLock&& o) : mData(o.mData), mLocked(o.mLocked) {
		o.mMoved = true;
	}

	~VertexBufferLock();

	DataView GetData() const {
		return mData;
	}

	T &operator[](size_t idx) {
		Expects(!mMoved);
		return mData[idx];
	}

	void Unlock();

	VertexBufferLock(VertexBufferLock&) = delete;
	VertexBufferLock& operator=(VertexBufferLock&) = delete;
	VertexBufferLock& operator=(VertexBufferLock&&) = delete;
private:
	DataView mData;
	VertexBuffer& mLocked;
	bool mMoved = false;
};

class VertexBuffer {
	template <typename T>
	friend class VertexBufferLock;
public:
	VertexBuffer(CComPtr<IDirect3DVertexBuffer9> vertexBuffer, size_t size);
	~VertexBuffer();

	void Update(gsl::array_view<uint8_t> data);

	template <typename T>
	void Update(gsl::array_view<T> data) {
		Update({ reinterpret_cast<uint8_t*>(&data[0]), data.size() * sizeof(T) });
	}

	template <typename T>
	VertexBufferLock<T> Lock(size_t count = 0) {
		Expects(mSize % sizeof(T) == 0);
		if (count == 0) {
			count = mSize / sizeof(T);
		}
		auto data = LockRaw(count * sizeof(T));
		return VertexBufferLock<T>(
		{ reinterpret_cast<T*>(&data[0]), count },
			*this
			);
	}

	IDirect3DVertexBuffer9* GetBuffer() const {
		return mVertexBuffer;
	}

	NO_COPY_OR_MOVE(VertexBuffer);
private:
	gsl::array_view<uint8_t> LockRaw(size_t size);
	void Unlock();

	std::shared_ptr<uint8_t[]> mData;
	size_t mSize;
	bool mLocked = false;
	CComPtr<IDirect3DVertexBuffer9> mVertexBuffer;
};

using VertexBufferPtr = std::shared_ptr<VertexBuffer>;

template <typename T>
VertexBufferLock<T>::~VertexBufferLock() {
	if (!mMoved) {
		mLocked.Unlock();
	}
}

template<typename T>
inline void VertexBufferLock<T>::Unlock()
{
	Expects(!mMoved);
	mLocked.Unlock();
	mMoved = true;
}

}
