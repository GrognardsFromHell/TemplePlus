# Temple+ new game start location hook.
#
# Called by the engine when a new game starts (after party creation), before the
# party is placed. To customise the start, a module ships scr\start_location.py with:
#
#   def get_start_location(alignment, default_map):
#       # alignment:   party alignment index (as used by the alignment vignettes)
#       # default_map: map the engine would use (start map global or alignment vignette)
#       return None            # keep the default (MapList START_MAP, then engine)
#       return 5001            # start on map 5001 at its MapList start position
#       return (5001, 623, 421)  # start on map 5001 at tile 623,421
#
# Precedence: this hook > Rules\MapList.mes "Flag: NEW_GAME_START" > engine default.

def get_start_location(alignment, default_map):
	# Module scripts are served by the game's file system importer, so just try importing
	try:
		import start_location
	except ImportError:
		return None
	if not hasattr(start_location, "get_start_location"):
		return None
	return start_location.get_start_location(alignment, default_map)
