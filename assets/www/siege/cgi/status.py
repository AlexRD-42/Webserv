import sys

body = b"created by CGI\n"
sys.stdout.buffer.write(b"Status: 201 Created\r\nContent-Type: text/plain\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
