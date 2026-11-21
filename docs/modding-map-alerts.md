# Per-map ally alert distance

An optional `AlertDistance: <tiles>` key in `rules/MapList.mes` caps the radius
used to recruit nearby allies into combat. It applies to both provoked-hostility
alerts and Temple+'s additional enemy-turn ally alert pass (normally 24 tiles).
Existing shorter ranges and the allegiance, visibility, hearing and door checks
remain in effect. It affects ordinary gameplay as well as the autoplayer.

Example, for an IWD conversion whose Orc Cave is map 5030:

```
{5030}{<existing map name and fields>, AlertDistance: 12}
```

Keep the actual map name and other fields. Values are integer tile radii from
0 to 1000. Zero disables these ally-alert passes, but does not disable enemies'
own detection, scripts or explicit attacks. Omitted or invalid values preserve
engine defaults; invalid values log a warning. Changes take effect on module
load (restart the game). The setting is read from the currently open map, so it
also follows travel and saved-game loads without becoming a global override.

This is a distance cap, not encounter grouping: alerts can still spread through
closely spaced allies across subsequent turns. No override is shipped for stock maps.

In the Temple+ Python console, use `import debug; debug.alerts(1)` to trace ally
recruitment. For five seconds after a recipient enters the fighting state or
initiative through either alert routine, an arrow connects the search center
to the recipient's position at the time of the event. Its label measures
horizontal center-to-center distance in tiles, including sub-tile offsets.
The native TemplePlus.log also records the map, actors, tile coordinates,
search radius and which search ran. Provocation can search around the agitator
and the provoked creature. The enemy-turn search is centered on the acting
creature. Independent detection and scripted attacks can start combat without
an arrow. Use `debug.alerts(0)` to disable and clear the display.
