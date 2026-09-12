"""Small URL-encoded CGI helpers shared by the demo forms."""
import html
import os
import sys
from urllib.parse import parse_qs


def read_form():
    if os.environ.get("REQUEST_METHOD", "GET") == "POST":
        try:
            length = max(0, int(os.environ.get("CONTENT_LENGTH", "0") or 0))
        except ValueError:
            length = 0
        raw = sys.stdin.buffer.read(length).decode("utf-8", errors="replace")
    else:
        raw = os.environ.get("QUERY_STRING", "")
    return {name: values[0] for name, values in parse_qs(raw, keep_blank_values=True).items()}


def page(title, messages):
    headings = "".join("<h2>%s</h2>" % html.escape(message) for message in messages)
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\nCache-Control: no-store\r\n\r\n"
                     "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                     "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                     "<title>%s</title></head><body>%s"
                     "<p><a href=\"/cgis.html\">Back to CGIs</a> · <a href=\"/cgi-bin/index.py\">Home</a></p>"
                     "</body></html>" % (html.escape(title), headings))
    sys.stdout.flush()
