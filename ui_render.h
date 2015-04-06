
#pragma once
#include "tig.h"

/*
	Helper methods for rendering UI elements.
*/
class UiRenderer {
public:
	
	/*
		Draws the full texture in the given screen rectangle.
	*/
	static void DrawTexture(int texId, const TigRect &destRect);

};
