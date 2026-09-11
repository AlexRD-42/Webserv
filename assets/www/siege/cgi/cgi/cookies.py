#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import os
from _fixture import emit_json
emit_json({"fixture": "webserv-siege:cookies", "cookie": os.environ.get("HTTP_COOKIE", "")}, extra=[("Set-Cookie", "siege_one=1; Path=/; SameSite=Lax"), ("Set-Cookie", "siege_two=2; Path=/; SameSite=Lax")])
