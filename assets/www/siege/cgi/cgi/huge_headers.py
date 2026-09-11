#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import emit
emit("webserv-siege:too-large-header\n", extra=[("X-Pad", "H" * 12000)])
