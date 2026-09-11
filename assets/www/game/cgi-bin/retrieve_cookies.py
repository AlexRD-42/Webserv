#!/usr/bin/python3

import html
import os
import sys

sys.stdout.write(
    "Set-Cookie: UserID=XYZ; Path=/; SameSite=Lax\r\n"
    "Set-Cookie: Password=XYZ123; Path=/; SameSite=Lax\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "\r\n"
)

user_id = "Not set"
password = "Not set"

if "HTTP_COOKIE" in os.environ:
    cookies = os.environ["HTTP_COOKIE"]
    for cookie in cookies.split(";"):
        cookie = cookie.strip()
        if "=" in cookie:
            key, value = cookie.split("=", 1)
            if key == "UserID":
                user_id = value
            elif key == "Password":
                password = value
    print(f"User ID = {html.escape(user_id)}<br>")
    print(f"Password = {html.escape(password)}")
else:
    print("No cookies found")
