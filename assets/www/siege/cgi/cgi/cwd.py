#!/usr/bin/env python3
import sys
sys.dont_write_bytecode = True
from pathlib import Path
from _fixture import emit
emit(Path("data/message.txt").read_bytes())
