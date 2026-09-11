#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from _fixture import emit
emit(bytes(range(256)) * 256, "application/octet-stream")
