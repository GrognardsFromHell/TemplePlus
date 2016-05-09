
#include "stdafx.h"
#include "tig_msg.h"
#include <util/fixes.h>
#include "common.h"
TigMsgFuncs msgFuncs;


class TigMsgReplacements: TempleFix
{
public: 
	static void Enqueue(TigMsg * msg);
	static void(__cdecl* orgEnqueue)(TigMsg*msg);

	static int Process(TigMsg* msg);
	static int(__cdecl *orgProcess)(TigMsg* msg);

	void apply() override 
	{
		orgEnqueue = replaceFunction(0x101DE660, Enqueue);
		orgProcess = replaceFunction(0x101DE750, Process);

		static int(__cdecl*orgUiMsgHandler2)(TigMsg*) = replaceFunction<int(TigMsg*)>(0x101F8A80, [](TigMsg* msg){

			auto result = orgUiMsgHandler2(msg);

			if (msg->type == TigMsgType::MOUSE){
				if (msg->arg1 == 0xDEADBEEF && msg->arg2 == 0xbeefb00f)
					return FALSE;
			}

			return result;
		});
	}
} tigReplacements;

void(__cdecl* TigMsgReplacements::orgEnqueue)(TigMsg* msg);
int(__cdecl* TigMsgReplacements::orgProcess)(TigMsg* msg);

void TigMsgReplacements::Enqueue(TigMsg* msg)
{
	//msgFuncs.Enqueue(msg);
	orgEnqueue(msg);
}

int TigMsgReplacements::Process(TigMsg* msg){
	int result = orgProcess(msg);
	int dummy = 1;

	return result;
	//return orgProcess(msg);
}

void TigMsg::Enqueue()
{
	tigReplacements.Process(this);
}
