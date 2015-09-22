
#include "stdafx.h"
#include "reputations.h"
#include <temple/dll.h>
#include "obj.h"
#include "party.h"

PartyReputation partyReputation;

static struct ReputationAddresses : temple::AddressTable {
	bool(__cdecl *ReputationHas)(objHndl target, int repId);
	void (__cdecl *ReputationAdd)(objHndl target, int repId);
	void (__cdecl *ReputationRemove)(objHndl target, int repId);

	ReputationAddresses() {
		rebase(ReputationHas, 0x100546E0);
		rebase(ReputationRemove, 0x10054820);
		rebase(ReputationAdd, 0x10054740);
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
