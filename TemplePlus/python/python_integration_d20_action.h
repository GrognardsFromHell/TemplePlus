#pragma once
#include "python_integration.h"
#include "common.h"

enum class D20ActionSpecFunc : int {

	GetActionDefinitionFlags,
	GetActionName,
	GetTargetingClassification,
	GetActionCostType,
	AddToSequence
};


struct D20Actn;
struct ActnSeq;
struct TurnBasedStatus;
enum ActionErrorCode : uint32_t;

class PythonD20ActionIntegration : public PythonIntegration {
public:
	PythonD20ActionIntegration();

	void GetActionEnums(std::vector<int>& actionEnums);
	std::string GetActionName(int actionEnum);
	
	int GetInt(int actionEnum, D20ActionSpecFunc specType, int defaultVal = 0);

	int GetActionDefinitionFlags(int actionEnum);
	int GetTargetingClassification(int actionEnum);
	ActionCostType GetActionCostType(int actionEnum);


	ActionErrorCode PyAddToSeq(int actionEnum, D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
	
protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonD20ActionIntegration pythonD20ActionIntegration;
