#pragma once
#include <util/fixes.h>
#include "common.h"

#define HELP_IDX_UI 1  // 1 to 74;   75,76 are separators
#define HELP_IDX_ALIGNMENT 76  // 76 to 86;    87, 88 are separators
#define HELP_IDX_CLASSES 88 // 89 to 99;    100,102 are separators
#define HELP_IDX_RACES 101 // 101 to 108;   108, 109 are separators
#define HELP_IDX_FEATS 109 // 109 to 757
#define HELP_IDX_SKILLS 759 // 759 to 858
#define HELP_IDX_SPELLS 860 // vanilla spell are mapped to 860 up to 860 + 802 (the vanilla spell count)
#define HELP_IDX_VANILLA_MAX 1661

struct D20HelpLink;

enum class D20HelpType {
	Default = 0,
	Alignments,
	Classes,
	Feats,
	Races,
	Skills,
	Spells,
	UI
};

struct D20HelpTopic
{ // all ids are elf hashes
	int topicId;
	int parentId; 
	int nextId;
	int prevId;
	int siblingId;
	int vParentsSize;
	uint32_t * virtualParents; // ids for topics which will list this topic when using the command [CMD_CHILDREN] inside the text body
	int vChildrenSize;
	int * virtualChildren;
	char * title;
	unsigned char * formattedLinkStrings; // up to 64 strings of 256 chars each, with formating symbols like @1 and stuff
	int numLinks;
	D20HelpLink * links;
};

struct D20HelpLink
{
	int isRoll; // is 1 for ROLL_ type links, 0 otherwise
	int linkedTopicId;
	int field8;
	int startPos;
	int endPos;
};

struct D20RollHistoryEntry
{
	char * string;
	int stringSize;
	int field8;
	int stringLen;
	D20HelpLink *links;
	int numLinks; //I think
	int idx;
	int num2;

	void Clear(){
		this->idx = 0;
		this->num2 = 0;
		this->field8 = 0;
		this->stringLen = 0;
		this->string = nullptr;
	}

	void CreateFromString(const char *stringWithRefs);
};

class HelpSystem : TempleFix
{
public: 
	void apply() override;
	void ClickForHelpToggle() const;
	static int HelpTabInit();
	static D20HelpTopic* GetTopic(int topicId);
	static int GenerateLinks(D20HelpTopic * d20ht);
	static int LinkParser(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);
	bool IsClickForHelpActive();
	void PresentWikiHelp(int topicIdx, D20HelpType helpType = D20HelpType::Default); // topicIdx is not the hashcode of a TAG_XXX string - it's a hardcoded index that gets converted via a lookup table
	void PresentWikiHelpWindow(int topicId);

	static int (__cdecl*orgGenerateLinks)(D20HelpTopic * d20ht);
	static int (__cdecl*orgLinkParser)(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);

protected:
	int GetTAG_ROOT();
};

extern HelpSystem helpSys;