
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

	void SpellTrigger(int spellId, SpellEvent evt);
	void SpellTriggerProjectile(int spellId, SpellEvent evt, objHndl projectile, int targetIdx);

	void UpdateSpell(int spellId);
	void RemoveSpell(int spellId);
	uint32_t SpellSoundPlay(SpellPacketBody* spellPkt, SpellEvent spellEvt);
	
protected:
	const char* GetFunctionName(EventId eventId) override;
};

extern PythonSpellIntegration pythonSpellIntegration;
