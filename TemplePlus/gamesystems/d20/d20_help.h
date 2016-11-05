#pragma once
#include <util/fixes.h>
#include "common.h"

struct D20HelpLink;

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
	void PresentWikiHelp(int topicId);
	void PresentWikiHelpWindow(int topicId);

	static int (__cdecl*orgGenerateLinks)(D20HelpTopic * d20ht);
	static int (__cdecl*orgLinkParser)(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);

};

extern HelpSystem helpSys;