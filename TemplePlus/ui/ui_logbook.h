#pragma once
#include "common.h"
#include "gametime.h"

#define LOGBOOK_MAX_PARTY_MEMBER_COUNT 5


struct LogbookEntry
{
	ObjectId id;
	int amount;
	int protoId;
};

struct PartyLogbookPacket
{
	LogbookEntry sub[LOGBOOK_MAX_PARTY_MEMBER_COUNT];
	BOOL HasObj(objHndl handle);
	BOOL AddObj(objHndl handle);
};

class UiLogbook
{
public:
	void IncreaseCritHits(objHndl handle);
	void IncreaseHits(objHndl handle);
	void IncreaseMisses(objHndl handle);
	void RecordHighestDamage(BOOL isWeaponDamage, int damageAmt, objHndl attacker, objHndl tgt);
	void MarkKey(int keyId, const GameTime& gameTime);

protected:
	void IncreaseAmount(PartyLogbookPacket & pkt, objHndl handle, int amount);
	
};


extern UiLogbook uiLogbook;