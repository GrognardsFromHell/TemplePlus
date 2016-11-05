#include "stdafx.h"
#include "d20_help.h"
#include <temple/dll.h>
#include <tig/tig_tabparser.h>
#include <hashtable.h>
#include <infrastructure/elfhash.h>
#include <tio/tio.h>
#include <tig/tig_tokenizer.h>
#include <spell.h>
#include <feat.h>
#include <skill.h>
#include <d20.h>

struct HelpTabEntry
{
	char * id;
	char * parentId;
	char * prevId;
	char * virtulParents;
	char * title;

};


HelpSystem helpSys;
ToEEHashtableSystem<D20HelpTopic> hashtableSystem;

class HelpSystemReplacements : TempleFix
{
public:
	static int TabLineParserPriliminary(TigTabParser const* tabParser, int lineIdxx, char** columns);
	void apply() override 
	{
		replaceFunction(0x100E7120, TabLineParserPriliminary);

		replaceFunction<void(__cdecl)(int)>(0x10124A40, [](int helpIdx){
			helpSys.PresentWikiHelp(helpIdx);
		});
	}
} fixes;


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

void HelpSystem::ClickForHelpToggle() const
{
	auto& clickForHelpActive = temple::GetRef<int>(0x10BDE3D8);
	clickForHelpActive = 1 - clickForHelpActive;
}

int HelpSystem::HelpTabInit()
{
	*addresses.hashTAG_ROOT = ElfHash::Hash("TAG_ROOT");
	*addresses.isEditor = 0;
	logger->info("Parsing Help Data...");

	// Open the files and do preliminary parsing
	int helpTabNumLines = 0;

	// Original help.tab file first
	TigTabParser tabOrg;
	tabOrg.Init(fixes.TabLineParserPriliminary);
	tabOrg.Open("mes\\help.tab");
	helpTabNumLines += tigTabParserFuncs.GetLineCount(&tabOrg);

	// Temple+ extension
	TigTabParser tabExt;
	tabExt.Init(fixes.TabLineParserPriliminary);
	tabExt.Open("tpmes\\help_extensions.tab");
	int helpExtNumLines = tigTabParserFuncs.GetLineCount(&tabExt);

	// User-files
	TioFileList helpFiles;
	tio_filelist_create(&helpFiles, "mes\\help\\*.tab");

	std::vector<TigTabParser> tabUserFiles;
	for (auto i=0; i < helpFiles.count; i++){
		TigTabParser p;
		p.Init(fixes.TabLineParserPriliminary);
		p.Open(fmt::format("mes\\help\\{}", helpFiles.files[i].name).c_str());
		helpExtNumLines += tigTabParserFuncs.GetLineCount(&p);
		tabUserFiles.push_back(p);
	}
	
	// Init Hashtable with the necessary amount of lines
	ToEEHashtableSystem<D20HelpTopic> hashtableSystem;
	auto helpHashtable = addresses.helpSysHashTable;
	
	int numLinesNew = helpTabNumLines + helpExtNumLines + 100;
	hashtableSystem.HashtableInit(helpHashtable, numLinesNew);

	// First process the original file
	tabOrg.Process();
	tabOrg.Close();

	// Then Temple+ Extension
	tabExt.Process();
	tabExt.Close();

	// Then the user files
	for (auto p : tabUserFiles){
		p.Process();
		p.Close();
	}

	// Generate the links
	int hashTableNumItems = hashtableSystem.HashtableNumItems(addresses.helpSysHashTable);
	D20HelpTopic * d20ht;
	for (int i = 0; i < hashTableNumItems; i++){
		d20ht = hashtableSystem.HashtableGetDataPtr(helpHashtable, i);
		addresses.HelpSystemTopicLinker(d20ht);
	}


	// Now do the post-processing

	tabOrg.Init(addresses.HelpSystemTabLineParserFinal);
	tabOrg.Open("mes\\help.tab");
	tabOrg.Process();
	tabOrg.Close();
	logger->info("Done Parsing Original Help Data.");

	// Temple+ extensions
	tabExt.Init(addresses.HelpSystemTabLineParserFinal);
	tabExt.Open("tpmes\\help_extensions.tab");
	tabExt.Process();
	tabExt.Close();
	
	// User files
	for (auto i = 0; i < helpFiles.count; i++) {
		TigTabParser p;
		p.Init(addresses.HelpSystemTabLineParserFinal);
		p.Open(fmt::format("mes\\help\\{}", helpFiles.files[i].name).c_str());
		p.Process();
		p.Close();
	}

	tio_filelist_destroy(&helpFiles);

	logger->info("Done Parsing Help Data Extensions.");
	return 1;
}


