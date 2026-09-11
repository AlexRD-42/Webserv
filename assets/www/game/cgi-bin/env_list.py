#!/usr/bin/python3

import os

print ("Content-type:text/html\r\n\r\n")
print ("<font size=+1>Environment</font><\br>")
for param in os.environ:
	print (f"<b>{param}</b>: {os.environ[param]}</br>")
