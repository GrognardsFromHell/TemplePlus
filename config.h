
#pragma once

struct TemplePlusConfig
{
	bool skipIntro = false;
	bool skipLegal = false;
	bool useDirect3d9Ex = true;
	bool windowed = true;
	bool featPrereqWarnings = false;
};

extern TemplePlusConfig config;
