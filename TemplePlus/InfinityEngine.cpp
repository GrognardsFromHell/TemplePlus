#include "stdafx.h"
#include "InfinityEngine.h"
#include "zlib.h"
#include <map>
#include <tio/tio.h>

InfiniData ieData;

void InfiniData::ReadChitin(){

	// read header
	auto chitin_file = tio_fopen("chitin.key", "rb");

	if (chitin_file == nullptr)
		return;

	tio_fread(&keyHeader, sizeof(ChitinKeyHeader), 1, chitin_file);

	// Get biff entries (file names really)
	for (auto i = 0; i<keyHeader.biffCount; i++) {
		ChitinBifEntry biffTemp;
		tio_fread(&biffTemp, sizeof(ChitinBifEntry), 1, chitin_file);
		biffEntries.push_back({ biffTemp });
	}

	auto curPos = tio_ftell(chitin_file);

	// get the BIF file names
	for (auto i = 0; i<keyHeader.biffCount; i++) {
		char tempBuffer[1024];
		auto &be = biffEntries[i];
		if (tio_ftell(chitin_file) != be.entry.biffFilenameOffset) {
			logger->warn("oy vey!");
		}
		tio_fread(&tempBuffer, sizeof(char), biffEntries[i].entry.biffFilenameLength, chitin_file);
		be.fileName = fmt::format("{}", tempBuffer);
	}


	// Get resource entries
	// Consist of string ID, type, and index
	curPos = tio_ftell(chitin_file);
	if (curPos > keyHeader.resourceOffset) {
		logger->warn("oy vey!");
	}

	tio_fseek(chitin_file, keyHeader.resourceOffset, 0);
	curPos = tio_ftell(chitin_file);
	for (auto i = 0; i<keyHeader.resourceCount; i++) {
		ChitinResEntry resTemp;
		tio_fread(&resTemp, sizeof(ChitinResEntry), 1, chitin_file);
		resKeyEntries.push_back(resTemp);
		resMapping[resTemp.resName] = resTemp.resLocator;

		// special casing for TIS resources (Tilesets)
		if (resTemp.resType == IeResourceType::IERT_Tileset) {
			tisKeyEntries.push_back(resTemp);
			tisMapping[resTemp.resName] = resTemp.resLocator;
		}
	}


	curPos = tio_ftell(chitin_file); // should be 522652 for IWD:EE
	tio_fclose(chitin_file);


}

void InfiniData::ReadBifFiles(){

	uint32_t curPos;

	TioFileList fl;
	tio_filelist_create(&fl, "data/*.BIF");

	std::map<int, std::vector<char>> fileCache;
	std::vector<uint8_t> bifBytes;

	for (auto i = 0; i< fl.count; i++) {
		// open the BIF file
		auto f = tio_fopen(fmt::format("data/{}", fl.files[i].name).c_str(), "rb");
		logger->info("Opening BIFF file: {}", f->filename);

		tio_fseek(f, 0, SEEK_END);
		auto fSize = tio_ftell(f);
		tio_fseek(f, 0, SEEK_SET);

		bifBytes.clear();
		bifBytes.resize(fSize);
		tio_fread(&bifBytes[0], 1, fSize, f);

		// read the BIFF header
		char bifSignature[5] = { 0, };
		char bifVersion[5] = { 0, };

		memcpy(bifSignature, &bifBytes[0],sizeof(char) * 4);
		memcpy(bifVersion, &bifBytes[4], sizeof(char) * 4);

		if (!strcmp(bifSignature, "BIFF")) {
			ReadBif(bifBytes);
		}
		else if (!_strcmpi(bifSignature, "BIF ")) { // BIFC V1 file format (zlib compressed BIF file(s), monoblock )
			ReadBifcV1(bifBytes);
		}
		else if (!strcmp(bifSignature, "BIFC")) { // BIFC V1.0 file format (block-wise compressed BIF file(s) )
			ReadBifcV10(bifBytes);
		}

		// close file
		tio_fclose(f);
	}

	tio_filelist_destroy(&fl);
}

