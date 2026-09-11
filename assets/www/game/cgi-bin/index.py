#!/usr/bin/env python3
import os
import re
import sys

VALID_USERNAME = re.compile(r"^[A-Za-z0-9_%-]{1,32}$")


def send(headers, body=""):
    sys.stdout.write("".join(h + "\r\n" for h in headers) + "\r\n" + body)
    sys.stdout.flush()


def session_name():
    for part in os.environ.get("HTTP_COOKIE", "").split(";"):
        name, _, value = part.strip().partition("=")
        if name == "cp_session" and VALID_USERNAME.match(value):
            return value
    return None


def main():
    if session_name() is None:
        send([
            "Status: 302 Found",
            "Location: /login.html",
            "Content-Type: text/html; charset=utf-8",
        ], "<html><body>Redirecting to login...</body></html>")
        return

    try:
        with open("../game.html", "r", encoding="utf-8") as f:
            page = f.read()
    except OSError:
        send(["Status: 500 Internal Server Error",
              "Content-Type: text/html; charset=utf-8"],
             "<h1>500</h1><p>game.html is missing</p>")
        return

    send(["Content-Type: text/html; charset=utf-8", "Cache-Control: no-store"], page)


if __name__ == "__main__":
    main()
