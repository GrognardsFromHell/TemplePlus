
#pragma once

#include "addresses.h"

enum TigMsgType {
	TMT_UNK0 = 0,
	TMT_UNK1 = 1,
	TMT_UNK2 = 2,
	TMT_UNK3 = 3,
	TMT_UNK4 = 4,
	TMT_KEYSTATECHANGE = 5,
	TMT_UNK6 = 6,
	TMT_UNK7 = 7
};

struct TigMsg {
	uint32_t createdMs;
	TigMsgType type;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint32_t arg4;
};

struct TigMsgFuncs : AddressTable {
	// Return code of 0 means a msg has been written to msgOut.
	int(__cdecl *Process)(TigMsg *msgOut);

	void rebase(Rebaser rebase) override {
		rebase(Process, 0x101DE750);
	}
};

extern TigMsgFuncs tigMsgFuncs;

inline void processTigMessages() {
	TigMsg msg;
	while (!tigMsgFuncs.Process(&msg)) {
		LOG(trace) << "Processed msg type " << msg.type;
	}
}