void InfiniData::ReadBif( std::vector<uint8_t> &bifBytes){

	BiffHeader bifHeader;

	// get the BIFF header
	auto bytePtr = 0;
	auto byteSize = sizeof(BiffHeader);
	memcpy(&bifHeader, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;

	
	if (bifHeader.fileCount > 10000 || bifHeader.fileCount < 0) { // indicates bad data
		int dummy = 1; 
	}

	// initialize the BIFF Contents
	biffContents.push_back({ bifHeader });
	auto &bifC = biffContents[biffContents.size()-1];

	// read file entries
	bytePtr = bifHeader.fileEntriesOff;

	for (auto j = 0; j < bifHeader.fileCount; j++) {
		BiffFileEntry fe;

		byteSize = sizeof(BiffFileEntry);
		memcpy(&fe, &bifBytes[bytePtr], byteSize); bytePtr += byteSize;
		bifC.files.push_back(fe);
	}

	// read tile entries 
	for (auto j = 0; j < bifHeader.tilesetCount; j++) {
		BiffTileEntry te;

		byteSize = sizeof(BiffTileEntry);
		memcpy(&te, &bifBytes[bytePtr], byteSize); bytePtr += byteSize;
		bifC.tiles.push_back(te);
	}

	// read the file data
	for (auto j = 0; j < bifHeader.fileCount; j++) {
		auto &fe = bifC.files[j];
		bytePtr = fe.dataOffset;
		// todo
	}
	
	// read the tile data
	for (auto i = 0; i < bifHeader.tilesetCount; i++) {
		auto &te = bifC.tiles[i];
		bytePtr = te.dataOffset;

		auto tileCount = te.tilesCount;
		auto tileSize = te.tileSize;

		TileData *tiledata = (TileData *)(&bifBytes[bytePtr]);

		for (auto j = 0; j < tileCount; j++){
			IeBitmapFromTile bm(*tiledata);
			
			string fname(fmt::format("AR{}.bmp", j));
			auto f = tio_fopen(fname.c_str(), "wb");
			tio_fwrite(&bm, 1, sizeof(IeBitmapFromTile), f);
			tio_fclose(f);
			tiledata++;
		}
	}
}

void InfiniData::ReadBifcV1(std::vector<uint8_t>& bifBytes)
{
	BifCHeader bifCHeader;
	char fname[1024] = { 0, };
	auto bytePtr = 0;

	// get header
	auto byteSize = sizeof(BifCHeader);
	memcpy(&bifCHeader, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;
	
	// get filename
	byteSize = bifCHeader.filenameLen;
	memcpy(&fname, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;

	// get data length
	uint32_t uncompressedDataLen = 0;
	byteSize = sizeof(uncompressedDataLen);
	memcpy(&uncompressedDataLen, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;

	uint32_t compressedDataLen = 0;
	byteSize = sizeof(compressedDataLen);
	memcpy(&compressedDataLen, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;

	// get raw compressed data
	std::vector<uint8_t> rawData;
	rawData.resize(compressedDataLen);
	byteSize = compressedDataLen;
	memcpy(&rawData[0], &bifBytes[bytePtr], byteSize);

	std::vector<uint8_t> uncompData;
	uncompData.resize(uncompressedDataLen);

	ReadBif(uncompData);
}

void InfiniData::ReadBifcV10(std::vector<uint8_t>& bifBytes){

	auto bytePtr = 0;

	// get header
	BifCV10Header bifCHeader;
	auto byteSize = sizeof(BifCV10Header);
	memcpy(&bifCHeader, &bifBytes[bytePtr], byteSize);
	bytePtr += byteSize;



	std::vector<uint8_t> rawData;
	std::vector<uint8_t> uncompData;
	uncompData.resize(bifCHeader.uncompBifSize);
	auto uncompDataCount = 0;
	
	// read compressed blocks
	while (bytePtr < bifBytes.size()){
		uint32_t decompressedBlockSize = 0;
		uint32_t compressedBlockSize = 0;

		byteSize = sizeof(decompressedBlockSize);
		memcpy(&decompressedBlockSize, &bifBytes[bytePtr], byteSize);
		bytePtr += byteSize;

		byteSize = sizeof(compressedBlockSize);
		memcpy(&compressedBlockSize, &bifBytes[bytePtr], byteSize);
		bytePtr += byteSize;

		// read raw data block
		rawData.clear();
		rawData.resize(compressedBlockSize);
		byteSize = compressedBlockSize;
		memcpy(&rawData[0], &bifBytes[bytePtr], compressedBlockSize);
		bytePtr += byteSize;
		

		// inflate
		uLongf uncompressedBlockSize;
		auto uncomResult = uncompress(&uncompData[uncompDataCount], &uncompressedBlockSize, &rawData[0], compressedBlockSize);
		uncompDataCount += uncompressedBlockSize;

	}
	assert(uncompData.size() == uncompDataCount);
	ReadBif(uncompData);
}
