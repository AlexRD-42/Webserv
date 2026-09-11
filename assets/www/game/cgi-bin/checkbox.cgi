#!/usr/bin/python3
import cgi, cgitb

form = cgi.FieldStorage()

if form.getvalue('maths'):
	math_flag = "ON"
else:
	math_flag = "OFF"

if form.getvalue('physics'):
	physics_flag = "ON"
else:
	physics_flag = "OFF"

print ("Content-type:text/html\r\n\r\n")
print ('<html>')
print ('<head')
print ('<title>Checkbox</title>')
print ('</head>')
print ('<body>')

print (f"<h2>CheckBox Maths is: {math_flag}</h2>")
print (f"<h2>CheckBox Physics is: {physics_flag}</h2>")
print ('</body>')
print ('</html>')
