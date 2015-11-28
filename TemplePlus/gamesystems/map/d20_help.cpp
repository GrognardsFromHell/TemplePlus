#include "stdafx.h"
#include "d20_help.h"
#include <temple/dll.h>
#include <tig/tig_tabparser.h>
#include <hashtable.h>
#include <infrastructure/elfhash.h>
#include <tio/tio.h>


HelpSystem helpSys;
int(__cdecl*HelpSystem::orgGenerateLinks)(D20HelpTopic * d20ht);
int(__cdecl*HelpSystem::orgLinkParser)(D20HelpLink* d20hl, char * topicTitle, char **pos1, char **pos2, int *offsetOut);

struct HelpSystemAddresses : temple::AddressTable
{
	/*
		generates strings for the headers and stuff and stores them in the hashtable
	*/
	TigTabLineParser * HelpSystemTabLineParserPreliminary;
	TigTabLineParser * HelpSystemTabLineParserFinal;
	ToEEHashtable<D20HelpTopic> *helpSysHashTable;
	int * hashTAG_ROOT; // contains the hash of the string "TAG_ROOT"
	int * isEditor;
	/*
	  creates the various relationships (siblings, parents, virtual parents, virtual children etc)
	*/
	int(__cdecl* HelpSystemTopicLinker)(D20HelpTopic * d20ht);

	HelpSystemAddresses()
	{
		rebase(HelpSystemTabLineParserPreliminary, 0x100E7120);
		rebase(HelpSystemTopicLinker, 0x100E7280);
		rebase(HelpSystemTabLineParserFinal, 0x100E7CF0);

		rebase(hashTAG_ROOT, 0x10BD01BC);
		rebase(isEditor, 0x10BD01B8);
		rebase(helpSysHashTable, 0x118676E0);
		
	}
} addresses;


void HelpSystem::apply()
{
	replaceFunction(0x100E7030, GetTopic);
	orgGenerateLinks = replaceFunction(0x100E7280, GenerateLinks);
	orgLinkParser = replaceFunction(0x100E7670, LinkParser);
	replaceFunction(0x100E7E80, HelpTabInit);
}

int HelpSystem::HelpTabInit()
{
	*addresses.hashTAG_ROOT = ElfHash::Hash("TAG_ROOT");
	*addresses.isEditor = 0;
	logger->info("Parsing Help Data...");

	TigTabParser tabOrg;
	tigTabParserFuncs.Init(&tabOrg, addresses.HelpSystemTabLineParserPreliminary);
	tigTabParserFuncs.Open(&tabOrg, "mes\\help.tab");
	int helpTabNumLines = tigTabParserFuncs.GetLineCount(&tabOrg);


	// the extensions
	TigTabParser tabExt;
	tigTabParserFuncs.Init(&tabExt, addresses.HelpSystemTabLineParserPreliminary);
	tigTabParserFuncs.Open(&tabExt, "tpmes\\help_extensions.tab");
	int helpExtNumLines = tigTabParserFuncs.GetLineCount(&tabExt);
	
	
	ToEEHashtableSystem<D20HelpTopic> hashtableSystem;
	auto helpHashtable = addresses.helpSysHashTable;
	hashtableSystem.HashtableInit(helpHashtable, helpTabNumLines + helpExtNumLines + 100);

	tigTabParserFuncs.Process(&tabOrg);
	tigTabParserFuncs.Close(&tabOrg);


	tigTabParserFuncs.Process(&tabExt);
	tigTabParserFuncs.Close(&tabExt);

	// generate the links
	int hashTableNumItems = hashtableSystem.HashtableNumItems(addresses.helpSysHashTable);
	D20HelpTopic * d20ht;
	for (int i = 0; i < hashTableNumItems; i++)
	{
		d20ht = hashtableSystem.HashtableGetDataPtr(helpHashtable, i);
		addresses.HelpSystemTopicLinker(d20ht);
	}

	tigTabParserFuncs.Init(&tabOrg, addresses.HelpSystemTabLineParserFinal);
	tigTabParserFuncs.Open(&tabOrg, "mes\\help.tab");
	tigTabParserFuncs.Process(&tabOrg);
	tigTabParserFuncs.Close(&tabOrg);

	logger->info("Done Parsing Original Help Data.");

	tigTabParserFuncs.Init(&tabExt, addresses.HelpSystemTabLineParserFinal);
	tigTabParserFuncs.Open(&tabExt, "tpmes\\help_extensions.tab");
	//auto fileLol = tio_fopen("tpmes\\help_extensions.tab", "rb");
	tigTabParserFuncs.Process(&tabExt);
	tigTabParserFuncs.Close(&tabExt);

	logger->info("Done Parsing Help Data Extensions.");
	return 1;
}

D20HelpTopic* HelpSystem::GetTopic(int topicId)
{
	ToEEHashtableSystem<D20HelpTopic> hashtableSystem;
	auto helpHashtable = addresses.helpSysHashTable;
	D20HelpTopic * d20ht;
//	if (ElfHash::Hash("TAG_DIVINE_MIGHT") == topicId)
//	{
//		int duum = 1;
//	}
	auto result = hashtableSystem.HashtableSearch(helpHashtable, topicId, &d20ht);
	if (result)
		return 0;
	return d20ht;
}

int HelpSystem::GenerateLinks(D20HelpTopic* d20ht)
{
	return orgGenerateLinks(d20ht);
}

int HelpSystem::LinkParser(D20HelpLink* d20hl, char* topicTitle, char** pos1, char** pos2, int* offsetOut)
{
	if (strstr(topicTitle,"Disable Attacks"))
	{
		int dum = 1;
	}
	int result = orgLinkParser(d20hl, topicTitle, pos1, pos2, offsetOut);
	return result;
}