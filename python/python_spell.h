#pragma once
#include "stdafx.h"
#include "..\common.h"


struct SpellPacketBody;
struct PySpell;

enum PySpellEventCode : uint32_t
{
	OnSpellEffect = 0,
	OnBeginSpellCast,
	OnEndSpellCast,
	OnBeginRound,
	OnEndRound,
	OnBeginProjectile,
	OnEndProjectile,
	OnBeginRoundD20Ping,
	OnEndRoundD20Ping,
	OnAreaOfEffectHit,
	OnSpellStruck
};


class PythonSpellSystem
{
public:
	char * GetSpellEventName(PySpellEventCode spellEventCode);
	uint32_t GetSpellFilename(char * filenameOut, uint32_t spellEnum);
	uint32_t SpellTrigger(uint32_t spellId, PySpellEventCode spellEventCode);
	PyObject * PySpellFromSpellId(uint32_t spellId);
	void PySpellLocSet(LocFull * locFull);
	void PySpellLoc2Set(LocFull * locFull);
	void SpellPacketUpdateFromPySpell(PySpell * pySpell);
	uint32_t SpellSoundPlay(SpellPacketBody * spellPktBody, PySpellEventCode pySpellEvent);
	PythonSpellSystem()
	{

	}
};

extern PythonSpellSystem pySpellSystem;


uint32_t _SpellTrigger(uint32_t spellId, PySpellEventCode spellEventCode);
