
#include "stdafx.h"
#include "tig_loadingscreen.h"
#include "ui/ui_assets.h"

LoadingScreenFuncs loadingScreenFuncs;

LoadingSequence::~LoadingSequence()
{
	delete image;
	delete[] messages;
}
