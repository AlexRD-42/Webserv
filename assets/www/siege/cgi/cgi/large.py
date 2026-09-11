#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import OUT, query, start
size = int(query().get("size", ["1048576"])[0])
size = max(0, min(size, 4 * 1024 * 1024))
block = bytes(range(256)) * 256
start("application/octet-stream", size)
for offset in range(0, size, len(block)):
    OUT.write(block[:min(len(block), size - offset)])
OUT.flush()
