#pragma once

namespace tig {

struct TigTextureRegistryEntry
{
	bool set_to_true_in_shader;
	int textureId;
	char name[260];
	int width;
	int height;
	TigRect rect;
	int field_124;
	TigBuffer *buffer;
};

}
