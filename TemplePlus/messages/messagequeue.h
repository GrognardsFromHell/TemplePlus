#pragma once

#include "tig/tig_msg.h"

#include <EASTL/deque.h>

using Message = TigMsg;

class MessageQueue {
public:
	MessageQueue();
	~MessageQueue();

	void Enqueue(const TigMsgBase &msg);
	bool Process(Message &unhandledMsgOut);
	
	bool HandleMessage(const Message &msg);

	void ProcessMessages();

	void PollExternalEvents();

	void SetSkipProcessing(bool enable);

private:
	eastl::deque<Message> mQueue;
	bool mSkipProcessing = false;
};

extern MessageQueue *messageQueue;
