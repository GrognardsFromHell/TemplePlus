
#pragma once

#include "python_integration.h"

enum class SpellEvent : uint32_t {
	SpellEffect = 0,
	BeginSpellCast,
	EndSpellCast,
	BeginRound,
	EndRound,
	BeginProjectile,
	EndProjectile,
	BeginRoundD20Ping,
	EndRoundD20Ping,
	AreaOfEffectHit,
	SpellStruck
};

/*
	Integration points between the ToEE engine and the Python scripting system.
*/
class PythonSpellIntegration : public PythonIntegration {
public:
	PythonSpellIntegration();

	int SpellTrigger(int spellId, SpellEvent evt);
	int SpellTrigger(int spellId, SpellEvent evt, objHndl handle, int someNumber);

	void UpdateSpell(int spellId);
	void RemoveSpell(int spellId);
	
protected:
	const char* GetFunctionName(EventId eventId) override;
};

extern PythonSpellIntegration pythonSpellIntegration;
