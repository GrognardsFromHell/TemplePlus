
#pragma once
#include "tig_startup.h"

// RAII for TIG initialization
class TigInitializer {
public:

	explicit TigInitializer(HINSTANCE hInstance);
	~TigInitializer();
	const TigConfig &config() const {
		return mConfig;
	}

private:
	TigConfig mConfig;
};
