
#include "d3d8to9_private.h"

inline D3DFORMAT convert(d3d8::D3DFORMAT format) {
	// TODO: Convert format
	switch (format) {
	case d3d8::D3DFMT_X8R8G8B8:
		return D3DFMT_X8R8G8B8;
	case d3d8::D3DFMT_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case d3d8::D3DFMT_D16:
		return D3DFMT_D16;
	case d3d8::D3DFMT_INDEX16:
		return D3DFMT_INDEX16;
	default:
		LOG(info) << "Unknown D3D Format used: " << format;
		abort();
	}
}

inline d3d8::D3DFORMAT convert(D3DFORMAT format) {
	switch (format) {
	case D3DFMT_X8R8G8B8:
		return d3d8::D3DFMT_X8R8G8B8;
	case D3DFMT_A8R8G8B8:
		return d3d8::D3DFMT_A8R8G8B8;
	case D3DFMT_D16:
		return d3d8::D3DFMT_D16;
	case D3DFMT_INDEX16:
		return d3d8::D3DFMT_INDEX16;
	default:
		LOG(info) << "Unknown D3D Format used: " << format;
		return (d3d8::D3DFORMAT)format;
	}
}

inline D3DRENDERSTATETYPE convert(d3d8::D3DRENDERSTATETYPE type) {
	switch (type) {
		// TODO: Probably needs tweaking
	case d3d8::D3DRS_ZBIAS: return D3DRS_DEPTHBIAS;
	case d3d8::D3DRS_ZENABLE: return D3DRS_ZENABLE;
	case d3d8::D3DRS_FILLMODE: return D3DRS_FILLMODE;
	case d3d8::D3DRS_SHADEMODE: return D3DRS_SHADEMODE;
	case d3d8::D3DRS_ZWRITEENABLE: return D3DRS_ZWRITEENABLE;
	case d3d8::D3DRS_ALPHATESTENABLE: return D3DRS_ALPHATESTENABLE;
	case d3d8::D3DRS_LASTPIXEL: return D3DRS_LASTPIXEL;
	case d3d8::D3DRS_SRCBLEND: return D3DRS_SRCBLEND;
	case d3d8::D3DRS_DESTBLEND: return D3DRS_DESTBLEND;
	case d3d8::D3DRS_CULLMODE: return D3DRS_CULLMODE;
	case d3d8::D3DRS_ZFUNC: return D3DRS_ZFUNC;
	case d3d8::D3DRS_ALPHAREF: return D3DRS_ALPHAREF;
	case d3d8::D3DRS_ALPHAFUNC: return D3DRS_ALPHAFUNC;
	case d3d8::D3DRS_DITHERENABLE: return D3DRS_DITHERENABLE;
	case d3d8::D3DRS_ALPHABLENDENABLE: return D3DRS_ALPHABLENDENABLE;
	case d3d8::D3DRS_FOGENABLE: return D3DRS_FOGENABLE;
	case d3d8::D3DRS_SPECULARENABLE: return D3DRS_SPECULARENABLE;
	case d3d8::D3DRS_FOGCOLOR: return D3DRS_FOGCOLOR;
	case d3d8::D3DRS_FOGTABLEMODE: return D3DRS_FOGTABLEMODE;
	case d3d8::D3DRS_FOGSTART: return D3DRS_FOGSTART;
	case d3d8::D3DRS_FOGEND: return D3DRS_FOGEND;
	case d3d8::D3DRS_FOGDENSITY: return D3DRS_FOGDENSITY;
	case d3d8::D3DRS_RANGEFOGENABLE: return D3DRS_RANGEFOGENABLE;
	case d3d8::D3DRS_STENCILENABLE: return D3DRS_STENCILENABLE;
	case d3d8::D3DRS_STENCILFAIL: return D3DRS_STENCILFAIL;
	case d3d8::D3DRS_STENCILZFAIL: return D3DRS_STENCILZFAIL;
	case d3d8::D3DRS_STENCILPASS: return D3DRS_STENCILPASS;
	case d3d8::D3DRS_STENCILFUNC: return D3DRS_STENCILFUNC;
	case d3d8::D3DRS_STENCILREF: return D3DRS_STENCILREF;
	case d3d8::D3DRS_STENCILMASK: return D3DRS_STENCILMASK;
	case d3d8::D3DRS_STENCILWRITEMASK: return D3DRS_STENCILWRITEMASK;
	case d3d8::D3DRS_TEXTUREFACTOR: return D3DRS_TEXTUREFACTOR;
	case d3d8::D3DRS_WRAP0: return D3DRS_WRAP0;
	case d3d8::D3DRS_WRAP1: return D3DRS_WRAP1;
	case d3d8::D3DRS_WRAP2: return D3DRS_WRAP2;
	case d3d8::D3DRS_WRAP3: return D3DRS_WRAP3;
	case d3d8::D3DRS_WRAP4: return D3DRS_WRAP4;
	case d3d8::D3DRS_WRAP5: return D3DRS_WRAP5;
	case d3d8::D3DRS_WRAP6: return D3DRS_WRAP6;
	case d3d8::D3DRS_WRAP7: return D3DRS_WRAP7;
	case d3d8::D3DRS_CLIPPING: return D3DRS_CLIPPING;
	case d3d8::D3DRS_LIGHTING: return D3DRS_LIGHTING;
	case d3d8::D3DRS_AMBIENT: return D3DRS_AMBIENT;
	case d3d8::D3DRS_FOGVERTEXMODE: return D3DRS_FOGVERTEXMODE;
	case d3d8::D3DRS_COLORVERTEX: return D3DRS_COLORVERTEX;
	case d3d8::D3DRS_LOCALVIEWER: return D3DRS_LOCALVIEWER;
	case d3d8::D3DRS_NORMALIZENORMALS: return D3DRS_NORMALIZENORMALS;
	case d3d8::D3DRS_DIFFUSEMATERIALSOURCE: return D3DRS_DIFFUSEMATERIALSOURCE;
	case d3d8::D3DRS_SPECULARMATERIALSOURCE: return D3DRS_SPECULARMATERIALSOURCE;
	case d3d8::D3DRS_AMBIENTMATERIALSOURCE: return D3DRS_AMBIENTMATERIALSOURCE;
	case d3d8::D3DRS_EMISSIVEMATERIALSOURCE: return D3DRS_EMISSIVEMATERIALSOURCE;
	case d3d8::D3DRS_VERTEXBLEND: return D3DRS_VERTEXBLEND;
	case d3d8::D3DRS_CLIPPLANEENABLE: return D3DRS_CLIPPLANEENABLE;
	case d3d8::D3DRS_POINTSIZE: return D3DRS_POINTSIZE;
	case d3d8::D3DRS_POINTSIZE_MIN: return D3DRS_POINTSIZE_MIN;
	case d3d8::D3DRS_POINTSPRITEENABLE: return D3DRS_POINTSPRITEENABLE;
	case d3d8::D3DRS_POINTSCALEENABLE: return D3DRS_POINTSCALEENABLE;
	case d3d8::D3DRS_POINTSCALE_A: return D3DRS_POINTSCALE_A;
	case d3d8::D3DRS_POINTSCALE_B: return D3DRS_POINTSCALE_B;
	case d3d8::D3DRS_POINTSCALE_C: return D3DRS_POINTSCALE_C;
	case d3d8::D3DRS_MULTISAMPLEANTIALIAS: return D3DRS_MULTISAMPLEANTIALIAS;
	case d3d8::D3DRS_MULTISAMPLEMASK: return D3DRS_MULTISAMPLEMASK;
	case d3d8::D3DRS_PATCHEDGESTYLE: return D3DRS_PATCHEDGESTYLE;
	case d3d8::D3DRS_DEBUGMONITORTOKEN: return D3DRS_DEBUGMONITORTOKEN;
	case d3d8::D3DRS_POINTSIZE_MAX: return D3DRS_POINTSIZE_MAX;
	case d3d8::D3DRS_INDEXEDVERTEXBLENDENABLE: return D3DRS_INDEXEDVERTEXBLENDENABLE;
	case d3d8::D3DRS_COLORWRITEENABLE: return D3DRS_COLORWRITEENABLE;
	case d3d8::D3DRS_TWEENFACTOR: return D3DRS_TWEENFACTOR;
	case d3d8::D3DRS_BLENDOP: return D3DRS_BLENDOP;
	default:
		LOG(error) << "Unsupported render state type: " << type;
		abort();
	}
}

