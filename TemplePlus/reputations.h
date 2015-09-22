
#pragma once

class PartyReputation {
public:
	/*
		Returns true if the party has the given reputation.
	*/
	bool Has(int reputationId);

	/*
		Adds the given reputation to the party.
	*/
	void Add(int reputationId);

	/*
		Removes the given reputation from the party.
	*/
	void Remove(int reputationId);

};

extern PartyReputation partyReputation;
