
#include "stdafx.h"
#include "config.h"
#include "dependencies/feather-ini/INI.h"

TemplePlusConfig config;

typedef INI <string, string, string> ini_t;

static ini_t ini("TemplePlus.ini", false);

void TemplePlusConfig::Load() {
	ini.parse();
}

void TemplePlusConfig::Save() {
	ini.save();
}
