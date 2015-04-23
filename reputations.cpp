
#include "stdafx.h"
#include "reputations.h"
#include "util/addresses.h"
#include "obj.h"
#include "party.h"

PartyReputation partyReputation;

static struct ReputationAddresses : AddressTable {
	bool(__cdecl *ReputationHas)(objHndl target, int repId);
	void (__cdecl *ReputationAdd)(objHndl target, int repId);
	void (__cdecl *ReputationRemove)(objHndl target, int repId);

	ReputationAddresses() {
	}
} addresses;

bool PartyReputation::Has(int reputationId) {
	auto leader = party.GroupListGetMemberN(0);
	return addresses.ReputationHas(leader, reputationId);
}

void PartyReputation::Add(int reputationId) {
	auto leader = party.GroupListGetMemberN(0);
	addresses.ReputationAdd(leader, reputationId);
}

void PartyReputation::Remove(int reputationId) {
	auto leader = party.GroupListGetMemberN(0);
	addresses.ReputationRemove(leader, reputationId);
}
