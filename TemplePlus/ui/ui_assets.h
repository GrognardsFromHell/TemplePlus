
#pragma once

#include <cstdint>
#include <obj.h>

#include <infrastructure/mesparser.h>

#include <EASTL/hash_map.h>
#include <EASTL/string.h>

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

	/**
	 * Replaces placeholders of the form #{main_menu:123} with the key 123 from the mes file registered
	 * as main_menu.
	 */
	std::string ApplyTranslation(const std::string &text);

	const char* GetTooltipString(int line) const;
	const char* GetStatShortName(Stat stat) const;
	const char* GetStatMesLine(int line) const;
	
private:
	eastl::hash_map<eastl::string, MesFile::Content> mTranslationFiles;

};

extern UiAssets* uiAssets;
