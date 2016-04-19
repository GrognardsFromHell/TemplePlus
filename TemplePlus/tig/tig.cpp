#include "stdafx.h"
#include "tig.h"
#include "tig_tabparser.h"
#include <util/fixes.h>
#include <tio/tio.h>
#include <temple/vfs.h>

TigTabParserFuncs tigTabParserFuncs;

void TigTabParser::Init(TigTabLineParser* parserFunc)
{
	filename = nullptr;
	lineCount = 0;
	maxColumns = 0;
	curLineIdx = 0;
	fileContent = nullptr;
	fileContentEndPos = 0;
	lineParser = parserFunc;
}

BOOL TigTabParser::Open(const char* filenameIn)
{
	if (filename)
		free(filename);
	if (fileContent)
		free(fileContent);
	filename = new char[strlen(filenameIn) + 1];
	strcpy(filename, filenameIn);
	auto file = tio_fopen(filenameIn, "rb");
	if (!file)
		return 17;

	// read the file
	auto fileLen = tio_filelength(file);
	fileContent = new char[fileLen + 1];
	fileContentEndPos = &fileContent[tio_fread(fileContent, 1, fileLen, file)];
	tio_fclose(file);


	// format the raw file string
	auto tabStartPos = fileContent;
	char * endPos = (char*)fileContentEndPos;
	int numTabs = 0;
	lineCount = 0;
	maxColumns = 0;

	for (auto curPos = fileContent; curPos < endPos; curPos++)
	{
		if (*curPos == '\t' ||  *curPos == '\n' )
		{ // end of column
			auto prevPos = curPos - 1;
			auto eol = *curPos == '\n';

			if (eol && curPos > fileContent && *prevPos == '\r')
			{
				*(curPos - 1) = ' ';
			}
			// trim whitespace
			auto tabPos = curPos - 1;
			for ( ; tabPos >= tabStartPos; tabPos--)
			{
				if (*tabPos == ' ' || *tabPos == '\v' || *curPos == '\r')
				{
					*tabPos = '\x15'; // replace whitespace and vertical tabs
				}
				else
				{
					break;
				}	
			}

			if (!eol)
			{
				if (tabPos != curPos - 1 && curPos < endPos) // meaning whitespace conversion has taken place; we need to prevent two column stops being in place!
				{
					*curPos = '\x15';
				}
				tabPos[1] = 0; // terminate string
			} else
			{
				*curPos = '\x15';
				tabPos[1] = '\n';
				if (maxColumns < numTabs)
					maxColumns = ++numTabs;
				numTabs = 0;
				lineCount++;
			}
			numTabs++;
			tabStartPos = curPos + 1;


		}
	}

	return 0;

}

void TigTabParser::Process()
{
	auto emptyStr = temple::GetPointer<char>(0x1026C67B); // in case some other functions compares the address, we'll use Troika's...
	if (!fileContent)
		return;

	std::vector<char*> columns(maxColumns );
	auto endPos = (char*)fileContentEndPos;
	curLineIdx = 0;

	// loop over lines
	for (auto curPos = fileContent; curPos < endPos; curLineIdx++)
	{
		// loop over columns
		int colIdx = 0;
		for ( ; *curPos != '\n' && colIdx < maxColumns; colIdx++)
		{
			Expects(colIdx < maxColumns);
			columns[colIdx] = curPos;
			bool trimmedCharsSkipped = false;
			while (*curPos != '\n')
			{
				auto curPosChar = *(curPos++);
				if (!curPosChar)
				{
					break;
				}
				if (curPosChar == '\t')
				{
					break;
				}
			}
			if (*curPos == '\x15')
			{
				trimmedCharsSkipped = true;
				while (*curPos == '\x15')
					curPos++;
			}
			if (curPos > endPos)
				break;
		}
		// if reached EOL before maxColumns, fill the rest with emptyStr
		for ( ; colIdx < maxColumns; colIdx++)
		{
			columns[colIdx] = emptyStr;
		}
			
		if (curPos < endPos && *curPos == '\n')
			*curPos = 0;
		if (curPos < endPos && *curPos == ' ')
			*curPos = 0;
		if (lineParser(this, curLineIdx, &columns[0]))
		{
			break;
		}
		curPos++;
		while (*curPos == '\x15')
			curPos++;

			
	}

}

