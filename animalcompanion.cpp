
#include "stdafx.h"
#include "fixes.h"

const int animalCompanionArraySize = 11; //adjust the writeHex below if you change this!!!!

uint32_t animalCompanionProtos[animalCompanionArraySize] = { 14050, 14049, 14051, 14056, 14362, 14090, 14052, 14053, 14054, 14055, 14506 };
uint32_t animalCompanionLevelRestrictions[animalCompanionArraySize] = { 0, 0, 0, 0, 0, 3, 3, 6, 9, 3, 13 };

class AnimalCompanionMod : public TempleFix {
public:
	const char* name() override {
		return "trip bug fix: the game would use the attacker's dex score for the defender's roll";
	}

	void apply() override;
} animalCompanionMod;

void AnimalCompanionMod::apply() {
	LOG(info) << "Applying Trip Bug fix...";
	//write(0x100FC52C, animalCompanionProtos,4);
	//write(0x100FC5AA, animalCompanionLevelRestrictions, 4);
	//write(0x100FC288, animalCompanionLevelRestrictions, 4);
	//writeHex(0x100FC2A4, "0B"); // this should be the number of possible animal companions!!! too lazy to make it dynamic :P

}