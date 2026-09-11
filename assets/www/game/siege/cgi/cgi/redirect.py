#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import emit
emit("webserv-siege:cgi:redirect\n", status="302 Found", extra=[("Location", "/cgi/hello.py")])
