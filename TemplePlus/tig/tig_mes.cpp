
#include "stdafx.h"
#include "tig_mes.h"
#include "util/fixes.h"
#include "infrastructure/stringutil.h"
#include <tio/tio.h>

MesFuncs mesFuncs;

void MergeContents(MesHandle tgt, MesHandle src);
bool FindMesHandle(const char* fname, MesHandle & handleOut);

struct MesFileEntry {
	char fileName[260];
	int refCount;
	int numLines;
	int capacity;
	MesLine* lines;
	char* rawBuffer; // note: this starts out as the raw file contents, but parsing can alter it somewhat
	size_t rawBufferLen; 
};

MesLine::MesLine()
{
	key = 0;
	value = nullptr;
}

MesLine::MesLine(uint32_t Key){
	key = Key;
	value = nullptr;
}

MesLine::MesLine(uint32_t Key, const char* Line){
	key = Key;
	value = Line;
}

void MesFuncs::AddToMap(MesHandle openedMesHandle, std::map<int, std::string>& mesMap, int *highestKey){
	auto mh = openedMesHandle;
	auto numLines = mesFuncs.GetNumLines(mh);
	for (auto i=0; i < numLines; i++){
		MesLine line;
		mesFuncs.ReadLineDirect(mh, i, &line);
		mesMap[line.key] = line.value;
		if (highestKey && *highestKey < line.key)
			*highestKey = line.key;
	}
}

void MesFuncs::AddToMap(std::string & mesFilename, std::map<int, std::string>& mesMap, int *highestKey){
	MesHandle mh;
	mesFuncs.Open(mesFilename.c_str(), &mh);
	AddToMap(mh, mesMap, highestKey);
	mesFuncs.Close(mh);
}

int MesFuncs::GetNumLines(MesHandle mesHandle){
	return temple::GetRef<int(__cdecl)(MesHandle)>(0x101E62F0)(mesHandle);
}

BOOL MesFuncs::ReadLineDirect(MesHandle mesHandle, int lineCount, MesLine* mesLine){
	return temple::GetRef<BOOL(__cdecl)(MesHandle, int, MesLine*)>(0x101E6310)(mesHandle, lineCount, mesLine);
}

bool MesFuncs::GetFirstLine(MesHandle mesHandle, MesLine* lineOut){
	// todo
	return temple::GetRef<BOOL(__cdecl)(MesHandle, MesLine*)>(0x101E62B0)(mesHandle, lineOut) != FALSE;
}


class MesFuncHooks : public TempleFix {

	void apply() override;
} mesHooks;

void MesFuncHooks::apply()
{

	static bool(__cdecl * orgOpen)(const char*, MesHandle*) = replaceFunction<bool(__cdecl)(const char*, MesHandle*)>(0x101E6D00, [](const char* fname, MesHandle* handle)->bool {

		//static auto findHandleAllocIfNot = temple::GetRef<BOOL(__cdecl)(const char*, MesHandle*)>(0x101E63E0);

		auto mesEntryList = temple::GetRef<MesFileEntry*>(0x10EEEA00);

		auto isAlreadyOpen = FindMesHandle(fname, *handle);

		if (!orgOpen(fname, handle)) {
			return false;
		}
		auto mes = &mesEntryList[*handle]; // for debug

		if (isAlreadyOpen) { // extend only once
			return true;
		}



		auto lowerFname = tolower(fname);
		auto mesPos = lowerFname.find(".mes");
		if (mesPos == std::string::npos) {
			logger->error("Bad mesname encountered: {}", fname);
			return orgOpen(fname, handle);
		}

		// Search for extension files under the directory with same filename
		auto baseDir = lowerFname.substr(0, mesPos);

		// search for fname_ext.mes
		auto extFname = baseDir + "_ext.mes";
		if (tio_fileexists(extFname.c_str() ) ){
			MesHandle tmp;
			if (orgOpen(extFname.c_str(), &tmp)) {
				MergeContents(*handle, tmp);
				
				mesFuncs.Close(tmp);
			}

		}

		auto extDirPat = baseDir + "_ext\\*.mes";
		TioFileList flist;
		tio_filelist_create(&flist, extDirPat.c_str());
		for (auto i = 0u; i < flist.count; ++i) {
			auto combinedFname = fmt::format("{}\\{}", baseDir, flist.files[i].name);
			MesHandle tmp;
			if (orgOpen(combinedFname.c_str(), &tmp)) {
				MergeContents(*handle, tmp);
				mesFuncs.Close(tmp);
			}
		}

		return true;
		});
}

