#pragma once


class ModSupport
{
friend class TigInitializer;
public:
	bool IsCo8NCEdition() const;
	bool IsKotB() const;
	bool IsPalCove() const;
	bool IsIWD() const;
	bool IsCo8() const;
	bool IsZMOD() const;
	bool HasTfexIni() const; // TFE-X.ini present in the ToEE dir
	const std::string& GetActiveModuleName() const; // first line of TFE-X.ini, empty if none
	void SetIsZMOD(bool value);
	static void SetNCGameFlag(bool value);
	const std::vector<std::string>& GetOverrides();
	void AddOverride(const std::string& overrideName);
private:
	void DetectCo8ActiveModule();
	void DetectZMOD();
	bool mIsCo8NC = false;
	bool mIsKotB = false;
	bool mIsIWD = false;
	bool mIsCo8 = false;
	bool mIsZMOD = false;
	bool mIsPalCove = false;
	bool mInited = false;
	bool mHasTfexIni = false;
	std::string mActiveModuleName;
	std::vector<std::string> mOverridesLoaded;
};

extern ModSupport modSupport;