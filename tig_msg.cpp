
#include "stdafx.h"
#include "tig_msg.h"

void __cdecl HookedEnqueue(TigMsg *msg) {

	switch (msg->type) {
	case TigMsgType::MOUSE:
		// LOG(info) << "MOUSE";
		break;
	case TigMsgType::CHAR:
		LOG(info) << "CHAR " << (uint32_t)msg->type
			<< " Args: " << msg->arg1 << " " << msg->arg2 << " " << msg->arg3 << " " << msg->arg4
			<< " (Created: " << msg->createdMs << ")";
		break;
	case TigMsgType::KEYSTATECHANGE:
		LOG(info) << "KEYSTATECHANGE " << (uint32_t)msg->type
			<< " Args: " << msg->arg1 << " " << msg->arg2 << " " << msg->arg3 << " " << msg->arg4
			<< " (Created: " << msg->createdMs << ")";
		break;
	case TigMsgType::KEYDOWN:
		LOG(info) << "KEYDOWN " << (uint32_t)msg->type
			<< " Args: " << msg->arg1 << " " << msg->arg2 << " " << msg->arg3 << " " << msg->arg4
			<< " (Created: " << msg->createdMs << ")";
		break;
	case TigMsgType::UPDATE_TIME:
		// Since this is called every frame, it produces logspam
		// LOG(info) << "Msg UPDATE_TIME (" << msg->createdMs << ")";
		break;
	default:
		LOG(info) << "Message " << (uint32_t) msg->type
			<< " Args: " << msg->arg1 << " " << msg->arg2 << " " << msg->arg3 << " " << msg->arg4
			<< " (Created: " << msg->createdMs << ")";
		break;
	}
	
	msgFuncs.Enqueue(msg);
}

TigMsgFuncs msgFuncs;

void hook_msgs() {
	MH_CreateHook(msgFuncs.Enqueue, HookedEnqueue, reinterpret_cast<void**>(&msgFuncs.Enqueue));
}
