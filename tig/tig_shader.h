
#pragma once

#include "util/addresses.h"
#include "idxtables.h"

struct TigShaderData;

struct TigShaderType {
	const char *name;
	void *vb_func;
	void *load_func;
	void *get_texture_id_func;
	void *get_color_func;
	void *set_color_func;
};

struct TigShader {
	int (__cdecl *vb_func)(); // Arguments not yet known
	int field_4;
	void(__cdecl *GetTextureId)(TigShaderData *shaderData, int *textureIdOut);
	void (__cdecl *GetColor)(TigShaderData *shaderData, const char *name, D3DCOLOR *colorOut);
	void(__cdecl *SetColor)(TigShaderData *shaderData, const char *name, D3DCOLOR *colorOut);
	TigShaderData *data; // Shader type dependent data
};

struct TigShaderData {
};

enum class TigShaderTexturedFlag : uint32_t {
	TTF_NOT_LIT = 1,
	TTF_COLOR_FILL_ONLY = 2,
	TTF_DISABLE_Z = 4,
	TTF_DOUBLE = 0x10,
	TTF_CLAMP = 0x20
};

struct TigShaderTexturedData : public TigShaderData {
	TigShaderTexturedFlag flags;
	int textureId;
	D3DCOLOR diffuse;
};

struct UvSpeed {
	float u; // Value from mdf times 60
	float v; // Value from mdf times 60
};

enum class TigShaderGenericFlag : uint32_t {
	tig_gsf_double = 1,
	tig_gsf_recalcnormal = 4,
	tig_gsf_zfillonly = 8,
	tig_gsf_colorfillonly = 0x10,
	tig_gsf_notlit = 0x20,
	tig_gsf_disablez = 0x40,
	tig_gsf_linear = 0x80
};

enum class TigShaderUvType : uint32_t {
	tig_uv_mesh = 0,
	tig_uv_env = 1,
	tig_uv_drift = 2,
	tig_uv_swirl = 3,
	tig_uv_wavey = 4
};

enum class TigShaderGenericBlendType : uint32_t {
	tig_bt_modulate = 0,
	tig_bt_add = 1,
	tig_bt_texturealpha = 2,
	tig_bt_currentalpha = 3,
	tig_bt_currentalphaadd = 4
};

enum class TigShaderGenericMaterialBlendType : uint32_t {
	tig_mbt_none = 0,
	tig_mbt_alpha = 1,
	tig_mbt_add = 2,
	tig_mbt_alphaadd = 3
};

struct TigShaderGenericData : public TigShaderData {
	int textureIds[4];
	TigShaderGenericBlendType blendTypes[4];
	TigShaderUvType uvTypes[4];
	int glossmapTexId;
	D3DCOLOR diffuse;
	D3DCOLOR specular;
	float specularPower;
	TigShaderGenericMaterialBlendType materialBlendType;
	TigShaderGenericFlag flags;
	UvSpeed uvSpeeds[4];
};

enum TigShaderClipperFlags {
	TSCT_ZFILL = 1,
	TSCT_WIRE = 2,
	TSCT_OUTLINE = 4
};

struct TigShaderClipperData : public TigShaderData {
	TigShaderClipperFlags flags;
};

struct TigShaderRegistryEntry {
	int id; // Id used to register this shade in the registry
	char name[MAX_PATH]; // Path to the file of the shader (is **INTERNAL MATERIAL** for id=0)
	TigShader shader;

	TigShaderRegistryEntry() {
		static_assert(sizeof(TigShaderRegistryEntry) == 0x120, "TigShaderRegistryEntry must be 0x120 bytes in size");
		memset(this, 0, sizeof(TigShaderRegistryEntry));
	}
};

struct ShaderFuncs : AddressTable {

	int(__cdecl *GetId)(const char *filename, int *shaderIdOut);
	int(__cdecl *GetLoaded)(int shaderId, TigShader *shaderOut);

	ShaderFuncs() {
		rebase(GetLoaded, 0x101E20C0);
		rebase(GetId, 0x101E2160);
	}
};

extern ShaderFuncs shaderFuncs;
extern IdxTableWrapper<TigShaderRegistryEntry> shaderRegistry;
