
#pragma once

enum class QuestState : uint32_t {
	Unknown = 0,
	Mentioned = 1,
	Accepted = 2,
	Achieved = 3,
	Completed = 4,
	Other = 5,
	Botched = 6
};

class Quests {
public:

	void SetState(int questId, QuestState state);

	QuestState GetState(int questId);

	QuestState Unbotch(int questId);
	
};
extern Quests quests;
