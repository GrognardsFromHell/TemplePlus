
#pragma once

#include <cstdint>
#include <obj.h>

struct ImgFile {
	int tilesX;
	int tilesY;
	int width;
	int height;
	int textureIds[16];
	int field_50;
};

enum class UiAssetType : uint32_t {
	Portraits = 0,
	Inventory,
	Generic, // Textures
	GenericLarge // IMG files
};

enum class UiGenericAsset : uint32_t {
	AcceptHover = 0,
	AcceptNormal,
	AcceptPressed,
	DeclineHover,
	DeclineNormal,
	DeclinePressed,
	DisabledNormal,
	GenericDialogueCheck
};

class UiAssets {
public:

	UiAssets();
	~UiAssets();

	bool GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int &textureIdOut);

	/*
		Loads a .img file.
	*/
	ImgFile* LoadImg(const char *filename);

	const char* GetTooltipString(int line) const;
	const char* GetStatShortName(Stat stat) const;
	const char* GetStatMesLine(int line) const;

};

extern UiAssets* uiAssets;
