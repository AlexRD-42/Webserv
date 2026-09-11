import sys

body = sys.stdin.buffer.read()
sys.stdout.buffer.write(b"Content-Type: application/octet-stream\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
