
#include "stdafx.h"
#include "tig_font.h"

TigFontFuncs tigFont;

temple::GlobalStruct<TigFontData, 0x10EEEEC8> fontData;


static_assert(temple::validate_size<TigTextStyle, 0x50>::value, "TigTextStyle has incorrect size");

ColorRect whiteColor(0xFFFFffff);
const TigTextStyle TigTextStyle::standardWhite{ &whiteColor };

TigTextStyle::TigTextStyle(ColorRect * color){
	this->textColor = color;
}

TigTextStyle::TigTextStyle(const ColorRect* textColor, const ColorRect* shadowColor, const ColorRect* backgroundColor)
{
	this->textColor = (ColorRect * )textColor;
	this->shadowColor= (ColorRect*)shadowColor;
	this->bgColor= (ColorRect*)backgroundColor;
}

TigTextStyle::TigTextStyle(){
}

void TigTextStyle::SetColors(ColorRect * color)
{
	this->textColor = this->colors2 = this->colors4 = this->shadowColor = color;
}


