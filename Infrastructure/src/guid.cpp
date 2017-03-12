
#include "guid.h"

bool operator<(const TPGUID& lhs, const TPGUID& rhs) {

	if (lhs.Data1 < rhs.Data1) {
		return true;
	}
	if (lhs.Data1 > rhs.Data1) {
		return false;
	}

	if (lhs.Data2 < rhs.Data2) {
		return true;
	}
	if (lhs.Data2 > rhs.Data2) {
		return false;
	}

	if (lhs.Data3 < rhs.Data3) {
		return true;
	}
	if (lhs.Data3 > rhs.Data3) {
		return false;
	}

	for (auto i = 0; i < 8; ++i) {
		if (lhs.Data4[i] < rhs.Data4[i]) {
			return true;
		}

		if (lhs.Data4[i] > rhs.Data4[i]) {
			return false;
		}
	}

	return false; // They are actually equal

}

bool operator==(const TPGUID &lhs, const TPGUID &rhs) {
	return lhs.Data1 == rhs.Data1 &&
		lhs.Data2 == rhs.Data2 &&
		lhs.Data3 == rhs.Data3 &&
		lhs.Data4 == rhs.Data4;
}
