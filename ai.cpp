#include "stdafx.h"
#include "common.h"
#include "ai.h"

class AiReplacements : public TempleFix
{
	macTempleFix(AI Replacements)
	{
		logger->info("Replacing AI functions...");
	}
} aiReplacements;


#pragma region AI System Implementation
struct AiSystem aiSys;

AiSystem::AiSystem()
{
	
}

#pragma endregion

#pragma region AI replacement functions


#pragma endregion 