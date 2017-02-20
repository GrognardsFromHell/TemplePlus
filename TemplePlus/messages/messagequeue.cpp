#include "stdafx.h"
#include "messagequeue.h"
#include "ui/ui.h"
#include "util/time.h"

static struct MessageQueueFuncs : temple::AddressTable {
	
	BOOL *tigProcessEvents;

	MessageQueueFuncs() {
		rebase(tigProcessEvents, 0x103012C4);		
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

void MessageQueue::Enqueue(const TigMsgBase& msg) {
	mQueue.push_back((TigMsg&)msg);
}

bool MessageQueue::Process(Message &unhandledMsgOut) {

	while (!mQueue.empty()) {
		const auto& msg = mQueue.front();
		mQueue.pop_front();

		if (!HandleMessage(msg)) {
			unhandledMsgOut = msg;
			return true;
		}
	}

	return false;

}

bool MessageQueue::HandleMessage(const Message& msg) {
	// TODO: Just dequeue directly in the movie player instead of having the dependency this way
/*	if (is_movie_playing_true()) {
		return false;
	}*/

	auto tigProcessEvents = (*functions.tigProcessEvents == TRUE);
	
	if (!tigProcessEvents) {
		return false;
	}
	
	if (msg.type == TigMsgType::MOUSE && uiManager->TranslateMouseMessage((TigMouseMsg&)msg.arg1)) {
		return true;
	}

	if (uiManager->ProcessMessage(const_cast<TigMsg&>(msg))) {
		if (msg.type == TigMsgType::TMT_UNK7) {
			return false;
		}

		return true;
	}
	
	return false;

}

void MessageQueue::ProcessMessages()
{
	TigMsg msg;
	while (!Process(msg))
		;
}

void MessageQueue::PollExternalEvents()
{
	// TODO: Move these dependencies out and make it a parameterizable "external event producer"
	static uint32_t& timePolled = temple::GetRef<uint32_t>(0x11E74578);
	timePolled = GetSystemTime();

	// Previously DirectInput was polled here for events, but this all happens in the 
	// Windows message loop now
	
	static auto process_window_messages = temple::GetPointer<int()>(0x101de880);
	process_window_messages();

	static auto sound_process_loop = temple::GetPointer<void()>(0x101e4360);
	sound_process_loop();
}

MessageQueue* messageQueue = nullptr;
