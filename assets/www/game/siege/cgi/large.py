import sys

body = (b"0123456789abcdef" * 65536)
sys.stdout.buffer.write(b"Content-Type: application/octet-stream\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
