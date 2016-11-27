#pragma once

#include <string>
#include <map>

#define IeTilePaletteCount 256
#define IeTileDim 4096 // Each tile is 64x64 px

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

#pragma region Data Types


struct IeFileHeader {
	char signature[4];
	char version[4];
};

#pragma pack(push, 1)

enum IeResourceType : ieWord {
	IERT_Bitmap = 0x1,
	IERT_Movie = 0x2,
	IERT_Wav = 0x4, // also WAC which is a modified ACM file with a header attached to simplify buffer estimation during file compression
	IERT_WFX = 0x5, // Wave FX file. WFX files are used to determine variation for playback of wave sounds.
	IERT_BAM = 0x03e8, // Graphics, specifically animations though also multi-frame static graphics. The format supports multiple animation cycles, each containing multiple frames. Can also be BAMC (zlib compressed BAM)
	IERT_WED = 0x03e9, // area region info
	IERT_Tileset = 1003, // Graphics, specifically area art
	IERT_Mos = 1004, //Graphics, specifically minimaps and GUI backgrounds.
};


struct ieResourceLoc{
	unsigned int raw;

	unsigned int GetNontileIdx(){
		return (raw & 0x3FFF);
	}

	unsigned int GetTilesetIdx(){
		return (( raw >> 14) & 0x3F);
	};

	unsigned int GetSourceIdx(){
		return ((raw >> 20) );
	}
};

const int testSizeofResourceLoc = sizeof(ieResourceLoc);


// Chitin.key file
struct ChitinKeyHeader : IeFileHeader
{
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
	IeResourceType resType;
	/*
	
	The IE resource manager uses 32-bit values as a 'resource index', 
	which codifies the source of the resource as well as which source 
	it refers to. 
	The layout of this value is below.
		bits 31-20: source index (the ordinal value giving the index of the corresponding BIF entry)
		bits 19-14: tileset index
		bits 13- 0: non-tileset file index (any 12 bit value, so long as it matches the value used in the BIF file)
*/
	ieResourceLoc resLocator;
	// ieDword resLocator;
	
};

const int testSizeofChitinResEntry = sizeof(ChitinResEntry); // should be 14 (0xE)
const int asd = offsetof(ChitinResEntry, resLocator); // should be 10 (0xa)

// BIF file headers
struct BiffHeader : IeFileHeader {
	ieDword fileCount;
	ieDword tilesetCount;
	ieDword fileEntriesOff;
};

struct BifCHeader : IeFileHeader {
	ieDword filenameLen;
	// char fileName[?]; // actual length is as specified by filenameLen
	// ieDword uncompressedDataLen;
	// ieDword compressedDataLen;
	// rawData; // compressed data
};

struct BifCV10Header : IeFileHeader {
	ieDword uncompBifSize; // Uncompressed BIF size
};

const int testSizeofBiffHeader = sizeof(BiffHeader);

struct BiffFileEntry {
	ieDword resLocator;
	ieDword dataOffset;
	ieDword fileSize;
	IeResourceType  type;
	ieWord  u1; //Unknown Field
};

const int testSizeofBiffFileEntry = sizeof(BiffFileEntry);

struct BiffTileEntry {
	//ieDword resLocator;
	ieResourceLoc resLocator;
	ieDword dataOffset;
	ieDword tilesCount;
	ieDword tileSize; //named tilesize so it isn't confused
	IeResourceType type; // should always be 1003 (Tileset)
	ieWord  u1; //Unknown Field
};

const int testSizeofBiffTileEntry = sizeof(BiffTileEntry); // should be 20 (0x14)

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
	BiffContent(const BiffHeader&Header) :header(Header) {};
};


// Tileset

struct TilesetHeader : IeFileHeader{
	ieDword tileCount;
	ieDword tileSectionLength;
	ieDword headerSize; // offset to tiles
	ieDword tileDimenPx; // Dimension of 1 tile in pixels (64x64)
};

const int testSizeofTilesetHeader = sizeof(TilesetHeader);

struct TilePalette
{
	RGBQUAD bgra[IeTilePaletteCount];
};

struct TileData {
	TilePalette pal;
	BYTE pxValues[IeTileDim]; // indices into the palette. Index 0 is hardcoded to be the transparent color.
};


struct IeBitmapFromTile
{
	BITMAPFILEHEADER fileHead;
	BITMAPINFOHEADER coreHead;
	TileData td;

	IeBitmapFromTile(){
		fileHead.bfType = 0x4D42;
		fileHead.bfSize = sizeof(IeBitmapFromTile);
		fileHead.bfOffBits = offsetof(IeBitmapFromTile, td.pxValues);
		fileHead.bfReserved1 = 0;
		fileHead.bfReserved2 = 0;

		coreHead.biSize = 40;
		coreHead.biWidth = 64;
		coreHead.biHeight = 64;
		coreHead.biPlanes = 1;
		coreHead.biBitCount = 8;
		coreHead.biCompression = 0;
		coreHead.biSizeImage = 0x1000;
		coreHead.biXPelsPerMeter = 0;
		coreHead.biYPelsPerMeter = 0;
		coreHead.biClrUsed = 0; // will default to 2^n
		coreHead.biClrImportant = 0;
	}

	IeBitmapFromTile(TileData &TileIn) : IeBitmapFromTile(){
		td = TileIn;
	}
};




#pragma pack(pop)



#pragma endregion 

class InfiniData {
public:
	/*
	  Process chitin.key file
	  This contructs the following:
		* biffEntries
		* resKeyEntries
		* resMapping      - mapping of resource name to resource locator (ieResourceLoc)

		Subset of the above:
		* tisKeyEntries 
		* tisMapping
	*/
	void ReadChitin();
	void ReadBifFiles(); // read biff files
	void ReadBif(std::vector<uint8_t> &bifBytes); // read single uncompressed BIF file
	void ReadBifcV1(std::vector<uint8_t> &bifBytes);
	void ReadBifcV10(std::vector<uint8_t> &bifBytes);

	
	std::vector<ChitinResEntry> resKeyEntries;
	std::map<std::string, ieResourceLoc> resMapping; // maps resource name to resource locator

	std::vector<ChitinResEntry> tisKeyEntries;
	std::map<std::string, ieResourceLoc> tisMapping; 


	ChitinKeyHeader keyHeader; // contains the number of BIFs and Resources and their starting offset
	std::vector<BifRecord> biffEntries; // from chitin.key file


	// Biff files
	std::vector<BiffContent> biffContents;

};

extern InfiniData ieData;