inline void convert(const D3DCAPS9 &caps, d3d8::D3DCAPS8 *pCaps) {
	// TOOD: Possbile conversions

	ZeroMemory(pCaps, sizeof(d3d8::D3DCAPS8));

	pCaps->DeviceType = (d3d8::D3DDEVTYPE) caps.DeviceType;
	pCaps->AdapterOrdinal = caps.AdapterOrdinal;

	pCaps->Caps = caps.Caps;
	pCaps->Caps2 = caps.Caps2;
	pCaps->Caps3 = caps.Caps3;
	pCaps->PresentationIntervals = caps.PresentationIntervals;

	pCaps->CursorCaps = caps.CursorCaps;

	pCaps->DevCaps = caps.DevCaps;

	pCaps->PrimitiveMiscCaps = caps.PrimitiveMiscCaps;
	pCaps->RasterCaps = caps.RasterCaps;
	pCaps->ZCmpCaps = caps.ZCmpCaps;
	pCaps->SrcBlendCaps = caps.SrcBlendCaps;
	pCaps->DestBlendCaps = caps.DestBlendCaps;
	pCaps->AlphaCmpCaps = caps.AlphaCmpCaps;
	pCaps->ShadeCaps = caps.ShadeCaps;
	pCaps->TextureCaps = caps.TextureCaps;
	pCaps->TextureFilterCaps = caps.TextureFilterCaps;          // D3DPTFILTERCAPS for IDirect3DTexture8's
	pCaps->CubeTextureFilterCaps = caps.CubeTextureFilterCaps;      // D3DPTFILTERCAPS for IDirect3DCubeTexture8's
	pCaps->VolumeTextureFilterCaps = caps.VolumeTextureFilterCaps;    // D3DPTFILTERCAPS for IDirect3DVolumeTexture8's
	pCaps->TextureAddressCaps = caps.TextureAddressCaps;         // D3DPTADDRESSCAPS for IDirect3DTexture8's
	pCaps->VolumeTextureAddressCaps = caps.VolumeTextureAddressCaps;   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture8's

	pCaps->LineCaps = caps.LineCaps;                   // D3DLINECAPS

	pCaps->MaxTextureWidth = caps.MaxTextureWidth;
	pCaps->MaxTextureHeight = caps.MaxTextureHeight;
	pCaps->MaxVolumeExtent = caps.MaxVolumeExtent;

	pCaps->MaxTextureRepeat = caps.MaxTextureRepeat;
	pCaps->MaxTextureAspectRatio = caps.MaxTextureAspectRatio;
	pCaps->MaxAnisotropy = caps.MaxAnisotropy;
	pCaps->MaxVertexW = caps.MaxVertexW;

	pCaps->GuardBandLeft = caps.GuardBandLeft;
	pCaps->GuardBandTop = caps.GuardBandTop;
	pCaps->GuardBandRight = caps.GuardBandRight;
	pCaps->GuardBandBottom = caps.GuardBandBottom;

	pCaps->ExtentsAdjust = caps.ExtentsAdjust;
	pCaps->StencilCaps = caps.StencilCaps;

	pCaps->FVFCaps = caps.FVFCaps;
	pCaps->TextureOpCaps = caps.TextureOpCaps;
	pCaps->MaxTextureBlendStages = caps.MaxTextureBlendStages;
	pCaps->MaxSimultaneousTextures = caps.MaxSimultaneousTextures;

	pCaps->VertexProcessingCaps = caps.VertexProcessingCaps;
	pCaps->MaxActiveLights = caps.MaxActiveLights;
	pCaps->MaxUserClipPlanes = caps.MaxUserClipPlanes;
	pCaps->MaxVertexBlendMatrices = caps.MaxVertexBlendMatrices;
	pCaps->MaxVertexBlendMatrixIndex = caps.MaxVertexBlendMatrixIndex;

	pCaps->MaxPointSize = caps.MaxPointSize;

	pCaps->MaxPrimitiveCount = caps.MaxPrimitiveCount; // max number of primitives per DrawPrimitive call
	pCaps->MaxVertexIndex = caps.MaxVertexIndex;
	pCaps->MaxStreams = caps.MaxStreams;
	pCaps->MaxStreamStride = caps.MaxStreamStride; // max stride for SetStreamSource

	pCaps->VertexShaderVersion = caps.VertexShaderVersion;
	pCaps->MaxVertexShaderConst = caps.MaxVertexShaderConst; // number of vertex shader constant registers

	pCaps->PixelShaderVersion = caps.PixelShaderVersion;
	pCaps->MaxPixelShaderValue = 8; // max value of pixel shader arithmetic component
}
