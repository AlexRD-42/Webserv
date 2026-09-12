#!/usr/bin/env python3
import os
from pathlib import Path
import re
import sys

SERVER_PORTS = {
    "america": 8001,
    "africa": 8002,
    "europe": 8003,
    "oceania": 8004,
}
VALID_USERNAME = re.compile(r"[A-Za-z0-9_-]{1,20}")
VALID_SERVER = re.compile(r"(?:america|africa|europe|oceania)")


def send(headers, body=""):
    sys.stdout.write("".join(h + "\r\n" for h in headers) + "\r\n" + body)
    sys.stdout.flush()


def cookie_value(name, pattern):
    for part in os.environ.get("HTTP_COOKIE", "").split(";"):
        key, _, value = part.strip().partition("=")
        if key == name and pattern.fullmatch(value):
            return value
    return None


def request_host():
    raw = os.environ.get("HTTP_HOST", "localhost").strip()
    if raw.startswith("["):
        end = raw.find("]")
        host = raw[:end + 1] if end >= 0 else ""
    elif raw.count(":") == 1:
        host = raw.rsplit(":", 1)[0]
    else:
        host = raw
    if not re.fullmatch(r"(?:[A-Za-z0-9.-]+|\[[0-9A-Fa-f:.]+\])", host):
        return "localhost"
    return host


def request_port():
    raw = os.environ.get("HTTP_HOST", "").strip()
    if raw.startswith("["):
        end = raw.find("]")
        if end >= 0 and raw[end + 1:].startswith(":"):
            return raw[end + 2:]
        return ""
    if raw.count(":") == 1:
        return raw.rsplit(":", 1)[1]
    return ""


def main():
    username = cookie_value("cp_session", VALID_USERNAME)
    if username is None:
        send([
            "Status: 302 Found",
            "Location: /login.html",
            "Cache-Control: no-store",
            "Content-Type: text/html; charset=utf-8",
        ], "<html><body>Redirecting to login...</body></html>")
        return

    # Login cookies are shared across ports. Remember the selected region so opening
    # the original/home URL again returns to that region instead of a dead/local game.
    server = cookie_value("cp_server", VALID_SERVER)
    if server is not None:
        target_port = str(SERVER_PORTS[server])
        current_port = request_port()
        if current_port != target_port:
            send([
                "Status: 302 Found",
                "Location: http://%s:%s/cgi-bin/index.py" % (request_host(), target_port),
                "Cache-Control: no-store",
                "Content-Type: text/html; charset=utf-8",
            ], "<html><body>Returning to your game server...</body></html>")
            return

    try:
        with (Path(__file__).resolve().parent.parent / "game.html").open(encoding="utf-8") as f:
            page = f.read()
    except OSError:
        send(["Status: 500 Internal Server Error",
              "Content-Type: text/html; charset=utf-8"],
             "<h1>500</h1><p>game.html is missing</p>")
        return

    send(["Content-Type: text/html; charset=utf-8", "Cache-Control: no-store"], page)


if __name__ == "__main__":
    main()
