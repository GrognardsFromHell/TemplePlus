
#include "stdafx.h"

#include <temple/dll.h>

#include "ui_assets.h"

UiAssets *uiAssets;

UiAssets::UiAssets()
{
	Expects(!uiAssets);
	uiAssets = this;
}

UiAssets::~UiAssets()
{
	if (uiAssets == this) {
		uiAssets = nullptr;
	}
}

bool UiAssets::GetAsset(UiAssetType assetType, UiGenericAsset assetIndex, int& textureIdOut) {
	static auto ui_get_common_texture_id = temple::GetPointer<signed int(UiAssetType assetType, UiGenericAsset assetIdx, int &textureIdOut, int a4)>(0x1004a360);
	return ui_get_common_texture_id(assetType, assetIndex, textureIdOut, 0) == 0;
}

ImgFile* UiAssets::LoadImg(const char* filename) {
	static auto load_img_file = temple::GetPointer<ImgFile *(const char *filename)>(0x101e8320);
	return load_img_file(filename);
}

const char* UiAssets::GetTooltipString(int line) const
{
	auto getTooltipString = temple::GetRef<const char*(__cdecl)(int)>(0x10122DA0);
	return getTooltipString(line);
}

const char* UiAssets::GetStatShortName(Stat stat) const
{
	return temple::GetRef<const char*(__cdecl)(Stat)>(0x10074980)(stat);
}

const char * UiAssets::GetStatMesLine(int lineNumber) const
{
	auto mesHandle = temple::GetRef<MesHandle>(0x10AAF1F4);
	MesLine line(lineNumber);
	mesFuncs.GetLine_Safe(mesHandle, &line);
	return line.value;
}
