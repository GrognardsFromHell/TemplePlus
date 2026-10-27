#pragma once
// Clickable info regions (e.g. converted Infinity Engine info points): rules/info_regions/<map id>.json holds
// {"regions": [{"id", "name", "poly": [[x, y], ...] in continuous tile units, "text"}]}. Clicking inside a region
// out of combat shows its text over the party leader and in the log, and the click does not move the party.
namespace InfoRegions {
	// returns true if the click at these screen coordinates hit a region (and was handled)
	bool HandleClick(int screenX, int screenY);
	// true if these screen coordinates are inside a region of the current map (hover cursor)
	bool IsOver(int screenX, int screenY);
	// hover state, set on mouse moves by the in-game UI (nothing targetable under the cursor, out of combat)
	void SetHover(bool hover);
	bool Hovered();
}
