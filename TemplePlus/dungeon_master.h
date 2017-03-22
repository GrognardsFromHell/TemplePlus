#pragma once

#include <map>

class DungeonMaster
{



public:
	bool IsActive();
	void Render();


	void InitEntry(int protoNum);

	struct Record{
		int protoId;
		std::string name;
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;


	std::map<int, Record > monsters;

};

extern DungeonMaster dmSys;