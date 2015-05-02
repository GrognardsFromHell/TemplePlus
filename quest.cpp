
#include "stdafx.h"
#include "quest.h"
#include "party.h"

Quests quests;

static struct QuestsAddresses : AddressTable {

	QuestState (__cdecl *GetState)(objHndl pc, int questId);
	void (__cdecl *SetState)(objHndl pc, int questId, QuestState state);
	// It's unconfirmed that this returns the new quest state but it looks like it
	QuestState (__cdecl *Unbotch)(objHndl pc, int questId);

	QuestsAddresses() {
		rebase(GetState, 0x1005F4C0);
		rebase(SetState, 0x1005F780);
		rebase(Unbotch, 0x1005F820);
	}
} addresses;

void Quests::SetState(int questId, QuestState state) {
	auto leader = party.GetLeader();
	addresses.SetState(leader, questId, state);
}

QuestState Quests::GetState(int questId) {
	auto leader = party.GetLeader();
	return addresses.GetState(leader, questId);
}

QuestState Quests::Unbotch(int questId) {
	auto leader = party.GetLeader();
	return addresses.Unbotch(leader, questId);
}