int HelpSystemReplacements::TabLineParserPriliminary(TigTabParser const* tabParser, int lineIdxx, char** columns)
{
	auto tabEntry = (HelpTabEntry*)columns;
	D20HelpTopic * d20ht = new D20HelpTopic;
	
	d20ht->topicId = ElfHash::Hash(tabEntry->id);

	if (*tabEntry->parentId)
		d20ht->parentId = ElfHash::Hash(tabEntry->parentId);
	else
		d20ht->parentId = *addresses.hashTAG_ROOT;

	if (*tabEntry->prevId)
		d20ht->prevId = ElfHash::Hash(tabEntry->prevId);
	else
		d20ht->prevId = *addresses.hashTAG_ROOT;

	d20ht->siblingId = 0;
	d20ht->nextId = 0;
	d20ht->vParentsSize = 0;
	d20ht->virtualParents = nullptr;
	StringTokenizer tok(tabEntry->virtulParents);
	std::vector<uint32_t> vParents;
	while( tok.next())
	{
		if (tok.token().type != StringTokenType::Identifier)
			continue;
		vParents.push_back(ElfHash::Hash(tok.token().text));
		d20ht->virtualParents;
	}
	
	d20ht->vParentsSize = vParents.size();
	if (d20ht->vParentsSize)
	{
		d20ht->virtualParents = new uint32_t[vParents.size()];
		for (auto i = 0u; i < vParents.size(); i++)
		{
			d20ht->virtualParents[i] = vParents[i];
		}
	}
	d20ht->vChildrenSize = 0;
	d20ht->virtualChildren = nullptr;
	d20ht->title = new char[strlen(tabEntry->title)+1];
	strcpy(d20ht->title, tabEntry->title);
	hashtableSystem.HashtableOverwriteItem(addresses.helpSysHashTable, d20ht->topicId, d20ht); // will overwrite if key exists, add item otherwise
	
	return 0;
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

bool HelpSystem::IsClickForHelpActive(){
	return temple::GetRef<int>(0x10BDE3D8) != 0;
}

void HelpSystem::PresentWikiHelp(int helpIdx, D20HelpType helpType){

	auto helpId = GetTAG_ROOT();

	if (helpType == D20HelpType::Default){
		
		if (helpIdx <= 0)
			helpType = D20HelpType::Default;
		else if (helpIdx >= HELP_IDX_UI && helpIdx < HELP_IDX_ALIGNMENT) {
			helpType = D20HelpType::UI;
		}
		else if (helpIdx >= HELP_IDX_ALIGNMENT && helpIdx < HELP_IDX_CLASSES) {
			helpType = D20HelpType::Alignments;
		}
		else if (helpIdx >= HELP_IDX_CLASSES && helpIdx < HELP_IDX_RACES) {
			helpType = D20HelpType::Classes;
		}
		else if (helpIdx >= HELP_IDX_RACES && helpIdx < HELP_IDX_FEATS) {
			helpType = D20HelpType::Races;
		}
		else if (helpIdx >= HELP_IDX_FEATS && helpIdx < HELP_IDX_SKILLS) {
			helpType = D20HelpType::Feats;
		}
		else if (helpIdx >= HELP_IDX_SKILLS && helpIdx < HELP_IDX_SPELLS) {
			helpType = D20HelpType::Skills;
		}
		else if (helpIdx >= HELP_IDX_SPELLS && helpIdx < HELP_IDX_VANILLA_MAX)
			helpType = D20HelpType::Spells;
	}

	int adjustedIdx = 0;
	switch (helpType){
	case D20HelpType::Default: 
		break;
	case D20HelpType::Alignments: 
		helpId = ElfHash::Hash(temple::GetRef<const char*[]>(0x102F8500)[helpIdx - HELP_IDX_ALIGNMENT ]  );
		break;
	case D20HelpType::Classes:
		adjustedIdx = helpIdx - HELP_IDX_CLASSES + stat_level_barbarian;
		helpId = ElfHash::Hash(d20ClassSys.GetClassHelpTopic((Stat)adjustedIdx));
		break;
	case D20HelpType::Feats:
		helpId = ElfHash::Hash(feats.GetFeatHelpTopic((feat_enums)(helpIdx - HELP_IDX_FEATS)) );
		break;
	case D20HelpType::Races:
		helpId = ElfHash::Hash(temple::GetRef<const char*[]>(0x102F84B8)[helpIdx - HELP_IDX_RACES]);
		break;
	case D20HelpType::Skills:
		helpId = ElfHash::Hash(skillSys.GetSkillHelpTopic((SkillEnum)(helpIdx - HELP_IDX_SKILLS)) );
		break;
	case D20HelpType::Spells:
		helpId = ElfHash::Hash(spellSys.GetSpellEnumTAG(helpIdx - HELP_IDX_SPELLS));
		break;
	case D20HelpType::UI:
		helpId = ElfHash::Hash(temple::GetRef<const char*[]>(0x102F838C)[helpIdx - HELP_IDX_UI]);
		break;
	default: 
		break;
	}
	
	PresentWikiHelpWindow(helpId);
	ClickForHelpToggle();


	// temple::GetRef<void(__cdecl)(int)>(0x10124A40)(helpIdx);
}

void HelpSystem::PresentWikiHelpWindow(int topicId)
{
	temple::GetRef<void(__cdecl)(int)>(0x100E6CF0)(topicId);
}

int HelpSystem::GetTAG_ROOT(){
	static int rootHash = ElfHash::Hash("TAG_ROOT");
	return rootHash;
}

void D20RollHistoryEntry::CreateFromString(const char * stringWithRefs){
	temple::GetRef<void(__cdecl)(D20RollHistoryEntry*, const char*)>(0x1010EE00)(this, stringWithRefs);
}
