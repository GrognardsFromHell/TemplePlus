
#pragma once

#include <temple/dll.h>
#include "idxtables.h"
#include "d3d8adapter.h"
#include "tig.h"

struct TigTexture
{
	int field_0;
	int field_4;
	int filename;
	int buffer;
	int field_10;
	int field_14;
	int field_18;
	int field_1C;
};

struct TigBuffer
{
	int flags_or_type;
	int field_4;
	int field_8;
	int originalwidth;
	int originalheight;
	int texturewidth;
	int textureheight;
	int locked_texture_mem;
	int createargs_field_c;
	int field_24;
	Direct3DTexture8Adapter *d3dtexture;
	int field_2C;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;
	int field_40;
	int field_44;
	int field_48;
	int field_4C;
	int field_50;
	int field_54;
	int field_58;
	int field_5C;
	int field_60;
	int field_64;
	int field_68;
	int field_6C;
	int field_70;
	int field_74;
	int field_78;
	int field_7C;
	int field_80;
	int field_84;
	int field_88;
	int field_8C;
	int field_90;
	int field_94;
	int field_98;
	int field_9C;
	int field_A0;
	int field_A4;
	int field_A8;
	int field_AC;
	int texturetype;
};

struct TigTextureRegistryEntry
{
	bool comes_from_mdf;
	int textureId;
	char name[260];
	int width;
	int height;
	TigRect rect;
	int field_124;
	TigBuffer *buffer;
};

struct TigBufferCreateArgs {
	int flagsOrSth;
	int width;
	int height;
	int zero4;
	int field_10;
	int texturetype;
};

struct TextureFuncs : temple::AddressTable {

	int(__cdecl *CreateBuffer)(TigBufferCreateArgs *createargs, TigBuffer **bufferOut);

	/*
		Registers a texture filename in the texture registry and returns the assigned texture id in in pTexIdOut.
		Returns 0 on success, 17 on failure.
	*/
	int (__cdecl *RegisterTexture)(const char *filename, int *pTexIdOut);

	/*
		Loads the given texture by id or if it has already been loaded, returns the loaded entry.
		Returns 0 on success, 17 on failure.
	*/
	int (__cdecl *LoadTexture)(int textureId, TigTextureRegistryEntry *pTextureOut);

	TextureFuncs() {
		rebase(CreateBuffer, 0x101DCE50);
		rebase(RegisterTexture, 0x101EE7B0);
		rebase(LoadTexture, 0x101EECA0);
	}
};

extern TextureFuncs textureFuncs;
extern IdxTableWrapper<TigTextureRegistryEntry> textureRegistry;
