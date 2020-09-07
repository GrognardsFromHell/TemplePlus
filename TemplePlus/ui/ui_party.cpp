#include "stdafx.h"
#include "ui_party.h"
#include <util/fixes.h>
#include "common.h"
#include <temple/dll.h>
#include <infrastructure/elfhash.h>
#include <tig/tig_texture.h>
#include <tig/tig_mes.h>
#include <ui/ui.h>
#include "ui_render.h"
#include <tig/tig_msg.h>
#include <gamesystems/d20/d20_help.h>
LegacyUiParty uiParty;

// TODO: merge this with UiParty

class UiPartyHooks : public TempleFix {
public:


	void apply() override {

		// hook the buff/debuff icon so it doesn't try to get a spell name if the spellEnum is -2
		static char* (__cdecl* orgBuffDebuffGetBaseText)(BuffDebuffPacket*, int, int) = 
			replaceFunction<char*(__cdecl)(BuffDebuffPacket* , int , int )>(0x100F45A0, [](BuffDebuffPacket* bdb, int iconType, int iconIdx)->char*{
			if (iconType == 0){ // buff
				auto spEnum = bdb->buffs[iconIdx ].spellEnum;
				if (spEnum == -2){
					return "";
				}
			}
			return orgBuffDebuffGetBaseText(bdb, iconType, iconIdx);
		});
		
		/*static void (__cdecl*orgBdbTooltip)(int, int, int*) = replaceFunction<void(__cdecl)(int, int, int*)>(0x10131EA0, [](int x, int y, int* widId)
		{
			orgBdbTooltip(x, y, widId);
		});*/

		replaceFunction<void(__cdecl)()>(0x100F4220, [](){
			uiParty.InitIndicatorSpecs();
		});


		// IndicatorRender
		replaceFunction<void(__cdecl)(int)>(0x10131DB0, [](int widId){
			uiParty.IndicatorRender(widId);
		});
		// IndicatorMsg
		replaceFunction<BOOL(__cdecl)(int, TigMsg*)>(0x101323D0, [](int widId, TigMsg* msg)
		{
			return uiParty.IndicatorMsg(widId, msg);
		});
		// IndicatorTextGet
		replaceFunction<void(__cdecl)(BuffDebuffPacket &bdb, int effectType, int iconIdx, char *text)>(0x100F46F0, [](BuffDebuffPacket &bdb, int effectType, int iconIdx, char *text){
			uiParty.IndicatorTextGet(bdb, effectType, iconIdx, text);
		});

	}
} uiPartyHooks;

int LegacyUiParty::IndicatorGetHelpHash(BuffDebuffPacket & bdb, int effectType, int iconIdx){
	auto effectId = -1, spellEnum = -1;
	switch (effectType)
	{
	case IT_BUFF:
		effectId = bdb.buffs[iconIdx].effectTypeId;
		spellEnum = bdb.buffs[iconIdx].spellEnum;
		break;
	case IT_AILMENT:
		effectId = bdb.debuffs[iconIdx].effectTypeId;
		spellEnum = bdb.debuffs[iconIdx].spellEnum;
		break;
	case IT_CONDITION:
		effectId = bdb.innerStatuses[iconIdx].effectTypeId;
		spellEnum = bdb.innerStatuses[iconIdx].spellEnum;
		break;
	default:
		return 0;
	}

	auto findSpec = buffDebuffSpecs.find(effectId);
	if (findSpec == buffDebuffSpecs.end())
		return 0;

	if (spellEnum <= 0){
		return findSpec->second.helpTopicId;
	}

	return ElfHash::Hash(spellSys.GetSpellEnumTAG((uint32_t)spellEnum));
}

int LegacyUiParty::IndicatorGetTextureId(BuffDebuffPacket & bdb, int effectType, int iconIdx){
	auto effectId = -1;
	switch (effectType)
	{
	case IT_BUFF:
		effectId = bdb.buffs[iconIdx].effectTypeId;
		break;
	case IT_AILMENT:
		effectId = bdb.debuffs[iconIdx].effectTypeId;
		break;
	case IT_CONDITION:
		effectId = bdb.innerStatuses[iconIdx].effectTypeId;
		break;
	default:
		return 0;
	}

	auto findSpec = buffDebuffSpecs.find(effectId);
	if (findSpec == buffDebuffSpecs.end())
		return 0;

	return findSpec->second.textureId;
}

int LegacyUiParty::IndicatorGetCount(BuffDebuffPacket & bdb, int effectType)
{
	switch (effectType){
	case IT_BUFF:
		return bdb.buffCount;
	case IT_AILMENT:
		return bdb.debuffCount;
	case IT_CONDITION:
		return bdb.innerCount;
	default:
		return 0;
	}
}

void LegacyUiParty::IndicatorFindWidId(int widId, PartyPortraitPacket **porPkt, int * effectType, int * iconIdx){
	temple::GetRef<void(__cdecl)(int, PartyPortraitPacket **, int*, int*)>(0x10131CD0)(widId, porPkt, effectType, iconIdx);
}

