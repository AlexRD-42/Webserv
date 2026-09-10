import os
import sys

keys = ("REQUEST_METHOD", "SCRIPT_NAME", "QUERY_STRING", "CONTENT_LENGTH", "CONTENT_TYPE", "HTTP_HOST", "HTTP_COOKIE")
body = "".join("%s=%s\n" % (key, os.environ.get(key, "")) for key in keys).encode()
sys.stdout.buffer.write(b"Content-Type: text/plain\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
