
#include "stdafx.h"
#include "tig_msg.h"

void __cdecl HookedEnqueue(TigMsg *msg) {

	switch (msg->type) {
	case TigMsgType::MOUSE:
		// logger->info("MOUSE";
		break;
	case TigMsgType::CHAR:
		break;
	case TigMsgType::KEYSTATECHANGE:
		break;
	case TigMsgType::KEYDOWN:
		break;
	case TigMsgType::UPDATE_TIME:
		// Since this is called every frame, it produces logspam
		// logger->info("Msg UPDATE_TIME (" << msg->createdMs << ")";
		break;
	default:
		break;
	}
	
	msgFuncs.Enqueue(msg);
}

TigMsgFuncs msgFuncs;

void hook_msgs() {
	MH_CreateHook(msgFuncs.Enqueue, HookedEnqueue, reinterpret_cast<void**>(&msgFuncs.Enqueue));
}
