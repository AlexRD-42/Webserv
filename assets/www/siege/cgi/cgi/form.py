#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from urllib.parse import parse_qs
from _fixture import body_info, emit_json, read_body
body = read_body()
result = body_info(body)
result["form"] = parse_qs(body.decode("utf-8"), keep_blank_values=True)
emit_json(result)
