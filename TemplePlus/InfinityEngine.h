#pragma once

#include <string>

typedef unsigned char ieByte;
typedef signed char ieByteSigned;
typedef unsigned short ieWord;
typedef signed short ieWordSigned;
typedef unsigned int ieDword;
typedef signed int ieDwordSigned;


/** string reference into TLK file */
typedef ieDword ieStrRef;

/** Resource reference */
typedef char ieResRef[9];
typedef char ieVariable[33];


#pragma pack(push, 1)

struct ChitinKeyHeader
{
	char signature[4];
	char version[4];
	ieDword biffCount;
	ieDword resourceCount;
	ieDword biffEntriesOffset;
	ieDword resourceOffset;
};

struct ChitinBifEntry
{
	ieDword biffLength;
	ieDword biffFilenameOffset; 
	ieWord biffFilenameLength;
	ieWord locationMarker;
};


struct ChitinResEntry{
	char resName[8];
	ieWord resType;
	/*
	
	The IE resource manager uses 32-bit values as a 'resource index', 
	which codifies the source of the resource as well as which source 
	it refers to. 
	The layout of this value is below.
		bits 31-20: source index (the ordinal value giving the index of the corresponding BIF entry)
		bits 19-14: tileset index
		bits 13- 0: non-tileset file index (any 12 bit value, so long as it matches the value used in the BIF file)
*/
	ieDword resLocator;

	int GetTilesetIdx(){
		return (( resLocator >> 14) & 0x3F);
	};

	int GetSourceIdx(){
		return ((resLocator >> 20) );
	}
};

const int testSizeofChitinResEntry = sizeof(ChitinResEntry);
const int asd = offsetof(ChitinResEntry, resLocator);


struct BiffHeader {
	char signature[4];
	char version[4];
	ieDword fileCount;
	ieDword tilesetCount;
	ieDword fileEntriesOff;
};

const int testSizeofBiffHeader = sizeof(BiffHeader);

struct BiffFileEntry {
	ieDword resLocator;
	ieDword dataOffset;
	ieDword fileSize;
	ieWord  type;
	ieWord  u1; //Unknown Field
};

const int testSizeofBiffFileEntry = sizeof(BiffFileEntry);

struct BiffTileEntry {
	ieDword resLocator;
	ieDword dataOffset;
	ieDword tilesCount;
	ieDword tileSize; //named tilesize so it isn't confused
	ieWord  type;
	ieWord  u1; //Unknown Field
};


struct TilesetHeader{
	char signature[4];
	char version[4];
	ieDword tileCount;
	ieDword tileSectionLength;
	ieDword headerSize; // offset to tiles
	ieDword tileDimenPx; // Dimension of 1 tile in pixels (64x64)
};


#pragma pack(pop)


struct BifRecord {
	std::string fileName;
	ChitinBifEntry entry;
	BifRecord() {};
	BifRecord(const ChitinBifEntry &Entry) :entry(Entry) {};
};


struct BiffContent
{
	BiffHeader header;
	std::vector<BiffFileEntry> files;
	std::vector<BiffTileEntry> tiles;

	BiffContent() {};
	BiffContent(const BiffHeader&Header) :header(Header){};
};