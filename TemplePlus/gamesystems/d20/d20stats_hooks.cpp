
#include "stdafx.h"
#include "util/fixes.h"

static class D20StatsHooks : public TempleFix {
public:
	const char* name() override {
		return "D20 Stats Hooks";
	}

	void apply() override {

	}

} hooks;
