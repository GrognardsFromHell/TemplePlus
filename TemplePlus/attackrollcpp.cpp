
#include "stdafx.h"
#include "util/fixes.h"

// trip bug fix: the game would use the attacker's dex score for the defender's roll
class TripBugFix: public TempleFix {
public:
	void apply() override {
		writeHex(0x100B632D, "5355");
	}
} tripBugFix;
