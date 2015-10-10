#include "stdafx.h"
#include "messagequeue.h"

static struct MessageQueueFuncs : temple::AddressTable {
	
	BOOL *tigProcessEvents;

	BOOL (*WftHandleMouseEvent)(const void* args);
	BOOL (*WftHandleOtherEvent)(const TigMsg* args);

	MessageQueueFuncs() {
		rebase(tigProcessEvents, 0x103012C4);
		rebase(WftHandleOtherEvent, 0x101F8A80);
		rebase(WftHandleMouseEvent, 0x101F9970);		
	}
} functions;

MessageQueue::MessageQueue() {
	if (messageQueue == nullptr) {
		messageQueue = this;
	}
}

MessageQueue::~MessageQueue() {
	if (messageQueue == this) {
		messageQueue = nullptr;
	}
}

void MessageQueue::Enqueue(const Message& msg) {
	mQueue.push_back(msg);
}

bool MessageQueue::Process(Message &unhandledMsgOut) {

	while (!mQueue.empty()) {
		const auto& msg = mQueue.front();

		if (!HandleMessage(msg)) {
			unhandledMsgOut = msg;
			mQueue.pop_front();
			return true;
		}
		
		mQueue.pop_front();
	}

	return false;

}

bool MessageQueue::HandleMessage(const Message& msg) {

	// TODO: Just dequeue directly in the movie player instead of having the dependency this way
/*	if (is_movie_playing_true())
		return;*/

	auto tigProcessEvents = (*functions.tigProcessEvents == TRUE);
	
	if (!tigProcessEvents) {
		return false;
	}
	
	if (msg.type == TigMsgType::MOUSE && functions.WftHandleMouseEvent(&msg.arg1)) {
		return true;
	}

	if (functions.WftHandleOtherEvent(&msg)) {
		if (msg.type == TigMsgType::TMT_UNK7) {
			return false;
		}

		return true;
	}
	
	return false;

}

MessageQueue* messageQueue = nullptr;
