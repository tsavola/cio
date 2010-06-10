#
# Copyright (c) 2010  Timo Savola
#

__all__ = [
	"launch",
]

import _cio

def launch(routine, *args):
	if args:
		routine = lambda: routine(*args)

	_cio.launch(routine)
