#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import time
from _fixture import body_info, emit_json, read_body
time.sleep(0.2)
emit_json(body_info(read_body()))
