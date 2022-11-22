
#pragma once

#include <util/fixes.h>

class AasHooks : public TempleFix {
public:
	virtual void apply() override;
};
class AasDebugHooks : public TempleFix {
public:
	virtual void apply() override;
};