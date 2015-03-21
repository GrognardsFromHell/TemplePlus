#include "stdafx.h"
#include "fixes.h"
#include "addresses.h"
#include "dependencies/python-2.2/Python.h"
#include "temple_functions.h"
#include "config.h"

class DebugMessageToggles : public TempleFix {
public:
	const char* name() override {
		return "Debug Message Toggles";
	}
	void apply() override;
} debugMessageToggles;

void DebugMessageToggles::apply() {

	// Hook the getattr function of obj handles
	if (config.featPrereqWarnings == false){
		TempleFix::writeHex(0x1007CF54, "EB");
	}

	if (config.spellAlreadyKnownWarnings == false){
		
		TempleFix::writeHex(0x10079FDD, "90 90 90 90 90"); // Get_Object_Field_Name
		TempleFix::writeHex(0x10079FCF, "90 90 90 90 90"); // Mesfile getline
		TempleFix::writeHex(0x10079FEE, "90 90 90 90 90"); // print debug message
	}
	
	
}