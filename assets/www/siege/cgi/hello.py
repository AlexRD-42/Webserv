import sys

body = b"CGI works\n"
sys.stdout.buffer.write(b"Content-Type: text/plain\r\nContent-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body)
