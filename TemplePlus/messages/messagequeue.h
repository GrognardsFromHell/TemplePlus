#pragma once

#include "tig/tig_msg.h"

#include <deque>

using Message = TigMsg;

class MessageQueue {
public:
	MessageQueue();
	~MessageQueue();

	void Enqueue(const Message &msg);
	bool Process(Message &unhandledMsgOut);
	
	bool HandleMessage(const Message &msg);

private:
	std::deque<Message> mQueue;
};

extern MessageQueue *messageQueue;
