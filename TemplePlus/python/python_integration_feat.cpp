#include "stdafx.h"
#include "python_integration_feat.h"
#include "python_object.h"
#include <infrastructure/elfhash.h>

PythonFeatIntegration pyFeatIntegration;

PythonFeatIntegration::PythonFeatIntegration()
	: PythonIntegration("scr\\feats\\feat*.py", "(feat[ \-]{0,}([\\w\\-' ]{1,}))\\.py", true) {
}

bool PythonFeatIntegration::CheckPrereq(int featId, objHndl handle, Stat classCodeBeingLevelledUp, Stat abilityScoreBeingIncreased)
{
	if (featId < NUM_FEATS) {
		auto featName = feats.GetFeatName(static_cast<feat_enums>(featId));
		featId = ElfHash::Hash(featName);
	}

	auto featEntry = mScripts.find(featId);
	if (featEntry == mScripts.end())
		return false;

	auto attachee = PyObjHndl_Create(handle);
	auto args = Py_BuildValue("(Oii)", attachee, classCodeBeingLevelledUp, abilityScoreBeingIncreased);
	Py_DECREF(attachee);
	auto result = RunScriptDefault0(featEntry->second.id, (EventId)FeatEvent::CheckPrereq, args) != 0;
	Py_DECREF(args);

	return result;
}



static std::map<FeatEvent, std::string> featFunctions = {
	{ FeatEvent::CheckPrereq,"CheckPrereq" },
};

const char* PythonFeatIntegration::GetFunctionName(EventId evt) {
	auto _evt = (FeatEvent)evt;
	return featFunctions[_evt].c_str();
}

