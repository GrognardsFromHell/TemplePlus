#pragma once
#include <map>


#define BUFF_INDICATOR_COUNT 91
#define AILMENT_INDICATOR_COUNT 77
#define CONDITION_INDICATOR_COUNT 12
#define INDICATOR_COUNT_VANILLA 180

struct WidgetType1;
struct WidgetType2;
struct TigMsg;

enum IndicatorType : int {
	IT_BUFF = 0,
	IT_AILMENT = 1,
	IT_CONDITION = 2
};

struct BuffDebuffSub
{
	uint32_t effectTypeId;
	const char * text;
	int spellEnum;
};

struct BuffDebuffPacket
{
	int buffCount;
	BuffDebuffSub buffs[8];
	int debuffCount;
	BuffDebuffSub debuffs[8];
	int innerCount;
	BuffDebuffSub innerStatuses[6]; // the icons appearing inside the portrait (e.g. Flatfooted)
};

struct PartyPortraitPacket
{
	int unk0;
	int flags;
	int unk2;
	int partyIdx;
	int isActive;
	WidgetType1 *partyUiMain;
	WidgetType2 *partyUiPortraitButton;
	WidgetType2 *hpButton;
	WidgetType2 *subdualButton;
	WidgetType2 *buffs[8];
	WidgetType2 *ailments[8];
	WidgetType2 *conditions[6];
	WidgetType2 *removeFromPartyButton;
	WidgetType2 *levelupButton;
	BuffDebuffPacket *bdb;
};

const int testSizeofPartyPorPacket = sizeof(PartyPortraitPacket); // should be 136 (0x88)

struct IndicatorSpec{
	int id;
	std::string tooltipText;
	std::string textureFilename;
	std::string helpTopicName;
	int textureId;
	int helpTopicId;
	int type; // 0 - buff, 1 - ailment, 2 - condition
	IndicatorSpec() {};
	IndicatorSpec(std::string &filename, std::string &help, int indType);
};

class UiParty
{
	friend class UiPartyHooks;
public:

	// utils
	int IndicatorGetHelpHash(BuffDebuffPacket &bdb, int effectType, int iconIdx); // 100F4500
	int IndicatorGetTextureId(BuffDebuffPacket &bdb, int effectType, int iconIdx); 
	int IndicatorGetCount(BuffDebuffPacket &bdb, int effectType);
	void IndicatorFindWidId(int widId, PartyPortraitPacket**, int *effectType, int *iconIdx);

	// Indicator Widget
	void IndicatorRender(int widId);
	BOOL IndicatorMsg(int widId, TigMsg *msg);
	void IndicatorTextGet(BuffDebuffPacket &bdb, int effectType, int iconIdx, char *text);
	IndicatorSpec& IndicatorSpecGet(int effectId);


	BOOL PortraitRenderEnabled();

protected:
	void InitIndicatorSpecs();
	void GetNewIndicatorSpecs();
	std::map<int, IndicatorSpec> buffDebuffSpecs;
};

extern UiParty uiParty;