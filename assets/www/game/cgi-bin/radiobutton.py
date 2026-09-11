#!/usr/bin/python3
import cgi, cgitb

form = cgi.FieldStorage()

if form.getvalue('dropdown'):
	subject = form.getvalue('dropdown')
else:
	subject = "Not entered"

print ("Content-type:text/html\r\n\r\n")
print ('<html>')
print ('<head')
print ('<title>Dropdown Box</title>')
print ('</head>')
print ('<body>')
print (f"<h2> Selected Subject is: {subject}</h2>")
print ('</body>')
print ('</html>')
