
#pragma once

#include "buffers.h"
#include "shaders.h"

namespace gfx {

class BufferBinding;

enum class VertexElementType : uint32_t {
	Float1,
	Float2,
	Float3,
	Float4,
	Color, // 4 byte ARGB
	UByte4, // 4 unsigned bytes
	Short2, // 2 signed shorts (expanded to (s1, s2, 0, 1))
	Short4, // 4 signed shorts
	UByte4N, // 4 unsigned bytes, normalized to [0, 1] by dividing by 255
	Short2N, // 2 signed shorts normalized to [0, 1] by dividing by 32767
	Short4N, // 2 signed shorts normalized to [0, 1] by dividing by 32767
	UShort2N, // 2 unsigned shorts normalized to [0, 1] by dividing by 65535
	UShort4N, // 4 unsigned shorts normalized to [0, 1] by dividing by 65535
};

enum class VertexElementSemantic : uint32_t {
	Position,
	BlendWeight,
	BlendIndices,
	Normal,
	PointSize,
	TexCoord,
	Tangent,
	BiNormal,
	TessFactor,
	PositionT,
	Color,
	Fog,
	Depth,
	Sample
};

struct VertexElement {
	uint16_t stream; // Stream index (0 based)
	uint16_t offset; // Offset from the start of the stream in bytes
	VertexElementType type; // Data type of the element
	VertexElementSemantic semantic; // Semantic for this element
    // If more than slot exists for the semantic, use this to identify which one
	uint8_t semanticIndex = 0;
};

class BufferBindingBuilder {
friend class BufferBinding;
public:

	BufferBindingBuilder& AddElement(
		VertexElementType type,
		VertexElementSemantic semantic,
		uint8_t semanticIndex = 0
		);

private:
	BufferBindingBuilder(BufferBinding &binding, int streamIdx);

	BufferBinding &mBinding;
	int mStreamIdx;
	int mCurOffset = 0;
};

class BufferBinding {
friend class BufferBindingBuilder;
public:

	/**
	 * The shader has to be set so the input declaration can be validated
	 * against the declared inputs of the shader.
	 */
	BufferBinding(const VertexShaderPtr &shader) : mShader(shader) {
		memset(&mOffsets, 0, sizeof(mOffsets));
		memset(&mStrides, 0, sizeof(mStrides));
	}

	BufferBinding& SetBuffer(int streamIdx, VertexBufferPtr buffer) {
		mStreams[streamIdx] = buffer;
		return *this;
	}

	BufferBindingBuilder AddBuffer(VertexBufferPtr buffer, int offset, size_t stride) {
		auto streamIdx = mStreamCount++;
		mStreams[streamIdx] = buffer;
		mOffsets[streamIdx] = offset;
		mStrides[streamIdx] = stride;

		return BufferBindingBuilder(*this, streamIdx);
	}

	void Bind();
	void Unbind() const;

private:
	std::array<VertexElement, 16> mElements;
	size_t mElementCount = 0; // Actual number of used elements in mElements
	std::array<VertexBufferPtr, 16> mStreams;
	std::array<int, 16> mOffsets;
	std::array<int, 16> mStrides;
	size_t mStreamCount = 0; // Actual number of used streams
	CComPtr<struct ID3D11InputLayout> mInputLayout;
	VertexShaderPtr mShader;

	static size_t GetElementSize(VertexElementType type);
};

}
