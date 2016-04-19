
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
	}
} tigReplacements;

void(__cdecl* TigMsgReplacements::orgEnqueue)(TigMsg* msg);
int(__cdecl* TigMsgReplacements::orgProcess)(TigMsg* msg);

void TigMsgReplacements::Enqueue(TigMsg* msg)
{
	//msgFuncs.Enqueue(msg);
	orgEnqueue(msg);
}

int TigMsgReplacements::Process(TigMsg* msg)
{
	return orgProcess(msg);
}

