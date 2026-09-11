#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import emit
emit("webserv-siege:created\n", status="201 Created")
