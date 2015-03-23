
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
