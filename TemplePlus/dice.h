
#pragma once

struct PackedDie {
	uint8_t count : 7;
	uint8_t sides : 7;
	int8_t bonus : 7;
};

class Dice {
public:
	Dice(int mCount, int mSides, int mModifier = 0)
		: mCount(mCount),
		  mSides(mSides),
		  mModifier(mModifier) {
	}

	Dice() : Dice(1, 1) {}

	/*
		Performs a dice roll with the given parameters and returns
		the result.
	*/
	static int Roll(int count, int sides, int modifier = 0);

	/*
		Returns the dice for a larger sized weapon.
	*/
	Dice IncreaseWeaponSize(int numIncreases = 1) const;

	int Roll();

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

	// Convert to a packed ToEE dice
	uint32_t ToPacked() const {
		uint32_t result = (mCount & 0x7F) | ((mSides & 0x7F) << 7);
		if (mModifier < 0) {
			result |= ((-mModifier) & 0x1FFFF) << 14;
			result |= 0x80000000;
		} else {
			result |= (mModifier & 0x1FFFF) << 14;
		}
		return result;
	}

	static Dice FromPacked(uint32_t packed) {
		bool modNegative = (packed & 0x80000000) != 0;
		int mod = (packed >> 14) & 0x1FFFF;
		int count = (packed & 0x7F);
		int sides = ((packed >> 7) & 0x7F);
		if (modNegative) {
			mod = -mod;
		}

		return Dice(count, sides, mod);
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
