#pragma once


class ModSupport
{
friend class TigInitializer;
public:
	bool IsCo8NCEdition() const;
	bool IsKotB() const;
	bool IsIWD() const;
	bool IsCo8() const;
	static void SetNCGameFlag(bool value);
private:
	void DetectCo8ActiveModule();
	bool mIsCo8NC = false;
	bool mIsKotB = false;
	bool mIsIWD = false;
	bool mIsCo8 = false;
};

extern ModSupport modSupport;