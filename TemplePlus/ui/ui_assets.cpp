
#include "stdafx.h"

#include <temple/dll.h>

#include "ui_assets.h"

#include <EASTL/fixed_string.h>

UiAssets *uiAssets;

UiAssets::UiAssets()
{
	Expects(!uiAssets);
	uiAssets = this;

	mTranslationFiles["main_menu"] = MesFile::ParseFile("mes/mainmenu.mes");
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

static bool IsStartOfTranslation(const std::string &text, size_t pos) {
	// #{} Is minimal
	if (pos + 2 >= text.size()) {
		return false;
	}

	return (text[pos] == '#' && text[pos + 1] == '{');
}

std::string UiAssets::ApplyTranslation(const std::string &text)
{
	std::string result;
	eastl::string mesFilename;
	result.reserve(text.size());
	for (size_t i = 0; i < text.size(); i++) {
		if (!IsStartOfTranslation(text, i)) {
			result.push_back(text[i]);
			continue;
		}

		size_t firstToken = i; // If parsing fails, we append the original
		mesFilename.clear();
		
		// Start pushing back tokens until we reach the marker or end of translation
		bool terminated = false;
		for (i = i + 2; i < text.size(); i++) {
			if (text[i] == ':' || text[i] == '}') {
				terminated = text[i] == ':';
				break;
			} else {
				mesFilename.push_back(text[i]);
			}
		}

		if (!terminated) {
			result.append(text.substr(firstToken, i - firstToken));
			continue;
		}

		auto it = mTranslationFiles.find(mesFilename);
		if (it == mTranslationFiles.end()) {
			result.append(text.substr(firstToken, i - firstToken));
			continue;
		}

		// Parse the mes id now
		terminated = false;
		std::string mesLine;
		for (i = i + 1; i < text.size(); i++) {
			if (text[i] == '}') {
				terminated = true;
				break;
			} else {
				mesLine.push_back(text[i]);
			}
		}

		if (!terminated) {
			result.append(text.substr(firstToken, i - firstToken));
			continue;
		}

		int mesLineNo;
		try {
			mesLineNo = std::stoi(mesLine);
		} catch (std::invalid_argument&) {
			result.append(text.substr(firstToken, i - firstToken));
			continue;
		}

		result.append(it->second[mesLineNo]);
	}

	return result;

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
