#!/usr/bin/python3
import cgi, cgitb

form = cgi.FieldStorage()

if form.getvalue('textcontent'):
	text_content = form.getvalue('textcontent')
else:
	text_content = "Not entered"

print ("Content-type:text/html\r\n\r\n")
print ('<html>')
print ('<head')
print ('<title>Text Area</title>')
print ('</head>')
print ('<body>')
print (f"<h2> Entered Text Content is: {text_content}</h2>")
print ('</body>')
print ('</html>')
