#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import os
from _fixture import emit_json
keys = ["REQUEST_METHOD", "QUERY_STRING", "CONTENT_LENGTH", "CONTENT_TYPE", "SCRIPT_NAME", "PATH_INFO", "PATH_TRANSLATED", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL", "GATEWAY_INTERFACE", "REMOTE_ADDR", "HTTP_HOST", "HTTP_COOKIE"]
emit_json({"fixture": "webserv-siege:environment", "environment": {key: os.environ.get(key, "") for key in keys}})
