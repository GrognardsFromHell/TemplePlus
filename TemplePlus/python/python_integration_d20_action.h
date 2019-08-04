#pragma once
#include "python_integration.h"
#include "common.h"

enum class D20ActionSpecFunc : int {

	GetActionDefinitionFlags,
	GetActionName,
	GetTargetingClassification,
	GetActionCostType,
	AddToSequence,
	ModifyPicker,
	ProjectileHit
};


struct D20Actn;
struct ActnSeq;
struct TurnBasedStatus;
struct PickerArgs;
enum ActionErrorCode : uint32_t;

class PythonD20ActionIntegration : public PythonIntegration {
public:
	PythonD20ActionIntegration();

	void GetActionEnums(std::vector<int>& actionEnums);
	std::string GetActionName(int actionEnum);
	
	int GetInt(int actionEnum, D20ActionSpecFunc specType, int defaultVal = 0);

	int GetActionDefinitionFlags(int actionEnum);
	int GetTargetingClassification(int actionEnum);
	void ModifyPicker(int actionEnum, PickerArgs * pickArgs);
	ActionCostType GetActionCostType(int actionEnum);


	ActionErrorCode PyAddToSeq(int actionEnum, D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *tbStat);
	BOOL PyProjectileHit(int actionEnum, D20Actn *d20a, objHndl projectile, objHndl obj2 );
	
protected:
	const char* GetFunctionName(EventId evt) override;
};

extern PythonD20ActionIntegration pythonD20ActionIntegration;
