
#pragma once

class Dice {
public:
	Dice(int mCount, int mSides, int mModifier = 0)
		: mCount(mCount),
		  mSides(mSides),
		  mModifier(mModifier) {
	}

	/*
		Performs a dice roll with the given parameters and returns
		the result.
	*/
	static int Roll(int count, int sides, int modifier = 0);

	/*
		Parses a dice string (i.e. 2d5+1) into its components and returns true
		on success. The modifier part is optional and can be negative.
	*/
	static bool Parse(const char *diceStr, int &count, int &sides, int &modifier);


	int GetCount() const {
		return mCount;
	}

	int GetSides() const {
		return mSides;
	}

	int GetModifier() const {
		return mModifier;
	}

private:
	int mCount = 0;
	int mSides = 0;
	int mModifier = 0;
};

/*
	Generates a random integer using the configured random number generator.
*/
int RandomIntRange(int from, int to);
