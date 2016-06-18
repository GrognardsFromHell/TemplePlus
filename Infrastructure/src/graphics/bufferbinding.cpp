
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

	// D3D11 version
	if (!mInputLayout) {
		eastl::fixed_vector<D3D11_INPUT_ELEMENT_DESC, 16> inputDesc;

		for (size_t i = 0; i < mElementCount; ++i) {
			auto& elemIn = mElements[i];

			D3D11_INPUT_ELEMENT_DESC desc;
			memset(&desc, 0, sizeof(desc));

			switch (elemIn.semantic) {
			case VertexElementSemantic::Position:
				desc.SemanticName = "POSITION";
				break;			
			case VertexElementSemantic::Normal:
				desc.SemanticName = "NORMAL";
				break;
			case VertexElementSemantic::TexCoord:
				desc.SemanticName = "TEXCOORD";
				break;
			case VertexElementSemantic::Color:
				desc.SemanticName = "COLOR";
				break;
			default:
				throw TempleException("Unsupported semantic");
			}
			desc.SemanticIndex = elemIn.semanticIndex;
			desc.AlignedByteOffset = elemIn.offset;
			desc.InstanceDataStepRate = 0;
			desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc.InputSlot = elemIn.stream;

			switch (elemIn.type) {
				case VertexElementType::Float1:
					desc.Format = DXGI_FORMAT_R32_FLOAT;
					break;
				case VertexElementType::Float2:
					desc.Format = DXGI_FORMAT_R32G32_FLOAT;
					break;
				case VertexElementType::Float3:
					desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					break;
				case VertexElementType::Float4:
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					break;
				case VertexElementType::Color:
					desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
					break;
				default:
					throw TempleException("Unsupported vertex element type.");
			}

			inputDesc.push_back(desc);
		}
		
		auto &shaderCode = mShader->mCompiledShader;

		D3DVERIFY(renderingDevice->mD3d11Device->CreateInputLayout(
			inputDesc.data(),
			inputDesc.size(),
			shaderCode.data(),
			shaderCode.size(),
			&mInputLayout
		));
	}

	renderingDevice->mContext->IASetInputLayout(mInputLayout);

	// Set the stream sources
	eastl::fixed_vector<ID3D11Buffer*, 16> vertexBuffers;
	for (size_t i = 0; i < mStreamCount; ++i) {
		vertexBuffers.push_back(mStreams[i]->mBuffer);
	}
	for (size_t i = mStreamCount; i < 16; i++) {
		vertexBuffers.push_back(nullptr);
	}
	UINT offsets[16] = { 0, };

	renderingDevice->mContext->IASetVertexBuffers(0, 16, vertexBuffers.data(), (UINT*)mStrides.data(), offsets);

}

void BufferBinding::Unbind() const {

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
		return sizeof(XMCOLOR);
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

}
