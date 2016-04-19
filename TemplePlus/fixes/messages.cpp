#include "stdafx.h"
#include "util/fixes.h"
#include "tig/tig_msg.h"
#include <messages/messagequeue.h>

// Message Queue Replacement
static class MessageQueueFix : public TempleFix {
public:
	void apply() override;

	static void EnqueueMsg(const TigMsg* msg);
	static int ProcessMsg(TigMsg* msgOut);

} fix;

void MessageQueueFix::apply() {
	// Disabled for now
	// replaceFunction(0x101DE660, EnqueueMsg);
	// replaceFunction(0x101DE750, ProcessMsg);
}

void MessageQueueFix::EnqueueMsg(const TigMsg* msg) {
	messageQueue->Enqueue(*msg);
}

int MessageQueueFix::ProcessMsg(TigMsg* msgOut) {
	if (messageQueue->Process(*msgOut)) {
		return 0;
	}

	return 10;
}
