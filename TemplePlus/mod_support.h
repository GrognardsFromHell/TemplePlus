#pragma once


class ModSupport
{
friend class TigInitializer;
public:
	bool IsCo8NCEdition() const;
	static void SetNCGameFlag(bool value);
private:
	void DetectCo8NewContentEdition();
	bool mIsCo8NC = false;
};

extern ModSupport modSupport;