
#pragma once

#include "tig/tig.h"
#include "tig/tig_font.h"

struct TigTextStyle;
struct TigBuffer;

// Fonts shipping with the base game
enum class PredefinedFont {
	ARIAL_10,
	ARIAL_12,
	ARIAL_BOLD_10,
	ARIAL_BOLD_24,
	PRIORY_12,
	SCURLOCK_48
};

/*
	Helper methods for rendering UI elements.
*/
class UiRenderer {
public:

	/*
		Draws the full texture in the given screen rectangle.
	*/
	static void DrawTexture(int texId, const TigRect &destRect, int flags = 0);
	/*
	 As above but allow for scaling
	*/
	static void DrawTexture(int texId, const TigRect &destRect, const TigRect &srcRect);

	static void DrawTextureInWidget(int widId, int texId, const TigRect &destRect, const TigRect &srcRect, int flags = 0);

	/*
		Pushes a font for further text rendering.
	*/
	static void PushFont(PredefinedFont font);

	/*
		Pushes a custom font for further text rendering.
	*/
	static void PushFont(const std::string &faceName, int pixelSize, bool antialiased = true);

	/*
		Pops the last pushed font from the font stack.
	*/
	static void PopFont();

	/*
		Draws text.
	*/
	static bool DrawText(const string& text, const TigRect& rect, const TigTextStyle& style);
	/*
		Draws text positioned relative to widget.
	*/
	static bool DrawTextInWidget(int widgetId, const string &text, const TigRect &rect, const TigTextStyle &style);

	/*
	Draws text positioned relative to widget.
	Will center the text relative to the specified rect.
	*/
	static bool DrawTextInWidgetCentered(int widgetId, const string &text, const TigRect &rect, const TigTextStyle &style);

	/*
		Draws text positioned in screen coordinates. Width of rectangle may be 0 to cause automatic
		measurement of the text.
	*/
	static bool RenderText(const string &text, TigRect &rect, const TigTextStyle &style);

	/*
		Measures the given text and returns the bounding rect.
	*/
	static TigRect MeasureTextSize(const string &text, const TigTextStyle &style, int maxWidth = 0, int maxHeight = 0);

};
