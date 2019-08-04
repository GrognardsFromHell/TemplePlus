#pragma once


#pragma pack(push, 1)
struct RandomEncounterEnemy {
	int protoId;
	int count;

};
struct RandomEncounter {
	int id;
	int flags;
	int title;
	int dc;
	int map;
	int field14;
	locXY location;
	int enemiesCount;
	RandomEncounterEnemy *enemies;
};
struct RandomEncounterSetup {
	int flags;
	int padding;
	locXY location;
};
#pragma pack(pop)