void TigTabParser::Close()
{
	if (filename)
		free(filename);
	if (fileContent)
		free(fileContent);
	filename = nullptr;
	lineCount = 0;
	maxColumns = 0;
	curLineIdx = 0;
	fileContent = nullptr;
	fileContentEndPos = nullptr;
	lineParser = nullptr;
}

TigTabParserFuncs::TigTabParserFuncs() {
	rebase(Init, 0x101F2C10);
	rebase(Open, 0x101F2E40);
	rebase(GetLineCount, 0x101F2D40);
	rebase(Process, 0x101F2C70);
	rebase(Close, 0x101F2C30);
}

class TigTabReplacements: TempleFix{
public:
	static void FormatRawString(TigTabParser* tab);

	static void(__cdecl*orgFormatRawString)(TigTabParser* tab);
	void apply() override {
		orgFormatRawString = replaceFunction(0x101F2DC0, FormatRawString);
	}
} tigTabReplacements;

void TigTabReplacements::FormatRawString(TigTabParser* tab)
{
	orgFormatRawString(tab);
	int dummy = 1;
}

void(__cdecl*TigTabReplacements::orgFormatRawString)(TigTabParser* tab);


void TigRect::FitInto(const TigRect& boundingRect) {

	/*
	Calculates the rectangle within the back buffer that the scene
	will be drawn in. This accounts for "fit to width/height" scenarios
	where the back buffer has a different aspect ratio.
	*/
	float w = static_cast<float>(boundingRect.width);
	float h = static_cast<float>(boundingRect.height);
	float wFactor = (float)w / width;
	float hFactor = (float)h / height;
	float scale = min(wFactor, hFactor);
	width = (int)round(scale * width);
	height = (int)round(scale * height);

	// Center in bounding Rect
	x = boundingRect.x + (boundingRect.width - width) / 2;
	y = boundingRect.y + (boundingRect.height - height) / 2;
}

bool TigRect::Intersects(const TigRect& other) {

	// Basically we select any component that leads to a 
	// smaller surface area of the intersection and then
	// discard if the surface area would become negative or zero
	auto intersectionX = std::max(x, other.x);
	auto intersectionY = std::max(y, other.y);

	auto right = x + width;
	auto otherRight = other.x + other.width;
	auto intersectionWidth = std::min(right, otherRight) - intersectionX;
	if (intersectionWidth <= 0) {
		return false;
	}

	auto bottom = y + height;
	auto otherBottom = other.y + other.height;
	auto intersectionHeight = std::min(bottom, otherBottom) - intersectionY;

	return intersectionHeight > 0;

}

bool TigRect::Intersects(const TigRect& other, TigRect& intersection) {
	// Basically we select any component that leads to a 
	// smaller surface area of the intersection and then
	// discard if the surface area would become negative or zero
	auto intersectionX = std::max(x, other.x);
	auto intersectionY = std::max(y, other.y);

	auto right = x + width;
	auto otherRight = other.x + other.width;
	auto intersectionWidth = std::min(right, otherRight) - intersectionX;
	if (intersectionWidth <= 0) {
		return false;
	}

	auto bottom = y + height;
	auto otherBottom = other.y + other.height;
	auto intersectionHeight = std::min(bottom, otherBottom) - intersectionY;
	if (intersectionHeight <= 0) {
		return false;
	}

	intersection.x = intersectionX;
	intersection.y = intersectionY;
	intersection.width = intersectionWidth;
	intersection.height = intersectionHeight;
	return true;	
}

RECT TigRect::ToRect() {
	return{x, y, x + width, y + height};
}