void LegacyUiParty::IndicatorRender(int widId){

	if (!PortraitRenderEnabled())
		return;

	auto btn = uiManager->GetButton(widId);
	if (!btn)
		return;

	PartyPortraitPacket *porPkt = nullptr;
	int effectType, iconIdx;
	IndicatorFindWidId(widId, &porPkt, &effectType, &iconIdx);
	if (!porPkt)
		return;

	auto count = IndicatorGetCount(*porPkt->bdb, effectType);
	if (iconIdx < count)
	{
		TigRect srcRect(0,0, btn->width, btn->height), destRect(btn->x, btn->y, btn->width, btn->height);
		UiRenderer::DrawTexture(IndicatorGetTextureId(*porPkt->bdb, effectType, iconIdx), destRect, srcRect);
	}
}

BOOL LegacyUiParty::IndicatorMsg(int widId, TigMsg * msg){

	if (msg->type != TigMsgType::WIDGET)
		return FALSE;
	auto _msg = (TigMsgWidget*)msg;
	if (_msg->widgetEventType != TigMsgWidgetEvent::MouseReleased)
		return FALSE;

	PartyPortraitPacket *porPkt = nullptr;
	int effectType, iconIdx;
	IndicatorFindWidId(widId, &porPkt, &effectType, &iconIdx);
	if (!porPkt)
		return FALSE;

	auto count = IndicatorGetCount(*porPkt->bdb, effectType);
	if (iconIdx >= count)
		return FALSE;

	auto helpId = IndicatorGetHelpHash(*porPkt->bdb, effectType, iconIdx);
	if (helpId)
		helpSys.PresentWikiHelpWindow(helpId);
	return TRUE;
}

void LegacyUiParty::IndicatorTextGet(BuffDebuffPacket & bdb, int effectType, int iconIdx, char * text){
	auto effectId = -1, spellEnum = -1;
	const char *extraText = nullptr;
	switch (effectType)
	{
	case IT_BUFF:
		effectId = bdb.buffs[iconIdx].effectTypeId;
		spellEnum = bdb.buffs[iconIdx].spellEnum;
		extraText = bdb.buffs[iconIdx].text;
		break;
	case IT_AILMENT:
		effectId = bdb.debuffs[iconIdx].effectTypeId;
		spellEnum = bdb.debuffs[iconIdx].spellEnum;
		extraText = bdb.debuffs[iconIdx].text;
		break;
	case IT_CONDITION:
		effectId = bdb.innerStatuses[iconIdx].effectTypeId;
		spellEnum = bdb.innerStatuses[iconIdx].spellEnum;
		extraText = bdb.innerStatuses[iconIdx].text;
		break;
	default:
		return;
	}

	auto findSpec = buffDebuffSpecs.find(effectId);
	if (findSpec == buffDebuffSpecs.end())
		return;

	if (extraText == nullptr)
		extraText = "";

	// get base text
	std::string baseText;

	auto indicatorMes = temple::GetRef<MesHandle>(0x10BD24C8);
	MesLine line;
	if (spellEnum > 0){
		baseText = spellSys.GetSpellMesline(spellEnum);
	}
	else if (effectId >=0 && effectId < BUFF_INDICATOR_COUNT){
		line.key = effectId;
		mesFuncs.GetLine_Safe(indicatorMes, &line);
		baseText = line.value;
	}
	else if (effectId >= BUFF_INDICATOR_COUNT && effectId < BUFF_INDICATOR_COUNT + AILMENT_INDICATOR_COUNT) {
		line.key = effectId - BUFF_INDICATOR_COUNT + 1000 ;
		mesFuncs.GetLine_Safe(indicatorMes, &line);
		baseText = line.value;
	}
	else if (effectId >= BUFF_INDICATOR_COUNT + AILMENT_INDICATOR_COUNT && effectId < INDICATOR_COUNT_VANILLA) {
		line.key = effectId - (BUFF_INDICATOR_COUNT + AILMENT_INDICATOR_COUNT ) + 2000;
		mesFuncs.GetLine_Safe(indicatorMes, &line);
		baseText = line.value;
	} else // this is the new shit
	{
		auto findSpec = IndicatorSpecGet(effectId);
		baseText = findSpec.tooltipText;
	}


	sprintf(text, "%s %s", baseText.c_str(), extraText);
}

IndicatorSpec & LegacyUiParty::IndicatorSpecGet(int effectId){
	return buffDebuffSpecs[effectId];
}

BOOL LegacyUiParty::PortraitRenderEnabled(){
	return temple::GetRef<int>(0x10BE33E4) == 0;
}

