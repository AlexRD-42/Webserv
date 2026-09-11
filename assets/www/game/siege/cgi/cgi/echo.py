#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import body_info, emit_json, read_body
emit_json(body_info(read_body()))
