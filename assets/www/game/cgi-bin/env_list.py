#!/usr/bin/python3

import html
import os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")
print("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Environment</title></head><body>")
print("<h1>Environment</h1>")
for param in sorted(os.environ):
    print(f"<p><b>{html.escape(param)}</b>: {html.escape(os.environ[param])}</p>")
print("</body></html>")
