#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "stdafx.h"

#include "idxtables.h"

struct TigShader
{
	int field_0; // Init to 0 when registered initially via tig_shader_get, set to a callback for the internal material (101E22B0)
	int field_4;
	int field_8; // set to a callback (101E1030) for the internal material
	int field_C; // set to a callback (101E1050) for the internal material
	int field_10;
	/*
	Init to 0 when registered in tig_shader_get.
	To a struct 12 bytes in size with 3 ints (0, 0, -1) for the internal material.
	tig_shader_shutdown will free this field (if it's not 0) for all registered shaders
	*/
	int field_14;
};

struct TigShaderRegistryEntry
{
	int id; // Id used to register this shade in the registry
	char name[MAX_PATH]; // Path to the file of the shader (is **INTERNAL MATERIAL** for id=0)
	TigShader shader;

	TigShaderRegistryEntry()
	{
		static_assert(sizeof(TigShaderRegistryEntry) == 0x120, "TigShaderRegistryEntry must be 0x120 bytes in size");
		memset(this, 0, sizeof(TigShaderRegistryEntry));
	}
};

extern IdxTableWrapper<TigShaderRegistryEntry> shaderRegistry;

void hook_graphics();

#endif // GRAPHICS_H

