#pragma once
// Clickable info regions (e.g. converted Infinity Engine info points): rules/info_regions/<map id>.json holds
// {"regions": [{"id", "name", "poly": [[x, y], ...] in continuous tile units, "text"}]}. Clicking inside a region
// out of combat shows its text over the party leader and in the log, and the click does not move the party.
namespace InfoRegions {
	// returns true if the click at these screen coordinates hit a region (and was handled)
	bool HandleClick(int screenX, int screenY);
}
