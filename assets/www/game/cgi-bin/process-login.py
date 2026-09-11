#!/usr/bin/env python3
import os
import re
import sys
from urllib.parse import parse_qs, quote

SERVER_PORTS = {
    "america": 8001,
    "africa":  8002,
    "europe":  8003,
    "oceania": 8004,
}

VALID_USERNAME = re.compile(r"^[A-Za-z0-9_-]{1,20}$")


def send(headers, body=""):
    out = "".join(h + "\r\n" for h in headers) + "\r\n" + body
    sys.stdout.write(out)
    sys.stdout.flush()


def read_body():
    try:
        length = int(os.environ.get("CONTENT_LENGTH", "0") or 0)
    except ValueError:
        length = 0
    return sys.stdin.buffer.read(length) if length > 0 else b""


def back_to_login(reason):
    send([
        "Status: 302 Found",
        "Location: /login.html?error=" + quote(reason),
        "Content-Type: text/html; charset=utf-8",
    ], "<html><body>Redirecting to login...</body></html>")
    sys.exit(0)


def main():
    if os.environ.get("REQUEST_METHOD", "GET") != "POST":
        back_to_login("method")

    form = parse_qs(read_body().decode("utf-8", errors="replace"))
    username = form.get("username", [""])[0].strip()
    server_choice = form.get("dropdown", [""])[0].strip().lower()
    accepted_terms = form.get("terms", [""])[0] == "on"

    if not VALID_USERNAME.match(username):
        back_to_login("username")
    if not accepted_terms:
        back_to_login("terms")
    if server_choice not in SERVER_PORTS:
        server_choice = "america"

    port = SERVER_PORTS[server_choice]
    host = os.environ.get("HTTP_HOST", "localhost").split(":")[0]

    send([
        "Status: 302 Found",
        "Location: http://%s:%d/cgi-bin/index.py" % (host, port),
        "Set-Cookie: cp_session=%s; Path=/; Max-Age=3600; SameSite=Lax" % quote(username),
        "Content-Type: text/html; charset=utf-8",
    ], "<html><body>Logged in. Redirecting...</body></html>")


if __name__ == "__main__":
    main()
