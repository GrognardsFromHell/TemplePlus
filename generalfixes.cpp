
#include "stdafx.h"
#include "fixes.h"



class SizeColossalFix : public TempleFix {
public:
	const char* name() override {
		return "fixes size_colossal (was misspelled as size_clossal)";
	}

	void apply() override {
		writeHex(0x10278078, "73 69 7A 65 5F 63 6F 6C 6F 73 73 61 6C");
	}
} sizeColossalFix;

class KukriFix : public TempleFix {
public:
	const char* name() override {
		return "Makes Kukri Proficiency Martial";
	}

	void apply() override {
		writeHex(0x102BFD78 + 30*4, "00 09"); // marks Kukri as Martial in the sense that picking "Martial Weapons Proficiency" will now list Kukri
		// TODO: makes the feat "Martial Weapons Proficiency: All" include Kukri proficiency
	}
} kukriFix;
