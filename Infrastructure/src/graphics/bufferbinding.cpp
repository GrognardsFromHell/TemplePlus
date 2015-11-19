
#include "graphics/bufferbinding.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"

namespace gfx {

BufferBindingBuilder& BufferBindingBuilder::AddElement(VertexElementType type, VertexElementSemantic semantic, uint8_t semanticIndex) {

	auto elemIdx = mBinding.mElementCount++;
	auto &elem = mBinding.mElements[elemIdx];
	elem.stream = mStreamIdx;
	elem.offset = mCurOffset;
	elem.type = type;
	elem.semantic = semantic;
	elem.semanticIndex = semanticIndex;

	mCurOffset += mBinding.GetElementSize(type);

	return *this;

}

BufferBindingBuilder::BufferBindingBuilder(BufferBinding& binding, int streamIdx) 
	: mBinding(binding), mStreamIdx(streamIdx) {
}

void BufferBinding::Bind() {

	auto device = renderingDevice->GetDevice();

	// Lazily create the binding as needed
	if (!mDecl) {
		std::array<D3DVERTEXELEMENT9, 16> elems;

		for (size_t i = 0; i < mElementCount; ++i) {
			auto& elemIn = mElements[i];
			elems[i].Stream = elemIn.stream;
			elems[i].Offset = elemIn.offset;
			elems[i].Type = ConvertType(elemIn.type);
			elems[i].Usage = ConvertUsage(elemIn.semantic);
			elems[i].Method = D3DDECLMETHOD_DEFAULT;
			elems[i].UsageIndex = elemIn.semanticIndex;
		}
		elems[mElementCount] = D3DDECL_END();

		if (D3DLOG(device->CreateVertexDeclaration(&elems[0], &mDecl)) != D3D_OK) {
			throw TempleException("Unable to generate vertex declaration.");
		}
	}

	if (D3DLOG(device->SetVertexDeclaration(mDecl)) != D3D_OK) {
		throw TempleException("Unable to set vertex declaration.");
	}

	// Set the stream sources
	for (size_t i = 0; i < mStreamCount; ++i) {
		device->SetStreamSource(i, mStreams[i]->GetBuffer(), 0, mStrides[i]);
	}

}

void BufferBinding::Unbind() const {

	auto device = renderingDevice->GetDevice();

	// Reset vertex declaration
	device->SetVertexDeclaration(nullptr);

	// Reset all streams
	for (size_t i = 0; i < mStreamCount; ++i) {
		device->SetStreamSource(i, nullptr, 0, 0);
	}

}

size_t BufferBinding::GetElementSize(VertexElementType type) {
	switch (type) {
	case VertexElementType::Float1:
		return sizeof(float);
	case VertexElementType::Float2:
		return sizeof(float) * 2;
	case VertexElementType::Float3:
		return sizeof(float) * 3;
	case VertexElementType::Float4:
		return sizeof(float) * 4;
	case VertexElementType::Color:
		return sizeof(D3DCOLOR);
	case VertexElementType::UByte4:
		return sizeof(uint8_t) * 4;
	case VertexElementType::Short2:
		return sizeof(int16_t) * 2;
	case VertexElementType::Short4:
		return sizeof(int16_t) * 4;
	case VertexElementType::UByte4N:
		return sizeof(uint8_t) * 4;
	case VertexElementType::Short2N:
		return sizeof(int16_t) * 2;
	case VertexElementType::Short4N:
		return sizeof(int16_t) * 4;
	case VertexElementType::UShort2N:
		return sizeof(uint16_t) * 2;
	case VertexElementType::UShort4N:
		return sizeof(uint16_t) * 4;
	default:
		throw TempleException("Unknown vertex element type.");
	}
}


BYTE BufferBinding::ConvertType(VertexElementType type) {
	switch (type) {
	case VertexElementType::Float1:
		return D3DDECLTYPE_FLOAT1;
	case VertexElementType::Float2:
		return D3DDECLTYPE_FLOAT2;
	case VertexElementType::Float3:
		return D3DDECLTYPE_FLOAT3;
	case VertexElementType::Float4:
		return D3DDECLTYPE_FLOAT4;
	case VertexElementType::Color:
		return D3DDECLTYPE_D3DCOLOR;
	case VertexElementType::UByte4:
		return D3DDECLTYPE_UBYTE4;
	case VertexElementType::Short2:
		return D3DDECLTYPE_SHORT2;
	case VertexElementType::Short4:
		return D3DDECLTYPE_SHORT4;
	case VertexElementType::UByte4N:
		return D3DDECLTYPE_UBYTE4N;
	case VertexElementType::Short2N:
		return D3DDECLTYPE_SHORT2N;
	case VertexElementType::Short4N:
		return D3DDECLTYPE_SHORT4N;
	case VertexElementType::UShort2N:
		return D3DDECLTYPE_USHORT2N;
	case VertexElementType::UShort4N:
		return D3DDECLTYPE_USHORT4N;
	default:
		throw TempleException("Unknown vertex element type: {}", (int)type);
	}
}

BYTE BufferBinding::ConvertUsage(VertexElementSemantic semantic) {
	switch (semantic) {
	case VertexElementSemantic::Position:
		return D3DDECLUSAGE_POSITION;
	case VertexElementSemantic::BlendWeight:
		return D3DDECLUSAGE_BLENDWEIGHT;
	case VertexElementSemantic::BlendIndices:
		return D3DDECLUSAGE_BLENDINDICES;
	case VertexElementSemantic::Normal:
		return D3DDECLUSAGE_NORMAL;
	case VertexElementSemantic::PointSize:
		return D3DDECLUSAGE_PSIZE;
	case VertexElementSemantic::TexCoord:
		return D3DDECLUSAGE_TEXCOORD;
	case VertexElementSemantic::Tangent:
		return D3DDECLUSAGE_TANGENT;
	case VertexElementSemantic::BiNormal:
		return D3DDECLUSAGE_BINORMAL;
	case VertexElementSemantic::TessFactor:
		return D3DDECLUSAGE_TESSFACTOR;
	case VertexElementSemantic::PositionT:
		return D3DDECLUSAGE_POSITIONT;
	case VertexElementSemantic::Color:
		return D3DDECLUSAGE_COLOR;
	case VertexElementSemantic::Fog:
		return D3DDECLUSAGE_FOG;
	case VertexElementSemantic::Depth:
		return D3DDECLUSAGE_DEPTH;
	case VertexElementSemantic::Sample:
		return D3DDECLUSAGE_SAMPLE;
	default:
		throw TempleException("Unknown vertex element semantic: {}", (int)semantic);
	}
}

}
