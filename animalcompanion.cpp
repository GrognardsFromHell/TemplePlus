
#include "stdafx.h"
#include "fixes.h"

const uint8_t animalCompanionArraySize = 11; //adjust if you add more ACs!!

uint32_t animalCompanionProtos[animalCompanionArraySize] = { 14050, 14049, 14051, 14056, 14362, 14090, 14052, 14053, 14054, 14055, 14506 };
uint32_t animalCompanionLevelRestrictions[animalCompanionArraySize] = { 0, 0, 0, 0, 0, 3, 3, 6, 9, 3, 13 };

uint32_t animalIndexOffset = 0;



int AnimalCompanionGetRadialMenuOptions(){

};


class AnimalCompanionMod : public TempleFix {
public:
	const char* name() override {
		return "Animal Companion For High Levels";
	}
	void apply() override;
} animalCompanionMod;

void AnimalCompanionMod::apply() {
	void* ptrToAnimalCompanionProtos = animalCompanionProtos;
	void* ptrToanimalCompanionLevelRestrictions = animalCompanionLevelRestrictions;

	write(0x100FC52C, &ptrToAnimalCompanionProtos, sizeof(ptrToAnimalCompanionProtos));
	write(0x100FC5AA, &ptrToanimalCompanionLevelRestrictions, sizeof(ptrToanimalCompanionLevelRestrictions));
	write(0x100FC288, &ptrToanimalCompanionLevelRestrictions, sizeof(ptrToanimalCompanionLevelRestrictions)); 
	write(0x100FC2A4, &animalCompanionArraySize, 1);

	//replaceFunction(0x100FC520, GiveXPAwards);
}