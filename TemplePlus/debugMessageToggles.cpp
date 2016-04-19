#include "stdafx.h"
#include "util/fixes.h"
#include "config/config.h"

class DebugMessageToggles : public TempleFix {
public:
	void apply() override;
} debugMessageToggles;

void DebugMessageToggles::apply() {

	// Hook the getattr function of obj handles
	if (config.featPrereqWarnings == false) {
		writeHex(0x1007CF54, "EB");
	}

	if (config.spellAlreadyKnownWarnings == false) {
		writeHex(0x10079FDD, "90 90 90 90 90"); // Get_Object_Field_Name
		writeHex(0x10079FCF, "90 90 90 90 90"); // Mesfile getline
		writeHex(0x10079FEE, "90 90 90 90 90"); // print debug message
	}


}
