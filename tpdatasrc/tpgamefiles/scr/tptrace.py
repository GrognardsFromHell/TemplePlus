# Print functions that hook into the Temple+ config settings for log level.
#
# Right now it just checks the configured log level itself and does its own
# logic of whether or not to print. This means that all messages will show up
# in the log tagged as the "info" level even if they are not tagged as such
# here. Perhaps at a later date this could be hooked into the overall Temple+
# logging system more directly for more precise output.

import tpdp

def log(at_level, stuff):
	level = tpdp.config_get_int("LogLevel")

	# nothing to print
	if len(stuff) <= 0: return

	# level 6 is "off", so cap at_level
	if min(at_level, 5) < level: return

	msg = ""
	for thing in stuff:
		msg = "{}{}".format(msg, thing)

	print msg

def trace(*stuff):
	log(0, stuff)

def debug(*stuff):
	log(1, stuff)

def info(*stuff):
	log(2, stuff)

def warn(*stuff):
	log(3, stuff)

def err(*stuff):
	log(4, stuff)

def critical(*stuff):
	log(5, stuff)
