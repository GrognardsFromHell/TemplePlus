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

class HelpSystem : TempleFix
{
public: 
	const char* name() override { return "Help System Replacements";}
	void apply() override;

	static int HelpTabInit();
	static D20HelpTopic* GetTopic(int topicId);
	static int GenerateLinks(D20HelpTopic * d20ht);
	static int LinkParser(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);


	static int (__cdecl*orgGenerateLinks)(D20HelpTopic * d20ht);
	static int (__cdecl*orgLinkParser)(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);

};

extern HelpSystem helpSys;