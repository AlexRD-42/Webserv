#!/usr/bin/python3

import os
import cgi

print ("Set-Cookie:UserID = XYZ;\r\n")
print ("Set-Cookie:Password = XYZ123;\r\n")

user_id = "Not set"
password = "Not set"

if 'HTTP_COOKIE' in os.environ:
	cookies = os.environ['HTTP_COOKIE']

	for cookie in cookies.split(';'):
		cookie = cookie.strip()
		if '=' in cookie:
			key, value = cookie.split('=', 1)
			if key == "UserID":
				user_id = value
			elif key == "Password":
				password = value
	print ("Content-type:text/html\r\n\r\n")
	print(f"User ID = {user_id}")
	print(f"Password = {password}")
else:
	print ("Content-type:text/html\r\n\r\n")
	print("No cookies found")
