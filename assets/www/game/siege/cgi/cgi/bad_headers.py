#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import OUT
OUT.write(b"This is not a CGI header\r\n\r\nbody\n")
OUT.flush()
