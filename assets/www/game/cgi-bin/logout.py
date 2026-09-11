#!/usr/bin/env python3
import sys

sys.stdout.write(
    "Status: 302 Found\r\n"
    "Location: /login.html\r\n"
    "Set-Cookie: cp_session=; Path=/; Max-Age=0; SameSite=Lax\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "\r\n"
    "<html><body>Logging out...</body></html>"
)
sys.stdout.flush()
