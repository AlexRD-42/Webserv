#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import json
from _fixture import OUT, body_info, read_body, start
# A legal CGI can write more than one pipe buffer before consuming stdin.
# No Content-Length: the response ends when this process closes stdout.
start()
OUT.write(b"D" * 131072 + b"\n")
OUT.flush()
OUT.write(json.dumps(body_info(read_body()), sort_keys=True).encode() + b"\n")
OUT.flush()
