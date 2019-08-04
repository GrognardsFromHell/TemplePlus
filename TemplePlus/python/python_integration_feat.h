#pragma once

#include "python_integration.h"


enum class FeatEvent : uint32_t {
	CheckPrereq = 0,
};

/*
Integration points between the ToEE engine and the Python scripting system.
*/
class PythonFeatIntegration : public PythonIntegration {
public:
	PythonFeatIntegration();

	bool CheckPrereq(int featId, objHndl handle, Stat classCodeBeingLevelledUp = Stat::stat_strength, Stat abilityScoreBeingIncreased = (Stat)-1);

protected:
	const char* GetFunctionName(EventId eventId) override;
};

extern PythonFeatIntegration pyFeatIntegration;