void MergeContents(MesHandle tgt, MesHandle src) {

	auto mesfileCount = temple::GetRef<int>(0x10EEE9FC);
	if (tgt >= mesfileCount || src >= mesfileCount || tgt < 0 || src < 0) {
		logger->error("Tried to merge invalid mes handle! handles {} {} count: {}", tgt, src, mesfileCount);
		return;
	}
	 
	auto mesEntryList = temple::GetRef<MesFileEntry*>(0x10EEEA00);

	auto mes = &mesEntryList[tgt];
	auto mes2 = &mesEntryList[src];
	if (!mes2->rawBuffer || !mes2->numLines) {
		return;
	}
	if (!mes->numLines) { // copying into an empty target, no worries of collisions
		return mesFuncs.CopyContents(tgt, src);
	}


	auto newLen = mes->rawBufferLen + mes2->rawBufferLen;
	auto contentsExt = (char*) realloc(mes->rawBuffer, newLen);
	auto oldBuffer = mes->rawBuffer;
	if (!contentsExt) {
		return;
	}
	
	mes->rawBuffer = contentsExt;
	memcpy( &mes->rawBuffer[mes->rawBufferLen], mes2->rawBuffer, mes2->rawBufferLen);
	auto newlinesStart = &mes->rawBuffer[mes->rawBufferLen];
	mes->rawBufferLen = newLen;
	
	// updates line pointers to realloc'd buffer
	for (auto i = 0; i < mes->numLines; ++i) {
		auto offset = mes->lines[i].value - oldBuffer;
		mes->lines[i].value = mes->rawBuffer + offset;
	}

	// co-iterate over lines, may need to overwrite some (cannot have duplicates!)
	auto tgtPos = 0, tgtKey = 0;
	MesLine *tgtMesLine = &mes->lines[tgtPos];
	
	std::vector<int> indicesToPush;
	for (auto i = 0; i < mes2->numLines; ++i) {
		MesLine* srcMesLine = &mes2->lines[i];
		auto srcKey = srcMesLine->key;

		for (auto j = tgtPos; j < mes->numLines; ++j) {
			tgtMesLine = &mes->lines[j];
			tgtKey = tgtMesLine->key;
			if (srcKey > tgtKey) {
				tgtPos++;
				continue;
			}

			break;
		}
		
		if (srcKey == tgtKey) { // overwrite
			auto lineOffset = srcMesLine->value - mes2->rawBuffer;
			tgtMesLine->value = &newlinesStart[lineOffset];
		}
		else /* if (srcKey < tgtKey) */ {
			indicesToPush.push_back(i);
		}
		
	}

	if (!indicesToPush.size()) {
		return;
	}

	auto linesExt = (MesLine*)realloc(mes->lines, sizeof(MesLine) * (mes->numLines + indicesToPush.size()));
	if (!linesExt) {
		return;
	}
	mes->lines = linesExt;
	
	auto newMesLinesStart = &mes->lines[mes->numLines];
	for (auto i = 0; i < indicesToPush.size(); ++i) {

		auto indToPush = indicesToPush[i];
		MesLine* srcMesLine = &mes2->lines[indToPush];
		auto lineOffset = srcMesLine->value - mes2->rawBuffer;
		auto srcMesText = &newlinesStart[lineOffset];


		tgtMesLine = &newMesLinesStart[i];

		tgtMesLine->key   = srcMesLine->key;
		tgtMesLine->value = srcMesText;
	}

	mes->numLines += indicesToPush.size();
	std::sort(&mes->lines[0], &mes->lines[mes->numLines], [](MesLine& a, MesLine& b) {
		return a.key < b.key;
		});

	return;
}

bool FindMesHandle(const char* fname, MesHandle& handleOut)
{ 
	auto mesfileCount = temple::GetRef<int>(0x10EEE9FC);
	auto mesEntryList = temple::GetRef<MesFileEntry*>(0x10EEEA00);

	for (auto i = 0; i < mesfileCount; ++i) {
		if (!mesEntryList[i].refCount) {
			continue;
		}
		if (!_stricmp(fname, mesEntryList[i].fileName)) {
			handleOut = i;
			return true;
		}
	}

	return false;
}
