#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
import time
from _fixture import OUT, start
block = b"0123456789ABCDEF" * 256
start(length=len(block) * 16)
for i in range(16):
    OUT.write(block)
    OUT.flush()
    time.sleep(0.025)
