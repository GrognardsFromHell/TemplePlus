#include "stdafx.h"
#include "messagequeue.h"
#include "ui/ui.h"

eastl::deque<Message> debugMsgs;
bool mDebugMsgsEn;

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
	/*if (mDebugMsgsEn && msg.type != TigMsgType::UPDATE_TIME) {
		debugMsgs.push_front((Message&)msg);
		if (msg.type == TigMsgType::MOUSE) {
			auto msgMouse = (TigMsgMouse&)msg;
			if (msgMouse.buttonStateFlags & MSF_POS_CHANGE_SLOW) {
				debugMsgs.push_front((Message&)msg);
			}
		}
	}*/
		
}

bool MessageQueue::Process(Message &unhandledMsgOut) {

	while (!mQueue.empty()) {
		auto msg = mQueue.front();
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
	while (!mQueue.empty() && !Process(msg))
		;
}

void MessageQueue::PollExternalEvents()
{
	// TODO: Move these dependencies out and make it a parameterizable "external event producer"
	static uint32_t& timePolled = temple::GetRef<uint32_t>(0x11E74578);
	timePolled = timeGetTime();

	// Previously DirectInput was polled here for events, but this all happens in the 
	// Windows message loop now
	
	static auto process_window_messages = temple::GetPointer<int()>(0x101de880);
	process_window_messages();

	static auto sound_process_loop = temple::GetPointer<void()>(0x101e4360);
	sound_process_loop();
}


void MessageQueue::DebugMessages()
{
	for (auto& msg : mQueue) {
		if (msg.type != TigMsgType::UPDATE_TIME)
			debugMsgs.push_front(msg);
	}
	while (debugMsgs.size() > 100)
		debugMsgs.pop_back();

	mDebugMsgsEn = true;
}

eastl::deque<Message>& MessageQueue::GetDebugMsgs()
{
	return debugMsgs;
}

MessageQueue* messageQueue = nullptr;
