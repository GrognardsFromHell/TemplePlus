
#pragma once

#include "addresses.h"
#include "idxtables.h"
#include "d3d8to9/d3d8to9.h"

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
	int set_to_true_in_shader;
	int textureId;
	char name[260];
	int field_10C; // Width??
	int field_110; // Height??
	int field_114;
	int field_118;
	int field_11C; // Width??
	int field_120; // Height??
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

struct TextureFuncs : AddressTable {

	int(__cdecl *GetLoaded)(int textureId, TigTextureRegistryEntry *textureOut);

	int(__cdecl *CreateBuffer)(TigBufferCreateArgs *createargs, TigBuffer **bufferOut);

	TextureFuncs() {
		rebase(GetLoaded, 0x101EECA0);
		rebase(CreateBuffer, 0x101DCE50);
	}
};

extern TextureFuncs textureFuncs;
extern IdxTableWrapper<TigTextureRegistryEntry> textureRegistry;
