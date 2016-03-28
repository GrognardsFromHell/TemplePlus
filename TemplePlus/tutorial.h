#pragma once


class Tutorial
{
public:
	bool IsTutorialActive() const;
	void Toggle() const;
	int ShowTopic(int topidId) const;
};

extern Tutorial tutorial;