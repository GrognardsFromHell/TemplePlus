
#pragma once

struct TemplePlusConfig
{
	bool skipIntro = true;
	bool skipLegal = true;
	bool useDirect3d9Ex = true;
	bool windowed = true;
	bool featPrereqWarnings = false;
};

extern TemplePlusConfig config;
