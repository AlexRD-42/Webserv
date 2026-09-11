#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import os
from _fixture import emit_json, query
emit_json({"fixture": "webserv-siege:query", "raw": os.environ.get("QUERY_STRING", ""), "query": query()})
