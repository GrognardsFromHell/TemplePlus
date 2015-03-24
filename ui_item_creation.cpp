
#include "stdafx.h"
#include "ui.h"
#include "fixes.h"
#include "tig_mes.h"
#include "temple_functions.h"
#include "tig_tokenizer.h"

struct UiSystemSpecs {
	UiSystemSpec systems[43];
};
static GlobalStruct<UiSystemSpecs, 0x102F6C10> templeUiSystems;

class UiSystem {
public:

	static UiSystemSpec *getUiSystem(const char *name) {
		// Search for the ui system to replace
		for (auto &system : templeUiSystems->systems) {
			if (!strcmp(name, system.name)) {
				return &system;
			}
		}

		logger->error("Couldn't find UI system {}! Replacement failed.", name);
		return nullptr;
	}
};

struct ButtonStateTextures {
	int normal;
	int hover;
	int pressed;
	ButtonStateTextures() : normal(-1), hover(-1), pressed(-1) {}

	void loadAccept() {
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptNormal, &normal);
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptHover, &hover);
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::AcceptPressed, &pressed);
	}

	void loadDecline() {
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineNormal, &normal);
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclineHover, &hover);
		uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::DeclinePressed, &pressed);
	}
};

static MesHandle mesItemCreationText;
static MesHandle mesItemCreationRules;
static MesHandle mesItemCreationNamesText;
static ImgFile *background = nullptr;
static ButtonStateTextures acceptBtnTextures;
static ButtonStateTextures declineBtnTextures;
static int disabledBtnTexture;

enum class ItemCreationType : uint32_t {
	Alchemy = 0,
	Potion,
	Scroll,
	Wand,
	Rod,
	Wondrous,
	ArmsAndArmor,
	Unk7,
	Unk8
};

static vector<uint64_t> craftingProtoHandles[8];

const char *getProtoName(uint64_t protoHandle) {
	/*
	 // gets item creation proto id
  if ( sub_1009C950((objHndl)protoHandle) )
    v1 = sub_100392E0(protoHandle);
  else
    v1 = sub_10039320((objHndl)protoHandle);

  line.key = v1;
  if ( tig_mes_get_line(ui_itemcreation_names, &line) )
    result = line.value;
  else
    result = ObjGetDisplayName((objHndl)protoHandle, (objHndl)protoHandle);
  return result;
  */

	return templeFuncs.ObjGetDisplayName(protoHandle, protoHandle);
}

static void loadProtoIds(MesHandle mesHandle) {

	for (uint32_t i = 0; i < 8; ++i) {
		auto protoLine = mesFuncs.GetLineById(mesHandle, i);
		if (!protoLine) {
			continue;
		}

		auto &protoHandles = craftingProtoHandles[i];

		StringTokenizer tokenizer(protoLine);
		while (tokenizer.next()) {
			auto handle = templeFuncs.GetProtoHandle(tokenizer.token().numberInt);
			protoHandles.push_back(handle);
		}

		// Sort by prototype name
		sort(protoHandles.begin(), protoHandles.end(), [](uint64_t a, uint64_t b)
		{
			auto nameA = getProtoName(a);
			auto nameB = getProtoName(b);
			return _strcmpi(nameA, nameB);
		});
		logger->info("Loaded {} prototypes for crafting type {}", craftingProtoHandles[i].size(), i);
	}
	
}

static int __cdecl systemInit(const GameSystemConf *conf) {

	mesFuncs.Open("mes\\item_creation.mes", &mesItemCreationText);
	mesFuncs.Open("mes\\item_creation_names.mes", &mesItemCreationNamesText);
	mesFuncs.Open("rules\\item_creation.mes", &mesItemCreationRules);
	loadProtoIds(mesItemCreationRules);

	acceptBtnTextures.loadAccept();
	declineBtnTextures.loadDecline();
	uiFuncs.GetAsset(UiAssetType::Generic, UiGenericAsset::DisabledNormal, &disabledBtnTexture);

	background = uiFuncs.LoadImg("art\\interface\\item_creation_ui\\item_creation.img");

	// TODO !sub_10150F00("rules\\item_creation.mes")

	/*
	tig_texture_register("art\\interface\\item_creation_ui\\craftarms_0.tga", &dword_10BEE38C)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_1.tga", &dword_10BECEE8)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_2.tga", &dword_10BED988)
    || tig_texture_register("art\\interface\\item_creation_ui\\craftarms_3.tga", &dword_10BECEEC)
    || tig_texture_register("art\\interface\\item_creation_ui\\invslot_selected.tga", &dword_10BECDAC)
    || tig_texture_register("art\\interface\\item_creation_ui\\invslot.tga", &dword_10BEE038)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button.tga", &dword_10BEE334)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_grey.tga", &dword_10BED990)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_hover.tga", &dword_10BEE2D8)
    || tig_texture_register("art\\interface\\item_creation_ui\\add_button_press.tga", &dword_10BED79C) )
	*/

	return 0;
}

static void __cdecl systemReset() {
}

static void __cdecl systemExit() {
}

class ItemCreation : public TempleFix {
public:
	const char* name() override {
		return "Item Creation UI";
	}
	void apply() override {
		// auto system = UiSystem::getUiSystem("ItemCreation-UI");		
		// system->init = systemInit;
	}
} itemCreation;
