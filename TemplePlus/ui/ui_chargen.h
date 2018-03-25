#pragma once
#include "common.h"
#include "widgets/widgets.h"

class ChargenBigButton;

struct CharEditorSelectionPacket {
	int abilityStats[6];
	int numRerolls; // number of rerolls
	int isPointbuy;
	char rerollString[128];
	Stat statBeingRaised;
	Race raceId; // RACE_INVALID is considered invalid
	int genderId; // 2 is considered invalid
	int height;
	int weight;
	float modelScale; // 0.0 is considered invalid
	int hairStyle;
	int hairColor;
	Stat classCode;
	int deityId;
	int domain1;
	int domain2;
	Alignment alignment;
	int alignmentChoice; // 1 is for Positive Energy, 2 is for Negative Energy
	feat_enums feat0;
	feat_enums feat1;
	feat_enums feat2;
	int skillPointsAdded[42]; // idx corresponds to skill enum
	int skillPointsSpent;
	int availableSkillPoints;
	int spellEnums[SPELL_ENUM_MAX_VANILLA];
	int spellEnumsAddedCount;
	int spellEnumToRemove; // for sorcerers who swap out spells
	int wizSchool;
	int forbiddenSchool1;
	int forbiddenSchool2;
	feat_enums feat3;
	feat_enums feat4;
	int portraitId;
	char voiceFile[256];
	int voiceId; // -1 is considered invalid
};


struct FeatInfo {
	int featEnum;
	int flag = 0;
	int minLevel = 1;
	FeatInfo(int FeatEnum) : featEnum(FeatEnum) {};
	FeatInfo(std::string &featName);
};


class PagianatedChargenSystem
{
public:
	//PagianatedChargenSystem(UiSystemConf & conf);
	int GetPage() { return mWndPage; }
	void SetPageCount(int pageCount) { mPageCount = pageCount; }
	void SetPage(int page);
	void AddPageButtonsToWnd( unique_ptr<WidgetContainer> &);
	void SetPageUpdateHandler(std::function<void()> updateHandler);
protected:
	int mWndPage = 0;
	int mPageCount = 0;
	LgcyWidgetId mPrevBtnId, mNextBtnId;
	std::function<void()> mPageUpdateHandler;
};

class ChargenBigButton : public WidgetButton
{
public:
	enum class ChargenButtonActivationState
	{
		/*
		Note on Active:
		This is not to be confused with WidgetButtonBase's mActivated, which designates
		that some associated state is currently active (e.g. choosing Class in char creation).
		Whereas Active in this case means the button is not Disabled or Done. Perhaps a better
		name would be "Normal", but we're sticking with ToEE nomenclature here.
		*/
		Active = 0,
		Disabled,
		Done,
	};
	void SetActivationState(ChargenButtonActivationState st);
	void UpdateStyleByActivationState();
	void Render() override;

	ChargenBigButton();


protected:
	int datum; // which piece of data does this item represent? influences hovering state
	ChargenButtonActivationState mActivationState = ChargenButtonActivationState::Active;
};

// Interface for paged buttons
class IPagedButton
{
public:
	void SetPage(int page) { mCurPage = page; };
	void SetPageText(int page, const string &text) { pagedText[page] = text; }
	void SetPageDatum(int page, int datum) { pagedData[page] = datum; }

	int GetPage() { return mCurPage; };
	int GetPageCount() { return pagedData.size(); }
	int GetDatum() { return pagedData[mCurPage]; }
	string GetText() { return pagedText[mCurPage]; }

protected:
	int mCurPage = 0;
	std::map<int, int> pagedData; // mapping of page number :  datum
	std::map<int, string> pagedText; // mapping of page number : text
};

class ChargenPagedButton : public ChargenBigButton, public IPagedButton
{
	
};