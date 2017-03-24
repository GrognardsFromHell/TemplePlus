#pragma once

#include <map>

class DungeonMaster
{



public:
	bool IsActive();
	void Show();
	void Hide();
	void Toggle();

	void Render();


	void InitEntry(int protoNum);

	struct Record{
		int protoId;
		std::string name;
	};

protected:
	//bool mIsVisible = false;
	// bool mIsActive = true;
	bool mIsInited = false;
	int mTexId;

	std::map<int, Record > monsters;
	std::map<int, Record > weapons;

};

extern DungeonMaster dmSys;