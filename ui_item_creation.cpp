
#include "stdafx.h"
#include "ui.h"
#include "fixes.h"
#include "tig_mes.h"
#include "temple_functions.h"
#include "tig_tokenizer.h"
#include "ui_item_creation.h"

struct UiSystemSpecs {
	UiSystemSpec systems[43];
};
static GlobalStruct<UiSystemSpecs, 0x102F6C10> templeUiSystems;
GlobalPrimitive<ItemCreationType, 0x10BEDF50> itemCreationType;

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




void CraftScrollWandPotionSetItemSpellData(objHndl objHndItem, objHndl objHndCrafter){

	// the new and improved Wands/Scroll Property Setting Function


	if (itemCreationType == ItemCreationType(3)){
		// do wand specific stuff
	};
	if (itemCreationType == ItemCreationType(2)){
		// do scroll specific stuff
	};

	if (itemCreationType == ItemCreationType(1)){
		// do potion specific stuff
	};

	int numItemSpells = templeFuncs.Obj_Get_IdxField_NumItems(objHndItem, obj_f_item_spell_idx);

	// loop thru the item's spells (can be more than one in principle, like Keoghthem's Ointment)

	// Current code - change this at will...
	for (int n = 0; n < numItemSpells; n++){
		SpellStoredData spellData;
		templeFuncs.Obj_Get_IdxField_256bit(objHndItem, obj_f_item_spell_idx, n, &spellData);

		// get data from caster - make this optional!

		uint32_t classCodes[SPELLENUMMAX] = { 0, };
		uint32_t spellLevels[SPELLENUMMAX] = { 0, };
		uint32_t spellFoundNum = 0;
		int casterKnowsSpell = templeFuncs.ObjSpellKnownQueryGetData(objHndCrafter, spellData.spellEnum, classCodes, spellLevels, &spellFoundNum);
		if (casterKnowsSpell){
			uint32_t spellClassFinal = classCodes[0];
			uint32_t spellLevelFinal = 0;
			uint32_t isClassSpell = classCodes[0] & (0x80);

			if (isClassSpell){
				spellLevelFinal = templeFuncs.ObjGetMaxSpellSlotLevel(objHndCrafter, classCodes[0] & (0x7F), 0);
			};
			if (spellFoundNum > 1){
				for (int i = 1; i < spellFoundNum; i++){
					if (spellLevels[i] > spellLevelFinal){
						spellData.classCode = classCodes[i];
						spellLevelFinal = spellLevels[i];
					};
				}
				spellData.spellLevel = spellLevelFinal;

			};
			spellData.spellLevel = spellLevelFinal;
			templeFuncs.Obj_Set_IdxField_byPtr(objHndItem, obj_f_item_spell_idx, n, &spellData);

		};

	};
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
		replaceFunction(0x10150DA0, CraftScrollWandPotionSetItemSpellData);
	}
} itemCreation;
