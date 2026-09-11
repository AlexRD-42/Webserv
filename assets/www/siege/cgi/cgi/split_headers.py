#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import time
from _fixture import OUT
body = b"webserv-siege:cgi:split-headers\n"
parts = [b"Content-T", b"ype: text/plain\r", b"\nContent-Length: " + str(len(body)).encode() + b"\r\nX-Split: yes\r\n\r", b"\n" + body]
for part in parts:
    OUT.write(part)
    OUT.flush()
    time.sleep(0.04)
