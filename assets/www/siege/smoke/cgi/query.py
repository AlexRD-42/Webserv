import os
import sys

body = ("QUERY_STRING=" + os.environ.get("QUERY_STRING", "") + "\n").encode()
sys.stdout.buffer.write(b"Content-Type: text/plain\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
