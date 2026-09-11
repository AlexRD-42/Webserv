#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import emit
emit("webserv-siege:cgi:bad-request\n", status="400 Bad Request")
