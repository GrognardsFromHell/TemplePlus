#pragma once


class ModSupport
{
friend class TigInitializer;
public:
	bool IsCo8NCEdition() const;
	bool IsKotB() const;
	bool IsIWD() const;
	bool IsCo8() const;
	bool IsZMOD() const;
	void SetIsZMOD(bool value);
	static void SetNCGameFlag(bool value);
private:
	void DetectCo8ActiveModule();
	void DetectZMOD();
	bool mIsCo8NC = false;
	bool mIsKotB = false;
	bool mIsIWD = false;
	bool mIsCo8 = false;
	bool mIsZMOD = false;
	bool mInited = false;
};

extern ModSupport modSupport;