
#include "stdafx.h"
#include "fixes.h"


class TripBugFix: public TempleFix {
public:
const char* name() override {
return "trip bug fix: the game would use the attacker's dex score for the defender's roll";
}

void apply() override {
	writeHex(0x100B632D, "5355");
}
} tripBugFix;
