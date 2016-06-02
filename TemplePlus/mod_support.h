#pragma once


class ModSupport
{
friend class TigInitializer;
public:
	bool IsCo8NCEdition() const;
	bool IsKotB() const;
	static void SetNCGameFlag(bool value);
private:
	void DetectCo8ActiveModule();
	bool mIsCo8NC = false;
	bool mIsKotB = false;
};

extern ModSupport modSupport;