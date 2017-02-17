
#include "stdafx.h"
#include "util/fixes.h"
#include "temple_functions.h"
#include "dispatcher.h"

const uint8_t animalIndexOffset = 8; // the game has a block of familiar/wildshape/anim comp stuff defined, and AC's have an index relative to that
/* just for testing
const uint8_t animalCompanionArraySize = 11 + 24; //adjust if you add more ACs!!
const uint8_t animalIndexOffsetMax = animalIndexOffset + animalCompanionArraySize;

uint32_t animalCompanionProtos[animalCompanionArraySize] = { 14265,14503,14004,14005,  14406,14407,14008,14009,  14410,14411,14012,14013,  14014,14015,14016,14017,  14018,14019,14020,14021   ,14022,14023,14024,14025  ,14050, 14049, 14051, 14056, 14362, 14090, 14052, 14053, 14054, 14055, 14506 };
uint32_t animalCompanionLevelRestrictions[animalCompanionArraySize] = { 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0,   0,0,0,0,  0,0,0,0,   0, 0, 0, 0, 0, 3, 3, 6, 9, 3, 12 };
*/

const uint8_t animalCompanionArraySize = 11 ; //adjust if you add more ACs!!
const uint8_t animalIndexOffsetMax = animalIndexOffset + animalCompanionArraySize;

uint32_t animalCompanionProtos[animalCompanionArraySize] = {  14050, 14049, 14051, 14056, 14362, 14090, 14052, 14053, 14054, 14055, 14506 };
uint32_t animalCompanionLevelRestrictions[animalCompanionArraySize] = { 0, 0, 0, 0, 0, 3, 3, 6, 9, 3, 12 };




void AnimalCompanionGetRadialMenuOptions(objHndl objHnd, int animIdx, int unknown){
	int animIdx2 = animIdx - animalIndexOffset;
	int animProto = 0;
	if (animIdx2 < animalCompanionArraySize){
		int animProto = animalCompanionProtos[animIdx2];
	};
	
	return ;
};

// Animal Companion For High Levels
class AnimalCompanionMod : public TempleFix {
public:
	void apply() override;
} animalCompanionMod;

void AnimalCompanionMod::apply() {
	void* ptrToAnimalCompanionProtos = animalCompanionProtos;
	void* ptrToanimalCompanionLevelRestrictions = animalCompanionLevelRestrictions;

	void* ptrToAnimalCompanionProtosForRadialMenu = animalCompanionProtos - animalIndexOffset;
	void* ptrToAnimalCompanionLevelRestrictionsForRadialMenu = animalCompanionLevelRestrictions - animalIndexOffset;

	write(0x100FC52C, &ptrToAnimalCompanionProtos, sizeof(ptrToAnimalCompanionProtos));
	write(0x100FC5AA, &ptrToanimalCompanionLevelRestrictions, sizeof(ptrToanimalCompanionLevelRestrictions));
	write(0x100FC288, &ptrToanimalCompanionLevelRestrictions, sizeof(ptrToanimalCompanionLevelRestrictions)); 
	write(0x100FC2A4, &animalCompanionArraySize, 1);
	write(0x100FD510 + 2, &animalIndexOffsetMax, 1);
	write(0x100FC30B + 2, &animalIndexOffsetMax, 1);
	
	write(0x100FC0FB + 3, &ptrToAnimalCompanionProtosForRadialMenu, sizeof(ptrToAnimalCompanionProtosForRadialMenu));
	write(0x100FC365 + 3, &ptrToAnimalCompanionLevelRestrictionsForRadialMenu, sizeof(ptrToAnimalCompanionLevelRestrictionsForRadialMenu));

	replaceFunction<void(__cdecl)(const char*, int)>(0x100FD3F0,[](const char* nameIn, int sthg)
	{
		if (!sthg)
		{
			auto animCompTrimName = temple::GetRef<BOOL(__cdecl)(const char*, char*)>(0x100FC4E0);
			char nameTrimmed[100];
			if (animCompTrimName(nameIn, nameTrimmed))
			{
				auto& animCompSubDispNode = temple::GetRef<SubDispNode*>(0x115B1ECC);
				auto& animCompIdx = temple::GetRef<int>(0x115B1EC8);
				auto& animCompOwner = temple::GetRef<objHndl>(0x115B1EB8);
				auto& animCompObj = temple::GetRef<objHndl>(0x115B1EB0);
				auto animCompAdd = temple::GetRef<void(__cdecl)(SubDispNode*, objHndl, int, char*)>(0x100FC520);
				animCompAdd(animCompSubDispNode, animCompOwner, animCompIdx, nameTrimmed);
			}
			
		}else
		{
			int dummy = 1;
		}
		
	});

	//replaceFunction(0x100FC520, GiveXPAwards);
}