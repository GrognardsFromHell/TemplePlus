
#pragma once
#include <atlcomcli.h>

struct RenderStatesData {
	RenderStatesData();
	D3DMATRIX mProjMatrix;
	D3DMATRIX mViewMatrix;
	bool mZEnable = true;
	D3DFILLMODE mFillMode = D3DFILL_SOLID;
	bool mZWriteenable = false;
	bool mAlphatestenable = false;
	bool mAlphablendenable = true;
	D3DBLEND mSrcBlend = D3DBLEND_SRCALPHA;
	D3DBLEND mDestBlend = D3DBLEND_INVSRCALPHA;
	D3DCULL mCullmode = D3DCULL_NONE;
	bool mLighting = true;
	bool mColorVertex = true;
	DWORD mColorwriteenable = 0x0000000F;
	D3DCMPFUNC mZFunc = D3DCMP_LESSEQUAL;
	bool mSpecularenable = false;
	int mZBias = 0;

	static constexpr auto MaxSamplers = 4;
	template <typename T>
	using SamplerState = std::array<T, MaxSamplers>;

	SamplerState<D3DTEXTUREOP> mTextureColorOp{
		D3DTOP_MODULATE,
		D3DTOP_DISABLE,
		D3DTOP_DISABLE,
		D3DTOP_DISABLE
	};
	SamplerState<DWORD> mTextureColorArg1{
		D3DTA_TEXTURE ,
		D3DTA_TEXTURE ,
		D3DTA_TEXTURE ,
		D3DTA_TEXTURE
	};
	SamplerState<DWORD> mTextureColorArg2{
		D3DTA_CURRENT ,
		D3DTA_CURRENT ,
		D3DTA_CURRENT ,
		D3DTA_CURRENT
	};
	SamplerState<D3DTEXTUREOP> mTextureAlphaOp{
		D3DTOP_SELECTARG1,
		D3DTOP_DISABLE,
		D3DTOP_DISABLE,
		D3DTOP_DISABLE
	};
	SamplerState<DWORD> mTextureAlphaArg1{
		D3DTA_TEXTURE ,
		D3DTA_DIFFUSE ,
		D3DTA_DIFFUSE ,
		D3DTA_DIFFUSE
	};
	SamplerState<DWORD> mTextureAlphaArg2{
		D3DTA_CURRENT ,
		D3DTA_CURRENT ,
		D3DTA_CURRENT ,
		D3DTA_CURRENT
	};
	SamplerState<int> mTextureCoordIndex{
		0,
		0,
		0,
		0
	};
	SamplerState<D3DTEXTUREFILTERTYPE> mTextureMipFilter{
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT
	};
	SamplerState<D3DTEXTUREFILTERTYPE> mTextureMagFilter{
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT
	};
	SamplerState<D3DTEXTUREFILTERTYPE> mTextureMinFilter{
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT,
		D3DTEXF_POINT
	};
	SamplerState<D3DTEXTUREADDRESS> mTextureAddressU{
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP
	};
	SamplerState<D3DTEXTUREADDRESS> mTextureAddressV{
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP,
		D3DTADDRESS_CLAMP
	};
	SamplerState<D3DTEXTURETRANSFORMFLAGS> mTextureTransformFlags{
		D3DTTFF_DISABLE,
		D3DTTFF_DISABLE,
		D3DTTFF_DISABLE,
		D3DTTFF_DISABLE
	};

	int mBaseIdx = 0;

	DWORD mFvf = 0;
};