void LegacyUiParty::InitIndicatorSpecs(){

	struct VanillaIndicatorDictEntry{
		const char *filename;
		const char * helpTopicName;
	};
	auto &buffDict = temple::GetRef<VanillaIndicatorDictEntry[BUFF_INDICATOR_COUNT]>(0x102E8DA8);
	auto &ailmentDict = temple::GetRef<VanillaIndicatorDictEntry[AILMENT_INDICATOR_COUNT]>(0x102E9080);
	auto &condIndDict = temple::GetRef<VanillaIndicatorDictEntry[CONDITION_INDICATOR_COUNT]>(0x102E92E8);

	
	for (auto i=0; i < BUFF_INDICATOR_COUNT; i ++){
		auto id = i;
		std::string helpTopic;
		if (buffDict[i].helpTopicName){
			helpTopic = fmt::format("{}", buffDict[i].helpTopicName);
		}

		buffDebuffSpecs[id] = IndicatorSpec(fmt::format("art\\interface\\player_conditions\\buffs\\{}", buffDict[i].filename), helpTopic, 0);
	}
	
	for (auto i = 0; i < AILMENT_INDICATOR_COUNT; i++) {
		auto id = i + BUFF_INDICATOR_COUNT;
		std::string helpTopic;
		if (ailmentDict[i].helpTopicName) {
			helpTopic = fmt::format("{}", ailmentDict[i].helpTopicName);
		}

		buffDebuffSpecs[id] = IndicatorSpec(fmt::format("art\\interface\\player_conditions\\ailments\\{}", ailmentDict[i].filename), helpTopic, 1);
	}

	for (auto i = 0; i < CONDITION_INDICATOR_COUNT; i++) {
		auto id = i + BUFF_INDICATOR_COUNT + AILMENT_INDICATOR_COUNT;
		std::string helpTopic;
		if (condIndDict[i].helpTopicName) {
			helpTopic = fmt::format("{}", condIndDict[i].helpTopicName);
		}
		buffDebuffSpecs[id] = IndicatorSpec(fmt::format("art\\interface\\player_conditions\\conditions\\{}", condIndDict[i].filename), helpTopic, 2);
	}

	GetNewIndicatorSpecs();

	// register textures
	for (auto it: buffDebuffSpecs){
		auto id = it.first;
		textureFuncs.RegisterTexture(buffDebuffSpecs[id].textureFilename.c_str(), &buffDebuffSpecs[id].textureId);
	}

	mesFuncs.Open("mes\\indicator.mes", &temple::GetRef<MesHandle>(0x10BD24C8));
}

void LegacyUiParty::GetNewIndicatorSpecs()
{
	TioFileList flist;
	tio_filelist_create(&flist, "rules\\indicators\\*.txt");

	for (auto i = 0; i<flist.count; i++) {

		auto &f = flist.files[i];
		auto indFile = tio_fopen(fmt::format("rules\\indicators\\{}", f.name).c_str(), "rt");

		char lineContent[2000] = { 0, };
		IndicatorSpec indSpec;
		while (tio_fgets(lineContent, 1000, indFile)) {
			auto len = strlen(lineContent);
			if (!len)
				continue;

			if (lineContent[len - 1] == *"\n") {
				lineContent[len - 1] = 0;
			}

			if (!_strnicmp(lineContent, "ID_string", 9)) {
				for (auto ch = lineContent + 9; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					indSpec.id = ElfHash::Hash(fmt::format("{}", ch));
					break;
				}

			}

			else if (!_strnicmp(lineContent, "effect_type", 11)) {

				for (auto ch = lineContent + 11; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					indSpec.type = atol(fmt::format("{}", ch).c_str());
					break;
				}
			}
			else if (!_strnicmp(lineContent, "texture_file", 12)) {

				for (auto ch = lineContent + 12; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					indSpec.textureFilename = fmt::format("{}", ch);
					break;
				}
			}

			else if (!_strnicmp(lineContent, "help_topic", 10))
			{
				for (auto ch = lineContent + 10; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					indSpec.helpTopicName = fmt::format("{}", ch);
					indSpec.helpTopicId = ElfHash::Hash(indSpec.helpTopicName);
					break;
				}
			}

			else if (!_strnicmp(lineContent, "tooltip_base_text", 17))
			{
				for (auto ch = lineContent + 17; *ch != 0; ch++)
				{
					if (*ch == ' ' || *ch == ':')
						continue;
					indSpec.tooltipText = fmt::format("{}", ch);
					break;
				}
			}
		}

		if (indSpec.id) {
			Expects((uint32_t)indSpec.id > INDICATOR_COUNT_VANILLA ); // ensure no collision with the normal ToEE indicator IDs
			buffDebuffSpecs[indSpec.id] = indSpec;
		}

		tio_fclose(indFile);

	}

	tio_filelist_destroy(&flist);
}

IndicatorSpec::IndicatorSpec(std::string & filename, std::string & help, int indType){
	this->textureFilename = filename;
	this->helpTopicName = help;
	this->helpTopicId = ElfHash::Hash(help);
	this->type = indType;
}
