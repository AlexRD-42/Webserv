#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import OUT, start
start()
OUT.write(b"webserv-siege:cgi:eof\n" * 4096)
OUT.flush()
