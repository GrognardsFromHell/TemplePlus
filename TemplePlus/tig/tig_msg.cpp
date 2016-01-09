
#include "stdafx.h"
#include "tig_msg.h"
#include <util/fixes.h>
#include "common.h"
TigMsgFuncs msgFuncs;


class TigMsgReplacements: TempleFix
{
public: 
	const char* name() override { 
		return "Tig Message Function Replacements";
	} 
	
	static void Enqueue(TigMsg * msg);
	static void(__cdecl* orgEnqueue)(TigMsg*msg);

	void apply() override 
	{
		orgEnqueue = replaceFunction(0x101DE660, Enqueue);
	}
} tigReplacements;

void(__cdecl* TigMsgReplacements::orgEnqueue)(TigMsg* msg);

void TigMsgReplacements::Enqueue(TigMsg* msg)
{
	//msgFuncs.Enqueue(msg);
	orgEnqueue(msg);
	if (msg->type == TigMsgType::WIDGET)
	{
		int dummy = 1;
	}
}
