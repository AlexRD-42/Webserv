#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import OUT
OUT.write(b"Status: 204 No Content\r\n\r\n")
OUT.flush()
